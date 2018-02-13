/*
 * RGBGame.h
 *
 *  Created on: Ene 2018
 *      Author: raulMrello
 *
 *	RGBGame es el módulo activo encargado de gestionar el juego RGB. Consta de los siguientes estados:
 *
 *	stInit: 	Realiza un efecto wave en color azul y conmuta a stWait.
 *
 *	stWait: 	En 10s sin eventos, apaga leds y servos.
 *				En Touch conmuta a stConfig
 *
 *	stConfig: 	En Entry hace un efecto wave con la configuración por defecto y publica $BASE/value/stat siendo $BASE=xrinst/rgbgame 
 *                el mensaje json: "{\"redMin\":R,\"greenMin\":G,\"blueMin\":B,\"redMax\":R,\"greenMax\":G,\"blueMax\":B,\"delay\":MS}"
 *				  para que la app ajuste el tipo de pompa a generar y actualice la simulación.
 *			  	En 30s sin eventos vuelve a stWait.
 *			  	En Touch cambia la configuración y vuelve a reentrar
 *			  	En 10s sin eventos (tras touch) conmuta a stGame
 *
 *	stGame:		En Entry arranca el efecto en bucle
 *				En 30s sin eventos vuelve a stWait
 *				En Touch vuelve a stConfig
 *				En EfectStart publica mensaje en $BASE/value/stat siendo $BASE=xrinst/rgbgame -m[NULL] para que la app simule la generación de pompa.
 *
 *  Los eventos Touch los recibe mediante suscripción al topic xrinst/rgbgame/elec/stat, el mensaje recibido es un stream 'ELEC,1' o 'ELEC,0'
 */
 
#ifndef __RGBGame__H
#define __RGBGame__H

#include "mbed.h"
#include "ActiveModule.h"
#include "WS281xLedStrip.h"
#include "PCA9685_ServoDrv.h"


   
class RGBGame : public ActiveModule {
  public:
              
    /** Constructor por defecto
     * 	@param ld Led driver
     * 	@param sd Servo driver
     * 	@param fs Objeto FSManager para operaciones de backup
     * 	@param defdbg Flag para habilitar depuración por defecto
     */
    RGBGame(WS281xLedStrip* ls, PCA9685_ServoDrv* sd, FSManager* fs, bool defdbg = false);


    /** Destructor
     */
    virtual ~RGBGame(){}
        
    /** Se suscribe a los eventos Touch
     *  @param touch_topic topic de publicación de eventos touch
     */
    void subscribeToTouchTopics(const char* touch_topic){
        _touch_topic = touch_topic;
        MQ::MQClient::subscribe(_touch_topic, &_subscriptionCb);
    }

    
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
     
    /** Número de leds en la tira */
    static const uint32_t NumLeds = 54;
     
    /** Número de servos */
    static const uint32_t NumServos = 3;
     
    /** Número de pasos para completar un efecto */
    static const uint32_t NumSteps = NumLeds/NumServos;
 
    /** Máximo número de mensajes alojables en la cola asociada a la máquina de estados */
    static const uint32_t MaxQueueMessages = 16;
 
    /** Define un número de ciclos infinito */
    static const int InfiniteCycles = 1000000;


    /** Flags de operaciones a realizar por la tarea */
    enum MsgEventFlags{
    	TouchPressEvt 	= (State::EV_RESERVED_USER << 0),  /// Flag al recibir un evento touch-press
    	TouchReleaseEvt = (State::EV_RESERVED_USER << 1),  /// Flag al recibir un evento touch-release
        EffectCycleEvt  = (State::EV_RESERVED_USER << 2),  /// Flag al iniciar un ciclo de effecto
    };

    /** Cola de mensajes de la máquina de estados */
    Queue<State::Msg, MaxQueueMessages> _queue;

    /** Datos de configuración */
	struct Config {
        WS281xLedStrip::Color_t colorMin;
        WS281xLedStrip::Color_t colorMax;
        uint8_t degMin;
        uint8_t degMax;
		uint32_t delayMs;
	};
	Config _cfg;

    /** Flags de estado */
    enum Flags{
        FlagDoServos    = (1 << 0), //!< Flag para habilitar servos
        FlagDoLedStart  = (1 << 1), //!< Flaga para habilitar señales led
        FlagUpdate      = (1 << 2), //!< Flag para notificar cambio en los parámetros del efecto
        FlagCancel      = (1 << 3), //!< Flag para cancelar un efecto en curso
        FlagFinished    = (1 << 4), //!< Flag para notificar que el efecto ha terminado
        FlagTouched     = (1 << 5), //!< Flag para notificar que se ha producido un evento touch-press
    };
    
    /** Variables de estado */
	struct Status {
        WS281xLedStrip::Color_t colorMinNext;
        WS281xLedStrip::Color_t colorMaxNext;
        WS281xLedStrip::Color_t colorMin;
        WS281xLedStrip::Color_t colorMax;
        int cycles;
        Flags flags;
        Mutex mtx;
	};
	Status _stat;
    
    /** Estado de la tira de leds */
    WS281xLedStrip::Color_t _strip[NumLeds];
    
    /** Estado de los servos */
    uint8_t _angles[NumSteps];
    
	/** Temporización (millis) del evento TIMED y variable de timeout acumulado para salir del estado por timeout */
	uint32_t _timeout;
	int32_t _acc_timeout;
	
	/** Gestor de la tira led */
	WS281xLedStrip* _ls;
	
	/** Gestor de los servos */
	PCA9685_ServoDrv* _sd;
    
    /** Topic para recibir actualizaciones de eventos Touch */
    const char* _touch_topic;

    /** Flags para el hilo de efectos */
    enum EffectsEventFlags{
    	WaveEffectEvt 	= (1 << 0),  /// Flag para iniciar un evento wave
    	SwitchOffEffectEvt  = (1 << 1),  /// Flag para apagar todo
    };

    /** Hilo de ejecución paralelo para la generación de efectos */
    Thread _th_effect;
    
    /** Manejadores de estados */
    State* _stWait;
    State::StateResult Wait_EventHandler(State::StateEvent* se);
    State* _stConfig;
    State::StateResult Config_EventHandler(State::StateEvent* se);
    State* _stGame;
    State::StateResult Game_EventHandler(State::StateEvent* se);
    
    
    /** Inicia un nuevo efecto, deteniendo el actual si lo hubiera.
     *  @param effect Efecto a generar
     */
    void startEffect(EffectsEventFlags effect);

    
    /** Genera un efecto en onda con la configuración establecida en _stat.
     */
    void waveEffect();


    /** Apaga los leds y los servos
     */
    void switchOff();
    
    /** Actualiza la configuración en función del electrodo tocado
     *  @param elec Electrodo tocado
     */
    void updateEffectConfig(uint8_t elec);
    
    /** Hilo paralelo para la ejecución de efectos led,servo
     */
    void effectsThread();
    
    
    /** Publica configuración de estado
     */
    void publishConfig();
    
    
    /** Publica estado del efecto
     */
    void publishStat();
    
};
     
#endif /*__RGBGame__H */

/**** END OF FILE ****/


