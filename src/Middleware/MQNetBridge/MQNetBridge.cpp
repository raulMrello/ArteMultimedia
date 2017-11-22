/*
 * MQNetBridge.cpp
 *
 *  Created on: Nov 2017
 *      Author: raulMrello
 */


#include "MQNetBridge.h"
#include "easy-connect.h"

//------------------------------------------------------------------------------------
//- STATIC ---------------------------------------------------------------------------
//------------------------------------------------------------------------------------

#define DEBUG_TRACE(format, ...)    if(_debug){_debug->printf(format, ##__VA_ARGS__);}

char* MBED_CONF_APP_WIFI_SSID = 0;      // Requerido en easy-connect
char* MBED_CONF_APP_WIFI_PASSWORD = 0;  // Requerido en easy-connect

static MQNetBridge* gThis=0;
static void remoteSubscriptionCb(MQTT::MessageData& md){
    if(gThis){
        gThis->notifyRemoteSubscription(md);
    }
}

    
//------------------------------------------------------------------------------------
//-- PUBLIC METHODS IMPLEMENTATION ---------------------------------------------------
//------------------------------------------------------------------------------------


MQNetBridge::MQNetBridge(const char* base_topic) { 
    _stat = Unknown;
    gThis = this;
    _client_id = 0;
    _user = 0;
    _userpass = 0;
    _host = 0;
    _port = 0;
    _essid = 0;
    _passwd = 0;
    _gw = 0;
    _network = 0;
    _net = 0;
    _client = 0;    
    
    _base_topic = (char*)Heap::memAlloc(strlen(base_topic)+1);
    if(_base_topic){
        strcpy(_base_topic, base_topic);

        // Carga callbacks est�ticas de publicaci�n/suscripci�n
        _subscriptionCb = callback(this, &MQNetBridge::localSubscriptionCb);
        _publicationCb = callback(this, &MQNetBridge::localPublicationCb);   
        
        // Inicializa par�metros del hilo de ejecuci�n propio
        _th.start(callback(this, &MQNetBridge::task));
    }
}
 


//------------------------------------------------------------------------------------
void MQNetBridge::notifyRemoteSubscription(MQTT::MessageData& md){
    MQTT::Message &message = md.message;
    MQTTString &topicName = md.topicName;
//    DEBUG_TRACE("Message arrived: qos %d, retained %d, dup %d, packetid %d\r\n", message.qos, message.retained, message.dup, message.id);
//    DEBUG_TRACE("Topic %.*s\r\n", topicName.lenstring, topicName.cstring);
//    DEBUG_TRACE("Payload %.*s\r\n", message.payloadlen, (char*)message.payload);
    char* topic = (char*)Heap::memAlloc(topicName.lenstring.len + 1);
    if(topic){
        strncpy(topic, topicName.cstring, topicName.lenstring.len);
        MQ::MQClient::publish(topic, (char*)message.payload, message.payloadlen, &_publicationCb);
        Heap::memFree(topic);
    }    
}
    
