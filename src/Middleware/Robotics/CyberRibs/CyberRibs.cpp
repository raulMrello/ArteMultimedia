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
    _sub_topic = 0;
    _pub_topic = 0;
    _pub_topic_unique = 0;
    
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
    if(_sub_topic){
        DEBUG_TRACE("\r\nCyberRibs: ERROR_SUB ya hecha!\r\n");
        return;
    }
    
    _sub_topic = (char*)sub_topic; 
 
    // Se suscribe a $sub_topic/#
    char* suscr = (char*)Heap::memAlloc(strlen(sub_topic) + strlen("/#")+1);
    if(suscr){
        sprintf(suscr, "%s/#", _sub_topic);
        MQ::MQClient::subscribe(suscr, &_subscriptionCb);
        DEBUG_TRACE("\r\nCyberRibs: Suscrito a %s/#\r\n", sub_topic);
    }     
}   


//------------------------------------------------------------------------------------
void CyberRibs::setPublicationBase(const char* pub_topic) {
    _pub_topic = (char*)pub_topic; 
    if(!_pub_topic_unique){
        _pub_topic_unique = (char*)Heap::memAlloc(MQ::MQClient::getMaxTopicLen());
    }
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
    
//    // recupera parámetros de calibración NV
//    uint32_t* caldata = (uint32_t*)Heap::memAlloc(NVFlash::getPageSize());
//    NVFlash::init();
//    NVFlash::readPage(0, caldata);
//    if(_servod->setNVData(caldata) != 0){
//        DEBUG_TRACE("\r\nERR_NVFLASH_READ, borrando...");
//        NVFlash::erasePage(0);
//        // establezco rangos de funcionamiento por defecto
//        DEBUG_TRACE("\r\nAjustando rangos por defecto... ");
//        for(uint8_t i=0;i<_num_ribs;i++){
//            if(_servod->setServoRanges(i, 0, 120, 180, 480) != PCA9685_ServoDrv::Success){
//                DEBUG_TRACE("ERR_servo_%d\r\n...", i);
//            }            
//        }
//        _servod->getNVData(caldata);
//        NVFlash::writePage(0, caldata);
//        DEBUG_TRACE("OK");
//    }
//    else{
//        DEBUG_TRACE("\r\nNVFLASH_RESTORE... OK!");
//    }
//    Heap::memFree(caldata);
    
    
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
    if(MQ::MQClient::isTopicToken(topic, "/cfg")){
        // lee el primer caracter y evalúa si se permiten o no las notificaciones de cambio de modo
        char mode_endis = *(char*)msg - 0x30;
        _enable_pub = (mode_endis == 1)? true : false;
        DEBUG_TRACE("\r\nCyberRibs: Config update, enable_pub=%c", mode_endis);        
        return;
    }    
        
    // si es un mensaje para realizar un cambio de modo...
    if(MQ::MQClient::isTopicToken(topic, "/mode")){
        DEBUG_TRACE("\r\nCyberRibs: Mode update requested... ");
               
        State::Msg* op = (State::Msg*)Heap::memAlloc(sizeof(State::Msg));
        if(!op){
            DEBUG_TRACE("ERR_HEAP!");
            return;
        }
        // lee el primer caracter para obtener el modo al que cambiar
        char mode = *(char*)msg - 0x30;
        op->msg = (void*)mode;
        op->sig = ModeUpdateMsg;            
        DEBUG_TRACE("OK!"); 
        _queue.put(op); 
        return;
    }   
    

    // si es un comando para mover un único servo
    if(MQ::MQClient::isTopicToken(topic, "/angle")){
        DEBUG_TRACE("\r\nCyberRibs: Set Servo angle... ");
        // obtengo los parámetros del mensaje ServoID,Deg
        char* data = (char*)Heap::memAlloc(msg_len);
        if(!data){
            DEBUG_TRACE("ERR_HEAP!");
            return;
        }        
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
    if(MQ::MQClient::isTopicToken(topic, "/duty")){
        DEBUG_TRACE("\r\nCyberRibs: Set Servo duty... ");
        // obtengo los parámetros del mensaje ServoID,Duty
        char* data = (char*)Heap::memAlloc(msg_len);
        if(!data){
            DEBUG_TRACE("ERR_HEAP!");
            return;
        }        
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
    if(MQ::MQClient::isTopicToken(topic, "/info")){
        DEBUG_TRACE("\r\nCyberRibs: Lee Servo info... ");
        // obtengo los parámetros del mensaje ServoID,Duty
        char* data = (char*)Heap::memAlloc(msg_len);
        if(!data){
            DEBUG_TRACE("ERR_HEAP!");
            return;
        }        
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
        DEBUG_TRACE("\r\nCyberRibs: Servo %d: ang=%d (%d,%d), duty=%d (%d,%d)\r\n", servo, angle, min_ang, max_ang, duty, min_duty, max_duty); 
        return;
    }        
    
    // si es un comando para calibrar el servo
    if(MQ::MQClient::isTopicToken(topic, "/cal")){
        DEBUG_TRACE("\r\nCyberRibs: Calibrando... ");
        // obtengo los parámetros del mensaje ServoID,Duty
        char* data = (char*)Heap::memAlloc(msg_len);
        if(!data){
            DEBUG_TRACE("ERR_HEAP!");
            return;
        }        
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
    if(MQ::MQClient::isTopicToken(topic, "/save")){
        DEBUG_TRACE("\r\nCyberRibs: Guardando calibración... ");
        // obtengo los datos de calibración y los actualizo
        uint32_t* caldata = (uint32_t*)Heap::memAlloc(NVFlash::getPageSize());
        if(!caldata){
            DEBUG_TRACE("ERR_HEAP!");
            return;
        }        
        NVFlash::readPage(0, caldata);
        _servod->getNVData(caldata);
        NVFlash::erasePage(0);
        if(NVFlash::writePage(0, caldata) == NVFlash::Success){
            DEBUG_TRACE("\r\nGuardados datos de calibración\r\n");               
        }
        else{
            DEBUG_TRACE("\r\nERROR guardando datos de calibración\r\n");
        }
        Heap::memFree(caldata);
        return;
    }    
}


//------------------------------------------------------------------------------------
void CyberRibs::putMessage(State::Msg* msg){
    _queue.put(msg);
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
        sprintf(_pub_topic_unique, "%s/mode", _pub_topic);
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
            DEBUG_TRACE("\r\nCyberRibs ENTER_OFF_MODE\r\n");
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





