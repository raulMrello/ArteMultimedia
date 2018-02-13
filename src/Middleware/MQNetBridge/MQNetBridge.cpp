/*
 * MQNetBridge.cpp
 *
 *  Created on: Nov 2017
 *      Author: raulMrello
 */


#include "MQNetBridge.h"
#include "easy-connect.h"

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
char* MBED_CONF_APP_WIFI_SSID = 0;      // Requerido en easy-connect
char* MBED_CONF_APP_WIFI_PASSWORD = 0;  // Requerido en easy-connect

//------------------------------------------------------------------------------------
static MQNetBridge* gThis=0;

//------------------------------------------------------------------------------------
static void MqttEventHandler(MQTT::MessageData& md){
    gThis->mqttEventHandler(md);    
}



//------------------------------------------------------------------------------------
//-- PUBLIC METHODS IMPLEMENTATION ---------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
MQNetBridge::MQNetBridge(const char* base_topic, uint32_t mqtt_yield_millis) : ActiveModule("RGBGame", osPriorityNormal, OS_STACK_SIZE, fs, defdbg)  { 
    
	DEBUG_TRACE("\r\nNetBridge\t Iniciando componente");

	_publicationCb = callback(this, &RGBGame::publicationCb);
	_subscriptionCb = callback(this, &RGBGame::subscriptionCb);
	
	_cfg.clientId = NULL;
	_cfg.userName = NULL;
	_cfg.userPass = NULL;
	_cfg.host = NULL;
	_cfg.essid = NULL;
	_cfg.passwd = NULL;
	_cfg.baseTopic = base_topic;
	_cfg.pollDelay = mqtt_yield_millis;
	gThis = this;
}


//------------------------------------------------------------------------------------
void MQNetBridge::mqttEventHandler(MQTT::MessageData& md){
    MQTT::Message &message = md.message;
    MQTTString &topicName = md.topicName;
    char* topic = (char*)Heap::memAlloc(topicName.lenstring.len + 1);
    if(!topic){
        return;
    }
    char* msg = (char*)Heap::memAlloc(message.payloadlen + 1);    
    if(!msg){
        Heap::memFree(topic);
        return;
    }
    strncpy(topic, topicName.lenstring.data, topicName.lenstring.len);
    topic[topicName.lenstring.len] = 0;
    memcpy(msg, message.payload, message.payloadlen);
    msg[message.payloadlen] = 0;
    MQ::MQClient::publish(topic, msg, strlen(msg)+1, &_publicationCb);
    Heap::memFree(topic);
    Heap::memFree(msg);
        
}
    
//------------------------------------------------------------------------------------
//-- PROTECTED METHODS IMPLEMENTATION ------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
void RGBGame::subscriptionCb(const char* topic, void* msg, uint16_t msg_len){
    // si es un comando para actualizar los flags de notificación o los flags de evento...
    if(MQ::MQClient::isTopicToken(topic, "/conn/cmd")){
        DEBUG_TRACE("\r\nNetBridge\t Recibido topic '%s' con msg '%s'", topic, (char*)msg);

        /* Chequea que el mensaje tiene formato correcto */
       // lee los parámetros esperados: ClientId,User,UserPass,Host,Port,ESSID, WifiPasswd
        char* cli = strtok((char*)msg, ",");
        char* usr = strtok(0, ",");
        char* usrpass = strtok(0, ",");
        char* host = strtok(0, ",");
        char* port = strtok(0, ",");
        char* essid = strtok(0, ",");
        char* passwd = strtok(0, ",");
        // si no se leen correctamente, lo descarta
        if(!cli || !usr || !usrpass || !host || !port || !essid || !passwd){   
			DEBUG_TRACE("\r\nNetBridge\t ERR_MSG, mensaje con formato incorrecto en topic '%s'", topic);
			return;	
		}
		// guarda la nueva configuración
		// desecha los antiguos...
		if(_cfg.clientId){
			Heap::memFree(_cfg.clientId);
		}
		if(_cfg.userName){
			Heap::memFree(_cfg.userName);
		}
		if(_cfg.userPass){
			Heap::memFree(_cfg.userPass);
		}
		if(_cfg.host){
			Heap::memFree(_cfg.host);
		}
		if(_cfg.essid){
			Heap::memFree(_cfg.essid);
		}
		if(_cfg.passwd){
			Heap::memFree(_cfg.passwd);
		}
		// actualiza...
		_cfg.clientId = (char*)Heap::memAlloc(strlen(cli)+1);
		MBED_ASSERT(_cfg.clientId);
		strcpy(_client_id, cli);
		_cfg.userName = (char*)Heap::memAlloc(strlen(usr)+1);
		MBED_ASSERT(_cfg.userName);
		strcpy(_cfg.userName, usr);
		_cfg.userPass = (char*)Heap::memAlloc(strlen(usrpass)+1);
		MBED_ASSERT(_cfg.userPass);
		strcpy(_cfg.userPass, usrpass);
		_cfg.host = (char*)Heap::memAlloc(strlen(host)+1);
		MBED_ASSERT(_cfg.host);
		strcpy(_cfg.host, host);
		_cfg.port = atoi(port);
		_cfg.essid = (char*)Heap::memAlloc(strlen(essid)+1);
		MBED_ASSERT(_cfg.essid);
		strcpy(_cfg.essid, essid);
		_cfg.passwd = (char*)Heap::memAlloc(strlen(passwd)+1);
		MBED_ASSERT(_cfg.passwd);
		strcpy(_cfg.passwd, passwd);
		// ajusta parámetros de conexión para easy-connect de mbed...
		MBED_CONF_APP_WIFI_SSID = _cfg.essid;
		MBED_CONF_APP_WIFI_PASSWORD = _cfg.passwd;
				
        // crea mensaje para publicar en la máquina de estados
        State::Msg* op = (State::Msg*)Heap::memAlloc(sizeof(State::Msg));
        MBED_ASSERT(op);

		/* Asigna el tipo de señal (evento) */
		op->sig = ConnReqEvt;
		
        /* Asigna el mensaje (anterior) o NULL si no se utiliza ninguno */
		op->msg = NULL;
        return;
    }
	
    // si es un comando para actualizar los flags de notificación o los flags de evento...
    if(MQ::MQClient::isTopicToken(topic, "/disc/cmd")){
        DEBUG_TRACE("\r\nNetBridge\t Recibido topic '%s' con msg '%s'", topic, (char*)msg);        
				
        // crea mensaje para publicar en la máquina de estados
        State::Msg* op = (State::Msg*)Heap::memAlloc(sizeof(State::Msg));
        MBED_ASSERT(op);

		/* Asigna el tipo de señal (evento) */
		op->sig = DiscReqEvt;
		
        /* Asigna el mensaje (anterior) o NULL si no se utiliza ninguno */
		op->msg = NULL;
        return;
    }
	
    DEBUG_TRACE("\r\nNetBridge\t ERR_TOPIC. No se puede procesar el topic '%s'", topic);
}

