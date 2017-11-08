/*
 * PCA9685_ServoDrv.cpp
 *
 *  Created on: Oct 2017
 *      Author: raulMrello
 */

#include "PCA9685_ServoDrv.h"



//------------------------------------------------------------------------------------
//- REGISTROS CHIP PCA9685 -----------------------------------------------------------
//------------------------------------------------------------------------------------

#define PCA9685_FIXED_ADDRESS       0x80

/* Slave addresses default values */
#define PCA9685_ALLCALL_DEFVALUE    0xE0
#define PCA9685_SWRESET_DEFVALUE    0x06
#define PCA9685_SUBCALL1_DEFVALUE   0xE2
#define PCA9685_SUBCALL2_DEFVALUE   0xE4
#define PCA9685_SUBCALL3_DEFVALUE   0xE8

/* Registers */
#define PCA9685_MODE1               0x00
#define PCA9685_MODE2               0x01
#define PCA9685_SUBADR1             0x02
#define PCA9685_SUBADR2             0x03
#define PCA9685_SUBADR3             0x04
#define PCA9685_ALLCALLADR          0x05
#define PCA9685_SERVO0_ON_L         0x06
#define PCA9685_SERVO0_ON_H         0x07
#define PCA9685_SERVO0_OFF_L        0x08
#define PCA9685_SERVO0_OFF_H        0x09
#define PCA9685_SERVO1_ON_L         0x0a
#define PCA9685_SERVO1_ON_H         0x0b
#define PCA9685_SERVO1_OFF_L        0x0c
#define PCA9685_SERVO1_OFF_H        0x0d
#define PCA9685_SERVO2_ON_L         0x0e
#define PCA9685_SERVO2_ON_H         0x0f
#define PCA9685_SERVO2_OFF_L        0x10
#define PCA9685_SERVO2_OFF_H        0x11
#define PCA9685_SERVO3_ON_L         0x12
#define PCA9685_SERVO3_ON_H         0x13
#define PCA9685_SERVO3_OFF_L        0x14
#define PCA9685_SERVO3_OFF_H        0x15
#define PCA9685_SERVO4_ON_L         0x16
#define PCA9685_SERVO4_ON_H         0x17
#define PCA9685_SERVO4_OFF_L        0x18
#define PCA9685_SERVO4_OFF_H        0x19
#define PCA9685_SERVO5_ON_L         0x1a
#define PCA9685_SERVO5_ON_H         0x1b
#define PCA9685_SERVO5_OFF_L        0x1c
#define PCA9685_SERVO5_OFF_H        0x1d
#define PCA9685_SERVO6_ON_L         0x1e
#define PCA9685_SERVO6_ON_H         0x1d
#define PCA9685_SERVO6_OFF_L        0x20
#define PCA9685_SERVO6_OFF_H        0x21
#define PCA9685_SERVO7_ON_L         0x22
#define PCA9685_SERVO7_ON_H         0x23
#define PCA9685_SERVO7_OFF_L        0x24
#define PCA9685_SERVO7_OFF_H        0x25
#define PCA9685_SERVO8_ON_L         0x26
#define PCA9685_SERVO8_ON_H         0x27
#define PCA9685_SERVO8_OFF_L        0x28
#define PCA9685_SERVO8_OFF_H        0x29
#define PCA9685_SERVO9_ON_L         0x2a
#define PCA9685_SERVO9_ON_H         0x2b
#define PCA9685_SERVO9_OFF_L        0x2c
#define PCA9685_SERVO9_OFF_H        0x2d
#define PCA9685_SERVO10_ON_L        0x2e
#define PCA9685_SERVO10_ON_H        0x2f
#define PCA9685_SERVO10_OFF_L       0x30
#define PCA9685_SERVO10_OFF_H       0x31
#define PCA9685_SERVO11_ON_L        0x32
#define PCA9685_SERVO11_ON_H        0x33
#define PCA9685_SERVO11_OFF_L       0x34
#define PCA9685_SERVO11_OFF_H       0x35
#define PCA9685_SERVO12_ON_L        0x36
#define PCA9685_SERVO12_ON_H        0x37
#define PCA9685_SERVO12_OFF_L       0x38
#define PCA9685_SERVO12_OFF_H       0x39
#define PCA9685_SERVO13_ON_L        0x3a
#define PCA9685_SERVO13_ON_H        0x3b
#define PCA9685_SERVO13_OFF_L       0x3c
#define PCA9685_SERVO13_OFF_H       0x3d
#define PCA9685_SERVO14_ON_L        0x3e
#define PCA9685_SERVO14_ON_H        0x3f
#define PCA9685_SERVO14_OFF_L       0x40
#define PCA9685_SERVO14_OFF_H       0x41
#define PCA9685_SERVO15_ON_L        0x42
#define PCA9685_SERVO15_ON_H        0x43
#define PCA9685_SERVO15_OFF_L       0x44
#define PCA9685_SERVO15_OFF_H       0x45
#define PCA9685_ALL_SERVO_ON_L      0xfa
#define PCA9685_ALL_SERVO_ON_H      0xfb
#define PCA9685_ALL_SERVO_OFF_L     0xfc
#define PCA9685_ALL_SERVO_OFF_H     0xfd
#define PCA9685_PRE_SCALE           0xfe

