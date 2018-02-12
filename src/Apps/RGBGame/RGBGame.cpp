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

/** Macro para imprimir trazas de depuraci�n, siempre que se haya configurado un objeto
 *	Logger v�lido (ej: _debug)
 */

#define DEBUG_TRACE(format, ...)			\
if(ActiveModule::_defdbg){					\
	printf(format, ##__VA_ARGS__);			\
}											\
 
//------------------------------------------------------------------------------------
//-- PUBLIC METHODS IMPLEMENTATION ---------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
RGBGame::RGBGame(WS281xLedStrip* ls, PCA9685_ServoDrv* sd, FSManager* fs, bool defdbg) : ActiveModule("RGBGame", osPriorityNormal, OS_STACK_SIZE, fs, defdbg) {
	_publicationCb = callback(this, &TemplateImpl::publicationCb);
	_subscriptionCb = callback(this, &TemplateImpl::subscriptionCb);
}


//------------------------------------------------------------------------------------
//-- PROTECTED METHODS IMPLEMENTATION ------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
void RGBGame::subscriptionCb(const char* topic, void* msg, uint16_t msg_len){
    // si es un comando para actualizar los flags de notificaci�n o los flags de evento...
    if(MQ::MQClient::isTopicToken(topic, "/elec/stat")){
        DEBUG_TRACE("\r\nRGBGame\t Recibido topic '%s'", topic);

        /* Chequea que el mensaje tiene formato correcto */
		// el mensaje es un stream 'ELEC,0'
		char* elec = strtok((char*)msg, ",");
		char* value = strtok(NULL, ",");
		
		if(!elec || !value){
			DEBUG_TRACE("\r\nTemplImp\t ERR_MSG, mensaje con formato incorrecto en topic '%s'", topic);
			return;
		}
				
        // crea mensaje para publicar en la m�quina de estados
        State::Msg* op = (State::Msg*)Heap::memAlloc(sizeof(State::Msg));
        MBED_ASSERT(op);
		
		/* Reserva espacio para alojar el mensaje y parsea el mensaje */
		uint32_t* pelec = (uint32_t*)Heap::memAlloc(sizeof(uint32_t));
		MBED_ASSERT(pelec);
		*pelec = atoi(elec);

		/* Asigna el tipo de se�al (evento) */
		op->sig = (atoi(value) == 0)? TouchReleaseEvt : TouchPressEvt;
		
        /* Asigna el mensaje (anterior) o NULL si no se utiliza ninguno */
		op->msg = pelec;

        // postea en la cola de la m�quina de estados
        _queue.put(op);
        return;
    }
    DEBUG_TRACE("\r\nRGBGame\t ERR_TOPIC. No se puede procesar el topic '%s'", topic);
}

//------------------------------------------------------------------------------------
State::StateResult RGBGame::Init_EventHandler(State::StateEvent* se){
	State::Msg* st_msg = (State::Msg*)se->oe->value.p;
    switch((int)se->evt){
        case State::EV_ENTRY:{
        	int err = osOK;
			// inicia temporizaci�n
			_timeout = 0;
			
        	DEBUG_TRACE("\r\nRGBGame\t Iniciando recuperaci�n de datos...");
        	// recupera los datos de memoria NV
        	err = _fs->restore("RGBGameCfg", &_cfg, sizeof(Config), NVSInterface::TypeBlob);
        	if(err == osOK){
            	// chequea la coherencia de los datos y en caso de algo no est� bien, establece los datos por defecto
            	// almacen�ndolos de nuevo en memoria NV.
            	if(!checkIntegrity()){
            		DEBUG_TRACE("\r\nRGBGame\t ERR_CFG. Ha fallado el check de integridad. Establece configuraci�n por defecto.");
					setDefaultConfig();
            	}
				else{
					DEBUG_TRACE("\r\nRGBGame\t Recuperaci�n de datos OK!");
				}
        	}
			else{
				DEBUG_TRACE("\r\nRGBGame\t ERR_FS. Error en la recuperaci�n de datos. Establece configuraci�n por defecto");
				setDefaultConfig();
			}
			
			// hace un �nico efecto en azul, con retardos de 50ms en la propagaci�n
			waveEffect(1, [0,0,2], [0,0,255], 50);
			
			// al terminar conmuta a stWait
			tranState(stWait);
        	
            return State::HANDLED;
        }

        case State::EV_EXIT:{
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
        	int err = osOK;
        	DEBUG_TRACE("\r\nRGBGame\t Iniciando estado stWait...");

			// inicializa el timeout 
			_timeout = 1000;
			_acc_timeout = 10 * 1000;
        	
            return State::HANDLED;
        }

        case State::EV_TIMED:{
			_acc_timeout -= _timeout;
			// si la temporizaci�n vence, apaga todo y espera indefinidamente nuevos eventos
			if(_acc_timeout <= 0){
				_acc_timeout = 10 * 1000;
				_timeout = osWaitForever;
				switchOff();
			}
			
            return State::HANDLED;
        }

        // Procesa evento de pulsaci�n
        case TouchPressEvt:{
			uint32_t elec = *((uint32_t*)st_msg->msg);
			
			// actualiza la configuraci�n
			updateConfig(elec);
			
			// vuelve a reentrar
			tranState(stWait);
			
            // libera el mensaje (tanto el contenedor de datos como el propio mensaje)
			if(st_msg->msg){
				Heap::memFree(st_msg->msg);
			}
            Heap::memFree(st_msg);

            return State::HANDLED;
        }

        case State::EV_EXIT:{
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
	return _queue.get();
}



//------------------------------------------------------------------------------------
void RGBGame::publicationCb(const char* topic, int32_t result){

}


//------------------------------------------------------------------------------------
bool RGBGame::checkIntegrity(){
	bool chk_ok = true;
	/* Chequea integridad de la configuraci�n */
	// TODO
	
	if(!chk_ok){
		setDefaultConfig();
		return false;
	}	
	return true;
}


//------------------------------------------------------------------------------------
void RGBGame::setDefaultConfig(){
	/* Establece configuraci�n por defecto */
	//TODO
	
	/* Guarda en memoria NV */
	//TODO
	_fs->save("TemplImplCfg", &_cfg, sizeof(Config), NVSInterface::TypeBlob);
	_fs->saveParameter("TemplImplParam", &_cfg.param, sizeof(Param), NVSInterface::TypeParam);
}

