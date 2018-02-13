/*
 * MQNetBridge.h
 *
 *  Created on: Nov 2017
 *      Author: raulMrello
 *
 *  MQNetBridge es el m�dulo C++ que proporciona un puente de comunicaciones MQ a trav�s de un enlace de red MQTT.
 *  Este m�dulo recibir� publicaciones a trav�s del protocolo MQTT y las insertar� como si las hubiera publicado su propio
 *  cliente. Por otro lado, los topics a los que est� suscrito, los replicar� hacia el enlace mqtt.
 *
 *  Por defecto este m�dulo se registra en MQLib escuchando en el topic base $(base)="mqnetbridge", de forma que 
 *  pueda ser configurado. Las configuraciones b�sicas que permite son las siguientes:
 * 
 *  A trav�s del enlace MQTT escuchar� en $BASE/+/cmd. Lo que llegue al topic $BASE/fwd/cmd ser� retransmitido a la red local.
 *  Para ello el mensaje ser� una dupla "topic,msg", as� para notificar el mensaje sys/cfg/cmd "1" desde un cliente MQTT remoto
 *  se har� ej: mosquitto_pub -t /xrinst/mqnetbridge/fwd/cmd -m "sys/cfg/cmd,1".
 *
 *  CONECTAR
 *  $(base)conn Cli,Usr,UsrPass,Host,Port,Essid,Passwd" 
 *      Solicita la conexi�n, configurando los par�metros necesarios.
 * 
 *  DESCONECTAR
 *  $(base)/disc/cmd 0" 
 *      Solicita la desconexi�n
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
        Ready,          /// Iniciado pero sin configurar la conexi�n
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
     * 	@param defdbg Flag para habilitar depuraci�n por defecto
     * 	@param mqtt_yield_millis Polling de espera de eventos mqtt
     */
    MQNetBridge(const char* base_topic, FSManager* fs, bool defdbg = false, uint32_t mqtt_yield_millis = 1000);


    /** Destructor
     */
    virtual ~MQNetBridge(){}
    
		
	/** A�ade suscripci�n local para hacer bridge MQTT
     *	@param topic Topic bridge local -> mqtt
     */
	void addBridgeTopic(const char* topic);
		
                
	/** Callback para procesar eventos mqtt 
	 *
     *  @param md Referencia del mensaje recibido
     */    
    void mqttEventHandler(MQTT::MessageData& md);


    /** Obtiene el estado del m�dulo
	 *
     *  @return Estado del m�dulo
     */
    Status getStatus() { return _stat; }

    
  protected:

    /** Interfaz para postear un mensaje de la m�quina de estados en el Mailbox de la clase heredera
     *  @param msg Mensaje a postear
     */
    virtual void putMessage(State::Msg *msg);


    /** Interfaz para obtener un evento osEvent de la clase heredera
     *  @param msg Mensaje a postear
     */
    virtual osEvent getOsEvent();


 	/** Interfaz para manejar los eventos en la m�quina de estados por defecto
      *  @param se Evento a manejar
      *  @return State::StateResult Resultado del manejo del evento
      */
    virtual State::StateResult Init_EventHandler(State::StateEvent* se);


 	/** Callback invocada al recibir una actualizaci�n de un topic local al que est� suscrito
      *  @param topic Identificador del topic
      *  @param msg Mensaje recibido
      *  @param msg_len Tama�o del mensaje
      */
    virtual void subscriptionCb(const char* topic, void* msg, uint16_t msg_len);


 	/** Callback invocada al finalizar una publicaci�n local
      *  @param topic Identificador del topic
      *  @param result Resultado de la publicaci�n
      */
    virtual void publicationCb(const char* topic, int32_t result);


   	/** Chequea la integridad de los datos de configuraci�n <_cfg>. En caso de que algo no sea
   	 * 	coherente, restaura a los valores por defecto y graba en memoria NV.
   	 * 	@return True si la integridad es correcta, False si es incorrecta
	 */
	virtual bool checkIntegrity();


   	/** Establece la configuraci�n por defecto grab�ndola en memoria NV
	 */
	virtual void setDefaultConfig();


   	/** Recupera la configuraci�n de memoria NV
	 */
	virtual void restoreConfig();


   	/** Graba la configuraci�n en memoria NV
	 */
	virtual void saveConfig();


	/** Graba un par�metro en la memoria NV
	 * 	@param param_id Identificador del par�metro
	 * 	@param data Datos asociados
	 * 	@param size Tama�o de los datos
	 * 	@param type Tipo de los datos
	 * 	@return True: �xito, False: no se pudo recuperar
	 */
	virtual bool saveParameter(const char* param_id, void* data, size_t size, NVSInterface::KeyValueType type){
		return ActiveModule::saveParameter(param_id, data, size, type);
	}


	/** Recupera un par�metro de la memoria NV
	 * 	@param param_id Identificador del par�metro
	 * 	@param data Receptor de los datos asociados
	 * 	@param size Tama�o de los datos a recibir
	 * 	@param type Tipo de los datos
	 * 	@return True: �xito, False: no se pudo recuperar
	 */
	virtual bool restoreParameter(const char* param_id, void* data, size_t size, NVSInterface::KeyValueType type){
		return ActiveModule::restoreParameter(param_id, data, size, type);
	}        
      
