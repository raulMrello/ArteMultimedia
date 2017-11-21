/*
 * MQNetBridge.h
 *
 *  Created on: Nov 2017
 *      Author: raulMrello
 *
 *  MQNetBridge es el módulo C++ que proporciona un puente de comunicaciones MQ a través de un enlace de red MQTT.
 *  Este módulo recibirá publicaciones a través del protocolo MQTT y las insertará como si las hubiera publicado su propio
 *  cliente. Por otro lado, los topics a los que esté suscrito, los replicará hacia el enlace mqtt.
 *
 *  Por defecto este módulo se registra en MQLib escuchando en el topic "mqnetbridge", de forma que pueda ser configurado.
 *  Las configuraciones básicas que permite son las siguientes:
 *
 *  "mqnetbridge/cmd/wifisetup Cli,Usr,UsrPass,Host,Port,Essid,Passwd" Permite configurar la conexión al servidor MQTT remoto si no se ha realizado
 *  durante la puesta en marcha, por medio de una red wifi. Inicia el proceso de conexión
 *
 *  "mqnetbridge/cmd/localsub TOPIC" Permite suscribirse al topic local (MQLib) TOPIC y redirigir las actualizaciones 
 *  recibidas al mismo topic pero vía MQTT.
 *
 *  "mqnetbridge/cmd/remotesub TOPIC" Permite suscribirse al topic remoto (MQTT) TOPIC y redirigir las actualizaciones 
 *  recibidas al mismo topic pero vía MQLib para que otros módulos internos puedan hacer uso de ellas.
 * 
 *  "mqnetbridge/cmd/remoteuns TOPIC" Permite quitar la suscribirse al topic remoto (MQTT) TOPIC.
 * 
 *  "mqnetbridge/cmd/disc 0" Permite desconectarse
 */
 
 
#ifndef _MQNETBRIDGE_H
#define _MQNETBRIDGE_H


#include "mbed.h"
#include "Logger.h"
#include "MQLib.h"
#include "MQTTNetwork.h"
#include "MQTTmbed.h"
#include "MQTTClient.h"
  
  
  
//---------------------------------------------------------------------------------
//- class MQNetBridge ----------------------------------------------------------
//---------------------------------------------------------------------------------


class MQNetBridge {

public:
        
    enum Status{
        Unknown,        /// No iniciado
        Ready,          /// Iniciado pero sin configurar la conexión
        Disconnected,   /// Configurado pero no conectado
        Connected,      /// Configurado y conectado
    };
    
    /** MQNetBridge()
     *  Crea el objeto asignando un puerto serie para la interfaz con el equipo digital
     *  @param base_topic Topic base, utilizado para poder ser configurado
     */
    MQNetBridge(const char* base_topic = "mqnetbridge");
    
  
	/** setDebugChannel()
     *  Instala canal de depuración
     *  @param dbg Logger
     */
    void setDebugChannel(Logger* dbg){ _debug = dbg; }
    
                
	/** notifyRmoteSubscription()
     *  Callback invocada de forma estática al recibir una actualización de un topic remoto al que está suscrito
     *  @param md Referencia del mensaje recibido
     */    
    void notifyRemoteSubscription(MQTT::MessageData& md);


    /** getStatus()
     *  Obtiene el estado del módulo
     *  @return Estado del módulo
     */
    Status getStatus() { return _stat; }
    
      
protected:

    /** Número de items para la cola de mensajes entrantes */
    static const uint8_t MaxQueueEntries = 8;
    static const uint8_t RetriesOnError = 3;

    /** Flags de tarea (asociados a la máquina de estados) */
    enum SigEventFlags{
        RemoteTopicUpdateSignal = (1<<0),
        LocalTopicUpdateSignal  = (1<<1),
        ConnectionReqSignal     = (1<<2),
    };
      
    
    Thread  _th;                                    /// Manejador del thread
    Status  _stat;                                  /// Estado del módulo
    Logger* _debug;                                 /// Canal de depuración
    
    NetworkInterface* _network;
    MQTTNetwork *_net;                              /// Conexión MQTT
    MQTT::Client<MQTTNetwork, Countdown> *_client;  /// Cliente MQTT
    MQTTPacket_connectData _data;
         
    MQ::SubscribeCallback   _subscriptionCb;        /// Callback de suscripción a topics
    MQ::PublishCallback     _publicationCb;         /// Callback de publicación en topics
   
    
    char* _base_topic;                              /// Topic de configuración    
    char* _client_id;                               /// Ciente id MQTT
    char* _user;                                    /// Nombre de usuario MQTT
    char* _userpass;                                /// Clave usuario MQTT
    char* _host;                                    /// Servidor MQTT
    int   _port;                                    /// Puerto MQTT
    char* _essid;                                   /// Red wifi
    char* _passwd;                                  /// Clave wifi
    char* _gw;                                      /// Gateway IP

    /** task()
     *  Hilo de ejecución asociado para el procesado de las comunicaciones serie
     */
    void task(); 

                
	/** localSubscriptionCb()
     *  Callback invocada al recibir una actualización de un topic local al que está suscrito
     *  @param topic Identificador del topic
     *  @param msg Mensaje recibido
     *  @param msg_len Tamaño del mensaje
     */    
    void localSubscriptionCb(const char* topic, void* msg, uint16_t msg_len);
    

	/** localPublicationCb()
     *  Callback invocada al finalizar una publicación local
     *  @param topic Identificador del topic
     *  @param result Resultado de la publicación
     */    
     void localPublicationCb(const char* topic, int32_t result);
    

	/** connect()
     *  Inicia el interfaz de red wifi, conecta socket tcp y conecta client mqtt
     *  @return Código de error, o 0 si Success.
     */    
     int connect();
        

	/** disconnect()
     *  Desconecta el cliente mqtt, cierra el socket mqtt y finaliza el interfaz de red wifi
     */    
     void disconnect();
    

	/** reconnect()
     *  Desconecta y vuelve a conectar 
     *  @return Código de error, o 0 si Success.
     */    
     int reconnect(){ disconnect(); return(connect()); }


};

#endif  /** _MQNETBRIDGE_H */

