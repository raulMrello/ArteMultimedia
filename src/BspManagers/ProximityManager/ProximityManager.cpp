/*
 * ProximityManager.cpp
 *
 *  Created on: Nov 2017
 *      Author: raulMrello
 */

#include "ProximityManager.h"



//------------------------------------------------------------------------------------
//- STATIC ---------------------------------------------------------------------------
//------------------------------------------------------------------------------------

#define DEBUG_TRACE(format, ...)    if(_debug){ _debug->printf(format, ##__VA_ARGS__);}




//------------------------------------------------------------------------------------
//- PUBLIC CLASS IMPL. ---------------------------------------------------------------
//- ProximityManager Class -------------------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
ProximityManager::ProximityManager(PinName trig, PinName echo, bool run_thread) : HCSR04(trig, echo){ 
                        
    _ready = false;
    _debug = 0;
    _msg = (char*)Heap::memAlloc(32);
    MBED_ASSERT(_msg);
    
    // Carga callbacks estáticas de publicación/suscripción    
    _pub_topic = (char*)Heap::memAlloc(MQ::MQClient::getMaxTopicLen());
	MBED_ASSERT(_pub_topic);
	strcpy(_pub_topic,"");
	
    _sub_topic = (char*)Heap::memAlloc(MQ::MQClient::getMaxTopicLen());
	MBED_ASSERT(_sub_topic);
	strcpy(_sub_topic,"");
	
    _pub_topic_unique = (char*)Heap::memAlloc(MQ::MQClient::getMaxTopicLen());
	MBED_ASSERT(_pub_topic_unique);
	strcpy(_pub_topic_unique,"");
	
    _publCb = callback(this, &ProximityManager::publicationCb);   
    _subscrCb = callback(this, &ProximityManager::subscriptionCb);   
    
    // Inicia callback de recepción de eventos
    _distCb = callback(this, &ProximityManager::distEventCb);
    
    // Inicializa parámetros del hilo de ejecución propio si corresponde
    if(run_thread){
        _th.start(callback(this, &ProximityManager::task));    
        return;
    }
    _ready = true;
}


//------------------------------------------------------------------------------------
void ProximityManager::job(uint32_t signals){
    if((signals & DistEventFlag) != 0){                
        sprintf(_pub_topic_unique, "%s/dist/stat", _pub_topic);
        sprintf(_msg, "%d,%d,%d", HCSR04::_last_event, HCSR04::_last_dist_cm, DistEventFlag);
        MQ::MQClient::publish(_pub_topic_unique, _msg, strlen(_msg)+1, &_publCb);             
    }    
    
    if((signals & InvalidDistEventFlag) != 0){        
		sprintf(_pub_topic_unique, "%s/dist/stat", _pub_topic);
		sprintf(_msg, "0,%d,%d", HCSR04::_filter.dist_cm[HCSR04::_filter.curr], InvalidDistEventFlag);
		MQ::MQClient::publish(_pub_topic_unique, _msg, strlen(_msg)+1, &_publCb);               
    }    
    
    if((signals & MeasureErrorEventFlag) != 0){        
		sprintf(_pub_topic_unique, "%s/dist/stat", _pub_topic);
		sprintf(_msg, "0,%d,%d", HCSR04::_last_error, MeasureErrorEventFlag);
		MQ::MQClient::publish(_pub_topic_unique, _msg, strlen(_msg)+1, &_publCb);               
    }        
}


//------------------------------------------------------------------------------------
void ProximityManager::setSubscriptionBase(const char* sub_topic) {
	MBED_ASSERT(sub_topic);
    sprintf(_sub_topic, "%s/+/cmd", sub_topic);
	MQ::MQClient::subscribe(_sub_topic, &_subscrCb);
    DEBUG_TRACE("\r\n[ProxMan]....... Suscrito a %s\r\n", _sub_topic);        
}   


//------------------------------------------------------------------------------------
void ProximityManager::setPublicationBase(const char* pub_topic) {
	MBED_ASSERT(pub_topic);
    sprintf(_pub_topic, "%s", pub_topic);
}   



//------------------------------------------------------------------------------------
//- PROTECTED CLASS IMPL. ------------------------------------------------------------
//- ProximityManager Class -------------------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
void ProximityManager::task(){
    _ready = true;
    for(;;){
        osEvent evt = _th.signal_wait(0, osWaitForever);
        if(evt.status == osEventSignal){   
            uint32_t sig = evt.value.signals;
            job(sig);
        }
    }
}


//------------------------------------------------------------------------------------
void ProximityManager::distEventCb(HCSR04::DistanceEvent ev, int16_t dist){
    switch(ev){
        case HCSR04::NoEvents:{
            _th.signal_set(InvalidDistEventFlag);
            break;
        }
        case HCSR04::MeasureError:{
            _th.signal_set(MeasureErrorEventFlag);
            break;
        }
        default:{
            _th.signal_set(DistEventFlag);
            break;
        }
    }
}


//------------------------------------------------------------------------------------
void ProximityManager::subscriptionCb(const char* topic, void* msg, uint16_t msg_len){
    // si es un comando para ajustar eventos D(cm)I(cm),O(cm),F,R
    if(MQ::MQClient::isTopicToken(topic, "/config/cmd")){
        DEBUG_TRACE("\r\n[ProxMan]....... Topic:%s msg:%s\r\n", topic, msg);
        // obtengo los parámetros del mensaje Tstep y Tmax
        char* data = (char*)Heap::memAlloc(msg_len);
        MBED_ASSERT(data);
        strcpy(data, (char*)msg);
        char* arg = strtok(data, ",");
        uint16_t max_dist = atoi(arg);
        arg = strtok(NULL, ",");
        uint16_t approach_dist = atoi(arg);
        arg = strtok(NULL, ",");
        uint16_t goaway_dist = atoi(arg);
        arg = strtok(NULL, ",");
        uint8_t filt_count = atoi(arg);
        arg = strtok(NULL, ",");
        uint16_t filt_range = atoi(arg);
        arg = strtok(NULL, ",");
        uint8_t endis_invalid_evts = atoi(arg);
        arg = strtok(NULL, ",");
        uint8_t endis_err_evts = atoi(arg);
        Heap::memFree(data);
        HCSR04::config(max_dist, approach_dist, goaway_dist, filt_count, filt_range, endis_invalid_evts, endis_err_evts);
        return;
    }
    
    // si es un comando para iniciar un movimiento repetitivo cada T(ms)
    if(MQ::MQClient::isTopicToken(topic, "/start/cmd")){
        DEBUG_TRACE("\r\n[ProxMan]....... Topic:%s msg:%s\r\n", topic, msg);
        // obtengo los parámetros del mensaje Tstep y Tmax
        char* data = (char*)Heap::memAlloc(msg_len);
        MBED_ASSERT(data);
        strcpy(data, (char*)msg);
        char* arg = strtok(data, ",");
        uint32_t lapse_ms = atoi(arg);
        arg = strtok(0, ",");
        uint32_t timeout_ms = atoi(arg);
        Heap::memFree(data);
        // inicia el movimiento
        HCSR04::start(_distCb, lapse_ms, timeout_ms);
        return;
    }

    // si es un comando para detener el movimiento
    if(MQ::MQClient::isTopicToken(topic, "/stop/cmd")){
        DEBUG_TRACE("\r\n[ProxMan]....... Topic:%s msg:%s\r\n", topic, msg);
        HCSR04::stop();
        return;
    }                      
}


//------------------------------------------------------------------------------------
void ProximityManager::publicationCb(const char* topic, int32_t result){
}



