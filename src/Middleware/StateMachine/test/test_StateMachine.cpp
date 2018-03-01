#include "mbed.h"
#include "MQLib.h"
#include "Logger.h"
#include "StateMachine.h"


// **************************************************************************
// *********** DEFINICIONES *************************************************
// **************************************************************************


/** Macro de impresión de trazas de depuración */
#define DEBUG_TRACE(format, ...)    if(logger){logger->printf(format, ##__VA_ARGS__);}


// **************************************************************************
// *********** OBJETOS  *****************************************************
// **************************************************************************

/** Canal de depuración */
static Logger* logger;
/** Callback de notificación de publicación */
static MQ::PublishCallback publ_cb;
/** Máquina de estados */
static StateMachine* sm;
static State stTest;
static State::StateResult Test_EventHandler(State::StateEvent* se);
static Ticker tick;
static Queue<State::Msg, 6> queue;
static uint32_t timeout;
static char* hello_msg = "Hello!!";

#define EVENT_0	(State::EV_RESERVED_USER << 0)
#define EVENT_1	(State::EV_RESERVED_USER << 1)

// **************************************************************************
// *********** TEST  ********************************************************
// **************************************************************************

static void tick0_callback(){
    tick.detach();
	sm->raiseEvent(EVENT_0);
}

static void tick1_callback(){
    tick.detach();
    State::Msg* msg = (State::Msg*)Heap::memAlloc(sizeof(State::Msg));
    MBED_ASSERT(msg);
    msg->sig = EVENT_1;
    msg->msg = hello_msg;
    queue.put(msg);    
}

static void putMsgCallback(State::Msg* msg){
    queue.put(msg);
}




//------------------------------------------------------------------------------------
void test_StateMachine(){
            

    // --------------------------------------
    // Inicia el canal de comunicación remota
    logger = new Logger(USBTX, USBRX, 16, 115200);
    DEBUG_TRACE("Iniciando test_MQNetBridge...\r\n");


    // --------------------------------------
    // Creo máquina de estados
    DEBUG_TRACE("Creando StateMachine...\r\n");    
	sm = new StateMachine();
    sm->attachMessageHandler(new Callback<void(State::Msg*)>(&putMsgCallback));
	stTest.setHandler(callback(&Test_EventHandler));
	sm->initState(&stTest);
	
    // Ejecuta máquinas de estados y espera mensajes
	DEBUG_TRACE("Corriendo test...\r\n"); 	
    timeout = osWaitForever;    

    for(;;){
        osEvent oe = queue.get(timeout);        
        sm->run(&oe);
    }	
}

static State::StateResult Test_EventHandler(State::StateEvent* se){
    /** Al iniciar la máquina de estados */
    switch(se->evt){
        case State::EV_ENTRY:{       
            DEBUG_TRACE("EV_ENTRY. WAIT 1sec\r\n");
            timeout = 1000;
            return State::HANDLED;                    
        }
        
        case State::EV_TIMED:{
            DEBUG_TRACE("EV_TIMED\r\n");
            timeout = osWaitForever;
            DEBUG_TRACE("TICK0_START = 2sec\r\n");
            tick.attach_us(callback(&tick0_callback), 2000000);
            return State::HANDLED;
        }    
                                       
        case EVENT_0:{
            DEBUG_TRACE("EVENT_0\r\n");
            Heap::memFree(se->oe->value.p);
            DEBUG_TRACE("DESTROY_MSG, TICK0_STOP, TICK1_START = 1sec\r\n");
            tick.attach_us(callback(&tick1_callback), 1000000);
            return State::HANDLED;
        }           
        case EVENT_1:{
            DEBUG_TRACE("EVENT_1\r\n");
            tick.detach();
            DEBUG_TRACE("TICK1_STOP\r\n");
            char* txt = (char*)((State::Msg*)(se->oe->value.p))->msg;
            DEBUG_TRACE("RECEIVED_MSG: %s", txt);
            Heap::memFree(se->oe->value.p);
            DEBUG_TRACE("DESTROY_MSG, REENTER AGAIN\r\n");
            sm->tranState(&stTest);
            return State::HANDLED;
        }           
        
        case State::EV_EXIT:{
            DEBUG_TRACE("EV_EXIT\r\n");
            sm->nextState();
            return State::HANDLED;
        }    
        
     }
    return State::IGNORED;
}

