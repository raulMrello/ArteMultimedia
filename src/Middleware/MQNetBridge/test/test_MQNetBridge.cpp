#include "mbed.h"
#include "MQLib.h"
#include "MQNetBridge.h"
#include "MQSerialBridge.h"
#include "Logger.h"


// **************************************************************************
// *********** DEFINICIONES *************************************************
// **************************************************************************


/** Macro de impresi�n de trazas de depuraci�n */
#define DEBUG_TRACE(format, ...)    if(logger){Thread::wait(2); logger->printf(format, ##__VA_ARGS__);}


// **************************************************************************
// *********** OBJETOS  *****************************************************
// **************************************************************************

/** Canal de comunicaci�n remota */
static MQSerialBridge* qserial;
/** Canal de depuraci�n */
static Logger* logger;
/** Canal de comunicaci�n remota MQTT */
static MQNetBridge* qnet;

static MQ::PublishCallback publ_cb;


// **************************************************************************
// *********** TEST  ********************************************************
// **************************************************************************

static void publCb(const char* name, int32_t){
}


//------------------------------------------------------------------------------------
static void staEvtSubscription(const char* name, void* msg, uint16_t msg_len){
    DEBUG_TRACE("%s %s\r\n", name, msg);
    MQ::MQClient::publish("stop", msg, msg_len, &publ_cb);
}


//------------------------------------------------------------------------------------
void test_MQNetBridge(){
            
    publ_cb = callback(&publCb);
    
    // --------------------------------------
    // Inicia el canal de comunicaci�n remota
    //  - Pines USBTX, USBRX a 115200bps y 256 bytes para buffers
    //  - Configurado por defecto en modo texto
    qserial = new MQSerialBridge(USBTX, USBRX, 115200, 256);
    

    // --------------------------------------
    // Inicia el canal de depuraci�n (compartiendo salida remota)
    logger = (Logger*)qserial;    
    DEBUG_TRACE("\r\nIniciando test_MQNetBridge...\r\n");


    // --------------------------------------
    // Creo m�dulo NetBridge MQTT
    DEBUG_TRACE("\r\nCreando NetBridge...");    
    qnet = new MQNetBridge("mqnetbridge");
    qnet->setDebugChannel(logger);
    while(!qnet->getStatus() != MQNetBridge::Ready){
        Thread::yield();
    }
    DEBUG_TRACE("OK!");    
        
    
    DEBUG_TRACE("\r\nSuscripci�n a eventos sta ...");
    MQ::MQClient::subscribe("sta", new MQ::SubscribeCallback(&staEvtSubscription));
    DEBUG_TRACE("OK!\r\n");
    
    // --------------------------------------
    // Arranca el test
    DEBUG_TRACE("\r\n...................INICIO DEL TEST.........................\r\n");    
    DEBUG_TRACE("\r\n- Conectar:           mqnetbridge/cmd/wifisetup Cli,Usr,Pass,Host,Port,SSID,Pass");    
    DEBUG_TRACE("\r\n- Suscribir local:    mqnetbridge/cmd/localsub Topic");    
    DEBUG_TRACE("\r\n- Suscribir remoto:   mqnetbridge/cmd/remotesub Topic");    
    DEBUG_TRACE("\r\n- Quitar suscripci�n: mqnetbridge/cmd/remoteuns Topic");        
    DEBUG_TRACE("\r\n- Desconectar:        mqnetbridge/cmd/disc 0");    
    DEBUG_TRACE("\r\n- Test en topics start, stop\r\n");    
}

