/*
 * CyberRibs.cpp
 *
 *  Created on: Nov 2017
 *      Author: raulMrello
 */


#include "CyberRibs.h"


//------------------------------------------------------------------------------------
//- STATIC ---------------------------------------------------------------------------
//------------------------------------------------------------------------------------

#define DEBUG_TRACE(format, ...)    if(_debug){_debug->printf(format, ##__VA_ARGS__);}


    
//------------------------------------------------------------------------------------
//-- PUBLIC METHODS IMPLEMENTATION ---------------------------------------------------
//------------------------------------------------------------------------------------


CyberRibs::CyberRibs(uint8_t num_ribs, uint8_t leds_per_rib, PinName sda_srv, PinName scl_srv, PinName pwm_led)
    : StateMachine() { 
    
    _debug = 0;        
    _ready = false;        
    
    // asigna propiedades
    _num_ribs = num_ribs;
    _leds_per_rib = leds_per_rib;
    _servod = new PCA9685_ServoDrv(sda_srv, scl_srv, _num_ribs, 0);
    _ledd = new WS281xLedStrip(pwm_led, 800000, _num_ribs * _leds_per_rib);
		
    _sub_topic = (char*)Heap::memAlloc(MQ::MQClient::getMaxTopicLen());
	MBED_ASSERT(_sub_topic);
	strcpy(_sub_topic,"");
		
    _pub_topic = (char*)Heap::memAlloc(MQ::MQClient::getMaxTopicLen());
	MBED_ASSERT(_pub_topic);
	strcpy(_pub_topic,"");
		
    _pub_topic_unique = (char*)Heap::memAlloc(MQ::MQClient::getMaxTopicLen());
	MBED_ASSERT(_pub_topic_unique);
	strcpy(_pub_topic_unique,"");
    
    // chequea y continúa
    if(!_servod || !_ledd){
        return;
    }
    
    // Carga callbacks estáticas de publicación/suscripción
    _subscriptionCb = callback(this, &CyberRibs::subscriptionCb);
    _publicationCb = callback(this, &CyberRibs::publicationCb);   
            
    // Asigno manejador de mensajes para la máquina de estados
    StateMachine::attachMessageHandler(new Callback<void(State::Msg*)>(this, &CyberRibs::putMessage));
    
    // creo máquinas de estados
    _stGoingDown.setHandler(callback(this, &CyberRibs::GoingDown_EventHandler));
    _stOff.setHandler(callback(this, &CyberRibs::Off_EventHandler));
    _stImplosion.setHandler(callback(this, &CyberRibs::Implosion_EventHandler));
    _stExplosion.setHandler(callback(this, &CyberRibs::Explosion_EventHandler));
    _stCongratulations.setHandler(callback(this, &CyberRibs::Congratulations_EventHandler));
    _stCongrat0.setHandler(callback(this, &CyberRibs::Congrat0_EventHandler));
    _stCongrat1.setHandler(callback(this, &CyberRibs::Congrat1_EventHandler));
    _stCongrat2.setHandler(callback(this, &CyberRibs::Congrat2_EventHandler));
    _stCondolences.setHandler(callback(this, &CyberRibs::Condolences_EventHandler));
    _stCondols0.setHandler(callback(this, &CyberRibs::Condols0_EventHandler));
    _stCondols1.setHandler(callback(this, &CyberRibs::Condols1_EventHandler));
    _stCondols2.setHandler(callback(this, &CyberRibs::Condols2_EventHandler));

    // Inicializa parámetros del hilo de ejecución propio
    _th.start(callback(this, &CyberRibs::task));    
}


//------------------------------------------------------------------------------------
void CyberRibs::setSubscriptionBase(const char* sub_topic) {
	MBED_ASSERT(sub_topic);
	sprintf(_sub_topic, "%s/+/cmd", sub_topic);
	MQ::MQClient::subscribe(_sub_topic, &_subscriptionCb);
	DEBUG_TRACE("[CyberRib]...... Suscrito a %s\r\n", _sub_topic);         
}   


