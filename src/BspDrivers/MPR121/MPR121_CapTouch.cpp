/*
 * MPR121_CapTouch.cpp
 *
 *  Created on: Oct 2017
 *      Author: raulMrello
 */

#include "MPR121_CapTouch.h"


//------------------------------------------------------------------------------------
//- REGISTROS CHIP MPR121 ------------------------------------------------------------
//------------------------------------------------------------------------------------


#define MPR121_TOUCHSTATUS_L    0x00
#define MPR121_TOUCHSTATUS_H    0x01
#define MPR121_FILTDATA_0L      0x04
#define MPR121_FILTDATA_0H      0x05
#define MPR121_BASELINE_0       0x1E
#define MPR121_MHDR             0x2B
#define MPR121_NHDR             0x2C
#define MPR121_NCLR             0x2D
#define MPR121_FDLR             0x2E
#define MPR121_MHDF             0x2F
#define MPR121_NHDF             0x30
#define MPR121_NCLF             0x31
#define MPR121_FDLF             0x32
#define MPR121_NHDT             0x33
#define MPR121_NCLT             0x34
#define MPR121_FDLT             0x35

#define MPR121_TOUCHTH_0        0x41
#define MPR121_RELEASETH_0      0x42
#define MPR121_DEBOUNCE         0x5B
#define MPR121_CONFIG1          0x5C
#define MPR121_CONFIG2          0x5D
#define MPR121_CHARGECURR_0     0x5F
#define MPR121_CHARGETIME_1     0x6C
#define MPR121_ECR              0x5E
#define MPR121_AUTOCONFIG0      0x7B
#define MPR121_AUTOCONFIG1      0x7C
#define MPR121_UPLIMIT          0x7D
#define MPR121_LOWLIMIT         0x7E
#define MPR121_TARGETLIMIT      0x7F

#define MPR121_GPIODIR          0x76
#define MPR121_GPIOEN           0x77
#define MPR121_GPIOSET          0x78
#define MPR121_GPIOCLR          0x79
#define MPR121_GPIOTOGGLE       0x7A

#define MPR121_SOFTRESET        0x80



//------------------------------------------------------------------------------------
//- STATIC ---------------------------------------------------------------------------
//------------------------------------------------------------------------------------

__packed struct Command{        /// Estructura de lectura/escritura por registro
    char op;
    char reg;
    char value;
};
static void defaultCb(){}
static const Command cmd_power_up[] = {
    {'W', MPR121_SOFTRESET, 0x63},      /// Reset por software
    {'W', MPR121_ECR, 0},               /// Desactivación de electrodos
    {'R', MPR121_CONFIG2, 0x24},        /// Lectura del filtro 2nd 
    {'T', 0, 0},                        /// Ajuste de thresholds para eventos de pulsación-release
    {'W', MPR121_MHDR, 0x01},           /// Ajuste de parámetros de filtrado
    {'W', MPR121_NHDR, 0x01},
    {'W', MPR121_NCLR, 0x0E},
    {'W', MPR121_FDLR, 0},
    {'W', MPR121_MHDF, 0x01},
    {'W', MPR121_NHDF, 0x05},
    {'W', MPR121_NCLF, 0x01},
    {'W', MPR121_FDLF, 0},
    {'W', MPR121_NHDT, 0},
    {'W', MPR121_NCLT, 0},
    {'W', MPR121_FDLT, 0},
    {'W', MPR121_DEBOUNCE, 0},
    {'W', MPR121_CONFIG1, 0x10},        /// default, 16uA charge current and 6 samples 1st filter
    {'W', MPR121_CONFIG2, 0x20},        /// 0.5uS encoding, 4 samples 2nd filter, 1ms period
    {'W', MPR121_ECR, 0x8f},            // 5 bits of baseline tracking. Activa electrodos
    {0, 0, 0}};                         // Fin de tabla de comandos


