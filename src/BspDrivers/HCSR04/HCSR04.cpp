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
//- HCSR04 Class ---------------------------------------------------------------------
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
    _last_event = NoEvents;
    _last_dist_cm = -1;
    _last_error = NoErrors;
}


//------------------------------------------------------------------------------------
void HCSR04::config(uint16_t max_dist, uint16_t approach_dist, uint16_t goaway_dist, uint8_t filt_count, uint16_t filt_range){
    _max_dist_cm = (max_dist > MaxDetectableRange)? MaxDetectableRange : max_dist;
    _approach_dist_cm = approach_dist;
    _goaway_dist_cm = goaway_dist;
    _filter.count = (filt_count > MaxFilterSamples)? MaxFilterSamples : filt_count; 
    _filter.range = filt_range;
}


//------------------------------------------------------------------------------------
void HCSR04::start(DistEventCallback evCb, uint32_t lapse_ms, uint32_t timeout_ms){
    // detiene el contador del echo e inicializa variables de control
    _echo_tmr.stop();
    _echo_tmr.reset();
    _lapse_us = 1000 * lapse_ms;
    _timeout_us = 1000 * timeout_ms;
    _callback = evCb;
    _num_meas = 0;
    _filter.curr = MaxFilterSamples;
    for(uint8_t i=0;i<MaxFilterSamples;i++){
        _filter.dist_cm[i] = MaxDetectableRange + _filter.range;
    }    
    // planifica inicio temporizado si corresponde
    _last_error = NoErrors;
    _stat = WaitingTrigger;
    _tick_trig.attach_us(callback(this, &HCSR04::trigger), _lapse_us);    
}


//------------------------------------------------------------------------------------
void HCSR04::stop(){
    // detiene todo
    _tick_trig.detach();
    _tick_fail.detach();
    _iin_echo->rise(callback(defaultCb));
    _iin_echo->fall(callback(defaultCb));
    _out_trig->write(0);
    _echo_tmr.stop();
    _echo_tmr.reset();
    _stat = Stopped;
}



//------------------------------------------------------------------------------------
void HCSR04::getLastEvent(DistanceEvent* ev, int16_t* dist){
    *ev = _last_event;
    *dist = _last_dist_cm;
}


//------------------------------------------------------------------------------------
//- PROTECTED CLASS IMPL. ------------------------------------------------------------
//- HCSR04 Class ---------------------------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
void HCSR04::trigger(){
    // en caso de estar en estado incorrecto, marca error y reinicia
    if(_stat != WaitingTrigger){
        restart();
        _last_error = TriggerError;
        _callback.call(MeasureError, _last_error);  
        return;
    }
    _tick_fail.attach_us(callback(this, &HCSR04::echoMissing), _timeout_us);
    _stat = Triggered;
    _out_trig->write(1);
    wait_us(10);
    _out_trig->write(0);
    _iin_echo->rise(callback(this, &HCSR04::echoStart));
    _iin_echo->fall(callback(this, &HCSR04::echoEnd));
    _stat = WaitingEcho;
}


//------------------------------------------------------------------------------------
void HCSR04::echoStart(){
    if(_stat != WaitingEcho){
        restart();
        _last_error = EchoStartError;
        _callback.call(MeasureError, _last_error);  
        return;
    }
    _echo_tmr.start();
    _stat = WaitingEchoEnds;
}


//------------------------------------------------------------------------------------
void HCSR04::echoEnd(){
    if(_stat != WaitingEchoEnds){        
        restart();
        _last_error = EchoEndError;
        _callback.call(MeasureError, _last_error);  
        return;
    }
    uint32_t flytime = _echo_tmr.read_us();
    _tick_fail.detach();
    _echo_tmr.stop();
    _echo_tmr.reset();
    _iin_echo->rise(callback(defaultCb));
    _iin_echo->fall(callback(defaultCb));
    uint16_t distance_cm = (uint16_t)(flytime/58);
    if(distance_cm < _max_dist_cm){
        if(filtData(distance_cm)){
            if(distance_cm < (_last_dist_cm - _approach_dist_cm)){
                _last_dist_cm = distance_cm;
                _last_event = Approaching;
                _callback.call(_last_event, _last_dist_cm);
            }
            else if(distance_cm > (_last_dist_cm + _goaway_dist_cm)){
                _last_dist_cm = distance_cm;
                _last_event = MovingAway;
                _callback.call(_last_event, _last_dist_cm);
            }
        }
        else{
            _callback.call(NoEvents, distance_cm);
        }
        _num_meas++;
    }
    _stat = WaitingTrigger;
}


//------------------------------------------------------------------------------------
void HCSR04::echoMissing(){
    restart();
    _last_error = EchoMissingError;
    _callback.call(MeasureError, _last_error);  
        
}


//------------------------------------------------------------------------------------
void HCSR04::restart(){
    _tick_fail.detach();
    _echo_tmr.stop();
    _echo_tmr.reset();
    _iin_echo->rise(callback(defaultCb));
    _iin_echo->fall(callback(defaultCb));
    _stat = WaitingTrigger;
    _tick_trig.attach_us(callback(this, &HCSR04::trigger), _lapse_us);    
}

//------------------------------------------------------------------------------------
bool HCSR04::filtData(int16_t dist){
    _filter.curr++;
    _filter.curr = (_filter.curr >= _filter.count)? 0 : _filter.curr;
    _filter.dist_cm[_filter.curr]= dist;
    
    
    // comprueba si todas las medidas están en rango para generar una medida concreta
    int16_t dist_max = 0, dist_min = MaxDetectableRange;
    for(uint8_t i=0; i<_filter.count; i++){
        dist_min = (dist_min <= _filter.dist_cm[i])? dist_min : _filter.dist_cm[i];
        dist_max = (dist_max >= _filter.dist_cm[i])? dist_max : _filter.dist_cm[i];
    }
    if((dist_max - dist_min) < _filter.range){
        return true;
    }
    return false;
}

