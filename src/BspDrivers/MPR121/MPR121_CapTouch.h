/*
 * MPR121_CapTouch.h
 *
 *  Created on: Oct 2017
 *      Author: raulMrello
 *
 *	MPR121_CapTouch es el driver del chip MPR121 que mediante un bus I2C proporciona el acceso a 12 canales táctiles
 *  capacitivos. Además el chip proporciona un pin de salida IRQ para notificar eventos, activo en el flanco de bajada,
 *  así como un pin para selección de dirección, que puede dejarse sin conectar para utilizar la dirección por defecto 5Ah.
 *
 *  El uso típico será: Crear objeto, ajustar parámetros, instalar callback, invocar a touched() fuera de contexto ISR para 
 *  leer estado de los pads.
 *
 *  Además para trazar el estado anterior de los pads y saber si ha sido pulsado, si sigue pulsado o si acaba de ser liberado
 *  es necesario de un manager de más alto nivel (ver clase TouchManager)
 *
 *
 *  void onIrq(){ thread.signal_set(1); }
 *
 *  main(){
 *    MPR121_CapTouch* mpr = new MPR121_CapTouch(PA_x, PA_y, PA_z);
 *    mpr->attachIrqCb(callback(onIrq));
 *    for(;;){
 *       thread.signal_wait(0, osWaitForever);
 *       uint6_t touch_values = mpr->touched();
 *       if(touch_values & 1){ printf("Touched elec_0");}
 *  }
 */
 
#ifndef MPR121_CAPTOUCH_H
#define MPR121_CAPTOUCH_H
 
#include "mbed.h"


class MPR121_CapTouch {
 public:
    /** Dirección por defecto (ADDR sin conectar) */
    static const uint8_t DefaultAddress = 0x5A;
    static const uint8_t SensorCount = 12;              /// Número de sensores capacitivos
    
 
    /** Estado del dispositivo */
    enum Status{
        Ready, 
        NotPresent,
        PresentWithErrors,
    };
    
    
    /** Errores devueltos por los diferentes servicios */
     enum ErrorResult{
        Success = 0,
        InvalidArguments,
        WriteError,
        ReadError,
        DutyOutOfRange,
        DeviceUndetected,
    };
    

    /** Constructor por defecto
     * @param sda Línea sda del bus i2c
     * @param scl Línea scl del bus i2c
     * @param irq Entrada de interrupción
     * @param addr Dirección i2c, por defecto (5Ah)
     */
    MPR121_CapTouch(PinName sda, PinName scl, PinName irq, uint8_t addr = DefaultAddress);
    

    /** Instala callback para la notificación de eventos hardware
     * @param irq_cb Callback a instalar en eventos pin IRQ
     */
    void attachIrqCb(Callback<void()> irq_cb) { _irq_cb = irq_cb; }    

    /** Ajusta los thresholds para la generación de eventos
     * @param touch_th Threshold de pulsación en num. cuentas
     * @param release_th Threshold de liberación en num. cuentas
     * @return Código de error
     */
    ErrorResult setThresholds(uint8_t touch_th, uint8_t release_th);
    

    /** Obtiene los datos filtrados de un electrodo
     * @param elec Electrodo
     * @return Datos filtrados
     */
    uint16_t getFilteredData(uint8_t elec);
    

    /** Obtiene los datos del baseline de un electrodo
     * @param elec Electrodo
     * @return Datos del baseline asociado
     */
    uint16_t getBaselineData(uint8_t elec);
    

    /** Obtiene el estado de pulsación de los electrodos, obtiene
     *  el bit activo de los electrodos pulsados bit0 = elec0, bit11 = elec11
     * @return Estado de pulsación
     */
    uint16_t touched();

  
  protected:
    static const uint8_t DefaultTouchThreshold = 12;    /// threshold en num. cuentas para detección pulsación
    static const uint8_t DefaultReleaseThreshold = 6;   /// threshold en num. cuentas para detección liberación
  
    I2C* _i2c;                      /// Puerto i2c 
    InterruptIn* _iin_irq;          /// Entrada de interrupción
    uint8_t _addr;                  /// Dirección i2c asignada
    int8_t _i2caddr;
    Status _stat;                   /// Estado de ejecución
    uint8_t _touch_thr;             /// Threshold actual de pulsación
    uint8_t _release_thr;           /// Threshold actual de liberación      
    Callback<void()> _irq_cb;       /// Callback a invocar tras evento irq
      

    /** Procesa una interrupción a nivel bajo en el pin irq
     */
    void onIrqFallingEdge();
     

    /** Lee un registro de 8 bits
     *  @param reg Registro a leer
     *  @param value Recibe el valor leído
     *  @return Código de error
     */
    ErrorResult readRegister8(const char reg, uint8_t* value);
   

    /** Lee un registro de 16 bits
     *  @param reg Registro a leer
     *  @param value Recibe el valor leído
     *  @return Código de error
     */
    ErrorResult readRegister16(const char reg, uint16_t* value);
   

    /** Escribe un registro de 8 bits
     *  @param reg Registro a escribir
     *  @param value Valor a escribir
     *  @return Código de error
     */
    ErrorResult writeRegister(const char reg, char value);
  
};

#endif // MPR121_CAPTOUCH_H

