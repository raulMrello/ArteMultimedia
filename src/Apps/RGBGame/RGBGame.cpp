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
	_ls = ls;
	_sd = sd;
	
	// asigno timer de juego
	_tmr = new RtosTimer(callback(this, &RGBGame::timed), osTimerOnce);
	MBED_ASSERT(_tmr);
	
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
        DEBUG_TRACE("[RGBGame]....... TOPIC_SUB %s | msg %s\r\n", topic, (char*)msg);

        /* Chequea que el mensaje tiene formato correcto */
		// el mensaje es un stream 'ELEC,0'
		char* elec = strtok((char*)msg, ",");
		char* value = strtok(NULL, ",");
		
		if(!elec || !value){
			DEBUG_TRACE("[RGBGame]....... TOPIC_SUB %s | ERR_MSG, mensaje con formato incorrecto en topic\r\n", topic);
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
        putMessage(op);
        return;
    }
    DEBUG_TRACE("[RGBGame]....... TOPIC_SUB %s | ERR_TOPIC. No se puede procesar el topic\r\n", topic);
}

//------------------------------------------------------------------------------------
State::StateResult RGBGame::Init_EventHandler(State::StateEvent* se){
    switch((int)se->evt){
        case State::EV_ENTRY:{
        	DEBUG_TRACE("[RGBGame]....... ST_INIT: EV_ENTRY\r\n");
        	// recupera los datos de memoria NV
        	restoreConfig();
			
			// carga la configuración por defecto
            _stat.colorMinNext = _cfg.colorMin;
            _stat.colorMaxNext = _cfg.colorMax;

            // Inicia el hilo para la ejecución de efectos y espera a que el hilo se inicie
            DEBUG_TRACE("[RGBGame]....... ST_INIT: [onEntry] Inicia thread de efectos\r\n");
            _th_effect.start(callback(this, &RGBGame::effectsThread));
            Thread::wait(100);
            
            // inicia un efecto wave de un ciclo con inicio de señales led y sin servos
            DEBUG_TRACE("[RGBGame]....... ST_INIT: [onEntry] Lanza efecto Wave x1\r\n");
            _stat.cycles = 1;
            _stat.flags = FlagDoLedStart;
            startEffect(WaveEffectEvt);
            return State::HANDLED;
        }

		// cuando el efecto haya finalizado, conmuta a stWait
        case EffectEndEvt:{
            DEBUG_TRACE("[RGBGame]....... ST_INIT: EV_EFFECT_END --> stWait\r\n");
			tranState(&_stWait);
        	return State::HANDLED;
        }

        case State::EV_EXIT:{
            DEBUG_TRACE("[RGBGame]....... ST_INIT: EV_EXIT\r\n");
            nextState();
            return State::HANDLED;
        }

        default:{
            DEBUG_TRACE("[RGBGame]....... ST_INIT: IGNORED [%d]\r\n", (int)se->evt);
        	return State::IGNORED;
        }

     }
}

