/*
 * HCSR04.cpp
 *
 *  Created on: Oct 2017
 *      Author: raulMrello
 */

#include "HCSR04.h"



//------------------------------------------------------------------------------------
//- STATIC ---------------------------------------------------------------------------
//------------------------------------------------------------------------------------


void defaultCb(){}


//------------------------------------------------------------------------------------
//- PUBLIC CLASS IMPL. ---------------------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
HCSR04::HCSR04(PinName trig, PinName echo){ 
    _out_trig = new DigitalOut(trig, 0);
    _iin_echo = new InterruptIn(echo);
    _iin_echo->rise(callback(defaultCb));
    _iin_echo->fall(callback(defaultCb));
    _stat = Stopped;
    _approach_dist_cm = DefaultDifDistance;
    _goaway_dist_cm = DefaultDifDistance;
    _num_meas = 0;
    _lapse_us = 0;
}


//------------------------------------------------------------------------------------
void HCSR04::setDistRange(int16_t approach_dist, int16_t goaway_dist){
    _approach_dist_cm = approach_dist;
    _goaway_dist_cm = goaway_dist;
}


//------------------------------------------------------------------------------------
void HCSR04::start(Callback<void(DistanceEvent, uint16_t)> evCb, uint32_t lapse_ms){
    // detiene el contador del echo
    _echo_tmr.stop();
    _echo_tmr.reset();
    _lapse_us = 1000 * lapse_ms;
    _callback = evCb;
    
    // planifica inicio temporizado si corresponde
    if(_lapse_us){
        _stat = WaitingTrigger;
        _tick_trig.attach_us(callback(this, &HCSR04::trigger), _lapse_us);
        return;
    }
    // en caso de ser un único disparo, lo ejecuta
    trigger();
}


//------------------------------------------------------------------------------------
void HCSR04::stop(){
    // detiene todo
    _tick_trig.detach();
    _iin_echo->rise(callback(defaultCb));
    _iin_echo->fall(callback(defaultCb));
    _out_trig->write(0);
    _echo_tmr.stop();
    _echo_tmr.reset();
    _stat = Stopped;
}


//------------------------------------------------------------------------------------
//- PROTECTED CLASS IMPL. ------------------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
void HCSR04::trigger(){
    // en caso de no haber recibido un echo previo, debe notificar el error primero
    if(_stat > WaitingTrigger){
        _callback.call(MeasureError, 0);        
        if(!_lapse_us){
            stop();
            return;
        }
    }
    _stat = Triggered;
    _out_trig->write(1);
    wait_us(10);
    _out_trig->write(0);
    _iin_echo->rise(callback(this, &HCSR04::echoStart));
    _iin_echo->fall(callback(this, &HCSR04::echoEnd));
    _stat = WaitingEcho;
    // en caso de ser un disparo único, utiliza el tiempo de error por defecto para detectar
    // ausencias de echo
    if(!_lapse_us){
        _tick_trig.attach_us(callback(this, &HCSR04::trigger), DefaultTimeoutUs);
    }
}


//------------------------------------------------------------------------------------
void HCSR04::echoStart(){
    _echo_tmr.start();
    _stat = WaitingEchoEnds;
}


//------------------------------------------------------------------------------------
void HCSR04::echoEnd(){
    uint32_t flytime = _echo_tmr.read_us();
    _echo_tmr.stop();
    _echo_tmr.reset();
    _iin_echo->rise(callback(defaultCb));
    _iin_echo->fall(callback(defaultCb));
    uint16_t distance_cm = (uint16_t)(flytime/58);
    if(_num_meas == 0){
        _last_dist_cm = distance_cm;
    }
    else{
        if(distance_cm < (_last_dist_cm - _approach_dist_cm)){
            _last_dist_cm = distance_cm;
            _callback.call(Approaching, _last_dist_cm);
        }
        else if(distance_cm > (_last_dist_cm + _goaway_dist_cm)){
            _last_dist_cm = distance_cm;
            _callback.call(MovingAway, _last_dist_cm);
        }
    }
    
    if(_lapse_us){    
        _stat = WaitingTrigger;
    }
    else{
        _stat = Stopped;
    }
    _num_meas++;
}
