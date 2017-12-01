/*
 * app_Countdown.cpp
 *
 *  Created on: Dec 2017
 *      Author: raulMrello
 *
 *  La aplicación CountDown es un firmware que permite configurar la estructura robotizada para implementar 
 *  el juego "The Countdown". Este juego simula una bomba que hay que estabilizar antes de que cumpla una temporización.
 *
 *  La lógica del juego será controlada por Unity, que mediante mensajes MQTT se comunicacará con esta estructura
 *  robotizada, de forma que pueda recibir eventos de los sensores y enviar acciones a los actuadores.
 *
 *  La estructura está formada por 3 discos móviles, apilados uno sobre otro y en los que cada uno cuenta con 1 servo,
 *  y 3 sensores capacitivos.
 *
 *  Además en la parte superior, sobre el disco superior, cuenta con un sensor de proximidad, para medir la distancia
 *  de objetos situados sobre la estructura.
 *
 *  Por lo tanto, los módulos necesarios para esta aplicación son los siguientes:
 *
 *  TouchManager (TM): Controlando 9 pads capacitivos, utiliza los pines I2C (PB_7, PB_6) e IRQ (PB_1)
 *  ServoManager (SM): Controlando 3 servos, utiliza los pines I2C (PB_4, PA_7).
 *  ProximityManager (PM): Controlando el sensor de proximidad, utiliza los pines (PA_0, PA_1).
 *  MQNetBridge (MNB): Haciendo de puente MQTT, utiliza los pines UART (PA_9, PA_10).
 *  WS281xLedStrip (WLS): Controlando la tira de leds, utiliza el pin (PA_8).
 *
 *  El pinout, por lo tanto queda como sigue:
 *                             _______________________________
 *                 [MNB.tx]<--|D1(PA_9)                    VIN|<--5V
 *                 [MNB.rx]-->|D0(PA_10)                   GND|<--0V
 *                            |NRST                       NRST|
 *                       0V-->|GND                          5V|
 *                            |D2(PA_12)              (PA_2)A7|
 *                            |D3(PB_0)               (PA_7)A6|-->[SM.scl]
 *                 [TM.sda]<->|D4(PB_7)     mbed      (PA_6)A5|
 *                 [TM.scl]<--|D5(PB_6)    LK432KC    (PA_5)A4|
 *                 [TM.irq]-->|D6(PB_1)               (PA_4)A3|
 *                            |D7                     (PA_3)A2|
 *                            |D8                     (PA_1)A1|<--[PM.echo]
 *                [WLS.pwm]<--|D9(PA_8)               (PA_0)A0|-->[PM.out]
 *                            |D10(PA_11)                 AREF|
 *                            |D11(PB_5)                  3.3V|-->VDD
 *                [SM.sda]<-->|D12(PB_4)             (PB_3)D13|
 *                            |_______________________________|
 *
 *  La funcionalidad será la siguiente, maximizando la simplicidad:
 *
 *  1) La aplicación se conectará al servidor MQTT y se suscribirá a los topics "xrinst/countdown/cmd/#"
 *
 *  2) AppXR podrá configurar los diferentes estados de la estructura robotizada publicando en el topic 
 *     "xrinst/countdown/cmd/config" estos mensajes:
 *          msg: "{"value":0}"   -> Detener estructura. Desactivar submódulos.
 *          msg: "{"value":1}"   -> Activar estructura con movimiento MUY_LENTO
 *          msg: "{"value":2}"   -> Activar estructura con movimiento LENTO
 *          msg: "{"value":3}"   -> Activar estructura con movimiento ALGO_LENTO
 *          msg: "{"value":4}"   -> Activar estructura con movimiento MEDIO
 *          msg: "{"value":5}"   -> Activar estructura con movimiento ALGO_RAPIDO
 *          msg: "{"value":6}"   -> Activar estructura con movimiento RAPIDO
 *          msg: "{"value":7}"   -> Activar estructura con movimiento MUY_RAPIDO
 *          msg: "{"value":8}"   -> Activar estructura con animación de NEUTRALIZACION
 *          msg: "{"value":9}"   -> Activar estructura con animación de EXTALLIDO
 *
 *  3) La aplicación Countdown configurará los submódulos de la siguiente forma:
 *      TM: activará los eventos PRESSED, RELEASED de 9 sensores publicando eventos en "xrinst/countdown/stat/touch"
 *      PM: activará los eventos para medidas de hasta 1m con pasos de 5cm, publicando en "xrinst/countdown/stat/prox"
 *      MNB: activará suscripción a los eventos de TM y PM y los reenviará a AppXR via MQTT.
 *      SM,WLS: Definirá un algoritmo para traducir los comandos de AppXR en actuaciones concretas:
 *          Movimientos lentos  --> Ralentización servos + Leds con todos azulados ténues
 *          Movimientos rápidos --> Aceleración servos + Leds con todos rojizos fuertes
 */
 
 
