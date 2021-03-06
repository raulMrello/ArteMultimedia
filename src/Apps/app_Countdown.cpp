/*
 * app_Countdown.cpp
 *
 *  Created on: Dec 2017
 *      Author: raulMrello
 *
 *  La aplicaci�n CountDown es un firmware que permite configurar la estructura robotizada para implementar 
 *  el juego "The Countdown". Este juego simula una bomba que hay que estabilizar antes de que cumpla una temporizaci�n.
 *
 *  La l�gica del juego ser� controlada por Unity, que mediante mensajes MQTT se comunicacar� con esta estructura
 *  robotizada, de forma que pueda recibir eventos de los sensores y enviar acciones a los actuadores.
 *
 *  La estructura est� formada por 3 discos m�viles, apilados uno sobre otro y en los que cada uno cuenta con 1 servo,
 *  varios leds RGB y 3 sensores capacitivos.
 *
 *  Adem�s en la parte superior, sobre el disco superior, cuenta con un sensor de proximidad, para medir la distancia
 *  de objetos situados sobre la estructura.
 *
 *  Por lo tanto, los m�dulos necesarios para esta aplicaci�n son los siguientes:
 *
 *  TouchManager (TM): Controlando 9 pads capacitivos, utiliza los pines I2C (PB_7, PB_6) e IRQ (PB_1)
 *  CyberRibs (CR): Controlando 3 servos [utiliza los pines I2C (PB_4, PA_7)] y la tira de leds [ pin (PA_8)].
 *  ProximityManager (PM): Controlando el sensor de proximidad, utiliza los pines (PA_0, PA_1).
 *  MQNetBridge (MNB): Haciendo de puente MQTT, utiliza los pines UART (PA_9, PA_10).
 *
 *  El pinout, por lo tanto queda como sigue:
 *                             _______________________________
 *                 [MNB.tx]<--|D1(PA_9)                    VIN|<--5V
 *                 [MNB.rx]-->|D0(PA_10)                   GND|<--0V
 *                            |NRST                       NRST|
 *                       0V-->|GND                          5V|
 *                            |D2(PA_12)              (PA_2)A7|
 *                            |D3(PB_0)               (PA_7)A6|-->[CR.scl]
 *                 [TM.sda]<->|D4(PB_7)     mbed      (PA_6)A5|
 *                 [TM.scl]<--|D5(PB_6)    LK432KC    (PA_5)A4|
 *                 [TM.irq]-->|D6(PB_1)               (PA_4)A3|
 *                            |D7                     (PA_3)A2|
 *                            |D8                     (PA_1)A1|-->[PM.out]
 *                 [CR.pwm]<--|D9(PA_8)               (PA_0)A0|<--[PM.echo]
 *                            |D10(PA_11)                 AREF|
 *                            |D11(PB_5)                  3.3V|-->VDD
 *                [CR.sda]<-->|D12(PB_4)             (PB_3)D13|
 *                            |_______________________________|
 *
 *  La funcionalidad ser� la siguiente, maximizando la simplicidad:
 *
 *  1) La aplicaci�n se conectar� al servidor MQTT y se suscribir� a los topics "xrinst/countdown/cmd/#" pudiendo 
 *     recibir los siguientes mensajes:
 *          "xrinst/countdown/sys/reset/cmd 1" -> Indica que hay que resetear la aplicaci�n
 *
 *  2) Por otro lado, desde la aplicaci�n AppXR se podr�n enviar diferentes mensajes al m�dulo CyberRibs:
 *          "xrinst/countdown/cyberribs/mode/cmd M", donde M indica uno de los posibles modos de funcionamiento:
 *              M=0 Estructura apagada, sin leds y sin movimiento. En su estado de reposo. Se desconecta la alimentaci�n 
 *                  de los servos y de los leds.
 *              M=1..7 Estructura en movimiento y con leds activos. Valores bajos (1,2) implican movimientos lentos y 
 *                   colores suaves (azulados), mientras que valores altos (6,7) implican movimientos r�pidos y colores intensos (rojos).
 *              M=8 Estructura en modo de animaci�n de �xito.
 *              M=9 Estructura en modo de animaci�n de fracaso.
 *
 *          "xrinst/countdown/cyberribs/config/cmd E", donde E permite activar notificaciones en cada cambio de estado, mediante la 
 *              publicaci�n en el topic "xrinst/countdown/stat/mode M,N siendo M el modo y N el subestado. Por ejemplo para
 *              notificar un cambio a modo Congratulations.Congrat2 enviar� el mensaje: ".../mode/stat 8,2"

 *  3) La aplicaci�n Countdown configurar� los subm�dulos de la siguiente forma:
 *      TM: activar� los eventos PRESSED, RELEASED de 9 sensores publicando eventos en "xrinst/countdown/touch/stat"
 *      PM: activar� los eventos para medidas de hasta 1m con pasos de 5cm, publicando en "xrinst/countdown/prox/dist/stat"
 *      CR: activar� las notificaciones de cambios de estado, que se publicar�n en "xrinst/countdown/cyberribs/mode/stat"
 *      MNB: activar� suscripci�n a los eventos de TM, PM y CR y los reenviar� a AppXR via MQTT.
 *
 *  4) Existen diferentes topics de configuraci�n puntual, que pueden verse en cada m�dulo. Por ejemplo, el m�dulo
 *      CyberRibs permite la calibraci�n de cada uno de los servos mediante diferentes mensajes.
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


/** Macro de impresi�n de trazas de depuraci�n */
#define DEBUG_TRACE(format, ...)    if(logger){logger->printf(format, ##__VA_ARGS__);}


