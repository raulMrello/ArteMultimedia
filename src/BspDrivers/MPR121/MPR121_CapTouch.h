/*
 * MPR121_CapTouch.h
 *
 *  Created on: Oct 2017
 *      Author: raulMrello
 *
 *	MPR121_CapTouch es el driver del chip MPR121 que mediante un bus I2C proporciona el acceso a 12 canales t�ctiles
 *  capacitivos. Adem�s el chip proporciona un pin de salida IRQ para notificar eventos, activo en el flanco de bajada,
 *  as� como un pin para selecci�n de direcci�n, que puede dejarse sin conectar para utilizar la direcci�n por defecto 5Ah.
 *  
 */
 
#ifndef MPR121_CAPTOUCH_H
#define MPR121_CAPTOUCH_H
 
#include "mbed.h"


class MPR121_CapTouch {
 public:
    /** Direcci�n por defecto (ADDR sin conectar) */
    static const uint8_t DefaultAddress = 0x5A;
    
 
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
     * @param sda L�nea sda del bus i2c
     * @param scl L�nea scl del bus i2c
     * @param irq Entrada de interrupci�n
     * @param addr Direcci�n i2c, por defecto (5Ah)
     */
    MPR121_CapTouch(PinName sda, PinName scl, PinName irq, uint8_t addr = DefaultAddress);
    

    /** Ajusta los thresholds para la generaci�n de eventos
     * @param touch_th Threshold de pulsaci�n en num. cuentas
     * @param release_th Threshold de liberaci�n en num. cuentas
     * @return C�digo de error
     */
    ErrorResult setThresholds(uint8_t touch_th, uint8_t release_th);
 
  boolean begin(uint8_t i2caddr = DefaultAddress);

  uint16_t filteredData(uint8_t t);
  uint16_t  baselineData(uint8_t t);

  uint8_t readRegister8(uint8_t reg);
  uint16_t readRegister16(uint8_t reg);
  void writeRegister(uint8_t reg, uint8_t value);
  uint16_t touched(void);
  // Add deprecated attribute so that the compiler shows a warning
  __attribute__((deprecated)) void setThreshholds(uint8_t touch, uint8_t release);
  

  protected:
    static const uint8_t DefaultTouchThreshold = 12;    /// threshold en num. cuentas para detecci�n pulsaci�n
    static const uint8_t DefaultReleaseThreshold = 6;   /// threshold en num. cuentas para detecci�n liberaci�n
    static const uint8_t SensorCount = 12;              /// N�mero de sensores capacitivos
  
    I2C* _i2c;                      /// Puerto i2c 
    InterruptIn* _iin_irq;          /// Entrada de interrupci�n
    uint8_t _addr;                  /// Direcci�n i2c asignada
    int8_t _i2caddr;
    Status _stat;                   /// Estado de ejecuci�n
    uint8_t _touch_thr;             /// Threshold actual de pulsaci�n
    uint8_t _release_thr;           /// Threshold actual de liberaci�n      
};

#endif // MPR121_CAPTOUCH_H

