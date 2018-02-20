/*
 * app_RGBGame.cpp
 *
 *  Created on: Dec 2017
 *      Author: raulMrello
 *
 *  La aplicación RGBGame es un firmware que permite configurar la estructura robotizada de la siguiente forma:
 *
 *  1) Al inicio arranca con una configuración de color RGB media si es la primera vez, o con la última que se guardó.
 *  2) Al tocar cualquiera de los paneles táctiles, comenzará el juego de colores en propagación hacia arriba.
 *  3) Al tocar durante más de 1s la estructura, entrará en modo ajuste de color, siendo el disco inferior el ajuste
 *     a rojo, el del medio a verde y el superior a azul. 
 *  4) Al volver a pulsar el de la izqda, el color correspondiente subirá a razón de 1 led cada 500ms. El de la derecha lo
 *     bajará y el del fondo volverá a modo juego.
 *  5) El modo juego funcionará 30seg desde la última pulsación y se detendrá pasando a bajo consumo, hasta que se vuelva a 
 *     pulsar un táctil con lo que se volverá al punto 1.
 *
 *
 *  Por lo tanto, los módulos necesarios para esta aplicación son los siguientes:
 *
 *  TouchManager (TM): Controlando 9 pads capacitivos, utiliza los pines I2C (PB_7, PB_6) e IRQ (PB_1)
 *  WS281xLedStrip (WLS): Controlando  la tira de leds [ pin (PA_8)].
 *  PCA9685_ServoDrv (SRV): Controlando los servos (PB_4, PA_7)
 *
 *  El pinout, por lo tanto queda como sigue:
 *                             _______________________________
 *                            |D1(PA_9)                    VIN|<--5V
 *                            |D0(PA_10)                   GND|<--0V
 *                            |NRST                       NRST|
 *                       0V-->|GND                          5V|
 *                            |D2(PA_12)              (PA_2)A7|
 *                            |D3(PB_0)               (PA_7)A6|-->[CR.scl] 
 *                 [TM.sda]<->|D4(PB_7)     mbed      (PA_6)A5|
 *                 [TM.scl]<--|D5(PB_6)    LK432KC    (PA_5)A4|
 *                 [TM.irq]-->|D6(PB_1)               (PA_4)A3|
 *                            |D7                     (PA_3)A2|
 *                            |D8                     (PA_1)A1| 
 *                [WLS.pwm]<--|D9(PA_8)               (PA_0)A0| 
 *                            |D10(PA_11)                 AREF|
 *                            |D11(PB_5)                  3.3V|-->VDD
 *                [CR.sda]<-->|D12(PB_4)             (PB_3)D13|
 *                            |_______________________________|
 *
 */
 
 
#include "mbed.h"
#include "MQLib.h"
#include "TouchManager.h"
#include "WS281xLedStrip.h"
#include "PCA9685_ServoDrv.h"
#include "FSManager.h"
#include "RGBGame.h"
#include "MQNetBridge.h"


// **************************************************************************
// *********** DEFINICIONES *************************************************
// **************************************************************************


/** Macro de impresión de trazas de depuración */
#define DEBUG_TRACE     printf


// **************************************************************************
// *********** OBJETOS  *****************************************************
// **************************************************************************

/** Controlador táctil */
static TouchManager* touchm;

/** Controlador de la tira led */
static WS281xLedStrip* leddrv;

/** Controlador de los servos */
static PCA9685_ServoDrv* servodrv;

/** Gestor backup NV */
static FSManager* fs;

/** Gestor del juego RGB */
static RGBGame* game;

/** Gestor de las comunicaciones MQTT */
static MQNetBridge* qnet;

/** Referencia estática a la callback de publicación mqlib */
static MQ::PublishCallback publ_cb;


// **************************************************************************
// *********** TEST  ********************************************************
// **************************************************************************

//------------------------------------------------------------------------------------
static void publCb(const char* name, int32_t){
}

//------------------------------------------------------------------------------------
#define NUM_SERVOS		3
#define NUM_LEDS        54



// **************************************************************************
// *********** CALLBACKS ****************************************************
// **************************************************************************

