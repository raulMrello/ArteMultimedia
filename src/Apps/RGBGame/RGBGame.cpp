/*
 * RGBGame.cpp
 *
 *  Created on: Ene 2018
 *      Author: raulMrello
 */

#include "RGBGame.h"

//------------------------------------------------------------------------------------
//-- PRIVATE TYPEDEFS ----------------------------------------------------------------
//------------------------------------------------------------------------------------

/** Macro para imprimir trazas de depuración, siempre que se haya configurado un objeto
 *	Logger válido (ej: _debug)
 */

#define DEBUG_TRACE(format, ...)			\
if(ActiveModule::_defdbg){					\
	printf(format, ##__VA_ARGS__);			\
}											\


//------------------------------------------------------------------------------------
//-- STATIC MEMBERS ------------------------------------------------------------------
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
static uint8_t servo_id[] = {4, 9, 14};

//------------------------------------------------------------------------------------
static uint8_t wavePoint(uint8_t i, uint8_t max, uint8_t min){
    static const float values[] = {0, 0.08f, 0.12f, 0.25f, 0.33f, 0.65f, 0.75f, 1.0f, 0.75f, 0.65f, 0.33f, 0.25f, 0.12f, 0.08f, 0}; 
    i = (i > 14)? 14 : i;
    int16_t result = (int16_t)(((float)(max - min) * values[i]) + min);
    result = (result > max)? max : result;
    result = (result < min)? min : result;
    return (uint8_t)result;
}



//------------------------------------------------------------------------------------
//-- PUBLIC METHODS IMPLEMENTATION ---------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
RGBGame::RGBGame(WS281xLedStrip* ls, PCA9685_ServoDrv* sd, FSManager* fs, bool defdbg) : ActiveModule("RGBGame", osPriorityNormal, OS_STACK_SIZE, fs, defdbg) {
    _timeout = osWaitForever;
    _touch_topic = "TBD";
    // creo máquinas de estado posteriores a init
    _stWait.setHandler(callback(this, &RGBGame::Wait_EventHandler));
    _stConfig.setHandler(callback(this, &RGBGame::Config_EventHandler));
    _stGame.setHandler(callback(this, &RGBGame::Game_EventHandler));

	_publicationCb = callback(this, &RGBGame::publicationCb);
	_subscriptionCb = callback(this, &RGBGame::subscriptionCb);
}


//------------------------------------------------------------------------------------
//-- PROTECTED METHODS IMPLEMENTATION ------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
void RGBGame::subscriptionCb(const char* topic, void* msg, uint16_t msg_len){
    // si es un comando para actualizar los flags de notificación o los flags de evento...
    if(MQ::MQClient::isTopicToken(topic, _touch_topic)){
        DEBUG_TRACE("\r\n[RGBGame]....... Recibido topic '%s' con msg '%s'", topic, (char*)msg);

        /* Chequea que el mensaje tiene formato correcto */
		// el mensaje es un stream 'ELEC,0'
		char* elec = strtok((char*)msg, ",");
		char* value = strtok(NULL, ",");
		
		if(!elec || !value){
			DEBUG_TRACE("\r\n[RGBGame]....... ERR_MSG, mensaje con formato incorrecto en topic '%s'", topic);
			return;
		}
				
        // crea mensaje para publicar en la máquina de estados
        State::Msg* op = (State::Msg*)Heap::memAlloc(sizeof(State::Msg));
        MBED_ASSERT(op);
		
		/* Reserva espacio para alojar el mensaje y parsea el mensaje */
		uint32_t* pelec = (uint32_t*)Heap::memAlloc(sizeof(uint32_t));
		MBED_ASSERT(pelec);
		*pelec = atoi(elec);

		/* Asigna el tipo de señal (evento) */
		op->sig = (atoi(value) == 0)? TouchReleaseEvt : TouchPressEvt;
		
        /* Asigna el mensaje (anterior) o NULL si no se utiliza ninguno */
		op->msg = pelec;

        // postea en la cola de la máquina de estados
        _queue.put(op);
        return;
    }
    DEBUG_TRACE("\r\n[RGBGame]....... ERR_TOPIC. No se puede procesar el topic '%s'", topic);
}

//------------------------------------------------------------------------------------
State::StateResult RGBGame::Init_EventHandler(State::StateEvent* se){
    switch((int)se->evt){
        case State::EV_ENTRY:{
        	DEBUG_TRACE("\r\n[RGBGame]....... EV_ENTRY en stInit");
        	// recupera los datos de memoria NV
        	restoreConfig();
			
			// carga la configuración por defecto
            _stat.colorMinNext = _cfg.colorMin;
            _stat.colorMaxNext = _cfg.colorMax;

            // Inicia el hilo para la ejecución de efectos y espera a que el hilo se inicie
            DEBUG_TRACE("\r\n[RGBGame]....... Inicia thread de efectos");
            _th_effect.start(callback(this, &RGBGame::effectsThread));
            Thread::wait(100);
            
            // inicia un efecto wave de un ciclo con inicio de señales led y sin servos
            DEBUG_TRACE("\r\n[RGBGame]....... Lanza efecto Wave x1");
            _stat.cycles = 1;
            _stat.flags = FlagDoLedStart;
            startEffect(WaveEffectEvt);
            
            // realiza una espera a que se complete para continuar
            while((_stat.flags & FlagFinished) == 0){
                Thread::wait(100);
            }
			
			// al terminar conmuta a stWait
            DEBUG_TRACE("\r\n[RGBGame]....... Conmuta a stWait");
			tranState(&_stWait);
        	
            return State::HANDLED;
        }

        case State::EV_EXIT:{
            DEBUG_TRACE("\r\n[RGBGame]....... EV_EXIT en stInit");
            nextState();
            return State::HANDLED;
        }

        default:{
        	return State::IGNORED;
        }

     }
}

//------------------------------------------------------------------------------------
State::StateResult RGBGame::Wait_EventHandler(State::StateEvent* se){
	State::Msg* st_msg = (State::Msg*)se->oe->value.p;
    switch((int)se->evt){
        
        case State::EV_ENTRY:{
        	DEBUG_TRACE("\r\n[RGBGame]....... EV_ENTRY en stWait");
            
			// inicializa el timeout para que interrumpa cada 1seg. 
            DEBUG_TRACE("\r\n[RGBGame]....... Inicia timeouts");
			_timeout = 1000;
            // marca un timeout de estado de 10seg.
			_acc_timeout = 10 * 1000;
        	
            return State::HANDLED;
        }

        case State::EV_TIMED:{
            DEBUG_TRACE("\r\n[RGBGame]....... EV_TIMED en stWait");
			_acc_timeout -= _timeout;
			// si la temporización vence, apaga todo y espera indefinidamente nuevos eventos
			if(_acc_timeout <= 0){
                DEBUG_TRACE("\r\n[RGBGame]....... Temporización cumplida. Apaga torre");
				_acc_timeout = 10 * 1000;
				_timeout = osWaitForever;
				startEffect(SwitchOffEffectEvt);
			}
			
            return State::HANDLED;
        }

        // Procesa evento de pulsación
        case TouchPressEvt:{
            DEBUG_TRACE("\r\n[RGBGame]....... EV_TOUCH en stWait, conmuta a stConfig");
            // conmuta a configuración
			tranState(&_stConfig);
			
            // libera el mensaje (tanto el contenedor de datos como el propio mensaje)
			if(st_msg->msg){
				Heap::memFree(st_msg->msg);
			}
            Heap::memFree(st_msg);

            return State::HANDLED;
        }

        case State::EV_EXIT:{
            DEBUG_TRACE("\r\n[RGBGame]....... EV_EXIT en stWait");
            nextState();
            return State::HANDLED;
        }

        default:{
        	return State::IGNORED;
        }

     }
}


//------------------------------------------------------------------------------------
State::StateResult RGBGame::Config_EventHandler(State::StateEvent* se){
    static int timed_play = 0;
	State::Msg* st_msg = (State::Msg*)se->oe->value.p;
    switch((int)se->evt){
        
        case State::EV_ENTRY:{
        	DEBUG_TRACE("\r\n[RGBGame]....... EV_ENTRY en stConfig");

			// inicializa el timeout para que interrumpa cada 1seg. 
			_timeout = 1000;
            // marca un timeout de estado de 30seg.
			_acc_timeout = 30 * 1000;
            // marca un timeout de standby tras touched de 10seg.
            timed_play = 10 * 1000;
            
            // publica mensaje con la configuración actual
            DEBUG_TRACE("\r\n[RGBGame]....... Publica configuración");
            publishConfig();
            
            // genera un wave con servos con la configuración por defecto (sin necesidad de reiniciar señales led)
            DEBUG_TRACE("\r\n[RGBGame]....... Genera efecto Wave x1");
            _stat.cycles = 1;
            _stat.flags = FlagDoServos;
            startEffect(WaveEffectEvt);
        	
            return State::HANDLED;
        }

        case State::EV_TIMED:{
            DEBUG_TRACE("\r\n[RGBGame]....... EV_TIMED en stConfig");
			_acc_timeout -= _timeout;
            
            // chequa timeout tras evento touch-press
            if((_stat.flags & FlagTouched) != 0){
                timed_play -= _timeout;
                if(timed_play <= 0){
                    DEBUG_TRACE("\r\n[RGBGame]....... Conmuta a stGame");
                    tranState(&_stGame);
                    return State::HANDLED;
                }
            }
            
			// si la temporización vence, vuelve a wait
			if(_acc_timeout <= 0){
                DEBUG_TRACE("\r\n[RGBGame]....... Timeout, conmuta a stWait");
				tranState(&_stWait);
                return State::HANDLED;
			}
			
            return State::HANDLED;
        }

        // Procesa evento de pulsación
        case TouchPressEvt:{
            DEBUG_TRACE("\r\n[RGBGame]....... EV_TOUCH en stConfig");
            
            // borra temporización y marca flag
            _stat.flags = (Flags)(_stat.flags|FlagTouched);
            _acc_timeout = 30 * 1000;
            timed_play = 10 * 1000;
            
            // lee el mensaje (electrodo que notifica pulsación)
            uint32_t elec = *((uint32_t*)st_msg->msg);			
					
			// actualiza la configuración en función del electrodo y genera un nuevo efecto
            DEBUG_TRACE("\r\n[RGBGame]....... Actualiza estado de la torre e inicia nuevo efecto");
            updateEffectConfig((uint8_t)elec);            
			startEffect(WaveEffectEvt);
			
            // libera el mensaje (tanto el contenedor de datos como el propio mensaje)
			if(st_msg->msg){
				Heap::memFree(st_msg->msg);
			}
            Heap::memFree(st_msg);

            return State::HANDLED;
        }
        
        case State::EV_EXIT:{
            DEBUG_TRACE("\r\n[RGBGame]....... EV_EXIT en stConfig");
            nextState();
            return State::HANDLED;
        }

        default:{
        	return State::IGNORED;
        }

     }
}


//------------------------------------------------------------------------------------
State::StateResult RGBGame::Game_EventHandler(State::StateEvent* se){
	State::Msg* st_msg = (State::Msg*)se->oe->value.p;
    switch((int)se->evt){
        
        case State::EV_ENTRY:{
        	DEBUG_TRACE("\r\n[RGBGame]....... Iniciando estado stGame...");

			// inicializa el timeout para que interrumpa cada 1seg. 
			_timeout = 1000;
            // marca un timeout de estado de 30seg.
			_acc_timeout = 30 * 1000;
                        
            // genera efecto wave en bucle con servos con la configuración por defecto (sin necesidad de reiniciar señales led)
            _stat.cycles = InfiniteCycles;
            _stat.flags = FlagDoServos;
            startEffect(WaveEffectEvt);
        	
            return State::HANDLED;
        }

        case State::EV_TIMED:{
            DEBUG_TRACE("\r\n[RGBGame]....... EV_TIMED en stGame");
			_acc_timeout -= _timeout;
                        
			// si la temporización vence, vuelve a wait
			if(_acc_timeout <= 0){
                DEBUG_TRACE("\r\n[RGBGame]....... Conmuta a stWait");
				tranState(&_stWait);
			}
			
            return State::HANDLED;
        }

        // Procesa evento de pulsación
        case TouchPressEvt:{
            DEBUG_TRACE("\r\n[RGBGame]....... EV_TOUCH en stGame, conmuta a stConfig");
            
            // vuelve a modo configuración
            tranState(&_stConfig);
			
            // libera el mensaje (tanto el contenedor de datos como el propio mensaje)
			if(st_msg->msg){
				Heap::memFree(st_msg->msg);
			}
            Heap::memFree(st_msg);

            return State::HANDLED;
        }

        // Procesa evento de inicio de ciclo de efecto
        case EffectCycleEvt:{           
            DEBUG_TRACE("\r\n[RGBGame]....... EV_EFFECT en stGame, publica estado");            
            // publica estado actual
            publishStat();
			
            // libera el mensaje (tanto el contenedor de datos como el propio mensaje)
			if(st_msg->msg){
				Heap::memFree(st_msg->msg);
			}
            Heap::memFree(st_msg);

            return State::HANDLED;
        }

        case State::EV_EXIT:{
            DEBUG_TRACE("\r\n[RGBGame]....... EV_EXIT en stGame");
            nextState();
            return State::HANDLED;
        }

        default:{
        	return State::IGNORED;
        }

     }
}


//------------------------------------------------------------------------------------
void RGBGame::putMessage(State::Msg *msg){
    _queue.put(msg);
}


//------------------------------------------------------------------------------------
osEvent RGBGame:: getOsEvent(){
	return _queue.get(_timeout);
}



//------------------------------------------------------------------------------------
void RGBGame::publicationCb(const char* topic, int32_t result){

}


//------------------------------------------------------------------------------------
bool RGBGame::checkIntegrity(){
	bool chk_ok = true;
    /* Chequea integridad de la configuración */
	if(_cfg.degMin > _cfg.degMax || _cfg.degMax > PCA9685_ServoDrv::MaxAngleValue){
        chk_ok = false;
    }    
	
	if(!chk_ok){
        DEBUG_TRACE("\r\n[RGBGame]....... ERR_CFG al chequear integridad de configuración");	
		setDefaultConfig();
		return false;
	}	
    DEBUG_TRACE("\r\n[RGBGame]....... Integridad de configuración OK!");	
	return true;
}


//------------------------------------------------------------------------------------
void RGBGame::setDefaultConfig(){
    DEBUG_TRACE("\r\n[RGBGame]....... Estableciendo configuración por defecto");
	
	/* Establece configuración por defecto */
	_cfg.colorMin = (WS281xLedStrip::Color_t){0,0,2};
    _cfg.colorMax = (WS281xLedStrip::Color_t){0, 0, 255};
    _cfg.degMin = 0;
    _cfg.degMax = 80;
    _cfg.delayMs = 50;
	
	/* Guarda en memoria NV */
	saveConfig();
}


//------------------------------------------------------------------------------------
void RGBGame::restoreConfig(){
    bool success = true;
	if(!restoreParameter("RGBGameCfg", &_cfg, sizeof(Config), NVSInterface::TypeBlob)){
        DEBUG_TRACE("\r\n[RGBGame]....... ERR_NV al recuperar la configuración");
        success = false;
    }
    if(success){
		DEBUG_TRACE("\r\n[RGBGame]....... Datos recuperados OK! Chequeando integridad...");
    	// chequea la coherencia de los datos y en caso de algo no esté bien, establece los datos por defecto
    	// almacenándolos de nuevo en memoria NV.
    	if(!checkIntegrity()){
    		return;
    	}
    	DEBUG_TRACE("\r\n[RGBGame]....... Configuración recuperada correctamente!");
        return;
	}
	DEBUG_TRACE("\r\n[RGBGame]....... ERR_NV. Error en la recuperación de datos. Establece configuración por defecto");
	setDefaultConfig();
}


//------------------------------------------------------------------------------------
void RGBGame::saveConfig(){
    DEBUG_TRACE("\r\n[RGBGame]....... Guardando configuración en memoria NV");	
	saveParameter("RGBGameCfg", &_cfg, sizeof(Config), NVSInterface::TypeBlob);
}


//------------------------------------------------------------------------------------
//-- PRIVATE METHODS IMPLEMENTATION --------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
void RGBGame::effectsThread(){
    DEBUG_TRACE("\r\n[RGBGame]....... Thread de efectos arrancado!");
	
    // espera señales, la variable _stat se actualizará para iniciar un nuevo efecto
    // cuando el efecto finalice (de forma automática o por intervención externa), volverá
    // a esperar una nueva ejecución que se iniciará mediante <startEffect>
    for(;;){
        osEvent oe = _th_effect.signal_wait(0, osWaitForever);
        _stat.flags = (Flags)(_stat.flags & ~(FlagUpdate | FlagCancel | FlagFinished));
        switch(oe.value.signals){
            case WaveEffectEvt:{
				DEBUG_TRACE("\r\n[RGBGame]....... WaveEffectEvt recibido");
                waveEffect();
                break;
            }
            case SwitchOffEffectEvt:{
				DEBUG_TRACE("\r\n[RGBGame]....... SwitchOffEffectEvt recibido");
                switchOff();
                break;
            }
        }        
		DEBUG_TRACE("\r\n[RGBGame]....... EFFECT_END");
        _stat.flags = (Flags)(_stat.flags | FlagFinished);
    }
}

//------------------------------------------------------------------------------------
void RGBGame::startEffect(EffectsEventFlags effect){
    _stat.flags = (Flags)(_stat.flags | FlagCancel);
    _th_effect.signal_set(effect);
}


//------------------------------------------------------------------------------------
void RGBGame::waveEffect(){
    int i;
	// Inicializa leds y servos
    _stat.mtx.lock();
    _stat.colorMinNext = _cfg.colorMin;
    _stat.colorMaxNext = _cfg.colorMax;
    _stat.colorMin = _cfg.colorMin;
    _stat.colorMax = _cfg.colorMax;
    _stat.mtx.unlock();
    
    for(i=0;i<NumLeds;i++){
        _strip[i] = _stat.colorMin;
        _ls->applyColor(i, _strip[i]);
    }
    
    for(i=0;i<NumSteps;i++){
        _angles[i] = _cfg.degMin;
    }
    
    // inicia señales en los leds si es necesario
    if((_stat.flags & FlagDoLedStart) != 0){
        _ls->start();
    }

	// inicia posición en los servos si es necesario
	if((_stat.flags & FlagDoServos) != 0){
        for(i=0;i<NumServos;i++){
            _sd->setServoAngle(i, _angles[servo_id[i]]);
        }	
        _sd->updateAll();
    }
    
	
    // continuamente, cada 50ms actualizo el wave propagándolo hacia el final de la tira    
    int8_t point = -1;
    
    for(;;){
        // si el flag Update o Cancel se han activado (generalmente cuando se ha realizado un toque en la estructura)
        if((_stat.flags & (FlagUpdate|FlagCancel)) != 0){
            // reinicia la secuencia
            point = -1;
            // el color se ha actualizado externamente
            _stat.mtx.lock();
            _stat.colorMin = _stat.colorMinNext;
            _stat.colorMax = _stat.colorMaxNext;
            _stat.mtx.unlock();
            // ajusta tira led al nuevo color
            for(i=0;i<NumLeds;i++){
                _strip[i] = _stat.colorMin;
                _ls->applyColor(i, _strip[i]);
            }
            // ajusta servos al estado inicial
			for(i=0;i<NumSteps;i++){
				_angles[i] = _cfg.degMin;
            }
            // si hay que actualizar la posición de los servos lo hace
			if((_stat.flags & FlagDoServos) != 0){
                for(i=0;i<NumServos;i++){
                    _sd->setServoAngle(i, _angles[servo_id[i]]);
                }
                _sd->updateAll();			
            }
            // borra el flag de actuación
            _stat.flags = (Flags)(_stat.flags & ~FlagUpdate);
            // chequea que la actuación sea definitiva y deba terminar
            if((_stat.flags & FlagCancel) != 0){
                // en ese caso, finaliza
                _stat.flags = (Flags)(_stat.flags & ~FlagCancel);
                return;
            }
        }
        
        // actualiza el punto de cresta
        point++;
        // si completa un ciclo, hace espera y chequea si debe finalizar
        if(point >= NumSteps+16){  
			point = 0;
            // si no hay que ejecutarlo de forma indefinida, chequea fin de ciclo
            if(_stat.cycles != InfiniteCycles){
                // si los ciclos cumplen, termina
                if(--_stat.cycles < 0){
                    Thread::wait(2000);
                    return;
                }
            }
            // crea mensaje para publicar en la máquina de estados que inicia un nuevo ciclo
            State::Msg* op = (State::Msg*)Heap::memAlloc(sizeof(State::Msg));
            MBED_ASSERT(op);                       
            /* Asigna el tipo de señal (evento) */
            op->sig = EffectCycleEvt;            
            /* Asigna el mensaje (anterior) o NULL si no se utiliza ninguno */
            op->msg = NULL;
            // postea en la cola de la máquina de estados
            _queue.put(op);
            Thread::wait(2000);            
        }
        // en otro caso, continúa con la propagación tras el retardo prefijado
        else{
            Thread::wait(_cfg.delayMs);
        }
        
        // propaga colores por la tira
        // disco superior
        for(i=NumLeds-1; i>NumLeds-3; i--){
            _strip[i] = _strip[i-1];
            _strip[i-3] = _strip[i-4];
            _strip[i-6] = _strip[i-7];
        }
        _strip[51] = _strip[39];
        _strip[45] = _strip[44];
        _strip[48] = _strip[34];
        
        // disco medio-sup
        for(i=NumLeds-10; i>NumLeds-14; i--){
            _strip[i] = _strip[i-1];
            _strip[i-5] = _strip[i-6];
            _strip[i-10] = _strip[i-11];
        }
        _strip[40] = _strip[24];
        _strip[30] = _strip[29];
        _strip[35] = _strip[19];
        
        // disco medio-inf
        for(i=NumLeds-25; i>NumLeds-29; i--){
            _strip[i] = _strip[i-1];
            _strip[i-5] = _strip[i-6];
            _strip[i-10] = _strip[i-11];
        }
        _strip[15] = _strip[14];
        _strip[20] = _strip[4];
        _strip[25] = _strip[9];
        
        // disco inferior
        for(i=NumLeds-40; i>NumLeds-44; i--){
            _strip[i] = _strip[i-1];
            _strip[i-5] = _strip[i-6];
            _strip[i-10] = _strip[i-11];
        }
        
        // calcula el nuevo valor al principio de la tira
        _strip[0].red = wavePoint(point, _stat.colorMax.red, _stat.colorMin.red);
        _strip[0].green = wavePoint(point, _stat.colorMax.green, _stat.colorMin.green);
        _strip[0].blue = wavePoint(point, _stat.colorMax.blue, _stat.colorMin.blue);
        _strip[5].red = _strip[0].red;
        _strip[5].green = _strip[0].green;
        _strip[5].blue = _strip[0].blue;
        _strip[10].red = _strip[0].red;
        _strip[10].green = _strip[0].green;
        _strip[10].blue = _strip[0].blue;
        
        // calcula la propagación a los servos
		for(i=NumSteps-1;i>0;i--){
			_angles[i] = _angles[i-1];
		}
		_angles[0] = wavePoint(point, _cfg.degMax, _cfg.degMin);
        if((_stat.flags & FlagDoServos) != 0){
            for(i=0;i<NumServos;i++){
                _sd->setServoAngle(i, _angles[servo_id[i]]);
            }
            // actualiza los servos
            _sd->updateAll();
        }
        // actualiza la tira
        for(i=0;i<NumLeds;i++){        
            _ls->applyColor(i, _strip[i]);
        }       
    }        
}


//------------------------------------------------------------------------------------
void RGBGame::switchOff(){
    int i;
    WS281xLedStrip::Color_t color_off = {0,0,0};
    for(i=0;i<NumLeds;i++){
        _strip[i] = color_off;
        _ls->applyColor(i, _strip[i]);
    }
    
    for(i=0;i<NumSteps;i++){
        _angles[i] = _cfg.degMin;
    }
    
    for(i=0;i<NumServos;i++){
        _sd->setServoAngle(i, _angles[servo_id[i]]);
    }	
    _sd->updateAll();        
}


//------------------------------------------------------------------------------------
void RGBGame::updateEffectConfig(uint8_t elec){
    #warning TODO
}

//------------------------------------------------------------------------------------
void RGBGame::publishConfig(){
    // publica mensaje con la configuración actual, el mensaje es en formato json del tipo:
    // "{\"redMin\":R,\"greenMin\":G,\"blueMin\":B,\"redMax\":R,\"greenMax\":G,\"blueMax\":B,\"delay\":MS}"
    char* msg = (char*) Heap::memAlloc(256);
    MBED_ASSERT(msg);
    sprintf(msg, "{\"redMin\":%d,\"greenMin\":%d,\"blueMin\":%d,\"redMax\":%d,\"greenMax\":%d,\"blueMax\":%d,\"delay\":%d}",
                 _cfg.colorMin.red, _cfg.colorMin.green, _cfg.colorMin.blue,  
                 _cfg.colorMax.red, _cfg.colorMax.green, _cfg.colorMax.blue,  
                 _cfg.delayMs);
    
    char *topic = (char*)Heap::memAlloc(MQ::MQClient::getMaxTopicLen());
    MBED_ASSERT(topic);
    sprintf(topic, "%s/config/stat", ActiveModule::_pub_topic_base);
    MQ::MQClient::publish(topic, msg, strlen(msg)+1, &_publicationCb);
}    


//------------------------------------------------------------------------------------
void RGBGame::publishStat(){
    // publica mensaje con la configuración actual, el mensaje es en formato json del tipo:
    // "{\"redMin\":R,\"greenMin\":G,\"blueMin\":B,\"redMax\":R,\"greenMax\":G,\"blueMax\":B,\"delay\":MS}"
    char* msg = (char*) Heap::memAlloc(256);
    MBED_ASSERT(msg);
    sprintf(msg, "{\"redMin\":%d,\"greenMin\":%d,\"blueMin\":%d,\"redMax\":%d,\"greenMax\":%d,\"blueMax\":%d,\"delay\":%d}",
                 _stat.colorMin.red, _stat.colorMin.green, _stat.colorMin.blue,  
                 _stat.colorMax.red, _stat.colorMax.green, _stat.colorMax.blue,  
                 _cfg.delayMs);
    
    char *topic = (char*)Heap::memAlloc(MQ::MQClient::getMaxTopicLen());
    MBED_ASSERT(topic);
    sprintf(topic, "%s/value/stat", ActiveModule::_pub_topic_base);
    MQ::MQClient::publish(topic, msg, strlen(msg)+1, &_publicationCb);
}   