/* MODE1 bits */
#define PCA9685_MODE1_RESTART   0x80
#define PCA9685_MODE1_EXTCLK    0x40
#define PCA9685_MODE1_AI        0x20
#define PCA9685_MODE1_SLEEP     0x10
#define PCA9685_MODE1_SUB1      0x08
#define PCA9685_MODE1_SUB2      0x04
#define PCA9685_MODE1_SUB3      0x02
#define PCA9685_MODE1_ALLCALL   0x01

/* MODE2 bits */
#define PCA9685_MODE2_INVRT     0x10
#define PCA9685_MODE2_OCH       0x08
#define PCA9685_MODE2_OUTDRV    0x04
#define PCA9685_MODE2_OUTNE1    0x02
#define PCA9685_MODE2_OUTNE0    0x01


//------------------------------------------------------------------------------------
//- STATIC ---------------------------------------------------------------------------
//------------------------------------------------------------------------------------


/* PWM Frequency macro calculator */
#define PCA9685_PWM_FREQ(oscMhz,updRate)    ((oscMhz)/(4096*(updRate)))

/** Trama de configuración inicial */
static const uint8_t INIT_CONFIG[]  = {PCA9685_MODE1, PCA9685_MODE1_AI, PCA9685_MODE2_OUTDRV};

/** Trama de bajo consumo */
static const uint8_t SLEEP_MODE[]   = {PCA9685_MODE1, PCA9685_MODE1_SLEEP};

/** Trama para cambio de frecuencia pwm */
static uint8_t CHANGE_FREQ[]        = {PCA9685_PRE_SCALE, 0};


//------------------------------------------------------------------------------------
//- PUBLIC CLASS IMPL. ---------------------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
PCA9685_ServoDrv::PCA9685_ServoDrv(PinName sda, PinName scl, uint8_t numServos, uint8_t addr, uint32_t period_us, PinName oe, uint32_t hz){
    uint8_t i;
    _addr = addr|PCA9685_FIXED_ADDRESS;
    _hz = hz;
    _period_us = period_us;
    _freq = 1000000/period_us;
    _num_servos = numServos;
    
    _i2c = new I2C(sda, scl);
    _i2c->frequency(1000000);
    _oe = NULL;
    if(oe != NC){
        _oe = new DigitalOut(oe, 1);
    }
    // establece valores por defecto
    for(i=0;i<ServoCount;i++){
        _minRange[i] = 0;
        _maxRange[i] = 4095;
        _dutyValue[i] = 0;
    }
    
    // chequea si el dispositivo está presente
    if (_i2c->write(_addr, NULL, 0) != 0){
        _stat = NotPresent;
        return;
    }
    
    // establece configuración por defecto para el driver:
    //  * Auto-incremento activado
    //  * Modo normal y salida invertida (no requiere driver auxiliar)
    if(_i2c->write(_addr, (const char*)INIT_CONFIG, sizeof(INIT_CONFIG)) != 0){
        _stat = PresentWithErrors;        
        return;
    }
    
	
    // Establece modo sleep para cambiar la frecuencia
    if(_i2c->write(_addr, (const char*)SLEEP_MODE, sizeof(SLEEP_MODE)) != 0){
        _stat = PresentWithErrors;       
        return;
    }
	    
    // cambia la frecuencia del chip pwm
    CHANGE_FREQ[1] = PCA9685_PWM_FREQ(_hz, _freq);
   if(_i2c->write(_addr, (const char*)CHANGE_FREQ, sizeof(CHANGE_FREQ)) != 0){
        _stat = PresentWithErrors;        
        return;
    }
 
    // set normal mode
    // initialize driver: enable auto-increment, normal mode and totem-pole output (no driver required)
    if(_i2c->write(_addr, (const char*)INIT_CONFIG, sizeof(INIT_CONFIG)) != 0){
        _stat = PresentWithErrors;        
        return;
    }
 	// retardo tras salida de modo sleep min=500us 
	wait_us(750);
           
    // Actualizo salidas (todas desactivadas)
    if(updateAll() != Success){
        _stat = PresentWithErrors;        
        return;
    }
    _stat = Ready;
 }


