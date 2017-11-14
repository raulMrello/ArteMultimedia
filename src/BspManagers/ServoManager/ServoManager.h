/*
 * ServoManager.h
 *
 *  Created on: Nov 2017
 *      Author: raulMrello
 *
 *	ServoManager es el m�dulo encargado de gestionar los servos por medio del driver PCA9685 y proporcionar funcionalidades
 *  de alto nivel, en el contexto de tarea.
 *
 *  El uso de este manager a m�s alto nivel puede ser, mediante la librer�a MQLib, mediante la instalaci�n de los topics
 *  de publicaci�n correspondientes y/o por medio de callbacks dedicadas.
 *
 *  Los topics en los que escucha este m�dulo son los siguientes: 
 *
 *  ${pub_topic}/cmd/servo S,A
 *      Mueve el servo S al �ngulo A (limitado por rangos min,max)
 *
 *  ${pub_topic}/cmd/duty S,D
 *      Mueve el servo S al duty D (sin limitaci�n por rango)
 *
 *  ${pub_topic}/cmd/move/start StepTimeUs,NumSteps,ServoOrigin,StepDif,AngIni,AngEnd
 *      Genera un patr�n de movimiento senoidal(-1,1,-1) con una cadencia de paso StepTimeUs a completar en NumSteps pasos y 
 *      centrado en el servo ServoOrigin. Los servos adyacentes replican el movimiento variando StepDif pasos del servo
 *      origen.
 *
 *  ${pub_topic}/cmd/move/stop 0
 *      Detiene el patr�n de movimiento
 *
 *  ${pub_topic}/cmd/info S
 *      Obtiene informaci�n sobre el servo S
 *
 *  ${pub_topic}/cmd/cal S,Ai,Af,Di,Df
 *      Calibra los rangos del servo S, con �ngulo minmax Ai,Af y duty minmax Di,Df.
 *
 *  ${pub_topic}/cmd/save 0
 *      Guarda los datos de calibraci�n de todos los servos en NVFlash
 */
 
#ifndef __ServoManager__H
#define __ServoManager__H

#include "mbed.h"

/** Librer�as relativas a m�dulos software */
#include "MQLib.h"
#include "Logger.h"
#include "PCA9685_ServoDrv.h"
#include "NVFlash.h"


   
class ServoManager : public PCA9685_ServoDrv{
  public:

  
    /** Constructor
     *  Asocia los pines gpio para el driver implementado. Utiliza la direcci�n i2c por defecto
     *  @param sda L�nea sda del bus i2c
     *  @param scl L�nea scl del bus i2c
     *  @param num_servos N�mero de servos
     */
    ServoManager(PinName sda, PinName scl, uint8_t num_servos);
  
  
    /** Destructor */
    ~ServoManager();
    
  
	/** setDebugChannel()
     *  Instala canal de depuraci�n
     *  @param dbg Logger
     */
    void setDebugChannel(Logger* dbg){ _debug = dbg; }
    
  
	/** setSubscriptionBase()
     *  Registra el topic base a los que se suscribir� el m�dulo
     *  @param sub_topic Topic base para la suscripci�n
     */
    void setSubscriptionBase(const char* sub_topic);  
    
  
	/** startMovement()
     *  Establece un movimiento repetitivo
     *  @param duty Array de movimientos en cuentas pwm
     *  @param steps N�mero de pasos en el movimiento
     *  @param step_tick_us Tiempo entre paso y paso en us
     *  @param servo_zero Servo que inicia el movimiento. 
     *  @param step_dif Diferencia de paso entre servos adyacentes
     */
    void startMovement(uint16_t* duty, uint8_t steps, uint32_t step_tick_us, uint8_t servo_zero, uint8_t step_dif);
    
  
	/** stopMovement()
     *  Detiene el movimiento repetitivo
     */
    void stopMovement();  

        
    /** Devuelve el estado del driver
     *  @return Estado del chip
     */
    bool ready() { return ((PCA9685_ServoDrv::getState() == PCA9685_ServoDrv::Ready)? true : false); }    
    
  
	/** getNVDataSize()
     *  Obtiene el tama�o de los datos NV
     */
    uint32_t getNVDataSize(){ return(sizeof(NVData_t)); }
    
  
	/** setNVData()
     *  Actualiza los datos NVData
     *  @param data Datos de recuperaci�n
     *  @return 0 si es correcto, !=0 si los datos son incorrectos o inv�lidos
     */
    int setNVData(void* data);
    
  
	/** getNVData()
     *  Lee los datos NVData
     *  @param data Recibe los datos NV de recuperaci�n
     */
    void getNVData(uint32_t* data); 
    
    
  protected:
      
    /** Flags de tarea (asociados a la m�quina de estados) */
    enum SigEventFlags{
        TickMoveFlag  = (1<<0),
    };
      
    /** Estructura de ejecuci�n de movimientos repetitivos */
    struct RepetitiveMovement_t{
        uint16_t *duty;
        uint8_t steps;
        uint32_t step_tick_us;
    };
    
    /** Estructura de datos con los par�metros de calibraci�n */    
    struct NVData_t{
        int16_t minAngle[PCA9685_ServoDrv::ServoCount];
        int16_t maxAngle[PCA9685_ServoDrv::ServoCount];
        uint16_t minDuty[PCA9685_ServoDrv::ServoCount];
        uint16_t maxDuty[PCA9685_ServoDrv::ServoCount];
        uint32_t crc32;
    };
    
    Thread      _th;                    /// Manejador del thread
    uint32_t    _timeout;               /// Manejador de timming en la tarea
    char*       _sub_topic;             /// Topic base para la suscripci�n
    Logger*     _debug;                 /// Canal de depuraci�n
    uint8_t     _num_servos;            /// N�mero de servos
    RepetitiveMovement_t _rmove;        /// Movimiento repetitivo
    uint8_t _move_step[PCA9685_ServoDrv::ServoCount];
    Ticker _tick_move;

    MQ::SubscribeCallback     _subscrCb;    /// Callback de suscripci�n en topics
    
	/** task()
     *  Hilo de ejecuci�n del protocolo 
     */
    void task();
  
    
	/** onIrqCb()
     *  Callback invocada tras recibir un evento de temporizaci�n
     */
    void onTickCb();        
    

	/** subscriptionCb()
     *  Callback invocada tras recibir una suscripci�n
     *  @param topic Identificador del topic
     *  @param msg Mensaje
     *  @param msg_len Tama�o del mensaje
     */    
     void subscriptionCb(const char* name, void* msg, uint16_t msg_len);    
    
};
     
#endif /*__ServoManager__H */

/**** END OF FILE ****/


