/*
 * TouchManager.cpp
 *
 *  Created on: Nov 2017
 *      Author: raulMrello
 */

#include "TouchManager.h"


//------------------------------------------------------------------------------------
//--- PRIVATE TYPES ------------------------------------------------------------------
//------------------------------------------------------------------------------------

#define DEBUG_TRACE(format, ...)    if(_debug){ printf(format, ##__VA_ARGS__);}

static void defaultCb(TouchManager::TouchMsg* msg){
}
 
    
//------------------------------------------------------------------------------------
//-- PUBLIC METHODS IMPLEMENTATION ---------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
TouchManager::TouchManager(PinName sda, PinName scl, PinName irq, uint16_t elec_mask, uint8_t addr, bool run_thread) : MPR121_CapTouch(sda, scl, irq, elec_mask, addr){
            
    _debug = false;
    _ready = false;
	_pub_topic = (char*)Heap::memAlloc(MQ::MQClient::getMaxTopicLen());
	MBED_ASSERT(_pub_topic);
	strcpy(_pub_topic,"");
    _curr_sns = 0;
    _evt_cb = callback(defaultCb);
                
    // Carga callbacks est�ticas de publicaci�n/suscripci�n
    _publicationCb = callback(this, &TouchManager::publicationCb);   

    // instala manejador isr en driver
    MPR121_CapTouch::attachIrqCb(callback(this, &TouchManager::onIrqCb));
    
    // Inicializa par�metros del hilo de ejecuci�n propio si corresponde
    if(run_thread){
         _th.start(callback(this, &TouchManager::task));     
        return;
    }
    _ready = true;
}


//------------------------------------------------------------------------------------
void TouchManager::job(uint32_t signals){    
    if((signals & IrqFlag)!=0){
        // lee el valor de los sensores
        _sns = MPR121_CapTouch::touched();
        // descarta eventos de electrodos no habilitados
        if(_sns == _curr_sns){
            return;
        }
        DEBUG_TRACE("[TouchMan]...... EvtPreGlitch=%d\r\n", _sns);
        // activa filtro antiglitch
        _tick_glitch.attach_us(callback(this, &TouchManager::isrTickCb), AntiGlitchTimeout);
    }
    
    // si es un evento repetitivo
    if((signals & HoldFlag)!=0){
        // lee el valor de los sensores
        _sns = MPR121_CapTouch::touched();
        // si el valor no es el mismo que en el instante anterior, o no hay pulsados descarta
        if(_sns != _curr_sns || _sns == ReleasedEvent){
            _tick_hold.detach();
            _curr_sns = _sns;
            return;
        }
        DEBUG_TRACE("[TouchMan]...... EvtHold=%d\r\n", _sns);
        // eval�a sensor a sensor
        for(uint8_t i = 0; i< MPR121_CapTouch::SensorCount; i++){
            if((_sns & ((uint16_t)1 << i)) != 0){
                TouchMsg msg = {i, TouchedEvent};
                // notifica evento en callback
                _evt_cb.call(&msg);
                // publica mensaje
                if(_pub_topic){
                    sprintf(_msg, "%d,%d", msg.elec, msg.evt);
                    MQ::MQClient::publish(_pub_topic, _msg, strlen(_msg)+1 , &_publicationCb);
                }
            }
        }
        _curr_sns = _sns;
    }

    // si es un evento antiglitch
    if((signals & AntiGlitchFlag)!=0){  
        uint16_t sns = MPR121_CapTouch::touched();        
        // descarta glitches
        if(sns != _sns){
            _tick_hold.detach();
            return;
        }
        DEBUG_TRACE("[TouchMan]...... EvtTouch=%d\r\n", _sns);
        // eval�a sensor a sensor
        for(uint8_t i = 0; i< MPR121_CapTouch::SensorCount; i++){
            if((_sns & ((uint16_t)1 << i)) != (_curr_sns & ((uint16_t)1 << i))){
                TouchMsg msg = {i, (((_sns & ((uint16_t)1 << i)) != 0)? TouchedEvent : ReleasedEvent)};
                // notifica evento en callback
                _evt_cb.call(&msg);
                // publica mensaje
                if(_pub_topic){
                    sprintf(_msg, "%d,%d", msg.elec, msg.evt);
                    MQ::MQClient::publish(_pub_topic, _msg, strlen(_msg)+1 , &_publicationCb);
                }
            }
        }
        _curr_sns = _sns;
        _tick_hold.attach_us(callback(this, &TouchManager::isrTickHoldCb), 500000);
    }  
}


//------------------------------------------------------------------------------------
void TouchManager::setPublicationBase(const char* pub_topic) {
		MBED_ASSERT(pub_topic);
    sprintf(_pub_topic, "%s/elec/stat", pub_topic);
}   


//------------------------------------------------------------------------------------
//- PROTECTED CLASS IMPL. ------------------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
void TouchManager::task(){
    DEBUG_TRACE("[TouchMan]...... Iniciando tarea.\r\n");
    while(MPR121_CapTouch::getState() != MPR121_CapTouch::Ready){
        Thread::wait(1);
    }
    _curr_sns = MPR121_CapTouch::touched();
    DEBUG_TRACE("[TouchMan]...... Esperando eventos.\r\n");
    _ready = true;
    
    // Arranca espera
    _timeout = osWaitForever;
    for(;;){
        osEvent evt = _th.signal_wait(0, _timeout);
        if(evt.status == osEventSignal){   
            uint32_t sig = evt.value.signals;
            job(sig);           
        }
    }
}
    

//------------------------------------------------------------------------------------
void TouchManager::onIrqCb(){
    _th.signal_set(IrqFlag);   
}   
    

//------------------------------------------------------------------------------------
void TouchManager::isrTickCb(){
    _tick_glitch.detach();
    _th.signal_set(AntiGlitchFlag);   
}
    

//------------------------------------------------------------------------------------
void TouchManager::isrTickHoldCb(){
    _th.signal_set(HoldFlag);   
}


//------------------------------------------------------------------------------------
void TouchManager::publicationCb(const char* topic, int32_t result){
}