//------------------------------------------------------------------------------------
void CyberRibs::setPublicationBase(const char* pub_topic) {
	MBED_ASSERT(pub_topic);
	sprintf(_pub_topic, "%s", pub_topic);    
}   

    
//------------------------------------------------------------------------------------
//-- PROTECTED METHODS IMPLEMENTATION ------------------------------------------------
//------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------
void CyberRibs::task(){    
    
    // espera a que el driver_servo esté preparado
    do{
        Thread::yield();
    }while(_servod->getState() != PCA9685_ServoDrv::Ready);
    
    // recupera parámetros de calibración NV
    uint32_t* caldata = (uint32_t*)Heap::memAlloc(_servod->getNVDataSize());
    MBED_ASSERT(caldata);
    NVFlash::readPage(0, caldata, _servod->getNVDataSize());
    if(_servod->setNVData(caldata) != 0){
        DEBUG_TRACE("ERR_NVFLASH_READ, rangos por defecto...\r\n");
        NVFlash::erasePage(0);
        for(uint8_t i=0;i<_num_ribs;i++){
            if(_servod->setServoRanges(i, 0, 120, 180, 480) != PCA9685_ServoDrv::Success){
                DEBUG_TRACE("ERR_servo_%d\r\n", i);
            }            
        }
        _servod->getNVData(caldata);
        NVFlash::writePage(0, caldata);
        DEBUG_TRACE("OK");
    }
    else{
        DEBUG_TRACE("NVFLASH_RESTORE... OK!\r\n");
    }
    Heap::memFree(caldata);
    
    
    // apaga la tira de leds
    WS281xLedStrip::Color_t color;
    color.red = 0; color.green = 0; color.blue = 0;            
    _ledd->setRange(0, _num_ribs * _leds_per_rib, color);
    _ledd->start();
            
    // asigna máquina de estados por defecto (estado apagado) y la inicia
    initState(&_stOff);
    
    // marca como listo para ejecución
    _ready = true;
    
    // Ejecuta máquinas de estados y espera mensajes
    for(;;){
        osEvent oe = _queue.get(osWaitForever);        
        run(&oe);
    }    
}


