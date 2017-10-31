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
    uint8_t reg;
    uint8_t value;
};
static void defaultCb(){}
static const Command cmd_power_up[] = {
    {},
}

//------------------------------------------------------------------------------------
//- PUBLIC CLASS IMPL. ---------------------------------------------------------------
//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
MPR121_CapTouch::MPR121_CapTouch(PinName sda, PinName scl, PinName irq, uint8_t addr = DefaultAddress) {
    Command cmd;
    _i2c = new I2C(sda, scl);
    _iin_irq = new InterruptIn(irq);
    _iin_irq->rise(callback(defaultCb));
    _iin_irq->fall(callback(defaultCb));
    _addr = addr;
    _stat = NotPresent;
    // ajusta thresholds por defecto
    _touch_thr = DefaultTouchThreshold;
    _release_thr = DefaultTouchThreshold;
    
    // soft reset
    cmd = {.reg = MPR121_SOFTRESET, .value = 0x63};
    if(_i2c->write(_addr, &cmd, 2) != 0){
        return;
    }
    _stat = PresentWithErrors;
    wait_us(1000);
    
    // configuración de los electrodos (desactivados, incluído el virtual de proximidad)
    cmd = {.reg = MPR121_ECR, .value = 0x0};
    if(_i2c->write(_addr, &cmd, 2) != 0){        
        return;
    }
    
    // lee el valor por defecto del filtro de segundo nivel que debe ser 0x24
    cmd = {.reg = MPR121_CONFIG2, .value = 0x0};
    if(_i2c->write(_addr, &cmd.reg, 1) != 0){
        return;
    }
    if(_i2c->read(_addr, &cmd.value, 1) != 0){
        return;
    }
    if(cmd.value != 0x24){
        return;
    }
    
    // establece thresholds por defecto
    if(setThresholds(_touch_thr, _release_thr) != Succes){
        return;
    }
    
    // ajusta parámetros de filtrado por defecto (ver nota de aplicación AN3892 y datasheet pag 12)
    cmd = {.reg = MPR121_MHDR, .value = 0x01};
    if(_i2c->write(_addr, &cmd, 2) != 0){        
        return;
    }
    cmd = {.reg = MPR121_NHDR, .value = 0x01};
    if(_i2c->write(_addr, &cmd, 2) != 0){        
        return;
    }
    cmd = {.reg = MPR121_NCLR, .value = 0x0E};
    if(_i2c->write(_addr, &cmd, 2) != 0){        
        return;
    }
    cmd = {.reg = MPR121_FDLR, .value = 0x00};
    if(_i2c->write(_addr, &cmd, 2) != 0){        
        return;
    }
    
    cmd = {.reg = MPR121_MHDF, .value = 0x01};
    if(_i2c->write(_addr, &cmd, 2) != 0){        
        return;
    }
    cmd = {.reg = MPR121_NHDF, .value = 0x05};
    if(_i2c->write(_addr, &cmd, 2) != 0){        
        return;
    }
    cmd = {.reg = MPR121_NCLF, .value = 0x01};
    if(_i2c->write(_addr, &cmd, 2) != 0){        
        return;
    }
    cmd = {.reg = MPR121_FDLF, .value = 0x00};
    if(_i2c->write(_addr, &cmd, 2) != 0){        
        return;
    }
    
    cmd = {.reg = MPR121_NHDT, .value = 0x00};
    if(_i2c->write(_addr, &cmd, 2) != 0){        
        return;
    }
    cmd = {.reg = MPR121_NCLT, .value = 0x00};
    if(_i2c->write(_addr, &cmd, 2) != 0){        
        return;
    }
    cmd = {.reg = MPR121_FDLT, .value = 0x00};
    if(_i2c->write(_addr, &cmd, 2) != 0){        
        return;
    }
    
    
    cmd = {.reg = MPR121_DEBOUNCE, .value = 0x00};
    if(_i2c->write(_addr, &cmd, 2) != 0){        
        return;
    }
    cmd = {.reg = MPR121_CONFIG1, .value = 0x10};   // default, 16uA charge current
    if(_i2c->write(_addr, &cmd, 2) != 0){        
        return;
    }
    cmd = {.reg = MPR121_CONFIG2, .value = 0x20};   // 0.5uS encoding, 1ms period
    if(_i2c->write(_addr, &cmd, 2) != 0){        
        return;
    }

//    cmd = {.reg = MPR121_AUTOCONFIG0, .value = 0x8F};   
//    if(_i2c->write(_addr, &cmd, 2) != 0){        
//        return;
//    }
//    cmd = {.reg = MPR121_UPLIMIT, .value = 150};   
//    if(_i2c->write(_addr, &cmd, 2) != 0){        
//        return;
//    }//    cmd = {.reg = MPR121_TARGETLIMIT, .value = 100};   // should be ~400 (100 shifted)
//    if(_i2c->write(_addr, &cmd, 2) != 0){        
//        return;
//    }//    cmd = {.reg = MPR121_LOWLIMIT, .value = 50};   
//    if(_i2c->write(_addr, &cmd, 2) != 0){        
//        return;
//    }

    // habilita todos los electrodos
    cmd = {.reg = MPR121_ECR, .value = 0x8F};   // start with first 5 bits of baseline tracking
    if(_i2c->write(_addr, &cmd, 2) != 0){        
        return;
    }
    _stat = Ready;
}