//------------------------------------------------------------------------------------
//-- PROTECTED METHODS IMPLEMENTATION ------------------------------------------------
//------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------
void MQNetBridge::task(){    
    
    // se suscribe a los topics de configuraci�n
    char* cfg_topics = (char*)Heap::memAlloc(strlen(_base_topic) + strlen("/#")+1);
    if(cfg_topics){
        sprintf(cfg_topics, "%s/#", _base_topic);
        MQ::MQClient::subscribe(cfg_topics, &_subscriptionCb);
    }
    
    _stat = Ready;
    

    for(;;){       
        DEBUG_TRACE("\r\nNetBridge: Wating events... ");
        // procesa solicitudes pendientes
        osEvent evt = _queue.get(osWaitForever);
        
//        // Cada 100ms, si est� conectado, mantiene la conexi�n activa
//        if(evt.status == osOK || osEventTimeout){
//            if(_client && _client->isConnected()){
//                DEBUG_TRACE("\r\nNetBridge: Yielding... ");
//                _client->yield(100); 
//            }
//            else{
//                Thread::yield();
//            }
//        }
        
        // Si hay que procesar un mensaje...
        if(evt.status == osEventMessage){
            RequestOperation_t* msg = (RequestOperation_t*)evt.value.p;
            switch(msg->id){
                               
                // Si hay que conectar...
                case ConnectSig:{
                    DEBUG_TRACE("\r\nNetBridge: Conectando... ");
                    reconnect();
                    break;
                }             
            
                
                // Si hay que desconectar...
                case DisconnectSig:{
                    // asegura que est� conectado...
                    DEBUG_TRACE("\r\nNetBridge: Desconectando... ");
                    disconnect();                                
                    break;
                }             
                
                
//                // Si hay que suscribirse a un topic...
//                case RemoteSubscriptionSig:{
//                    DEBUG_TRACE("\r\nNetBridge: Suscribi�ndose a %s ... ", msg->data);
//                    if (_client->subscribe(msg->data, MQTT::QOS0, remoteSubscriptionCb) != 0){
//                        DEBUG_TRACE("ERROR");
//                    }
//                    else{
//                        DEBUG_TRACE("OK!!!!");
//                    }             
//                    break;
//                }  
//                
//                
//                // Si hay que quitar la suscripci�n a un topic...
//                case RemoteUnsubscriptionSig:{
//                    DEBUG_TRACE("\r\nNetBridge: Quitando suscripci�n a %s ... ", msg->data);  
//                    if(_client->unsubscribe(msg->data) != 0){
//                        DEBUG_TRACE("ERROR");
//                    }
//                    else{
//                        DEBUG_TRACE("OK!!!!");
//                    }           
//                    break;
//                }         
                
                
                // Si hay que publicar en un topic... 
                case RemotePublishSig:
                case RemoteSubscriptionSig:
                case RemoteUnsubscriptionSig:{
                    DEBUG_TRACE("OK!!!! \r\nYielding...");
                    _client->yield(100);                            

//                    char* topic = strtok(msg->data, ",");
//                    char* msend = strtok(0, ",");                    
//                    if(topic && msend){
//                        MQTT::Message message;
//                        message.qos = MQTT::QOS0;
//                        message.retained = false;
//                        message.dup = false;
//                        message.payload = msend;
//                        message.payloadlen = strlen(msend) + 1;
//                        uint8_t err = 0;
//                        DEBUG_TRACE("\r\nNetBridge: Publicando en topic %s ... ", topic);
//                        if(_client->publish(topic, message) != 0){            
//                            DEBUG_TRACE("ERROR=%d", err); 
//                        }
//                        else{
//                            DEBUG_TRACE("OK!!!! \r\nYielding...");
//                            _client->yield(100);                            
//                        }
//                    }                   
                    break;
                }             
             }
            
            // Elimina la memoria asignada al mensaje
            if(msg->data){
                Heap::memFree(msg->data);
            }
            Heap::memFree(msg);
        }    
        
        // en caso de que haya habido un error y se haya cerrado la conexi�n, habr� que intentar reconectar
        if(_client && _stat == Connected && !_client->isConnected()){
            DEBUG_TRACE("\r\nNetBridge: Iniciando reconexi�n...");
            reconnect(); 
        }
    }
}