#include "mbed.h"
#include "Logger.h"
#include "MQLib.h"
#include "MQNetBridge.h"
#include "TouchManager.h"
#include "ProximityManager.h"
#include "CyberRibs.h"



// **************************************************************************
// *********** DEFINICIONES *************************************************
// **************************************************************************


/** Macro de impresión de trazas de depuración */
#define DEBUG_TRACE(format, ...)    if(logger){Thread::wait(2); logger->printf(format, ##__VA_ARGS__);}


// **************************************************************************
// *********** OBJETOS  *****************************************************
// **************************************************************************

/** Canal de depuración */
static Logger* logger;

/** Canal de comunicación remota MQTT */
static MQNetBridge* qnet;

/** Controlador táctil */
static TouchManager* touchm;

/** Controlador de proximidad */
static ProximityManager* proxm;

/** Controlador del costillar */
static CyberRibs* cybribs;


/** Callback para las notificaciones de publicación */
static MQ::PublishCallback publ_cb;

/** Callback para las notificaciones de suscripción */
static MQ::SubscribeCallback subsc_cb;


// **************************************************************************
// *********** CALLBACKS ****************************************************
// **************************************************************************

static void publCallback(const char* name, int32_t){
}


//------------------------------------------------------------------------------------
static void subscCallback(const char* name, void* msg, uint16_t msg_len){
    DEBUG_TRACE("%s %s\r\n", name, msg);
    if(strcmp(name, "xrinst/countdown/cmd/config") == 0){
        char* pos = strstr((char*)msg, """value"":");
        if(pos){
            pos += strlen("""value"":") + 1;
            char* pvalue = strtok((char*)pos, ",");
            if(pvalue){
                int value = atoi(pvalue);
                DEBUG_TRACE("Set Mode %d\r\n", value);
                #warning TODO: switch
                switch(value){
                    case 0:{
                        break;
                    }
                    case 1:{
                        break;
                    }
                    case 2:{
                        break;
                    }
                    case 3:{
                        break;
                    }
                    case 4:{
                        break;
                    }
                    case 5:{
                        break;
                    }
                    case 6:{
                        break;
                    }
                    case 7:{
                        break;
                    }
                    case 8:{
                        break;
                    }
                    case 9:{
                        break;
                    }
                }
            }
        }
    }
}