//------------------------------------------------------------------------------------
void CyberRibs::subscriptionCb(const char* topic, void* msg, uint16_t msg_len){
    // en primer lugar chequea qué tipo de mensaje es
    
    // si es un mensaje para establecer la configuración...
    if(MQ::MQClient::isTopicToken(topic, "/cfg/cmd")){
        // lee el primer caracter y evalúa si se permiten o no las notificaciones de cambio de modo
        char mode_endis = *(char*)msg - 0x30;
        _enable_pub = (mode_endis == 1)? true : false;
        DEBUG_TRACE("[CyberRib]...... Config update, enable_pub=%c\r\n", mode_endis);        
        return;
    }    
        
    // si es un mensaje para realizar un cambio de modo...
    if(MQ::MQClient::isTopicToken(topic, "/mode/cmd")){
        DEBUG_TRACE("[CyberRib]...... Mode update requested... \r\n");
               
        State::Msg* op = (State::Msg*)Heap::memAlloc(sizeof(State::Msg));
        MBED_ASSERT(op);
        // lee el primer caracter para obtener el modo al que cambiar
        char mode = *(char*)msg - 0x30;
        op->msg = (void*)mode;
        op->sig = ModeUpdateMsg;            
        DEBUG_TRACE("OK!"); 
        putMessage(op);
        return;
    }   
    

    // si es un comando para mover un único servo
    if(MQ::MQClient::isTopicToken(topic, "/angle/cmd")){
        DEBUG_TRACE("[CyberRib]...... Set Servo angle... \r\n");
        // obtengo los parámetros del mensaje ServoID,Deg
        char* data = (char*)Heap::memAlloc(msg_len);
        MBED_ASSERT(data);      
        strcpy(data, (char*)msg);
        char* arg = strtok(data, ",");
        uint8_t servo = atoi(arg);
        arg = strtok(NULL, ",");
        uint8_t deg = atoi(arg);
        Heap::memFree(data);
        // mueve el servo
        _servod->setServoAngle(servo, deg, true);                               
        return;
    }    

    // si es un comando para mover un único servo
    if(MQ::MQClient::isTopicToken(topic, "/duty/cmd")){
        DEBUG_TRACE("[CyberRib]...... Set Servo duty... \r\n");
        // obtengo los parámetros del mensaje ServoID,Duty
        char* data = (char*)Heap::memAlloc(msg_len);
        MBED_ASSERT(data);      
        strcpy(data, (char*)msg);
        char* arg = strtok(data, ",");
        uint8_t servo = atoi(arg);
        arg = strtok(NULL, ",");
        uint16_t duty = atoi(arg);
        Heap::memFree(data);        
        // mueve el servo
        _servod->setServoDuty(servo, duty, true);                       
        return;
    }  

    // si es un comando para obtener los parámetros de un servo
    if(MQ::MQClient::isTopicToken(topic, "/info/cmd")){
        DEBUG_TRACE("[CyberRib]...... Lee Servo info... \r\n");
        // obtengo los parámetros del mensaje ServoID,Duty
        char* data = (char*)Heap::memAlloc(msg_len);
        MBED_ASSERT(data);   
        strcpy(data, (char*)msg);
        char* arg = strtok(data, ",");
        uint8_t servo = atoi(arg);
        Heap::memFree(data);        
        // mueve el servo
        uint8_t angle = _servod->getServoAngle(servo);                       
        uint16_t duty = _servod->getServoDuty(servo);
        int16_t min_ang, max_ang;
        uint16_t min_duty, max_duty;
        _servod->getServoRanges(servo, &min_ang, &max_ang, &min_duty, &max_duty);             
        DEBUG_TRACE("[CyberRib]...... Servo %d: ang=%d (%d,%d), duty=%d (%d,%d)\r\n", servo, angle, min_ang, max_ang, duty, min_duty, max_duty); 
        return;
    }        
    
    // si es un comando para calibrar el servo
    if(MQ::MQClient::isTopicToken(topic, "/cal/cmd")){
        DEBUG_TRACE("[CyberRib]...... Calibrando... ");
        // obtengo los parámetros del mensaje ServoID,Duty
        char* data = (char*)Heap::memAlloc(msg_len);
        MBED_ASSERT(data);      
        strcpy(data, (char*)msg);
        char* arg = strtok(data, ",");
        uint8_t servo = atoi(arg);
        arg = strtok(NULL, ",");
        uint8_t ang_min = atoi(arg);
        arg = strtok(NULL, ",");
        uint8_t ang_max = atoi(arg);
        arg = strtok(NULL, ",");
        uint16_t d_min = atoi(arg);
        arg = strtok(NULL, ",");
        uint16_t d_max = atoi(arg);
        Heap::memFree(data);        
        // calibra el servo
        _servod->setServoRanges(servo, ang_min, ang_max, d_min, d_max);
        return;
    }                  

    // si es un comando para guardar la calibración de los servos
    if(MQ::MQClient::isTopicToken(topic, "/save/cmd")){
        DEBUG_TRACE("[CyberRib]...... Guardando calibración... ");
        // obtengo los datos de calibración y los actualizo
        uint32_t* caldata = (uint32_t*)Heap::memAlloc(NVFlash::getPageSize());
        MBED_ASSERT(caldata);      
        NVFlash::readPage(0, caldata);
        _servod->getNVData(caldata);
        NVFlash::erasePage(0);
        if(NVFlash::writePage(0, caldata) == NVFlash::Success){
            DEBUG_TRACE("Guardados datos de calibración\r\n");               
        }
        else{
            DEBUG_TRACE("ERROR guardando datos de calibración\r\n");
        }
        Heap::memFree(caldata);
        return;
    }    
}


//------------------------------------------------------------------------------------
void CyberRibs::putMessage(State::Msg* msg){
    osStatus ost = _queue.put(msg);
    if(ost != osOK){
        DEBUG_TRACE("[CyberRib]...... QUEUE_PUT_ERROR %d\r\n", ost);
    }
}  


//------------------------------------------------------------------------------------
void CyberRibs::publicationCb(const char* topic, int32_t result){
    // Hacer algo si es necesario...
}  


