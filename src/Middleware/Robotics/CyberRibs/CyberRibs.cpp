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


CyberRibs::CyberRibs(uint8_t num_ribs, uint8_t leds_per_rib, PinName sda_srv, PinName scl_srv, PinName pwm_led, const char* base_topic)
    : StateMachine() { 
    
    _ready = false;        
    
    // asigna propiedades
    _num_ribs = num_ribs;
    _leds_per_rib = leds_per_rib;
    _servod = new PCA9685_ServoDrv(sda_srv, scl_srv, _num_ribs, 0);
    _ledd = new WS281xLedStrip(pwm_led, 800000, _num_ribs * _leds_per_rib);
    _base_topic = (char*)Heap::memAlloc(strlen(base_topic)+1);        
    
    // chequea y continúa
    if(!_servod || !_ledd || !_base_topic){
        return;
    }
    
    // Carga topic base
    strcpy(_base_topic, base_topic);

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
//-- PROTECTED METHODS IMPLEMENTATION ------------------------------------------------
//------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------
void CyberRibs::task(){    
    
    // espera a que el driver_servo esté preparado
    do{
        Thread::yield();
    }while(_servod->getState() != PCA9685_ServoDrv::Ready);
    
    #warning TODO recuperar parámetros de calibración
    
    // apaga la tira de leds
    WS281xLedStrip::Color_t color;
    color.red = 0; color.green = 0; color.blue = 0;            
    _ledd->setRange(0, _num_ribs * _leds_per_rib, color);
    _ledd->start();
    
    // se suscribe a los topics de configuración y de actuación
    char* cmd_topics = (char*)Heap::memAlloc(strlen(_base_topic) + strlen("/cmd/#")+1);
    if(cmd_topics){
        sprintf(cmd_topics, "%s/cmd/#", _base_topic);
        MQ::MQClient::subscribe(cmd_topics, &_subscriptionCb);
    }    
        
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
    if(strstr(topic, "cmd/cfg") != 0){
        // lee el primer caracter y evalúa si se permiten o no las notificaciones de cambio de modo
        char mode_endis = *(char*)msg - 0x30;
        _enable_pub = (mode_endis == 1)? true : false;
        DEBUG_TRACE("\r\nCyberRibs: Config update, enable_pub=%c", mode_endis);        
        return;
    }    
        
    // si es un mensaje para realizar un cambio de modo...
    if(strstr(topic, "cmd/mode") != 0){
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
}


//------------------------------------------------------------------------------------
void CyberRibs::putMessage(State::Msg* msg){
    _queue.put(msg);
}  


//------------------------------------------------------------------------------------
void CyberRibs::publicationCb(const char* topic, int32_t result){
    // Hacer algo si es necesario...
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