//------------------------------------------------------------------------------------
void app_RGBGame(){
        
        
    DEBUG_TRACE("\r\n[appRGB]........ Iniciando app_RGBGame...");          
    publ_cb = callback(&publCb);

    
    // --------------------------------------
    // Crea gestor de memoria NV
    fs = new FSManager("fs");

    
    // --------------------------------------
    // Creo módulo NetBridge MQTT 
    DEBUG_TRACE("\r\n[appRGB]........ Creando NetBridge ");    
    qnet = new MQNetBridge("sys/qnet", "xrinst/rgbgame", fs, 1000, true);
    while(!qnet->ready()){
        Thread::wait(1);
    }
    DEBUG_TRACE("\r\n[appRGB]........ MQNetBridge Ready!"); 

    // Configuro el acceso al servidor mqtt
    DEBUG_TRACE("\r\n[appRGB]........ Configurando conexión...");     
	static char* mnb_cfg = "cli,usr,pass,192.168.1.63,1883,MOVISTAR_9BCC,hh9DNmVvV3Km6ZzdKrkx";	
    //static char* mnb_cfg = "client,user,pass,192.168.254.29,1883,Invitado,11FF00DECA";
    //static char* mnb_cfg = "cli,usr,pass,test.mosquitto.org,1883,Invitado,11FF00DECA";
    MQ::MQClient::publish("sys/qnet/conn/cmd", mnb_cfg, strlen(mnb_cfg)+1, &publ_cb);
	
	DEBUG_TRACE("\r\n[appRGB]........ Esperando conexión al broker MQTT...");
	while(qnet->getStatus() != MQNetBridge::Connected){
        Thread::wait(100);
    }
    DEBUG_TRACE("\r\n[appRGB]........ Conectado al broker MQTT!");
    DEBUG_TRACE("\r\n[appRGB]........ Creando bridge en 'xrinst/rgbgame/+/+/stat'");
	qnet->addBridgeTopic("xrinst/rgbgame/+/+/stat");
    
    
    // --------------------------------------
    // Creo módulo TouchManager
    DEBUG_TRACE("\r\n[appRGB]........ Creando TouchManager...");    
    touchm = new TouchManager(PB_7, PB_6, PB_1, 0x1ff);
    touchm->setDebugChannel(true);
    while(!touchm->ready()){
        Thread::wait(1);
    }
    // establezco topic base 'touch'
    touchm->setPublicationBase("sys/touch");
    DEBUG_TRACE("\r\n[appRGB]........ TouchManager READY!");            
    

    // --------------------------------------
    // Creo driver de control para los leds
    //  - Dirección I2C = 0h
    //  - Número de leds controlables = NUM_LEDS    
    DEBUG_TRACE("\r\n[appRGB]........ Creando Driver WS281xLedStrip");    
    leddrv = new WS281xLedStrip(PA_8, 800000, NUM_LEDS);	
        
    
    // --------------------------------------
    // Creo driver de control para los servos
    DEBUG_TRACE("\r\n[appRGB]........ Creando Driver PCA9685_ServoDrv...");    
    servodrv = new PCA9685_ServoDrv(PB_4, PA_7, NUM_SERVOS);
    // espero a que esté listo
    do{
        Thread::wait(1);
    }while(servodrv->getState() != PCA9685_ServoDrv::Ready);
    DEBUG_TRACE("\r\n[appRGB]........ PCA9685_ServoDrv READY!");
    
    // establezco rangos de funcionamiento y marco como deshabilitaos
    DEBUG_TRACE("\r\n[appRGB]........ Ajustando rangos para los servos entre 0º y 180º...");
    for(uint8_t i=0;i<NUM_SERVOS;i++){
        if(servodrv->setServoRanges(i, 0, 180, 800, 2200) != PCA9685_ServoDrv::Success){
            DEBUG_TRACE("\r\n[appRGB]........ ERR_servo '%d'", i);
        }            
    }
    DEBUG_TRACE("\r\n[appRGB]........ Rangos para servos READY!");

	
    // --------------------------------------
    // Creo gestor del juego
    game = new RGBGame(leddrv, servodrv, fs, true);
    game->setPublicationBase("xrinst/rgbgame/game");
    game->setSubscriptionBase("xrinst/rgbgame/game");
    // espero a que esté listo
    do{
        Thread::wait(1);
    }while(!game->ready());
    game->subscribeToTouchTopics("sys/touch/elec/stat");
    
    DEBUG_TRACE("\r\n[appRGB]........ RGBGame READY!");
    MQ::MQClient::publish("xrinst/rgbgame/game/sys/evt/stat", (void*)"Ready!", strlen("Ready!")+1, &publ_cb);
    
    DEBUG_TRACE("\r\n@@@@@@@@ Aplicación iniciada @@@@@@@@\r\n");
    DEBUG_TRACE("\r\n");
}

