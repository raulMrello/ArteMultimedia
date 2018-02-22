/*
 * PushButton.cpp
 *
 *  Created on: 20/04/2015
 *      Author: raulMrello
 */

#include "PushButton.h"



//------------------------------------------------------------------------------------
//--- PRIVATE TYPES ------------------------------------------------------------------
//------------------------------------------------------------------------------------
/** Macro para imprimir trazas de depuración, siempre que se haya configurado un objeto
 *	Logger válido (ej: _debug)
 */

#define DEBUG_TRACE(format, ...)			\
if(_defdbg){								\
	printf(format, ##__VA_ARGS__);			\
}


//------------------------------------------------------------------------------------
static void nullCallback(uint32_t id){
}


//------------------------------------------------------------------------------------
//-- PUBLIC METHODS IMPLEMENTATION ---------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
PushButton::PushButton(PinName btn, uint32_t id, LogicLevel level, PinMode mode, bool defdbg) : _defdbg(defdbg) {
    // Crea objeto
	DEBUG_TRACE("[PushBtn]....... Creando PushButton en pin %d", btn);
	_iin = new InterruptIn(btn);
	MBED_ASSERT(_iin);
	_iin->mode(mode);
    _level = level;
    _id = id;
    
    // Desactiva las callbacks de notificación
    DEBUG_TRACE("[PushBtn]....... Desactivando callbacks\r\n");
    _pressCb = callback(&nullCallback);
    _holdCb = callback(&nullCallback);
    _releaseCb = callback(&nullCallback);


    // Asocia manejadores de interrupción
    DEBUG_TRACE("[PushBtn]....... Deteniendo ticker\r\n");
    _tick_filt = new RtosTimer(callback(this, &PushButton::gpioFilterCallback), osTimerOnce);
    _tick_hold = new RtosTimer(callback(this, &PushButton::holdTickCallback), osTimerPeriodic);
    _enable_ticker = false;


    DEBUG_TRACE("[PushBtn]....... Asignando RISE & FALL callbacks\r\n");
    _curr_value = _iin->read();
    enableRiseFallCallbacks();
}


//------------------------------------------------------------------------------------
void PushButton::enablePressEvents(Callback<void(uint32_t)>pressCb){
	if(pressCb){
		_pressCb = pressCb;
	}
	else{
		_pressCb = callback(&nullCallback);
	}
}


//------------------------------------------------------------------------------------
void PushButton::enableHoldEvents(Callback<void(uint32_t)>holdCb, uint32_t millis){
	if(holdCb && millis > 0){
		_holdCb = holdCb;
		_hold_us = 1000 * millis;
		_enable_ticker = true;
	}
	else{
		_holdCb = callback(&nullCallback);
		_enable_ticker = false;
	}
}


//------------------------------------------------------------------------------------
void PushButton::enableReleaseEvents(Callback<void(uint32_t)>releaseCb){
	if(releaseCb){
		_releaseCb = releaseCb;
	}
	else{
		_releaseCb = callback(&nullCallback);
	}
}


//------------------------------------------------------------------------------------
void PushButton::disablePressEvents(){
    _pressCb = callback(&nullCallback);
}


//------------------------------------------------------------------------------------
void PushButton::disableHoldEvents(){
    _holdCb = callback(&nullCallback);
    _enable_ticker = false;
}


//------------------------------------------------------------------------------------
void PushButton::disableReleaseEvents(){
    _releaseCb = callback(&nullCallback);
}



 
//------------------------------------------------------------------------------------
//-- PRIVATE METHODS IMPLEMENTATION --------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
void PushButton::isrRiseCallback(){
	_iin->rise(NULL);
	_curr_value = 1;
	_tick_filt->start(GlitchFilterTimeoutUs/1000);
	_releaseCb.call(_id);	
}


//------------------------------------------------------------------------------------
void PushButton::isrFallCallback(){
	_iin->fall(NULL);
	_curr_value = 0;
	_tick_filt->start(GlitchFilterTimeoutUs/1000);
	_pressCb.call(_id);
}

//------------------------------------------------------------------------------------
void PushButton::gpioFilterCallback(){
	// desactivo timming
	// leo valor del pin
    uint8_t pin_level = (uint8_t)_iin->read();

	// En caso de glitch, descarto y vuelvo a habilitar interrupciones
	if(_curr_value != pin_level){
		DEBUG_TRACE("[PushBtn]....... ERR_NOISE\r\n");
		_curr_value = pin_level;
		_tick_hold->stop();
		enableRiseFallCallbacks();
        return;
	}

	// En caso de evento RELEASE
	if((pin_level == 1 && _level == PressIsLowLevel) || (pin_level == 0 && _level == PressIsHighLevel)){
		DEBUG_TRACE("[PushBtn]....... EV_RELEASE\r\n");
		_tick_hold->stop();
		_releaseCb.call(_id);
		enableRiseFallCallbacks();
		return;
	}

    // En caso de evento PRESS
    if((pin_level == 1 && _level == PressIsHighLevel) || (pin_level == 0 && _level == PressIsLowLevel)){
    	DEBUG_TRACE("[PushBtn]....... EV_PRESS\r\n");
        if(_enable_ticker){
        	_tick_hold->start(_hold_us/1000);
        }
        _pressCb.call(_id);
        enableRiseFallCallbacks();
        return;
    }

    // No debería llegar a este punto nunca, pero por si acaso, reasigna isr's
    DEBUG_TRACE("[PushBtn]....... ERR_LEVEL\r\n");
    _tick_hold->stop();
    enableRiseFallCallbacks();
}


//------------------------------------------------------------------------------------
void PushButton::holdTickCallback(){
	DEBUG_TRACE("[PushBtn]....... EV_HOLD\r\n");
    _holdCb.call(_id);
}


//------------------------------------------------------------------------------------
void PushButton::enableRiseFallCallbacks(){
	if((_curr_value = _iin->read())){
		_iin->rise(NULL);
		_iin->fall(callback(this, &PushButton::isrFallCallback));
	}
	else{
		_iin->rise(callback(this, &PushButton::isrRiseCallback));
		_iin->fall(NULL);
	}
	
}