//------------------------------------------------------------------------------------
State::StateResult RGBGame::Init_EventHandler(State::StateEvent* se){
    switch((int)se->evt){
        case State::EV_ENTRY:{
        	DEBUG_TRACE("\r\nNetBridge\t EV_ENTRY en stInit");
        	
			// recupera los datos de memoria NV
        	restoreConfig();
			
			// se suscribe a los topics locales $BASE/+/cmd
			char* topic = (char*)Heap::memAlloc(MQ::MQClient::getMaxTopicLen());
			MBED_ASSERT(topic);
			sprintf(topic, "%s/+/cmd", _cfg.baseTopic);
			DEBUG_TRACE("\r\nNetBridge\t Suscribiendose a '%s'", topic);
			MQ::MQClient::subscribe(topic, &_subscriptionCb);
			#warning TODO cambiar MQLib para que reserve espacio para <name> y aquí se pueda liberar
			//Heap::memFree(topic);
			
			// el estado del thread mqtt por el momento es desconocido
            _stat = Unknown;
        	
            return State::HANDLED;
        }
                               
		// Evento de solicitud de conexión
		case ConnReqEvt:{
			DEBUG_TRACE("\r\nNetBridge\t EV_CONN_REQ en stInit");
			if(reconnect() == 0){
				// si el hilo mqtt no se ha iniciado, lo inicia
				if(_stat == Unknown){
					DEBUG_TRACE("\r\nNetBridge\t Iniciando thread mqtt");
					_th_mqtt.start(callback(this, &MQNetBridge::mqttThread));
				}
			}
            // libera el mensaje (tanto el contenedor de datos como el propio mensaje)
			if(st_msg->msg){
				Heap::memFree(st_msg->msg);
			}
            Heap::memFree(st_msg);
			
			return State::HANDLED;
		}
				
		// si se notifica una conexión correcta...
		case ConnAckEvt:{
			DEBUG_TRACE("\r\nNetBridge\t EV_CONN_ACK en stInit, iniciando suscripción");
			// se suscribe a $BASE/+/cmd
			char* topic = (char*)Heap::memAlloc(strlen(_cfg.baseTopic) + strlen("/+/cmd") + 1);
			MBED_ASSERT(topic);
			sprintf(topic, "%s/+/cmd", _cfg.baseTopic);
			if (_client->subscribe(topic, MQTT::QOS0, MqttEventHandler) != 0){
				DEBUG_TRACE("\r\nNetBridge\t ERR_SUBSC al suscribirse al topic '%s'", topic);
			}
			Heap::memFree(topic);
			
			// libera el mensaje (tanto el contenedor de datos como el propio mensaje)
			if(st_msg->msg){
				Heap::memFree(st_msg->msg);
			}
            Heap::memFree(st_msg);

			return State::HANDLED;
		}             
				
		// si se notifica una suscripción correcta...
		case SubAckEvt:{
			DEBUG_TRACE("\r\nNetBridge\t EV_SUB_ACK en stInit");
			
			// libera el mensaje (tanto el contenedor de datos como el propio mensaje)
			if(st_msg->msg){
				Heap::memFree(st_msg->msg);
			}
            Heap::memFree(st_msg);

			return State::HANDLED;
		}             
				
		// si se notifica una publicación correcta...
		case PubAckEvt:{
			DEBUG_TRACE("\r\nNetBridge\t EV_PUB_ACK en stInit");
			
			// libera el mensaje (tanto el contenedor de datos como el propio mensaje)
			if(st_msg->msg){
				Heap::memFree(st_msg->msg);
			}
            Heap::memFree(st_msg);

			return State::HANDLED;
		}             

        case State::EV_EXIT:{
            DEBUG_TRACE("\r\nNetBridge\t EV_EXIT en stInit");
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
    /* Chequea integridad de la configuración */
	if(!_cfg.clientId || !_cfg.userName || !_cfg.userPass || !_cfg.host || !_cfg.essid || !_cfg.passwd){
		chk_ok = false;
	}  
	
	if(!chk_ok){
        DEBUG_TRACE("\r\nNetBridge\t ERR_CFG al chequear integridad de configuración");	
		setDefaultConfig();
		return false;
	}	
    DEBUG_TRACE("\r\nNetBridge\t Integridad de configuración OK!");	
	return true;
}


//------------------------------------------------------------------------------------
void RGBGame::setDefaultConfig(){
    DEBUG_TRACE("\r\nNetBridge\t Estableciendo configuración por defecto: TBD");
	
	/* Guarda en memoria NV */
	saveConfig();
}


//------------------------------------------------------------------------------------
void RGBGame::restoreConfig(){
    bool success = true;
	if(!restoreParameter("MQNetBridgeCfg", &_cfg, sizeof(Config), NVSInterface::TypeBlob)){
        DEBUG_TRACE("\r\nNetBridge\t ERR_NV al recuperar la configuración");
        success = false;
    }
    if(success){
		DEBUG_TRACE("\r\nNetBridge\t Datos recuperados OK! Chequeando integridad...");
    	// chequea la coherencia de los datos y en caso de algo no esté bien, establece los datos por defecto
    	// almacenándolos de nuevo en memoria NV.
    	if(!checkIntegrity()){
    		return;
    	}
    	DEBUG_TRACE("\r\nNetBridge\t Configuración recuperada correctamente!");
        return;
	}
	DEBUG_TRACE("\r\nNetBridge\t ERR_NV. Error en la recuperación de datos. Establece configuración por defecto");
	setDefaultConfig();
}


//------------------------------------------------------------------------------------
void RGBGame::saveConfig(){
    DEBUG_TRACE("\r\nNetBridge\t Guardando configuración en memoria NV");	
	saveParameter("MQNetBridgeCfg", &_cfg, sizeof(Config), NVSInterface::TypeBlob);
}


//------------------------------------------------------------------------------------
//-- PRIVATE METHODS IMPLEMENTATION --------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
int MQNetBridge::connect(){
    // inicia el proceso de conexión...
    // Levanta el interfaz de red wifi
    int rc;    
    if(!_network || _stat == Ready){
		DEBUG_TRACE("\r\nNetBridge\t Levantando interfaz de red");
        _network = easy_connect(true);
        if(!_network){
			DEBUG_TRACE("\r\nNetBridge\t ERR_IFUP, no se ha podido conectar a la red wifi");
            _stat = WifiError;
            return -1;
        }
    }
    
    // Prepara socket tcp...
    if(!_net){
		DEBUG_TRACE("\r\nNetBridge\t Abriendo socket");
        _net = new MQTTNetwork(_network);
    }
    
    // Prepara para funcionamiento asíncrono
    _net->set_blocking(true);
        
    // Abre socket tcp...
    if((rc = _net->connect(_host, _port)) != 0){
		DEBUG_TRACE("\r\nNetBridge\t ERR_CONN, al conectar el socket");
        _stat = SockError;
        return rc;
    }                
	
    // Prepara cliente mqtt...
    if(!_client){
		DEBUG_TRACE("\r\nNetBridge\t Conectando cliente MQTT");
        _client = new MQTT::Client<MQTTNetwork, Countdown>(*_net);
    }
    
    // Conecta cliente MQTT...
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.clientID.cstring = _client_id;
    data.username.cstring = _user;
    data.password.cstring = _userpass;
    if ((rc = _client->connect(data)) != 0){
		DEBUG_TRACE("\r\nNetBridge\t ERR_MQTT al conectar con el broker MQTT");
        _stat = MqttError;
        return rc;
    }     
    DEBUG_TRACE("\r\nNetBridge\t CONN_OK!");
    _stat = Connected;
    return 0;
}


//------------------------------------------------------------------------------------
void MQNetBridge::disconnect(){
    // desconecta si estuviera conectado
    // cierra conexión mqtt...    
    if(_client && _client->isConnected()){
		DEBUG_TRACE("\r\nNetBridge\t Deteniendo  cliente mqtt");
        _client->disconnect();
    }
    // cierra el socket tcp...
    if(_net){
		DEBUG_TRACE("\r\nNetBridge\t Cerrando socket");
        _net->disconnect();                
    }
    // finaliza conexión wifi...
    if(_network){
		DEBUG_TRACE("\r\nNetBridge\t Deteniendo interfaz de red");
        _network->disconnect();
    }
    _stat = Ready;
}


//---------------------------------------------------------------------------------
void MQNetBridge::mqttThread(){    
    DEBUG_TRACE("\r\nNetBridge\t Thread mqtt iniciado:");

	// marca thread como iniciado
    _stat = Ready;
    
    DEBUG_TRACE("\r\nNetBridge\t Thread mqtt esperando eventos...");
    for(;;){       
        _client->yield(_cfg.pollDelay);
	}
//        _timeout = osWaitForever;        
//        // procesa solicitudes pendientes
//        if(_stat == Connected){
//            _client->yield(_yield_millis);
//            _timeout = 0;
//        }
//        osEvent evt = _queue.get(_timeout);
//        
//        // Si hay que procesar un mensaje...
//        if(evt.status == osEventMessage){
//            RequestOperation_t* msg = (RequestOperation_t*)evt.value.p;
//            switch(msg->id){
//                               
//                // Si hay que conectar...
//                case ConnectSig:{
//                    DEBUG_TRACE("\r\nNetBridge\t SIG_CONN, iniciando conexión");
//                    reconnect();
//					break;
//				}
//				
//				// si se notifica una conexión correcta...
//				case ConnAckSig:{
//					DEBUG_TRACE("\r\nNetBridge\t SIG_CONN_ACK, iniciando suscripción");
//					// se suscribe a $BASE/+/cmd
//					char* topic = (char*)Heap::memAlloc(MQ::MQClient::getMaxTopicLength());
//					MBED_ASSERT(topic);
//					sprintf(topic, "%s/+/cmd", _base_topic);
//					if (_client->subscribe(topic, MQTT::QOS0, remoteSubscriptionCb) != 0){
//						DEBUG_TRACE("\r\nNetBridge\t ERR_SUBSC al suscribirse al topic '%s'", topic);
//					}
//					else{
//						DEBUG_TRACE("OK!!!!");
//					}  	
//					Heap::memFree(topic);
//                    break;
//                }             
//				
//				// si se notifica una conexión correcta...
//				case SubAckSig:{
//					DEBUG_TRACE("\r\nNetBridge\t SIG_SUB_ACK, suscripción hecha!");
//					break;
//                }             
//                            
//                // Si hay que desconectar...
//                case DisconnectSig:{
//                    // asegura que esté conectado...
//                    DEBUG_TRACE("\r\nNetBridge\t SIG_DISC, iniciando desconexión");
//                    disconnect();                                
//                    break;
//                }                                              
//                                                               
//                // Si hay que publicar en un topic... 
//                case RemotePublishSig: {
//                    char* topic = strtok(msg->data, "\n");
//                    char* msend = strtok(0, "\n");                    
//                    if(topic && msend){
//                        MQTT::Message message;
//                        message.qos = MQTT::QOS0;
//                        message.retained = false;
//                        message.dup = false;
//                        message.payload = msend;
//                        message.payloadlen = strlen(msend) + 1;
//                        uint8_t err = 0;
//                        DEBUG_TRACE("\r\nNetBridge: Publicando en topic[%s] el mensaje[%s] ... ", topic, msend);
//                        if(_client->publish(topic, message) != 0){            
//                            DEBUG_TRACE("ERROR=%d", err); 
//                        }
//                        else{
//                            DEBUG_TRACE("OK!");
//                        }
//                    }     
//                    break;
//                }             
//             }
//            
//            // Elimina la memoria asignada al mensaje
//            if(msg->data){
//                Heap::memFree(msg->data);
//            }
//            Heap::memFree(msg);
//        }    
//        
//        // en caso de que haya habido un error y se haya cerrado la conexión, habrá que intentar reconectar
//        if(_client && _stat == Connected && !_client->isConnected()){
//            DEBUG_TRACE("\r\nNetBridge: Iniciando reconexión...");
//            reconnect(); 
//        }
//    }
}