// **************************************************************************
// *********** OBJETOS  *****************************************************
// **************************************************************************

/** Canal de depuraci�n */
static Logger* logger;

/** Canal de comunicaci�n remota MQTT */
static MQNetBridge* qnet;

/** Controlador t�ctil */
static TouchManager* touchm;

/** Controlador de proximidad */
static ProximityManager* proxm;

/** Controlador del costillar */
static CyberRibs* cybribs;


/** Callback para las notificaciones de publicaci�n */
static MQ::PublishCallback publ_cb;

/** Callback para las notificaciones de suscripci�n */
static MQ::SubscribeCallback subsc_cb;


// **************************************************************************
// *********** CALLBACKS ****************************************************
// **************************************************************************

static void publCallback(const char* topic, int32_t){
}


//------------------------------------------------------------------------------------
static void subscCallback(const char* topic, void* msg, uint16_t msg_len){
    DEBUG_TRACE("\r\nRecibido en topic[%s], mensaje[%s]", topic, (char*)msg);       
}


//------------------------------------------------------------------------------------
void app_Countdown(){
//    
//    // Inicializa m�dulo control de FLASH
//    NVFlash::init();

//    // asigno callbacks mqlib
//    publ_cb = callback(&publCallback);
//    subsc_cb = callback(&subscCallback);
//    
//        
//    // --------------------------------------
//    // Inicia el canal de depuraci�n
//    //  - Pines USBTX, USBRX a 115200bps y 256 bytes para buffers
//    //  - Configurado por defecto en modo texto
//    logger = new Logger(USBTX, USBRX, 16, 115200);
//    DEBUG_TRACE("\r\nIniciando app_Countdown...\r\n");
//    
//    
//    
//    // --------------------------------------
//    // Creo m�dulo TouchManager
//    DEBUG_TRACE("\r\nNew TouchManager... ");    
//    touchm = new TouchManager(PB_7, PB_6, PB_1, 0x1ff);
//    touchm->setDebugChannel(logger);
//    while(!touchm->ready()){
//        Thread::yield();
//    }
//    DEBUG_TRACE("OK!");    
//        
//    // establezco topic base 'touch'
//    DEBUG_TRACE("\r\n[appCntDwn]..... pub_base  = xrinst/countdown/stat/touch\r\n");    
//    touchm->setPublicationBase("xrinst/countdown/stat/touch");
//    
//    
//    
//    // --------------------------------------
//    // Creo m�dulo ProximityManager
//    DEBUG_TRACE("\r\nNew ProximityManager... ");    
//    proxm = new ProximityManager(PA_1, PA_0);
//    proxm->setDebugChannel(logger);
//    while(!proxm->ready()){
//        Thread::yield();
//    }
//    DEBUG_TRACE("OK!");    
//    
//    // Establezco topic de configuraci�n y de publicaci�n
//    DEBUG_TRACE("\r\n[appCntDwn]..... subs_base = xrinst/countdown/cmd/prox");  
//    proxm->setSubscriptionBase("xrinst/countdown/cmd/prox");    
//    DEBUG_TRACE("\r\n[appCntDwn]..... pub_base  = xrinst/countdown/stat/prox");  
//    proxm->setPublicationBase("xrinst/countdown/stat/prox");
//       
//    // Configuro sensor con una detecci�n m�xima de 1m con cambios de 5cm
//    DEBUG_TRACE("\r\n[appCntDwn]..... config 100,8,8,2,4,0,0... ");
//    static char* pm_cfg = "100,8,8,2,4,0,0";
//    MQ::MQClient::publish("xrinst/countdown/cmd/prox/cfg", pm_cfg, strlen(pm_cfg)+1, &publ_cb);    
//    DEBUG_TRACE("OK!\r\n");
//       
//    // Inicia funcionamiento para medida cada 500ms con timeout 400ms
//    DEBUG_TRACE("\r\n[appCntDwn]..... start 500,400... ");
//    static char* pm_start = "500,400";
//    MQ::MQClient::publish("xrinst/countdown/cmd/prox/start", pm_start, strlen(pm_start)+1, &publ_cb);    
//    DEBUG_TRACE("OK!\r\n");
//    
//    
//    
//    // --------------------------------------
//    // Creo m�dulo CyberRibs
//    //  - N�mero de servos controlables = 3
//    //  - N�mero de leds por servo = 6
//    DEBUG_TRACE("\r\nNew CyberRibs... ");    
//    static const uint8_t SERVO_COUNT = 3;
//    static const uint8_t LEDS_x_RIB = 6;
//    cybribs = new CyberRibs(SERVO_COUNT, LEDS_x_RIB, PB_4, PA_7, PA_8);
//    cybribs->setDebugChannel(logger);
//    // espero a que est� listo
//    do{
//        Thread::yield();
//    }while(!cybribs->ready());
//    DEBUG_TRACE("OK!");
//    
//    // Establezco topic de configuraci�n y de publicaci�n
//    DEBUG_TRACE("\r\n[appCntDwn]..... subs_base = xrinst/countdown/cmd/cyberribs");  
//    cybribs->setSubscriptionBase("xrinst/countdown/cmd/cyberribs");    
//    DEBUG_TRACE("\r\n[appCntDwn]..... pub_base  = xrinst/countdown/stat/cyberribs\r\n");  
//    cybribs->setPublicationBase("xrinst/countdown/stat/cyberribs");    


//    
//    // --------------------------------------
//    // Creo m�dulo NetBridge MQTT que escuchar� en el topic local "mqnetbridge"
//    DEBUG_TRACE("\r\nNew MQNetBridge... ");    
//    qnet = new MQNetBridge("sys/cmd/mqnetbridge");
//    qnet->setDebugChannel(logger);
//    while(qnet->getStatus() != MQNetBridge::Ready){
//        Thread::yield();
//    }
//    DEBUG_TRACE("OK!"); 

//    // Configuro el acceso al servidor mqtt
//    DEBUG_TRACE("\r\n[appCntDwn]..... set_conn_params... ");            
//    static char* mnb_cfg = "cli,usr,pass,192.168.1.63,1883,MOVISTAR_9BCC,hh9DNmVvV3Km6ZzdKrkx";
//    //static char* mnb_cfg = "cli,usr,pass,192.168.254.29,1883,Invitado,11FF00DECA";
//    MQ::MQClient::publish("sys/cmd/mqnetbridge/conn", mnb_cfg, strlen(mnb_cfg)+1, &publ_cb);
//    while(qnet->getStatus() != MQNetBridge::Connected){
//        Thread::yield();
//        MQNetBridge::Status stat = qnet->getStatus();
//        if(stat >= MQNetBridge::WifiError){
//            char *zeromsg = "0";
//            DEBUG_TRACE("ERR_CONN %d. Desconectando...", (int)stat);      
//            MQ::MQClient::publish("sys/cmd/mqnetbridge/disc", zeromsg, strlen(zeromsg)+1, &publ_cb);
//            while(qnet->getStatus() != MQNetBridge::Ready){
//                Thread::yield();
//            }
//            DEBUG_TRACE("\r\n[appCntDwn]..... reconnect... ");     
//            MQ::MQClient::publish("sys/cmd/mqnetbridge/conn", mnb_cfg, strlen(mnb_cfg)+1, &publ_cb);
//        }
//    }
//    DEBUG_TRACE("OK!");
//    
//    // Hago que escuche todos los topics del recurso "xrinst/countdown/cmd"
//    static char* mnb_rsubtopic = "xrinst/countdown/cmd/#";
//    DEBUG_TRACE("\r\n[appCntDwn]..... set_remote_subs_base = %s... ", mnb_rsubtopic);
//    MQ::MQClient::publish("sys/cmd/mqnetbridge/rsub", mnb_rsubtopic, strlen(mnb_rsubtopic)+1, &publ_cb);
//    while(qnet->getStatus() != MQNetBridge::Connected){
//        Thread::yield();        
//    }
//    DEBUG_TRACE("OK!");
//    
//    // Hago que escuche topics locales para redireccionarlos al exterior
//    static char* mnb_lsubtopic0 = "xrinst/countdown/stat/#";
//    DEBUG_TRACE("\r\n[appCntDwn]..... set_local_subs_base = %s\r\n", mnb_lsubtopic0);    
//    MQ::MQClient::publish("sys/cmd/mqnetbridge/lsub", mnb_lsubtopic0, strlen(mnb_lsubtopic0)+1, &publ_cb);


//    
//    // --------------------------------------
//    // Inicio aplicaci�n
//    
//    // Se suscribe a los topics de sistema
//    MQ::MQClient::subscribe("xrinst/countdown/cmd/sys/#", new MQ::SubscribeCallback(&subscCallback));
//    
//    // Se suscribe a los topics de estado
//    MQ::MQClient::subscribe("xrinst/countdown/stat/#", new MQ::SubscribeCallback(&subscCallback));
//    
//    // Publico topic de notificaci�n de estado
//    MQ::MQClient::publish("xrinst/countdown/stat/sys", (void*)"Ready!", strlen("Ready!") + 1, &publ_cb);
//    DEBUG_TRACE("\r\n ------ APPLICATION RUNNING ------- ");
}