//------------------------------------------------------------------------------------
void MQNetBridge::localSubscriptionCb(const char* topic, void* msg, uint16_t msg_len){
    // en primer lugar chequea qu� tipo de mensaje es
    
    // si es un mensaje para establecer la conexi�n con el servidor mqtt...
    if(strstr(topic, "cmd/conn") != 0){
        DEBUG_TRACE("\r\nNetBridge: Solicitando conexi�n...");
        // lee los par�metros esperados: ClientId,User,UserPass,Host,Port,ESSID, WifiPasswd
        char* cli = strtok((char*)msg, ",");
        char* usr = strtok(0, ",");
        char* usrpass = strtok(0, ",");
        char* host = strtok(0, ",");
        char* port = strtok(0, ",");
        char* essid = strtok(0, ",");
        char* passwd = strtok(0, ",");
        // si se leen correctamente...
        if(cli && usr && usrpass && host && port && essid && passwd){
            // reserva espacio para el mensaje
            RequestOperation_t* op = (RequestOperation_t*)Heap::memAlloc(sizeof(RequestOperation_t));
            if(!op){
                return;
            }
            
            // desecha los antiguos...
            if(_client_id){
                Heap::memFree(_client_id);
            }
            if(_user){
                Heap::memFree(_user);
            }
            if(_userpass){
                Heap::memFree(_userpass);
            }
            if(_host){
                Heap::memFree(_host);
            }
            if(_essid){
                Heap::memFree(_essid);
            }
            if(_passwd){
                Heap::memFree(_passwd);
            }
            // actualiza...
            _client_id = (char*)Heap::memAlloc(strlen(cli)+1);
            strcpy(_client_id, cli);
            _user = (char*)Heap::memAlloc(strlen(usr)+1);
            strcpy(_user, usr);
            _userpass = (char*)Heap::memAlloc(strlen(usrpass)+1);
            strcpy(_userpass, usrpass);
            _host = (char*)Heap::memAlloc(strlen(host)+1);
            strcpy(_host, host);
            _port = atoi(port);
            _essid = (char*)Heap::memAlloc(strlen(essid)+1);
            strcpy(_essid, essid);
            _passwd = (char*)Heap::memAlloc(strlen(passwd)+1);
            strcpy(_passwd, passwd);
            // ajusta par�metros de conexi�n para easy-connect de mbed...
            MBED_CONF_APP_WIFI_SSID = _essid;
            MBED_CONF_APP_WIFI_PASSWORD = _passwd;

            DEBUG_TRACE("\r\nNetBridge: Conectando... ");
            reconnect();
//            
//            DEBUG_TRACE("\r\nNetBridge: Conexi�n solicitada...");              
//            op->data = 0;
//            op->id = ConnectSig;
//            _queue.put(op);  
        }
        return;
    }    
        
    // si es un mensaje para desconectar...
    if(strstr(topic, "cmd/disc") != 0){
        DEBUG_TRACE("\r\nNetBridge: Desconectando... ");
        disconnect();                                
               
//        RequestOperation_t* op = (RequestOperation_t*)Heap::memAlloc(sizeof(RequestOperation_t));
//        if(!op){
//            return;
//        }
//        DEBUG_TRACE("\r\nNetBridge: Desconexi�n solicitada..."); 
//        op->data = 0;
//        op->id = DisconnectSig;            
//        _queue.put(op); 
//        return;
    }   
    
    // si es un mensaje para suscribirse a un topic local en MQLib...
    if(strstr(topic, "cmd/lsub") != 0){
        char* topic = (char*)Heap::memAlloc(msg_len);
        strcpy(topic, (char*)msg);
        MQ::MQClient::subscribe(topic, &_subscriptionCb);
        DEBUG_TRACE("\r\nNetBridge: Suscripci�n local a %s", topic);
        return;
    }    
    
    // si es un mensaje para suscribirse a un topic remoto en MQTT...
    if(strstr(topic, "cmd/rsub") != 0){
        // s�lo lo permite si est� conectado
        if(_stat == Connected){
            DEBUG_TRACE("\r\nNetBridge: Suscribi�ndose a %s ... ", (char*)msg);
            if (_client->subscribe((char*)msg, MQTT::QOS0, remoteSubscriptionCb) != 0){
                DEBUG_TRACE("ERROR");
            }
            else{
                DEBUG_TRACE("OK!");
            }                   
//            RequestOperation_t* op = (RequestOperation_t*)Heap::memAlloc(sizeof(RequestOperation_t));
//            if(!op){
//                return;
//            }
//            op->data = 0;
//            op->id = RemoteSubscriptionSig;
//            _queue.put(op);    
//            
//            RequestOperation_t* op = (RequestOperation_t*)Heap::memAlloc(sizeof(RequestOperation_t));
//            if(!op){
//                return;
//            }
//            op->data = (char*)Heap::memAlloc(msg_len);
//            if(!op->data){
//                Heap::memFree(op);
//                return;
//            }
//            strcpy(op->data, (char*)msg);
//            op->id = RemoteSubscriptionSig;
//            DEBUG_TRACE("\r\nNetBridge: Suscripci�n remota a %s solicitada...", op->data);  
//            _queue.put(op);                                    
        }
        return;
    }    
    
    // si es un mensaje para cancelar una suscripci�n remota en MQTT...
    if(strstr(topic, "cmd/runs") != 0){
        // asegura que est� conectado
        if(_stat == Connected){
            DEBUG_TRACE("\r\nNetBridge: Quitando suscripci�n a %s ... ", (char*)msg);  
            if(_client->unsubscribe((char*)msg) != 0){
                DEBUG_TRACE("ERROR");
            }
            else{
                DEBUG_TRACE("OK!");
            }                  
//            RequestOperation_t* op = (RequestOperation_t*)Heap::memAlloc(sizeof(RequestOperation_t));
//            if(!op){
//                return;
//            }
//            op->data = 0;
//            op->id = RemoteUnsubscriptionSig;
//            _queue.put(op);    
//            
//            RequestOperation_t* op = (RequestOperation_t*)Heap::memAlloc(sizeof(RequestOperation_t));
//            if(!op){
//                return;
//            }
//            op->data = (char*)Heap::memAlloc(msg_len);
//            if(!op->data){
//                Heap::memFree(op);
//                return;
//            }
//            strcpy(op->data, (char*)msg);
//            op->id = RemoteUnsubscriptionSig;
//            DEBUG_TRACE("\r\nNetBridge: Unsuscripci�n remota a %s solicitada...", op->data);
//            _queue.put(op);                                    
        }
        return;
    }     
    
    // en cualquier otro caso, redirecciona el mensaje local a mqtt siempre que est� conectado        
    DEBUG_TRACE("\r\nNetBridge: Solicitando reenv�o a MQTT topic %s msg %s... ", topic, (char*)msg); 
    if(_stat != Connected){
        DEBUG_TRACE("ERROR. No conectado!"); 
        return;
    }
    MQTT::Message message;
    message.qos = MQTT::QOS0;
    message.retained = false;
    message.dup = false;
    message.payload = (char*)msg;
    message.payloadlen = msg_len;
    uint8_t err = 0;
    DEBUG_TRACE("\r\nNetBridge: Publicando en topic %s ... ", topic);
    if(_client->publish(topic, message) != 0){            
        DEBUG_TRACE("ERROR=%d", err); 
    }
    else{
        DEBUG_TRACE("OK!"); 
    }
    RequestOperation_t* op = (RequestOperation_t*)Heap::memAlloc(sizeof(RequestOperation_t));
    if(!op){
        return;
    }
    op->data = 0;
    op->id = RemotePublishSig;
    _queue.put(op);    
//        
//    RequestOperation_t* op = (RequestOperation_t*)Heap::memAlloc(sizeof(RequestOperation_t));
//    if(!op){
//        return;
//    }
//    op->data = (char*)Heap::memAlloc(msg_len + strlen(topic) + 1);
//    if(!op->data){
//        Heap::memFree(op);
//        return;
//    }
//    sprintf(op->data, "%s,%s", topic, (char*)msg);
//    op->id = RemotePublishSig;
    _queue.put(op);    
}