//------------------------------------------------------------------------------------
PCA9685_ServoDrv::ErrorResult PCA9685_ServoDrv::setServoRanges(uint8_t servoId, int16_t min_angle, int16_t max_angle, uint32_t min_pulse_us, uint32_t max_pulse_us){
    // check params to avoid malfunctions
    if(servoId >=ServoCount || min_pulse_us >= max_pulse_us || min_angle >= max_angle){
        return InvalidArguments;
    }
    
    _minAngle[servoId] = min_angle;
    _maxAngle[servoId] = max_angle;
    
    uint32_t checker = (4096 * min_pulse_us)/_period_us;
    if(checker >= 4096){
        return DutyOutOfRange;
    }
    _minRange[servoId] = (uint16_t)checker;
    checker = (4096 * max_pulse_us)/_period_us;
    if(checker >= 4096){
        return DutyOutOfRange;
    }
    _maxRange[servoId] = (uint16_t)checker;
    return Success;
}


//------------------------------------------------------------------------------------
PCA9685_ServoDrv::ErrorResult PCA9685_ServoDrv::setServoAngle(uint8_t servoId, uint8_t angle, bool update){
    
    // check params to avoid malfunctions
    if(servoId >=_num_servos || angle > 180)
        return InvalidArguments;
    
    float step = (float)(_maxRange[servoId] - _minRange[servoId])/(_maxAngle[servoId] - _minAngle[servoId]);
    uint16_t duty = (uint16_t)(_minRange[servoId] + (step * angle));
    
    if(update){
        uint8_t buffer[5];
        buffer[0] = PCA9685_SERVO0_ON_L+(servoId*4);
        buffer[1] = 0; //ON_L always 0
        buffer[2] = 0; //ON_H always 0
        buffer[3] = (uint8_t)(duty & 0x00ff);
        buffer[4] = (uint8_t)((duty>>8) & 0x00ff);
        // write buffer and check errors
        if(_i2c->write(_addr, (const char*)buffer, 5) != 0){
            return WriteError;
        }
        // Habilita las salidas pwm todas a 0
        if(_oe){
            _oe->write(0);
        }
    }
    // if success, update values
    _dutyValue[servoId] = duty;
    return Success;
}


//------------------------------------------------------------------------------------
uint8_t PCA9685_ServoDrv::getServoAngle(uint8_t servoId){
    float step = (float)(_maxRange[servoId] - _minRange[servoId])/(_maxAngle[servoId] - _minAngle[servoId]);
    return ((uint8_t)((float)_dutyValue[servoId] - (float)_minRange[servoId])/step);
}