//------------------------------------------------------------------------------------
//- PUBLIC CLASS IMPL. ---------------------------------------------------------------
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
MPR121_CapTouch::MPR121_CapTouch(PinName sda, PinName scl, PinName irq, uint8_t addr) {
    uint8_t id = 0;
    _i2c = new I2C(sda, scl);
    _iin_irq = new InterruptIn(irq);
    _irq_cb = callback(defaultCb);
    _iin_irq->rise(callback(defaultCb));
    _iin_irq->fall(callback(this, &MPR121_CapTouch::onIrqFallingEdge));
    _addr = addr;
    _stat = NotPresent;
    // ajusta thresholds por defecto
    _touch_thr = DefaultTouchThreshold;
    _release_thr = DefaultTouchThreshold;
    
    // soft reset
    if(_i2c->write(_addr, &cmd_power_up[id++].reg, 2) != 0){
        return;
    }
    
    // inicializa estado para el caso de que falle la inicialización
    _stat = PresentWithErrors;
    // espera 1ms a estabilizar tras reset software
    wait_us(1000);
    
    // inicia volcado de parámetros por defecto
    while(cmd_power_up[id].op != 0){
        // proceso escritura de registros...
        if(cmd_power_up[id].op == 'W'){
            if(_i2c->write(_addr, &cmd_power_up[id].reg, 2) != 0){
                return;
            }
        }
        // procesa lectura de registros...
        else if(cmd_power_up[id].op == 'R'){
            if(_i2c->write(_addr, &cmd_power_up[id].reg, 1) != 0){
                return;
            }
            char val;
            if(_i2c->read(_addr, &val, 1) != 0){
                return;
            }         
            // si es la configuración del filtro, chequea que el valor leído coincide con el de por defecto
            if(cmd_power_up[id].reg == MPR121_CONFIG2 && val != cmd_power_up[id].value){
                return;
            }            
        }   
        // procesa ajusta de thresholds de eventos
        else if(cmd_power_up[id].op == 'T'){
            if(setThresholds(_touch_thr, _release_thr) != Success){
                return;
            }
        }
        id++;
    }
    
    _stat = Ready;
}


//------------------------------------------------------------------------------------
MPR121_CapTouch::ErrorResult MPR121_CapTouch::setThresholds(uint8_t touch_th, uint8_t release_th) {
    Command cmd;
    for (uint8_t i=0; i<SensorCount; i++) {
        cmd.reg = (MPR121_TOUCHTH_0 + (2*i));
        cmd.value = touch_th;
        if(_i2c->write(_addr, &cmd.reg, 2) != 0){
            return WriteError;
        }
        
        cmd.reg = (MPR121_RELEASETH_0 + (2*i));
        cmd.value = release_th;
        if(_i2c->write(_addr, &cmd.reg, 2) != 0){
            return WriteError;
        }
    }
    return Success;
}


//------------------------------------------------------------------------------------
uint16_t MPR121_CapTouch::getFilteredData(uint8_t elec) {
    if (elec > SensorCount){
        return 0;
    }
    uint16_t value = 0;
    if(readRegister16(MPR121_FILTDATA_0L + (elec * 2), &value) != Success){
        return 0;
    }
    return value;
}


//------------------------------------------------------------------------------------
uint16_t  MPR121_CapTouch::getBaselineData(uint8_t elec) {
    if (elec > SensorCount){
        return 0;
    }
    uint8_t value = 0;
    if(readRegister8(MPR121_BASELINE_0 + elec, &value) != Success){
        return 0;
    }
    return (value << 2);
}


//------------------------------------------------------------------------------------
uint16_t MPR121_CapTouch::touched(void) {
    uint16_t value = 0;
    if(readRegister16(MPR121_TOUCHSTATUS_L, &value) != Success){
        return 0;
    }
    return (value & 0x0FFF);
}
    


//------------------------------------------------------------------------------------
//- PROTECTED CLASS IMPL. ------------------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
void MPR121_CapTouch::onIrqFallingEdge() {
    _irq_cb.call();
}


//------------------------------------------------------------------------------------
MPR121_CapTouch::ErrorResult MPR121_CapTouch::readRegister8(const char reg, uint8_t *value) {
    if(_i2c->write(_addr, &reg, 1) != 0){
        return WriteError;
    }
    char val;
    if(_i2c->read(_addr, &val, 1) != 0){
        return ReadError;
    }
    *value = (uint8_t)val;
    return Success;
}


//------------------------------------------------------------------------------------
MPR121_CapTouch::ErrorResult MPR121_CapTouch::readRegister16(const char reg, uint16_t* value) {
    if(_i2c->write(_addr, &reg, 1) != 0){
        return WriteError;
    }
    char val[2];
    if(_i2c->read(_addr, val, 2) != 0){
        return ReadError;
    }
    *value = (uint16_t)val[0] + (((uint16_t)val[1]) << 8);
    return Success;
}


//------------------------------------------------------------------------------------
MPR121_CapTouch::ErrorResult MPR121_CapTouch::writeRegister(const char reg, char value) {
    if(_i2c->write(_addr, &reg, 1) != 0){
        return WriteError;
    }
    if(_i2c->write(_addr, &value, 1) != 0){
        return WriteError;
    }    
    return Success;
}