//------------------------------------------------------------------------------------
State::StateResult RGBGame::Wait_EventHandler(State::StateEvent* se){
    switch((int)se->evt){
        
        case State::EV_ENTRY:{
        	DEBUG_TRACE("[RGBGame]....... ST_WAIT: EV_ENTRY\r\n");
            
			// inicializa el timeout para que interrumpa a los 10s 
            DEBUG_TRACE("[RGBGame]....... ST_WAIT: [onEntry] Inicia timeouts\r\n");
			_timeout = 10000;
        	
            return State::HANDLED;
        }

        case State::EV_TIMED:{
            DEBUG_TRACE("[RGBGame]....... ST_WAIT: EV_TIMED\r\n");
            DEBUG_TRACE("[RGBGame]....... ST_WAIT: [onTimed] Temporización cumplida. Apaga torre\r\n");
			_timeout = osWaitForever;
			startEffect(SwitchOffEffectEvt);
			
            return State::HANDLED;
        }

        // Procesa evento de pulsación
        case TouchPressEvt:{
            DEBUG_TRACE("[RGBGame]....... ST_WAIT: EV_TOUCH --> stConfig\r\n");
            // conmuta a configuración
			tranState(&_stConfig);
			
            return State::HANDLED;
        }

        case State::EV_EXIT:{
            DEBUG_TRACE("[RGBGame]....... ST_WAIT: EV_EXIT\r\n");
            nextState();
            return State::HANDLED;
        }

        default:{
            DEBUG_TRACE("[RGBGame]....... ST_WAIT: IGNORED [%d]\r\n", (int)se->evt);
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
        	DEBUG_TRACE("[RGBGame]....... ST_CONFIG: EV_ENTRY\r\n");

			// inicializa el timeout para que interrumpa cada 10seg. 
			_timeout = 10000;
            // marca un timeout de estado de 30seg.
			_acc_timeout = 30 * 1000;
            // marca un timeout de standby tras touched de 10seg.
            timed_play = 10 * 1000;
            
            // publica mensaje con la configuración actual
            DEBUG_TRACE("[RGBGame]....... ST_CONFIG: [onEntry] Publica configuración\r\n");
            publishConfig();
            
            // genera un wave con servos con la configuración por defecto (sin necesidad de reiniciar señales led)
            DEBUG_TRACE("[RGBGame]....... ST_CONFIG: [onEntry] Genera efecto Wave x1\r\n");
            _stat.cycles = 1;
            _stat.flags = FlagDoServos;
            startEffect(WaveEffectEvt);
        	
            return State::HANDLED;
        }

        case State::EV_TIMED:{
            DEBUG_TRACE("[RGBGame]....... ST_CONFIG: EV_TIMED\r\n");
			_acc_timeout -= _timeout;
            
            // chequa timeout tras evento touch-press
            if((_stat.flags & FlagTouched) != 0){
                timed_play -= _timeout;
                if(timed_play <= 0){
                    DEBUG_TRACE("[RGBGame]....... ST_CONFIG: [onTimed] Timeout vencido tras pulsación --> stGame\r\n");
                    tranState(&_stGame);
                    return State::HANDLED;
                }
            }
            
			// si la temporización vence, vuelve a wait
			if(_acc_timeout <= 0){
                DEBUG_TRACE("[RGBGame]....... ST_CONFIG: [onTimed] Timeout vencido sin pulsación --> stWait\r\n");
				tranState(&_stWait);
                return State::HANDLED;
			}
			
            return State::HANDLED;
        }

        // Procesa evento de pulsación
        case TouchPressEvt:{
            DEBUG_TRACE("[RGBGame]....... ST_CONFIG: EV_TOUCH\r\n");
            
            // borra temporización y marca flag
            _stat.flags = (Flags)(_stat.flags|FlagTouched);
            _acc_timeout = 30 * 1000;
            timed_play = 10 * 1000;
            
            // lee el mensaje (electrodo que notifica pulsación)
            uint32_t elec = *((uint32_t*)st_msg->msg);			
					
			// actualiza la configuración en función del electrodo y genera un nuevo efecto
            DEBUG_TRACE("[RGBGame]....... ST_CONFIG: [onTouch] Actualiza estado de la torre e inicia nuevo efecto\r\n");
            updateEffectConfig((uint8_t)elec);            
			startEffect(WaveEffectEvt);
			
            return State::HANDLED;
        }
        
        case State::EV_EXIT:{
            DEBUG_TRACE("[RGBGame]....... ST_CONFIG: EV_EXIT\r\n");
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
    switch((int)se->evt){
        
        case State::EV_ENTRY:{
        	DEBUG_TRACE("[RGBGame]....... ST_GAME: EV_ENTRY\r\n");

			// inicializa el timeout para que no interrumpa
			_timeout = osWaitForever;
			// utiliza timer del juego para notificar timeout a los 30seg
			_tmr->start(30000);
                        
            // genera efecto wave en bucle con servos con la configuración por defecto (sin necesidad de reiniciar señales led)
            _stat.cycles = InfiniteCycles;
            _stat.flags = FlagDoServos;
            startEffect(WaveEffectEvt);
        	
            return State::HANDLED;
        }

        case GameTimedEvt:{
            DEBUG_TRACE("[RGBGame]....... ST_GAME: EV_TIMED\r\n");
            // cancela los efectos activos
            DEBUG_TRACE("[RGBGame]....... ST_GAME: [onTimed] Cancelando efecto activo\r\n");            
            _stat.flags = (Flags)(_stat.flags | FlagCancel);		
            return State::HANDLED;
        }

        // Procesa evento de pulsación
        case TouchPressEvt:{
			// detiene el tiempo de juego
			_tmr->stop();
			
            DEBUG_TRACE("[RGBGame]....... ST_GAME: EV_TOUCH --> stConfig\r\n");
            
            // vuelve a modo configuración
            tranState(&_stConfig);
			
            return State::HANDLED;
        }

        // Procesa evento de inicio de ciclo de efecto
        case EffectCycleEvt:{           
            DEBUG_TRACE("[RGBGame]....... ST_GAME: EV_EFFECT publica estado\r\n");            
            // publica estado actual
            publishStat();

            return State::HANDLED;
        }

        // Procesa evento de fin de ciclo de efecto
        case EffectEndEvt:{           
            DEBUG_TRACE("[RGBGame]....... ST_GAME: EV_EFFECT_END --> stWait\r\n");     
			tranState(&_stWait);

            return State::HANDLED;
        }

        case State::EV_EXIT:{
            DEBUG_TRACE("[RGBGame]....... ST_GAME: EV_EXIT\r\n");
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
    osStatus ost = _queue.put(msg);
    if(ost != osOK){
        DEBUG_TRACE("[RGBGame]....... QUEUE_PUT_ERROR %d\r\n", ost);
    }
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
    
    #warning TODO; 
    DEBUG_TRACE("~~~~~~ TODO ~~~~~~ Analizar los valores válidos del parámetro delayMs\r\n");    
    
    if(_cfg.delayMs != 50){
        chk_ok = false;
    }
	
	if(!chk_ok){
        DEBUG_TRACE("[RGBGame]....... ERR_CFG al chequear integridad de configuración\r\n");	
		setDefaultConfig();
		return false;
	}	
    DEBUG_TRACE("[RGBGame]....... Integridad de configuración OK!\r\n");	
	return true;
}


//------------------------------------------------------------------------------------
void RGBGame::setDefaultConfig(){
    DEBUG_TRACE("[RGBGame]....... Estableciendo configuración por defecto\r\n");
	
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
        DEBUG_TRACE("[RGBGame]....... ERR_NV al recuperar la configuración\r\n");
        success = false;
    }
    if(success){
		DEBUG_TRACE("[RGBGame]....... Datos recuperados OK! Chequeando integridad...\r\n");
    	// chequea la coherencia de los datos y en caso de algo no esté bien, establece los datos por defecto
    	// almacenándolos de nuevo en memoria NV.
    	if(!checkIntegrity()){
    		return;
    	}
    	DEBUG_TRACE("[RGBGame]....... Configuración recuperada correctamente!\r\n");
        return;
	}
	DEBUG_TRACE("[RGBGame]....... ERR_NV. Error en la recuperación de datos. Establece configuración por defecto\r\n");
	setDefaultConfig();
}


//------------------------------------------------------------------------------------
void RGBGame::saveConfig(){
    DEBUG_TRACE("[RGBGame]....... Guardando configuración en memoria NV\r\n");	
	saveParameter("RGBGameCfg", &_cfg, sizeof(Config), NVSInterface::TypeBlob);
}


//------------------------------------------------------------------------------------
//-- PRIVATE METHODS IMPLEMENTATION --------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
void RGBGame::effectsThread(){
    DEBUG_TRACE("[RGBGame]....... Thread de efectos arrancado!\r\n");
	
    // espera señales, la variable _stat se actualizará para iniciar un nuevo efecto
    // cuando el efecto finalice (de forma automática o por intervención externa), volverá
    // a esperar una nueva ejecución que se iniciará mediante <startEffect>
    for(;;){
        osEvent oe = _th_effect.signal_wait(0, osWaitForever);
        _stat.flags = (Flags)(_stat.flags & ~(FlagUpdate | FlagCancel | FlagFinished));
        switch(oe.value.signals){
            case WaveEffectEvt:{
				DEBUG_TRACE("[RGBGame]....... WaveEffectEvt recibido\r\n");
                waveEffect();
                break;
            }
            case SwitchOffEffectEvt:{
				DEBUG_TRACE("[RGBGame]....... SwitchOffEffectEvt recibido\r\n");
                switchOff();
                break;
            }
        }        
		DEBUG_TRACE("[RGBGame]....... Efecto finalizado!\r\n");
        _stat.flags = (Flags)(_stat.flags | FlagFinished);
		
		// crea mensaje para publicar en la máquina de estados que ha concluido el efecto
		State::Msg* op = (State::Msg*)Heap::memAlloc(sizeof(State::Msg));
		MBED_ASSERT(op);                       
		/* Asigna el tipo de señal (evento) */
		op->sig = EffectEndEvt;            
		/* Asigna el mensaje (anterior) o NULL si no se utiliza ninguno */
		op->msg = NULL;
		// postea en la cola de la máquina de estados
		putMessage(op);						
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
        point = (point < NumSteps+16)? (point+1) : 0;
        // si completa un ciclo, hace espera y chequea si debe finalizar
        if(point == 0){  
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
            putMessage(op);
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
    #warning TODO; 
    DEBUG_TRACE("~~~~~~ TODO ~~~~~~ Actualizar parámetros del efecto\r\n");
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
    Heap::memFree(topic);
    Heap::memFree(msg);
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
    Heap::memFree(topic);
    Heap::memFree(msg);
}   


void RGBGame::timed(){
	DEBUG_TRACE("[RGBGame]....... Notificando fin del juego...\r\n");
	// crea mensaje para publicar en la máquina de estados que ha concluido el efecto
	State::Msg* op = (State::Msg*)Heap::memAlloc(sizeof(State::Msg));
	MBED_ASSERT(op);                       
	/* Asigna el tipo de señal (evento) */
	op->sig = GameTimedEvt;            
	/* Asigna el mensaje (anterior) o NULL si no se utiliza ninguno */
	op->msg = NULL;
	// postea en la cola de la máquina de estados
	putMessage(op);
}
