/*
 * HCSR04.h
 *
 *  Created on: Oct 2017
 *      Author: raulMrello
 *
 *  HCSR04 es un módulo C++ que gestiona la detección de objetos cercanos mediante el sensor HC-SR04 de ultrasonidos.
 *  Se requiere una salida digital que emita un pulso de 10us y una entrada de interrupción, para conocer la distancia
 *  del objeto (ver https://os.mbed.com/components/HC-SR04/). Para ello se medirá la anchura del semiciclo activo del
 *  pin de entrada y se aplicará la fórmula: Distancia (cm) = tON(us)/58.
 *
 *  El driver se puede configurar con los siguientes parámetros:
 *
 *  - Cadencia de chequeo de presencia (cada X ms)
 *  - Rango de alerta, cuando dos medidas de distancia varían más de +-Dif. notificando el evento ACERCANDOSE|ALEJANDOSE y
*     la distancia medida. 
 *
 */
 
 
#ifndef HCSR04_H
#define HCSR04_H

 
#include "mbed.h"



//------------------------------------------------------------------------------------
//- CLASS HCSR04 ------------------------------------------------------------
//------------------------------------------------------------------------------------


class HCSR04 {
  public:
	
    /** @enum DistanceEvent
     *  @brief Tipos de eventos generados por el driver
     */	
    enum DistanceEvent{
        NoEvents,               /// No hay eventos
        Approaching,            /// Evento al acercarse
        MovingAway,             /// Evento al alejarse
        MeasureError,           /// Error en la operación de medida
    };
	
    /** @enum State
     *  @brief Estado de ejecución del driver
     */	
    enum State{
        Stopped,            /// Detenido, no hace nada
        WaitingTrigger,     /// Esperando evento del ticker para iniciar un nuevo trigger
        Triggered,          /// Trigger iniciado
        WaitingEcho,        /// Trigger concluido, esperando echo
        WaitingEchoEnds,    /// Echo recibido, esperando final de echo
    };
	
    /** @enum ErrorResult
     *  @brief Tipos de errores generados por el driver
     */	
    enum ErrorResult{
        NoErrors,               /// No hay ningún error
        TriggerError,           /// Error al realizar el trigger
        EchoStartError,         /// Error al recibir un inicio de eco indebido
        EchoEndError,           /// Evento al recibir un fin de eco indebido
        EchoMissingError,       /// Error al no completar un echo recibido en el plazo establecido
    };    
		
    /** Callback definida para la notificación de eventos de medida */
    typedef Callback<void(DistanceEvent, int16_t)> DistEventCallback;
    
    /** @fn HCSR04()
     *  @brief Constructor, que asocia una salida digital y una interrupción entrante
     *  @param trig Pin de salida digital
     *  @param echo Pin de entrada con el echo devuelto
     */
    HCSR04(PinName trig, PinName echo);

	
    /** @fn ~HCSR04()
     *  @brief Destructor por defecto
     */
    virtual ~HCSR04(){}

	
    /** @fn config()
     *  @brief Establece los parámetros configurables como límite de variación entre medidas para la notificación de
     *  eventos y número de medidas consecutivas para el filtrado antiglitch.
     *  @param max_dist Máxima distancia a la que responderá (limitada a MaxDetectableRange)
     *  @param approach_dist Diferencia de distancia al acercarse en cm
     *  @param goaway_dist Diferencia de distancia al alejarse en cm
     *  @param filt_count Número de medidas para el filtro antiglitch (=0 filtro desactivado). El valor máximo no puede
     *  @param filt_range Rango en cm para considerar medidas del filtro como similares meas[1] = meas[0]+-filt_range
     *  superar el valor MaxFilterSamples, de lo contrario lo limitará a ese valor máximo.
     */
    void config(uint16_t max_dist, uint16_t approach_dist, uint16_t goaway_dist, uint8_t filt_count=0, uint16_t filt_range = (uint16_t)MaxDetectableRange);
	
        
    /** @fn start()
     *  @brief Inicia la medida.
     *  @param evCb Callback instalada para recibir los eventos de medida
     *  @param lapse_ms Lapso en ms para repetir la medida contínuamente. Si se deja a 0 sólo realiza una medida.
     *  @param timeout_ms Tiempo en ms para completar la captura antes de generar error.
     */
    void start(DistEventCallback evCb, uint32_t lapse_ms, uint32_t timeout_ms = DefaultFailTimeout);

	
    /** @fn stop()
     *  @brief Detiene la medida
     */
    void stop();

	
    /** @fn getLastEvent()
     *  @brief Obtiene los parámetros del último evento generado
     *  @param ev Recibe el evento
     *  @param dist Recibe la distancia en cm
     */
    void getLastEvent(DistanceEvent* ev, int16_t* dist);
    
        
  protected:           
    
    static const uint8_t  MaxFilterSamples = 5;         /// Máximo número de muestras utilizadas en el filtro
    static const int16_t  MaxDetectableRange = 400;     /// Longitud máxima que puede detectar (~400cm)
    static const uint32_t DefaultDifDistance = 50;      /// Eventos por defecto al detectar cambios de al menos 50cm
    static const uint32_t DefaultTimeoutUs = 2000000;   /// Tiempo de vuelo por defecto en modo one-shot (2sec)
    static const uint32_t DefaultFailTimeout = 100;     /// Tiempo para generar error en la medida

  /** Estructura para el filtro */
    struct FiltData_t{
        uint8_t count;
        uint8_t curr;
        uint16_t range;
        int16_t dist_cm[MaxFilterSamples];
    };
  
    State _stat;                            /// Estado de ejecución
    ErrorResult _last_error;                /// Último error 
    uint16_t _max_dist_cm;                  /// Máxima distancia a la que será receptivo
    uint16_t _approach_dist_cm;             /// Diferencia de distancia al aproximarse
    uint16_t _goaway_dist_cm;               /// Diferencia de distancia al alejarse
    uint8_t  _filt_count;                   /// Número de medidas para el filtro antiglitch
    uint32_t  _echo_time;                   /// Tiempo del echo en us
    DistEventCallback _callback;            /// Callback a invocar en cada evento
  
    DigitalOut* _out_trig;                  /// Salida digital trigger
    InterruptIn* _iin_echo;                 /// Entrada de interrupción de echo
    
    void echoStart();                       /// Callback a invocar al detectar inicio de echo
    void echoEnd();                         /// Callback a invocar al detectar fin de echo
    void echoMissing();                     /// Callback a invocar al detectar fallo por echo no devuelto
    void trigger();                         /// Callback a invocar al iniciar un trigger
  
    uint32_t _lapse_us;                     /// Cadencia en us para eventos del trigger
    uint32_t _timeout_us;                   /// Temporización de error por timeout
    Ticker _tick_trig;                      /// Ticker para la generación de triggers
    Ticker _tick_fail;                      /// Ticker para la detección de errores
    Timer _echo_tmr;                        /// Timer para el cálculo del echo
  
    DistanceEvent _last_event;              /// Último evento generado
    int16_t _last_dist_cm;                  /// Distancia anterior del objeto    
    uint32_t _num_meas;                     /// Número de medidas realizadas
    FiltData_t _filter;                     /// Filtro de datos

	
    /** @fn restart()
     *  @brief Reinicia la medida (generalmente tras una condición de error)
     */
    void restart();
    
	
    /** @fn filtData()
     *  @brief Realiza el filtrado de la medida actual
     *  @param dist Medida actual
     *  @return True si el datos es válido, False si el dato es un glitch
     */
    bool filtData(int16_t dist);            /// Rutina para filtrar el último dato registrado
    
};    

#endif   /* HCSR04_H */
