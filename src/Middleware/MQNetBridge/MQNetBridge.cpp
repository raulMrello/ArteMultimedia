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
if(_defdbg){					            \
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
MQNetBridge::MQNetBridge(const char* local_base_topic, const char* remote_base_topic, FSManager* fs, uint32_t mqtt_yield_millis, bool defdbg) : ActiveModule("MQNetBridge", osPriorityNormal, OS_STACK_SIZE, fs, defdbg)  { 
    
	DEBUG_TRACE("\r\n[MQNetBridge]... Iniciando componente");

	_publicationCb = callback(this, &MQNetBridge::publicationCb);
	_subscriptionCb = callback(this, &MQNetBridge::subscriptionCb);
	
	_cfg.clientId = NULL;
	_cfg.userName = NULL;
	_cfg.userPass = NULL;
	_cfg.host = NULL;
	_cfg.essid = NULL;
	_cfg.passwd = NULL;
	_cfg.localBaseTopic = local_base_topic;
	_cfg.remoteBaseTopic = remote_base_topic;
	_cfg.pollDelay = mqtt_yield_millis;
    _network = NULL;
    _net = NULL;
    _client = NULL;
	gThis = this;
	ActiveModule::setPublicationBase(_cfg.localBaseTopic);
	ActiveModule::setSubscriptionBase(_cfg.localBaseTopic);
}


//------------------------------------------------------------------------------------
void MQNetBridge::mqttEventHandler(MQTT::MessageData& md){
    MQTT::Message &message = md.message;
    MQTTString &topicName = md.topicName;
    char* topic = (char*)Heap::memAlloc(topicName.lenstring.len + 1);
    MBED_ASSERT(topic);
    char* msg = (char*)Heap::memAlloc(message.payloadlen + 1);    
    MBED_ASSERT(msg);
    
    strncpy(topic, topicName.lenstring.data, topicName.lenstring.len);
    topic[topicName.lenstring.len] = 0;
    
    memcpy(msg, message.payload, message.payloadlen);
    msg[message.payloadlen] = 0;
    
    MQ::MQClient::publish(topic, msg, message.payloadlen + 1, &_publicationCb);
    Heap::memFree(topic);
    Heap::memFree(msg);
        
}

    
//------------------------------------------------------------------------------------
//-- PROTECTED METHODS IMPLEMENTATION ------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
void MQNetBridge::subscriptionCb(const char* topic, void* msg, uint16_t msg_len){
    // si es un comando para actualizar los flags de notificación o los flags de evento...
    if(MQ::MQClient::isTopicToken(topic, "/conn/cmd")){
        DEBUG_TRACE("\r\n[MQNetBridge]... Recibido topic '%s' con msg '%s'", topic, (char*)msg);

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
			DEBUG_TRACE("\r\n[MQNetBridge]... ERR_MSG, mensaje con formato incorrecto en topic '%s'", topic);
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
		strcpy(_cfg.clientId, cli);
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
		
		/** Inserta mensaje en cola de proceso */
		_queue.put(op);
        return;
    }
    // si es mensaje para hacer bridge hacia el broker mqtt...
    if(MQ::MQClient::isTopicToken(topic, "/stat")){
        DEBUG_TRACE("\r\n[MQNetBridge]... Recibido topic '%s' con msg '%s'", topic, (char*)msg);

        // directamente lo publica en el topic correspondiente siempre que la conexión esté operativa
        if((_stat & Connected) != 0){
            MQTT::Message message;
            message.qos = MQTT::QOS0;
            message.retained = false;
            message.dup = false;
            message.payload = msg;
            message.payloadlen = msg_len;
            int rc = 0;
            if((rc = _client->publish(topic, message)) != 0){            
                DEBUG_TRACE("\r\nNetBridge: ERR_MQTT_PUB, publicando en topic '%s', error %d", topic, rc); 
            }
            else{
                DEBUG_TRACE("\r\nNetBridge: PUB_OK, mensaje publicado en topic '%s'", topic); 
            }            
        }
        return;
    }
		
    // si es un comando para actualizar los flags de notificación o los flags de evento...
    if(MQ::MQClient::isTopicToken(topic, "/disc/cmd")){
        DEBUG_TRACE("\r\n[MQNetBridge]... Recibido topic '%s' con msg '%s'", topic, (char*)msg);        
				
        // crea mensaje para publicar en la máquina de estados
        State::Msg* op = (State::Msg*)Heap::memAlloc(sizeof(State::Msg));
        MBED_ASSERT(op);

		/* Asigna el tipo de señal (evento) */
		op->sig = DiscReqEvt;
		
        /* Asigna el mensaje (anterior) o NULL si no se utiliza ninguno */
		op->msg = NULL;
		
		/** Inserta mensaje en cola de proceso */
		_queue.put(op);
        return;
    }
	
    DEBUG_TRACE("\r\n[MQNetBridge]... ERR_TOPIC. No se puede procesar el topic '%s'", topic);
}

//------------------------------------------------------------------------------------
State::StateResult MQNetBridge::Init_EventHandler(State::StateEvent* se){
    switch((int)se->evt){
        case State::EV_ENTRY:{
        	DEBUG_TRACE("\r\n[MQNetBridge]... EV_ENTRY en stInit");
        	
			// recupera los datos de memoria NV
        	restoreConfig();
			
			// se suscribe a los topics locales $BASE/+/cmd
			char* topic = (char*)Heap::memAlloc(MQ::MQClient::getMaxTopicLen());
			MBED_ASSERT(topic);
			sprintf(topic, "%s/+/cmd", _cfg.localBaseTopic);
			DEBUG_TRACE("\r\n[MQNetBridge]... Suscribiendose a '%s'", topic);
			MQ::MQClient::subscribe(topic, &_subscriptionCb);
			Heap::memFree(topic);
			
			// el estado del thread mqtt por el momento es desconocido
            _stat = Unknown;
        	
            return State::HANDLED;
        }
                               
		// Evento de solicitud de conexión
		case ConnReqEvt:{
			DEBUG_TRACE("\r\n[MQNetBridge]... EV_CONN_REQ en stInit");
			// si el hilo mqtt no se ha iniciado, lo inicia
			if(_stat == Unknown){
				DEBUG_TRACE("\r\n[MQNetBridge]... Iniciando thread mqtt");
				_th_mqtt.start(callback(this, &MQNetBridge::mqttThread));			
			}
            else{
                DEBUG_TRACE("\r\n[MQNetBridge]... ERR_CONN, conexión ya iniciada anteriormente.");
            }
            
			return State::HANDLED;
		}            

        case State::EV_EXIT:{
            DEBUG_TRACE("\r\n[MQNetBridge]... EV_EXIT en stInit");
            nextState();
            return State::HANDLED;
        }

        default:{
        	return State::IGNORED;
        }

    }
}


//------------------------------------------------------------------------------------
void MQNetBridge::putMessage(State::Msg *msg){
    _queue.put(msg);
}


//------------------------------------------------------------------------------------
osEvent MQNetBridge:: getOsEvent(){
	return _queue.get();
}



//------------------------------------------------------------------------------------
void MQNetBridge::publicationCb(const char* topic, int32_t result){

}


//------------------------------------------------------------------------------------
bool MQNetBridge::checkIntegrity(){
	bool chk_ok = true;
    /* Chequea integridad de la configuración */
	if(!_cfg.clientId || !_cfg.userName || !_cfg.userPass || !_cfg.host || !_cfg.essid || !_cfg.passwd){
		chk_ok = false;
	}  
	
	if(!chk_ok){
        DEBUG_TRACE("\r\n[MQNetBridge]... ERR_CFG al chequear integridad de configuración");	
		setDefaultConfig();
		return false;
	}	
    DEBUG_TRACE("\r\n[MQNetBridge]... Integridad de configuración OK!");	
	return true;
}


//------------------------------------------------------------------------------------
void MQNetBridge::setDefaultConfig(){
    DEBUG_TRACE("\r\n[MQNetBridge]... Estableciendo configuración por defecto: TBD");
	
	/* Guarda en memoria NV */
	saveConfig();
}


//------------------------------------------------------------------------------------
void MQNetBridge::restoreConfig(){
    bool success = true;
	if(!restoreParameter("MQNetBridgeCfg", &_cfg, sizeof(Config), NVSInterface::TypeBlob)){
        DEBUG_TRACE("\r\n[MQNetBridge]... ERR_NV al recuperar la configuración");
        success = false;
    }
    if(success){
		DEBUG_TRACE("\r\n[MQNetBridge]... Datos recuperados OK! Chequeando integridad...");
    	// chequea la coherencia de los datos y en caso de algo no esté bien, establece los datos por defecto
    	// almacenándolos de nuevo en memoria NV.
    	if(!checkIntegrity()){
    		return;
    	}
    	DEBUG_TRACE("\r\n[MQNetBridge]... Configuración recuperada correctamente!");
        return;
	}
	DEBUG_TRACE("\r\n[MQNetBridge]... ERR_NV. Error en la recuperación de datos. Establece configuración por defecto");
	setDefaultConfig();
}


//------------------------------------------------------------------------------------
void MQNetBridge::saveConfig(){
    DEBUG_TRACE("\r\n[MQNetBridge]... Guardando configuración en memoria NV");	
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
    if(!_network){
        DEBUG_TRACE("\r\n[MQNetBridge]... Levantando interfaz de red");
        _network = easy_connect(true);
    }
    else if(_network->get_ip_address() == NULL){
        DEBUG_TRACE("\r\n[MQNetBridge]... Levantando interfaz de red");
        _network = easy_connect(true);
    }
    if(!_network){
        DEBUG_TRACE("\r\n[MQNetBridge]... ERR_IFUP, no se ha podido conectar a la red wifi");
        _stat = WifiError;
        return -1;
    }
   
    // Prepara socket tcp...
    if(!_net){        
        _net = new MQTTNetwork(_network);
        MBED_ASSERT(_net);
        _net->set_blocking(true);
    }
         
    // Prepara cliente mqtt...
    if(!_client){        
        _client = new MQTT::Client<MQTTNetwork, Countdown>(*_net);
        MBED_ASSERT(_client);
    }
           
    // Abre socket tcp...
    if((rc = _net->connect(_cfg.host, _cfg.port)) != 0){
        DEBUG_TRACE("\r\n[MQNetBridge]... ERR_CONN, al conectar el socket");
        _stat = SockError;
        return rc;
    }
    DEBUG_TRACE("\r\n[MQNetBridge]... SOCK_CONN, socket conectado");    
    
    // Conecta cliente MQTT...
    DEBUG_TRACE("\r\n[MQNetBridge]... Conectando cliente MQTT");
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    if(strcmp(_cfg.clientId, " ") != 0){
        data.clientID.cstring = _cfg.clientId;
    }
    if(strcmp(_cfg.userName, " ") != 0){
        data.username.cstring = _cfg.userName;
    }
    if(strcmp(_cfg.userPass, " ") != 0){
        data.password.cstring = _cfg.userPass;
    }

    if((rc = _client->connect(data)) != 0){
        DEBUG_TRACE("\r\n[MQNetBridge]... ERR_MQTT al conectar con el broker MQTT");
        _stat = MqttError;
        return rc;
    }    
    DEBUG_TRACE("\r\n[MQNetBridge]... CONN_OK!");
    _stat = Connected;
    return 0;
}


//------------------------------------------------------------------------------------
void MQNetBridge::disconnect(){
    // desconecta si estuviera conectado
    // cierra conexión mqtt...    
    if(_client && _client->isConnected()){
		DEBUG_TRACE("\r\n[MQNetBridge]... Deteniendo  cliente mqtt");
        _client->disconnect();
    }
    // cierra el socket tcp...
    if(_net){
		DEBUG_TRACE("\r\n[MQNetBridge]... Cerrando socket");
        _net->disconnect();                
    }
    // finaliza conexión wifi...
    if(_network){
		DEBUG_TRACE("\r\n[MQNetBridge]... Deteniendo interfaz de red");
        _network->disconnect();
        _network = NULL;
    }
    _stat = Disconnected;
}


//------------------------------------------------------------------------------------
int MQNetBridge::reconnect(){    
    // si el interfaz wifi está caído, inicia conexión completa
    if(!_network){
        disconnect();
        return connect();
    }
    if(_network->get_ip_address() == NULL){
        disconnect();
       return connect();     
    }
    
    // en este punto el interfaz wifi está levantado
    // si el cliente ha fallado, reinicia el socket y el cliente mqtt
    if(!_client && !_net){
        return connect();
    }
    if(!_client && _net){
        _net->disconnect();
        return connect();
    }
    if(!_client->isConnected()){
        _net->disconnect();
        return connect();
    }

	DEBUG_TRACE("\r\n[MQNetBridge]... No hace ninguna acción, ya está todo levantado.");
    return 0;
}

//---------------------------------------------------------------------------------
void MQNetBridge::mqttThread(){    
    DEBUG_TRACE("\r\n[MQNetBridge]... Thread mqtt iniciado:");

	// marca thread como iniciado
    _stat = Ready;
    
    for(;;){
    
        // si ha llegado a este punto, inicia la conexión de forma reiterada hasta que lo consiga.
        while(reconnect() != 0){
            DEBUG_TRACE("\r\n[MQNetBridge]... ERR_MQTT_CONN, error conectando cliente MQTT, reintentando en 5seg.");
            Thread::wait(5000);
        }
        
        // una vez que la conexión está establecida, se suscribe a $BASE/+/+/cmd
        char* topic = (char*)Heap::memAlloc(strlen(_cfg.remoteBaseTopic) + strlen("/+/+/cmd") + 1);
        MBED_ASSERT(topic);
        sprintf(topic, "%s/+/+/cmd", _cfg.remoteBaseTopic);
        int rc;
        if ((rc =_client->subscribe(topic, MQTT::QOS0, MqttEventHandler)) != 0){
            DEBUG_TRACE("\r\n[MQNetBridge]... ERR_SUBSC %d al suscribirse al topic '%s'", rc, topic);
        }
        else{
            DEBUG_TRACE("\r\n[MQNetBridge]... SUBSC_OK, suscrito al topic '%s'", topic);
        }    
        Heap::memFree(topic);
        
        DEBUG_TRACE("\r\n[MQNetBridge]... Thread mqtt esperando eventos...");
		MBED_ASSERT(_client);
        while(_client->isConnected()){       
            _client->yield(_cfg.pollDelay);
        }
    }
}

