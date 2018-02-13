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
 *  Por defecto este módulo se registra en MQLib escuchando en el topic base $(base)="mqnetbridge", de forma que 
 *  pueda ser configurado. Las configuraciones básicas que permite son las siguientes:
 * 
 *  A través del enlace MQTT escuchará en $BASE/+/cmd. Lo que llegue al topic $BASE/fwd/cmd será retransmitido a la red local.
 *  Para ello el mensaje será una dupla "topic,msg", así para notificar el mensaje sys/cfg/cmd "1" desde un cliente MQTT remoto
 *  se hará ej: mosquitto_pub -t /xrinst/mqnetbridge/fwd/cmd -m "sys/cfg/cmd,1".
 *
 *  CONECTAR
 *  $(base)conn Cli,Usr,UsrPass,Host,Port,Essid,Passwd" 
 *      Solicita la conexión, configurando los parámetros necesarios.
 * 
 *  DESCONECTAR
 *  $(base)/disc/cmd 0" 
 *      Solicita la desconexión
 * 
 *  SUSCRIPCION LOCAL (MQLIB)
 *  $(base)/lsub/cmd TOPIC" 
 *      Permite suscribirse al topic local (MQLib) TOPIC y redirigir las actualizaciones recibidas al mismo topic mqtt.
 */
 
 
#ifndef _MQNETBRIDGE_H
#define _MQNETBRIDGE_H


#include "mbed.h"
#include "MQLib.h"
#include "MQTTNetwork.h"
#include "MQTTmbed.h"
#include "MQTTClient.h"
#include "ActiveModule.h"
  
  
  
//---------------------------------------------------------------------------------
//- class MQNetBridge ----------------------------------------------------------
//---------------------------------------------------------------------------------


class MQNetBridge : public ActiveModule {

public:
        
    enum Status{
        Unknown,        /// No iniciado
        Ready,          /// Iniciado pero sin configurar la conexión
        Disconnected,   /// Configurado pero no conectado
        Connected,      /// Configurado y conectado
        WifiError,      /// Error al conectar con la red wifi
        SockError,      /// Error al abrir el socket con el servidor mqtt
        MqttError,      /// Error al conectar el cliente mqtt
    };
    
    /** Crea el objeto asignando un puerto serie para la interfaz con el equipo digital
	 *
     *  @param base_topic Topic base, utilizado para poder ser configurado
     * 	@param fs Objeto FSManager para operaciones de backup
     * 	@param defdbg Flag para habilitar depuración por defecto
     * 	@param mqtt_yield_millis Polling de espera de eventos mqtt
     */
    MQNetBridge(const char* base_topic, FSManager* fs, bool defdbg = false, uint32_t mqtt_yield_millis = 1000);


    /** Destructor
     */
    virtual ~MQNetBridge(){}
    
		
	/** Añade suscripción local para hacer bridge MQTT
     *	@param topic Topic bridge local -> mqtt
     */
	void addBridgeTopic(const char* topic);
		
                
	/** Callback para procesar eventos mqtt 
	 *
     *  @param md Referencia del mensaje recibido
     */    
    void mqttEventHandler(MQTT::MessageData& md);


    /** Obtiene el estado del módulo
	 *
     *  @return Estado del módulo
     */
    Status getStatus() { return _stat; }

    
  protected:

    /** Interfaz para postear un mensaje de la máquina de estados en el Mailbox de la clase heredera
     *  @param msg Mensaje a postear
     */
    virtual void putMessage(State::Msg *msg);


    /** Interfaz para obtener un evento osEvent de la clase heredera
     *  @param msg Mensaje a postear
     */
    virtual osEvent getOsEvent();


 	/** Interfaz para manejar los eventos en la máquina de estados por defecto
      *  @param se Evento a manejar
      *  @return State::StateResult Resultado del manejo del evento
      */
    virtual State::StateResult Init_EventHandler(State::StateEvent* se);


 	/** Callback invocada al recibir una actualización de un topic local al que está suscrito
      *  @param topic Identificador del topic
      *  @param msg Mensaje recibido
      *  @param msg_len Tamaño del mensaje
      */
    virtual void subscriptionCb(const char* topic, void* msg, uint16_t msg_len);