//------------------------------------------------------------------------------------
void app_Countdown(){
    
    // asigno callbacks 
    publ_cb = callback(&publCallback);
    subsc_cb = callback(&subscCallback);
    
    
    
    // --------------------------------------
    // Inicia el canal de depuración
    //  - Pines USBTX, USBRX a 115200bps y 256 bytes para buffers
    //  - Configurado por defecto en modo texto
    logger = new Logger(USBTX, USBRX, 256, 115200);
    DEBUG_TRACE("\r\nIniciando app_Countdown...\r\n");


    
    // --------------------------------------
    // Creo módulo NetBridge MQTT que escuchará en el topic local "mqnetbridge"
    DEBUG_TRACE("\r\nCreando NetBridge...");    
    qnet = new MQNetBridge("mqnetbridge");
    qnet->setDebugChannel(logger);
    while(!qnet->getStatus() != MQNetBridge::Ready){
        Thread::yield();
    }
    DEBUG_TRACE("OK!"); 

    DEBUG_TRACE("\r\nConfigurando conexión...");            
    static char* mnb_cfg = "cli,usr,pass,192.168.254.98,1883,Invitado,11FF00DECA";
    MQ::MQClient::publish("mqnetbridge/cmd/conn", mnb_cfg, strlen(mnb_cfg)+1, &publ_cb);
    while(!qnet->getStatus() != MQNetBridge::Connected){
        Thread::yield();
    }
    DEBUG_TRACE("OK!");
    
    static char* mnb_rsubtopic = "xrinst/countdown/cmd/#";
    DEBUG_TRACE("\r\nSuscribiendo a topic remoto: %s...", mnb_rsubtopic);
    MQ::MQClient::publish("mqnetbridge/cmd/rsub", mnb_rsubtopic, strlen(mnb_rsubtopic)+1, &publ_cb);
    while(!qnet->getStatus() != MQNetBridge::Connected){
        Thread::yield();
    }
    DEBUG_TRACE("OK!");
    
    static char* mnb_lsubtopic0 = "xrinst/countdown/stat/touch";
    DEBUG_TRACE("\r\nSuscribiendo a topic local: %s", mnb_lsubtopic0);    
    MQ::MQClient::publish("mqnetbridge/cmd/lsub", mnb_lsubtopic0, strlen(mnb_lsubtopic0)+1, &publ_cb);

    static char* mnb_lsubtopic1 = "xrinst/countdown/stat/prox";
    DEBUG_TRACE("\r\nSuscribiendo a topic local: %s", mnb_lsubtopic1);
    MQ::MQClient::publish("mqnetbridge/cmd/lsub", mnb_lsubtopic1, strlen(mnb_lsubtopic1)+1, &publ_cb);
    
    
    
    // --------------------------------------
    // Creo módulo TouchManager
    DEBUG_TRACE("\r\nCreando Controlador táctil...");    
    touchm = new TouchManager(PB_7, PB_6, PB_1, 0x1ff);
    touchm->setDebugChannel(logger);
    while(!touchm->ready()){
        Thread::yield();
    }
    DEBUG_TRACE("OK!");    
        
    // establezco topic base 'touch'
    DEBUG_TRACE("\r\nEstablece topic base de publicación: xrinst/countdown/stat/touch");    
    touchm->setPublicationBase("xrinst/countdown/stat/touch");
    
    
    
    // --------------------------------------
    // Creo módulo ProximityManager
    DEBUG_TRACE("\r\nCreando Controlador de proximidad...");    
    proxm = new ProximityManager(PA_0, PA_1);
    proxm->setDebugChannel(logger);
    while(!proxm->ready()){
        Thread::yield();
    }
    DEBUG_TRACE("OK!");    
    
    // Establezco topic de configuración y de publicación
    DEBUG_TRACE("\r\nEstablece topic base de configuración local: prox");  
    proxm->setSubscriptionBase("prox");    
    DEBUG_TRACE("\r\nEstablece topic base de publicación: xrinst/countdown/stat/prox");  
    proxm->setPublicationBase("xrinst/countdown/stat/prox");
       
    // Configuro 9 sensores con una detección máxima de 1m con cambios de 5cm
    DEBUG_TRACE("\r\nConfigurando detector de proximidad... ");
    static char* pm_cfg = "100,8,8,2,4,0,0";
    MQ::MQClient::publish("prox/cmd/cfg", pm_cfg, strlen(pm_cfg)+1, &publ_cb);    
    DEBUG_TRACE("OK!");;
    
    
    
    // --------------------------------------
    // Creo manejador del costillar (Servos + Leds)
    //  - Número de servos controlables = 3
    //  - Número de leds por servo = 6
    DEBUG_TRACE("\r\nCreando Costillar Cibernético...");    
    static const uint8_t SERVO_COUNT = 3;
    static const uint8_t LEDS_x_RIB = 6;
    cybribs = new CyberRibs(SERVO_COUNT, LEDS_x_RIB, PB_4, PA_7, PA_8, "cyber_ribs");
    cybribs->setDebugChannel(logger);
    
    // espero a que esté listo
    DEBUG_TRACE("\r\n¿Listo?... ");
    do{
        Thread::yield();
    }while(!cybribs->ready());
    DEBUG_TRACE(" OK");

//    // recupera parámetros de calibración NV
//    DEBUG_TRACE("\r\nRecuperando parámetros de calibración de los servos... ");
//    uint32_t* caldata = (uint32_t*)Heap::memAlloc(NVFlash::getPageSize());
//    NVFlash::init();
//    NVFlash::readPage(0, caldata);
//    if(servom->setNVData(caldata) != 0){
//        DEBUG_TRACE("ERR_NVFLASH_READ, borrando...");
//        NVFlash::erasePage(0);
//        // establezco rangos de funcionamiento por defecto
//        DEBUG_TRACE("\r\nAjustando rangos por defecto... ");
//        for(uint8_t i=0;i<SERVO_COUNT;i++){
//            if(servom->setServoRanges(i, 0, 120, 180, 480) != PCA9685_ServoDrv::Success){
//                DEBUG_TRACE("ERR_servo_%d\r\n...", i);
//            }            
//        }
//        servom->getNVData(caldata);
//        NVFlash::writePage(0, caldata);
//        DEBUG_TRACE("OK");
//    }
//    else{
//        DEBUG_TRACE(" NVFLASH_RESTORE... OK!");
//    }
//    Heap::memFree(caldata);
//    
//    // situo todos a 0º y doy la orden sincronizada
//    DEBUG_TRACE("\r\nGirando servos a 0º... ");
//    for(uint8_t i=0;i<SERVO_COUNT;i++){
//        if(servom->setServoAngle(i, 0) != PCA9685_ServoDrv::Success){
//            DEBUG_TRACE("ERR_servo_%d\r\n...", i);
//        }               
//    }
//    if(servom->updateAll() != PCA9685_ServoDrv::Success){
//        DEBUG_TRACE("ERR_update");
//    }                   
//    DEBUG_TRACE("OK");
//    
//    // establezco topic base 'servo'
//    DEBUG_TRACE("\r\nEstablece topic base para los servos: servo"); 
//    servom->setSubscriptionBase("servo");

   

    // --------------------------------------
    // --------------------------------------
    for(;;){
        Thread::yield();        
    }        
}