//------------------------------------------------------------------------------------
void CyberRibs::notifyModeUpdate(){
    // si la publicación está permitida...
    if(_enable_pub){
        // apunto al topic de publicación
        sprintf(_pub_topic_unique, "%s/mode/stat", _pub_topic);
        // creo el mensaje
        char msg[4];
        sprintf(msg, "%d,%d", _mode, _submode);
        // publico en mqlib
        MQ::MQClient::publish(_pub_topic_unique, msg, strlen(msg)+1, &_publicationCb);
    }
}  

//---------------------------------------------------------------------------------
State::StateResult CyberRibs::GoingDown_EventHandler(State::StateEvent* se){
    switch(se->evt){
        case State::EV_ENTRY:{       
            return State::HANDLED;                    
        }
                                       
        case State::EV_TIMED:{
            return State::HANDLED;
        }        
        
        case ModeUpdateMsg:{
            
            
            // Publica el cambio de modo si corresponde
            notifyModeUpdate();
            // libera el mensaje
            Heap::memFree(se->oe->value.p);
            return State::HANDLED;
        }           
        
        case State::EV_EXIT:{
            nextState();
            return State::HANDLED;
        }    
        
     }
    return State::IGNORED;
}


//-------------------------------------------------------------------------
State::StateResult CyberRibs::Off_EventHandler(State::StateEvent* se){
    switch(se->evt){
        case State::EV_ENTRY:{       
            DEBUG_TRACE("CyberRibs ENTER_OFF_MODE\r\n");
            return State::HANDLED;                    
        }
                                       
        case State::EV_TIMED:{
            return State::HANDLED;
        }        
        
        case ModeUpdateMsg:{
            
            Heap::memFree(se->oe->value.p);
            return State::HANDLED;
        }           
        
        case State::EV_EXIT:{
            nextState();
            return State::HANDLED;
        }    
        
     }
    return State::IGNORED;
}


//-------------------------------------------------------------------------
State::StateResult CyberRibs::Implosion_EventHandler(State::StateEvent* se){
    switch(se->evt){
        case State::EV_ENTRY:{       
            return State::HANDLED;                    
        }
                                       
        case State::EV_TIMED:{
            return State::HANDLED;
        }        
        
        case ModeUpdateMsg:{
            
            Heap::memFree(se->oe->value.p);
            return State::HANDLED;
        }           
        
        case State::EV_EXIT:{
            nextState();
            return State::HANDLED;
        }    
        
     }
    return State::IGNORED;
}


//-------------------------------------------------------------------------
    State::StateResult CyberRibs::Explosion_EventHandler(State::StateEvent* se){
    switch(se->evt){
        case State::EV_ENTRY:{       
            return State::HANDLED;                    
        }
                                       
        case State::EV_TIMED:{
            return State::HANDLED;
        }        
        
        case ModeUpdateMsg:{
            
            Heap::memFree(se->oe->value.p);
            return State::HANDLED;
        }           
        
        case State::EV_EXIT:{
            nextState();
            return State::HANDLED;
        }    
        
     }
    return State::IGNORED;
}


//-------------------------------------------------------------------------
State::StateResult CyberRibs::Congratulations_EventHandler(State::StateEvent* se){
    switch(se->evt){
        case State::EV_ENTRY:{       
            return State::HANDLED;                    
        }
                                       
        case State::EV_TIMED:{
            return State::HANDLED;
        }        
        
        case ModeUpdateMsg:{
            
            Heap::memFree(se->oe->value.p);
            return State::HANDLED;
        }           
        
        case State::EV_EXIT:{
            nextState();
            return State::HANDLED;
        }    
        
     }
    return State::IGNORED;
}


//-------------------------------------------------------------------------
State::StateResult CyberRibs::Congrat0_EventHandler(State::StateEvent* se){
    switch(se->evt){
        case State::EV_ENTRY:{       
            // Establece estado padre
            setParent(&_stCongratulations);     
            return State::HANDLED;                    
        }
                                       
        case State::EV_TIMED:{
            return State::HANDLED;
        }        
        
        case ModeUpdateMsg:{
            
            Heap::memFree(se->oe->value.p);
            return State::HANDLED;
        }           
        
        case State::EV_EXIT:{
            nextState();
            return State::HANDLED;
        }    
        
     }
    return State::IGNORED;
}