 	/** Callback invocada al finalizar una publicación local
      *  @param topic Identificador del topic
      *  @param result Resultado de la publicación
      */
    virtual void publicationCb(const char* topic, int32_t result);


   	/** Chequea la integridad de los datos de configuración <_cfg>. En caso de que algo no sea
   	 * 	coherente, restaura a los valores por defecto y graba en memoria NV.
   	 * 	@return True si la integridad es correcta, False si es incorrecta
	 */
	virtual bool checkIntegrity();


   	/** Establece la configuración por defecto grabándola en memoria NV
	 */
	virtual void setDefaultConfig();


   	/** Recupera la configuración de memoria NV
	 */
	virtual void restoreConfig();


   	/** Graba la configuración en memoria NV
	 */
	virtual void saveConfig();


	/** Graba un parámetro en la memoria NV
	 * 	@param param_id Identificador del parámetro
	 * 	@param data Datos asociados
	 * 	@param size Tamaño de los datos
	 * 	@param type Tipo de los datos
	 * 	@return True: éxito, False: no se pudo recuperar
	 */
	virtual bool saveParameter(const char* param_id, void* data, size_t size, NVSInterface::KeyValueType type){
		return ActiveModule::saveParameter(param_id, data, size, type);
	}


	/** Recupera un parámetro de la memoria NV
	 * 	@param param_id Identificador del parámetro
	 * 	@param data Receptor de los datos asociados
	 * 	@param size Tamaño de los datos a recibir
	 * 	@param type Tipo de los datos
	 * 	@return True: éxito, False: no se pudo recuperar
	 */
	virtual bool restoreParameter(const char* param_id, void* data, size_t size, NVSInterface::KeyValueType type){
		return ActiveModule::restoreParameter(param_id, data, size, type);
	}        
      
private:
 
    /** Máximo número de mensajes alojables en la cola asociada a la máquina de estados */
    static const uint32_t MaxQueueMessages = 16;

	/** Número de intentos tras operación con error */
    static const uint8_t RetriesOnError = 3;

    /** Flags de operaciones a realizar por la tarea */
    enum MsgEventFlags{
    	ConnReqEvt = (State::EV_RESERVED_USER << 0),  /// Flag al solicitar conexión
    	ConnAckEvt = (State::EV_RESERVED_USER << 1),  /// Flag al confirmar conexión
        SubAckEvt  = (State::EV_RESERVED_USER << 2),  /// Flag al confirmar suscripción
        PubAckEvt  = (State::EV_RESERVED_USER << 3),  /// Flag al confirmar publicación
		DiscReqEvt = (State::EV_RESERVED_USER << 4),  /// Flag al solicitar desconexión
    };

    /** Cola de mensajes de la máquina de estados */
    Queue<State::Msg, MaxQueueMessages> _queue;

    /** Datos de configuración */
	struct Config {
		const char* baseTopic;  /// Topic base de escucha
		char* clientId;       	/// Ciente id MQTT
		char* userName;       	/// Nombre de usuario MQTT
		char* userPass;       	/// Clave usuario MQTT
		char* host;           	/// Servidor MQTT
		int   port;         	/// Puerto MQTT
		char* essid;          	/// Red wifi
		char* passwd;         	/// Clave wifi
		char* gw;             	/// Gateway IP
		uint32_t pollDelay; 	/// Tiemout para mqtt yield
	};
	Config _cfg;
	
	/** flags de estado de la conexión mqtt */
	Status _stat;

	/** Manejador del interfaz de red */
    NetworkInterface* _network;
	
	/** Manejador del socket */
    MQTTNetwork *_net;                            
	
	/** Manejador del cliente MQTT */
    MQTT::Client<MQTTNetwork, Countdown> *_client; 
	
	/** Datos de conexión mqtt */
    MQTTPacket_connectData _data;	

    /** Hilo de ejecución paralelo para la gestión de las comunicaciones mqtt */
    Thread _th_mqtt;
	   
    /** Hilo paralelo para la ejecución de las comunicaciones mqtt
     */
    void mqttThread();
         

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
     int reconnect(){ 
		disconnect(); 
		return(connect()); 
	 }


};

#endif  /** _MQNETBRIDGE_H */

