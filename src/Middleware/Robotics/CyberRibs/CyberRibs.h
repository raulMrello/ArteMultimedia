/*
 * CyberRibs.h
 *
 *  Created on: Dic 2017
 *      Author: raulMrello
 *
 *  CyberRibs es un m�dulo C++ que implementa el control de un costillar cibern�tico formado por un n�mero de 
 *  costillas m�viles. Cada costilla consta de un servomotor que abre o cierra el di�metro del costillar y de una serie
 *  de leds que emiten diferentes tonos de luz, de acuerdo al di�metro del costillar y a la velocidad de apertura del
 *  mismo.
 *
 *  CyberRibs proporciona su propia tarea o en su defecto un punto de entrada <job>, para utilizar los subm�dulos:
 *  Driver/WS281xLedStrip (tira de leds) y Driver/PCA9685 (controlador servos).
 *
 *  Utilizar� la librer�a MQLib y esperar� mensajes en su topic base $(base) de esta forma:
 *
 *  MODO
 *  $(base)/mode/cmd M" 
 *      M indica uno de los posibles modos de funcionamiento:
 *      M=0 Estructura apagada, sin leds y sin movimiento. En su estado de reposo. Se desconecta la alimentaci�n de los
 *          servos y de los leds. Se compone de dos estados: StGoingDown, StOff.
 *      M=1..7 Estructura en movimiento y con leds activos. Valores bajos (1,2) implican movimientos lentos y colores
 *          suaves (azulados), mientras que valores altos (6,7) implican movimientos r�pidos y colores intensos (rojos).
 *          Se compone de 1 �nico estado: StWave (se define una forma de onda que afectar� al color y al movimiento).
 *      M=8 Estructura en modo de animaci�n. Se compone del �nico estado StCongratulations que elabora 3 tipos de juego  
 *      de movimientos y luces que se repiten c�clicamente durante un tiempo dado. Cada juego es un subestado del anterior
 *      y se denominan como StCongrat0, StCongrat1, StCongrat2. Los juegos ser�n arm�nicos, r�pidos e intensos.
 *
 *      M=9 Estructura en modo de animaci�n. Se compone del �nico estado StCondonlences que de igual forma elabora 3 tipos de juego  
 *      de movimientos y luces que se repiten c�clicamente durante un tiempo dado. Cada juego es un subestado del anterior
 *      y se denominan como StCondols0, StCondols1, StCondols2. Los juegos ser�n aleatorios, inconexos, lentos y apagados.
 * 
 *  CONFIGURACION
 *  $(base)/cfg/cmd E
 *      Ajusta diferentes par�metros de configuraci�n:
 *      E (enable notifications) Permite activar notificaciones en cada cambio de estado, mediante la publicaci�n en el
 *      topic $(pubbase)/mode M,N siendo M el modo en el que se encuentra y N el subestado. Por ejemplo para notificar
 *      que ha cambiado a modo Congratulations.Congrat2 enviar� el mensaje: "$(pubbase)/mode/stat 8,2"
 *
 *  AJUSTE Y CALIBRACI�N DE LOS SERVOS
  *  $(base)/angle/cmd S,A
 *      Mueve el servo S al �ngulo A (limitado por rangos min,max)
 *
 *  $(base)/duty/cmd S,D
 *      Mueve el servo S al duty D (sin limitaci�n por rango)
 *
 *   $(base)/info/cmd S
 *      Obtiene informaci�n sobre el servo S (rangos min max de �ngulo y duty)
 *
 *   $(base)/cal/cmd S,Ai,Af,Di,Df
 *      Calibra los rangos del servo S, con �ngulo minmax Ai,Af y duty minmax Di,Df.
 *
 *   $(base)/save/cmd 0
 *      Guarda los datos de calibraci�n de todos los servos en NVFlash
 *
 */
 
 
#ifndef _CYBERRIBS_H
#define _CYBERRIBS_H


#include "mbed.h"
#include "Logger.h"
#include "MQLib.h"
#include "NVFlash.h"
#include "WS281xLedStrip.h"
#include "PCA9685_ServoDrv.h"
#include "StateMachine.h"
  
  
  
//---------------------------------------------------------------------------------
//- class CyberRibs ----------------------------------------------------------
//---------------------------------------------------------------------------------


class CyberRibs : public StateMachine {

public:
            