//------------------------------------------------------------------------------------
MPR121_CapTouch::ErrorResult MPR121_CapTouch::setThresholds(uint8_t touch_th, uint8_t release_th) {
    Command cmd;
    for (uint8_t i=0; i<SensorCount; i++) {
        cmd = {.reg = (MPR121_TOUCHTH_0 + (2*i)), .value = touch_th};
        if(_i2c->write(_addr, &cmd.reg, 2) != 0){
            return WriteError;
        }
        
        cmd = {.reg = (MPR121_RELEASETH_0 + (2*i)), .value = release_th};
        if(_i2c->write(_addr, &cmd.reg, 2) != 0){
            return WriteError;
        }
    }
    return Success;
}





void MPR121_CapTouch::setThreshholds(uint8_t touch, uint8_t release) {

  setThresholds(touch, release);
  }



uint16_t  MPR121_CapTouch::filteredData(uint8_t t) {
  if (t > 12) return 0;
  return readRegister16(MPR121_FILTDATA_0L + t*2);
}

uint16_t  MPR121_CapTouch::baselineData(uint8_t t) {
  if (t > 12) return 0;
  uint16_t bl = readRegister8(MPR121_BASELINE_0 + t);
  return (bl << 2);
}

uint16_t  MPR121_CapTouch::touched(void) {
  uint16_t t = readRegister16(MPR121_TOUCHSTATUS_L);
  return t & 0x0FFF;
}

/*********************************************************************/


uint8_t MPR121_CapTouch::readRegister8(uint8_t reg) {
    Wire.beginTransmission(_i2caddr);
    Wire.write(reg);
    Wire.endTransmission(false);
    while (Wire.requestFrom(_i2caddr, 1) != 1);
    return ( Wire.read());
}

uint16_t MPR121_CapTouch::readRegister16(uint8_t reg) {
    Wire.beginTransmission(_i2caddr);
    Wire.write(reg);
    Wire.endTransmission(false);
    while (Wire.requestFrom(_i2caddr, 2) != 2);
    uint16_t v = Wire.read();
    v |=  ((uint16_t) Wire.read()) << 8;
    return v;
}

/**************************************************************************/
/*!
    @brief  Writes 8-bits to the specified destination register
*/
/**************************************************************************/
void MPR121_CapTouch::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(_i2caddr);
    Wire.write((uint8_t)reg);
    Wire.write((uint8_t)(value));
    Wire.endTransmission();
}

    


//------------------------------------------------------------------------------------
//- PROTECTED CLASS IMPL. ------------------------------------------------------------
//------------------------------------------------------------------------------------