//-------------------------------------------------------------------------
State::StateResult CyberRibs::Congrat1_EventHandler(State::StateEvent* se){
    switch(se->evt){
        case State::EV_ENTRY:{       
            // Establece estado padre
            setParent(&_stCongratulations);     
            return State::HANDLED;                    
        }
                                       
        case State::EV_TIMED:{
            return State::HANDLED;
        }        
        
        case ModeUpdateMsg:{
            
            Heap::memFree(se->oe->value.p);
            return State::HANDLED;
        }           
        
        case State::EV_EXIT:{
            nextState();
            return State::HANDLED;
        }    
        
     }
    return State::IGNORED;
}


//-------------------------------------------------------------------------
State::StateResult CyberRibs::Congrat2_EventHandler(State::StateEvent* se){
    switch(se->evt){
        case State::EV_ENTRY:{       
            // Establece estado padre
            setParent(&_stCongratulations);     
            return State::HANDLED;                    
        }
                                       
        case State::EV_TIMED:{
            return State::HANDLED;
        }        
        
        case ModeUpdateMsg:{
            
            Heap::memFree(se->oe->value.p);
            return State::HANDLED;
        }           
        
        case State::EV_EXIT:{
            nextState();
            return State::HANDLED;
        }    
        
     }
    return State::IGNORED;
}


//-------------------------------------------------------------------------
    State::StateResult CyberRibs::Condolences_EventHandler(State::StateEvent* se){
    switch(se->evt){
        case State::EV_ENTRY:{       
            return State::HANDLED;                    
        }
                                       
        case State::EV_TIMED:{
            return State::HANDLED;
        }        
        
        case ModeUpdateMsg:{
            
            Heap::memFree(se->oe->value.p);
            return State::HANDLED;
        }           
        
        case State::EV_EXIT:{
            nextState();
            return State::HANDLED;
        }    
        
     }
    return State::IGNORED;
}


//-------------------------------------------------------------------------
State::StateResult CyberRibs::Condols0_EventHandler(State::StateEvent* se){
    switch(se->evt){
        case State::EV_ENTRY:{       
            // Establece estado padre
            setParent(&_stCondolences);     
            return State::HANDLED;                    
        }
                                       
        case State::EV_TIMED:{
            return State::HANDLED;
        }        
        
        case ModeUpdateMsg:{
            
            Heap::memFree(se->oe->value.p);
            return State::HANDLED;
        }           
        
        case State::EV_EXIT:{
            nextState();
            return State::HANDLED;
        }    
        
     }
    return State::IGNORED;
}


//-------------------------------------------------------------------------
State::StateResult CyberRibs::Condols1_EventHandler(State::StateEvent* se){
    switch(se->evt){
        case State::EV_ENTRY:{       
            // Establece estado padre
            setParent(&_stCondolences);     
            return State::HANDLED;                    
        }
                                       
        case State::EV_TIMED:{
            return State::HANDLED;
        }        
        
        case ModeUpdateMsg:{
            
            Heap::memFree(se->oe->value.p);
            return State::HANDLED;
        }           
        
        case State::EV_EXIT:{
            nextState();
            return State::HANDLED;
        }    
        
     }
    return State::IGNORED;
}


//-------------------------------------------------------------------------
State::StateResult CyberRibs::Condols2_EventHandler(State::StateEvent* se){
    switch(se->evt){
        case State::EV_ENTRY:{       
            // Establece estado padre
            setParent(&_stCondolences);     
            return State::HANDLED;                    
        }
                                       
        case State::EV_TIMED:{
            return State::HANDLED;
        }        
        
        case ModeUpdateMsg:{
            
            Heap::memFree(se->oe->value.p);
            return State::HANDLED;
        }           
        
        case State::EV_EXIT:{
            nextState();
            return State::HANDLED;
        }    
        
     }
    return State::IGNORED;
}