//------------------------------------------------------------------------------------
PCA9685_ServoDrv::ErrorResult PCA9685_ServoDrv::setServoDuty(uint8_t servoId, uint16_t duty, bool update){
    
    // check params to avoid malfunctions
    if(servoId >=_num_servos || duty >= 4096)
        return InvalidArguments;
    
    if(update){
        uint8_t buffer[5];
        buffer[0] = PCA9685_SERVO0_ON_L+(servoId*4);
        buffer[1] = 0; //ON_L always 0
        buffer[2] = 0; //ON_H always 0
        buffer[3] = (uint8_t)(duty & 0x00ff);
        buffer[4] = (uint8_t)((duty>>8) & 0x00ff);
        // write buffer and check errors
        if(_i2c->write(_addr, (const char*)buffer, 5) != 0){
            return WriteError;
        }
        // Habilita las salidas pwm todas a 0
        if(_oe){
            _oe->write(0);
        }
    }
    // if success, update values
    _dutyValue[servoId] = duty;
    return Success;
} 


//------------------------------------------------------------------------------------
uint16_t PCA9685_ServoDrv::getServoDuty(uint8_t servoId){
    return _dutyValue[servoId];
}


//------------------------------------------------------------------------------------
PCA9685_ServoDrv::ErrorResult PCA9685_ServoDrv::updateAll(){
    uint8_t i,j;
    uint8_t buffer[1+(ServoCount*4)];

    // setup buffer address
    j=0;
    buffer[j++] = PCA9685_SERVO0_ON_L;
    // setup servo values
    for(i=0;i<_num_servos;i++){
        buffer[j++] = 0; //ON_L always 0
        buffer[j++] = 0; //ON_H always 0
        buffer[j++] = (uint8_t)(_dutyValue[i] & 0x00ff);
        buffer[j++] = (uint8_t)((_dutyValue[i] >> 8) & 0x00ff);
    }
    // reset unused servos
    for(i=_num_servos;i<ServoCount;i++){
        buffer[j++] = 0;
        buffer[j++] = 0;
        buffer[j++] = 0;
        buffer[j++] = 0;
    }
    // write buffer and check errors
    if(_i2c->write(_addr, (const char*)buffer, j) != 0)
        return WriteError;
    // Habilita las salidas pwm todas a 0
    if(_oe){
        _oe->write(0);
    }
    return Success;
}


//------------------------------------------------------------------------------------
uint16_t PCA9685_ServoDrv::getDutyFromAngle(uint8_t servoId, uint8_t angle){
    
    // check params to avoid malfunctions
    if(servoId >=_num_servos || angle > 180)
        return 0;
    
    float step = (float)(_maxRange[servoId] - _minRange[servoId])/(_maxAngle[servoId] - _minAngle[servoId]);
    return((uint16_t)(_minRange[servoId] + (step * angle)));
}
    


//------------------------------------------------------------------------------------
//- PROTECTED CLASS IMPL. ------------------------------------------------------------
//------------------------------------------------------------------------------------



//------------------------------------------------------------------------------------
PCA9685_ServoDrv::ErrorResult PCA9685_ServoDrv::getAllDuty(){
    char addr = PCA9685_SERVO0_ON_L;
    char buffer[96];
	uint8_t i;

    // set read pointer at position 0
    if(_i2c->write(_addr, &addr, 1) != 0){
        return WriteError;
    }    
    // lee los valores en el buffer auxiliar
    if(_i2c->read(_addr, buffer, 4 * _num_servos) != 0){
        return ReadError;
    }
    // actualiza los duties
	for(i=0;i<_num_servos;i++){
		uint16_t value = buffer[(i*4)+3];
		value = ((value << 8) & 0xff00) + buffer[(i*4)+2];
		_dutyValue[i] = value;		
	}
    return Success;
}

/** Get driver content, by register
 * @returns error code <= 0
 */
PCA9685_ServoDrv::ErrorResult PCA9685_ServoDrv::getDriverContent(void){
    char addr = 0;
    char buffer[96];
    buffer[0] = 0;
    // set read pointer at position 0
    if(_i2c->write(_addr, &addr, 1) != SUCCESS)
        return WriteError;
    if(_i2c->read(_addr, buffer, 70) != SUCCESS)
        return ReadError;
    // set read pointer at position 0xfa
    addr = 0xfa;
    if(_i2c->write(_addr, &addr, 1) != SUCCESS)
        return WriteError;
    if(_i2c->read(_addr, buffer+70, 5) != SUCCESS)
        return ReadError;
    return Success;
}