private:
 
    /** M�ximo n�mero de mensajes alojables en la cola asociada a la m�quina de estados */
    static const uint32_t MaxQueueMessages = 16;

	/** N�mero de intentos tras operaci�n con error */
    static const uint8_t RetriesOnError = 3;

    /** Flags de operaciones a realizar por la tarea */
    enum MsgEventFlags{
    	ConnReqEvt = (State::EV_RESERVED_USER << 0),  /// Flag al solicitar conexi�n
    	ConnAckEvt = (State::EV_RESERVED_USER << 1),  /// Flag al confirmar conexi�n
        SubAckEvt  = (State::EV_RESERVED_USER << 2),  /// Flag al confirmar suscripci�n
        PubAckEvt  = (State::EV_RESERVED_USER << 3),  /// Flag al confirmar publicaci�n
		DiscReqEvt = (State::EV_RESERVED_USER << 4),  /// Flag al solicitar desconexi�n
    };

    /** Cola de mensajes de la m�quina de estados */
    Queue<State::Msg, MaxQueueMessages> _queue;

    /** Datos de configuraci�n */
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
	
	/** flags de estado de la conexi�n mqtt */
	Status _stat;

	/** Manejador del interfaz de red */
    NetworkInterface* _network;
	
	/** Manejador del socket */
    MQTTNetwork *_net;                            
	
	/** Manejador del cliente MQTT */
    MQTT::Client<MQTTNetwork, Countdown> *_client; 
	
	/** Datos de conexi�n mqtt */
    MQTTPacket_connectData _data;	

    /** Hilo de ejecuci�n paralelo para la gesti�n de las comunicaciones mqtt */
    Thread _th_mqtt;
	   
    /** Hilo paralelo para la ejecuci�n de las comunicaciones mqtt
     */
    void mqttThread();
         

	/** connect()
     *  Inicia el interfaz de red wifi, conecta socket tcp y conecta client mqtt
     *  @return C�digo de error, o 0 si Success.
     */    
     int connect();
        

	/** disconnect()
     *  Desconecta el cliente mqtt, cierra el socket mqtt y finaliza el interfaz de red wifi
     */    
     void disconnect();
    

	/** reconnect()
     *  Desconecta y vuelve a conectar 
     *  @return C�digo de error, o 0 si Success.
     */    
     int reconnect(){ 
		disconnect(); 
		return(connect()); 
	 }


};

#endif  /** _MQNETBRIDGE_H */