    /** CyberRibs()
     *  Crea el objeto asignando un n�mero de costillas y los pines utilizados por los controladores
     *  @param num_ribs N�mero de costillas
     *  @param leds_per_rib N�mero de leds por costilla
     *  @param sda_srv Pin SDA del bus i2c que controla el driver Servo
     *  @param scl_srv Pin SCL del bus i2c que controla el driver Servo
     *  @param pwm_led Pin PWM del controlador led
     */
    CyberRibs(uint8_t num_ribs, uint8_t leds_per_rib, PinName sda_srv, PinName scl_srv, PinName pwm_led);
    
  
	/** setDebugChannel()
     *  Instala canal de depuraci�n
     *  @param dbg Logger
     */
    void setDebugChannel(Logger* dbg){ _debug = dbg; }

        
    /** Devuelve el estado del controlador
     *  @return Estado del controlador
     */
    bool ready() { return _ready; }   
    
  
	/** setSubscriptionBase()
     *  Registra el topic base a los que se suscribir� el m�dulo
     *  @param sub_topic Topic base para la suscripci�n
     */
    void setSubscriptionBase(const char* sub_topic);          
    
  
	/** setPublicationBase()
     *  Registra el topic base a los que publicar� el m�dulo
     *  @param pub_topic Topic base para la publicaci�n
     */
    void setPublicationBase(const char* pub_topic);  
    
protected:

    /** N�mero de items para la cola de mensajes entrantes */
    static const uint32_t MaxQueueEntries = 8;
    static const uint8_t RetriesOnError = 3;

    /** Flags de tarea (asociados a la m�quina de estados) */
    enum MsgEventFlags{
        ModeUpdateMsg  = (State::EV_RESERVED_USER << 0),  /// Flag para solicitar un cambio de modo
    };
    
    Queue<State::Msg, MaxQueueEntries> _queue;  /// Request queue
    
    Thread  _th;                                /// Manejador del thread
    Logger* _debug;                             /// Canal de depuraci�n
    
    bool    _enable_pub;                        /// Flag para activar publicaciones de cambio de modo
    bool    _ready;                             /// Flag de estado
    uint8_t _num_ribs;                          /// N�mero de costillas
    uint8_t _leds_per_rib;                      /// N�mero de leds por costilla
    char*   _sub_topic;                         /// Topic base para la suscripci�n
    char*   _pub_topic;                         /// Topic base para la publicaci�n
    char*   _pub_topic_unique;                  /// Topic para publicar
   
    PCA9685_ServoDrv* _servod;                  /// Driver servos
    WS281xLedStrip*   _ledd;                    /// Driver leds
             
    MQ::SubscribeCallback   _subscriptionCb;    /// Callback de suscripci�n a topics
    MQ::PublishCallback     _publicationCb;     /// Callback de publicaci�n en topics
      

    /** Estados y manejadores de eventos */
    
    uint8_t _mode;                              /// Identificador del modo
    uint8_t _submode;                           /// Identificador del submodo
    
    // Estados para modo 0     
    State _stGoingDown;
    State::StateResult GoingDown_EventHandler(State::StateEvent* se);
    State _stOff;
    State::StateResult Off_EventHandler(State::StateEvent* se);
    
    // Estados para modos 1 al 7     
    State _stImplosion;
    State::StateResult Implosion_EventHandler(State::StateEvent* se);
    State _stExplosion;
    State::StateResult Explosion_EventHandler(State::StateEvent* se);
    
    // Estados para modo 8     
    State _stCongratulations;
    State::StateResult Congratulations_EventHandler(State::StateEvent* se);
    State _stCongrat0;
    State::StateResult Congrat0_EventHandler(State::StateEvent* se);
    State _stCongrat1;
    State::StateResult Congrat1_EventHandler(State::StateEvent* se);
    State _stCongrat2;
    State::StateResult Congrat2_EventHandler(State::StateEvent* se);
    
    // Estados para modo 9     
    State _stCondolences;
    State::StateResult Condolences_EventHandler(State::StateEvent* se);
    State _stCondols0;
    State::StateResult Condols0_EventHandler(State::StateEvent* se);
    State _stCondols1;
    State::StateResult Condols1_EventHandler(State::StateEvent* se);
    State _stCondols2;
    State::StateResult Condols2_EventHandler(State::StateEvent* se);



    /** task()
     *  Hilo de ejecuci�n asociado para el procesado de las comunicaciones serie
     */
    void task(); 


    /** putMessage()
     *  Postea un mensaje en la cola
     *  @param msg Mensaje a postear
     */
    void putMessage(State::Msg *msg); 
                
                
	/** subscriptionCb()
     *  Callback invocada al recibir una actualizaci�n de un topic local al que est� suscrito
     *  @param topic Identificador del topic
     *  @param msg Mensaje recibido
     *  @param msg_len Tama�o del mensaje
     */    
    void subscriptionCb(const char* topic, void* msg, uint16_t msg_len);
    

	/** publicationCb()
     *  Callback invocada al finalizar una publicaci�n local
     *  @param topic Identificador del topic
     *  @param result Resultado de la publicaci�n
     */    
     void publicationCb(const char* topic, int32_t result);
    

	/** notifyModeUpdate()
     *  Notifica el cambio de modo, publicando en el topic correspondiente
     */    
     void notifyModeUpdate();
    


};

#endif  /** _CYBERRIBS_H */