//------------------------------------------------------------------------------------
void MQNetBridge::localPublicationCb(const char* topic, int32_t result){
    // Hacer algo si es necesario...
}  


//------------------------------------------------------------------------------------
int MQNetBridge::connect(){
    // inicia el proceso de conexi�n...
    // Levanta el interfaz de red wifi
    int rc;
    DEBUG_TRACE("\r\nNetBridge: Levantando red... ");
    if(!_network || _stat == Ready){
        _network = easy_connect(true);
        if(!_network){
            return -1;
        }
    }
    
    DEBUG_TRACE("wifi_OK... ");
    // Prepara socket tcp...
    if(!_net){
        _net = new MQTTNetwork(_network);
    }
    
    // Prepara cliente mqtt...
    if(!_client){
        _client = new MQTT::Client<MQTTNetwork, Countdown>(*_net);
    }
    
    // Abre socket tcp...
    if((rc = _net->connect(_host, _port)) != 0){
        return rc;
    }                
    DEBUG_TRACE("socket_OK... ");
    
    // Conecta cliente MQTT...
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.clientID.cstring = _client_id;
    data.username.cstring = _user;
    data.password.cstring = _userpass;
    if ((rc = _client->connect(data)) != 0){
        return rc;
    }     
    DEBUG_TRACE("mqtt_OK... CONECTADO!");
    _stat = Connected;
    return 0;
}

//------------------------------------------------------------------------------------
void MQNetBridge::disconnect(){
    // desconecta si estuviera conectado
    // cierra conexi�n mqtt...
    DEBUG_TRACE("\r\nNetBridge: Deteniendo red... ");
    if(_client && _client->isConnected()){
        _client->disconnect();
    }
    DEBUG_TRACE("\r\nmqtt cerrado... ");
    // cierra el socket tcp...
    if(_net){
        _net->disconnect();                
    }
    DEBUG_TRACE("socket cerrado... ");
    // finaliza conexi�n wifi...
    if(_network){
        _network->disconnect();
    }
    DEBUG_TRACE("interfaz cerrado. DESCONECTADO!");
    _stat = Ready;
}
