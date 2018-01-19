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
 *
 *  El pinout, por lo tanto queda como sigue:
 *                             _______________________________
 *                            |D1(PA_9)                    VIN|<--5V
 *                            |D0(PA_10)                   GND|<--0V
 *                            |NRST                       NRST|
 *                       0V-->|GND                          5V|
 *                            |D2(PA_12)              (PA_2)A7|
 *                            |D3(PB_0)               (PA_7)A6| 
 *                 [TM.sda]<->|D4(PB_7)     mbed      (PA_6)A5|
 *                 [TM.scl]<--|D5(PB_6)    LK432KC    (PA_5)A4|
 *                 [TM.irq]-->|D6(PB_1)               (PA_4)A3|
 *                            |D7                     (PA_3)A2|
 *                            |D8                     (PA_1)A1| 
 *                [WLS.pwm]<--|D9(PA_8)               (PA_0)A0| 
 *                            |D10(PA_11)                 AREF|
 *                            |D11(PB_5)                  3.3V|-->VDD
 *                            |D12(PB_4)             (PB_3)D13|
 *                            |_______________________________|
 *
 */
 
 
#include "mbed.h"
#include "Logger.h"
#include "MQLib.h"
#include "TouchManager.h"
#include "WS281xLedStrip.h"
#include "NVFlash.h"


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

/** Controlador táctil */
static TouchManager* touchm;

/** Controlador de proximidad */
static WS281xLedStrip* leddrv;


/** Callback para las notificaciones de publicación */
static MQ::PublishCallback publ_cb;

/** Callback para las notificaciones de suscripción */
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
// **************************************************************************
// *********** TEST  ********************************************************
// **************************************************************************


#define NUM_LEDS        54
#define OFFSET_SAME     5
#define OFFSET_NEXT     15
#define OFFSET_RED      0
#define OFFSET_GREEN    5
#define OFFSET_BLUE     10

static WS281xLedStrip::Color_t strip[NUM_LEDS];

//------------------------------------------------------------------------------------
static uint8_t wavePoint(uint8_t i, uint8_t max, uint8_t min){
    static const float values[] = {0, 0.08f, 0.12f, 0.25f, 0.33f, 0.65f, 0.75f, 1.0f, 0.75f, 0.65f, 0.33f, 0.25f, 0.12f, 0.08f, 0}; 
    i = (i > 14)? 14 : i;
    int16_t result = (int16_t)(((float)(max - min) * values[i]) + min);
    result = (result > max)? max : result;
    result = (result < min)? min : result;
    return (uint8_t)result;
}

//------------------------------------------------------------------------------------
void app_RGBGame(){
    
    // Inicializa módulo control de FLASH
    NVFlash::init();

    // asigno callbacks mqlib
    publ_cb = callback(&publCallback);
    subsc_cb = callback(&subscCallback);
    
        
    // --------------------------------------
    // Inicia el canal de depuración
    //  - Pines USBTX, USBRX a 115200bps y 256 bytes para buffers
    //  - Configurado por defecto en modo texto
    logger = new Logger(USBTX, USBRX, 16, 115200);
    DEBUG_TRACE("\r\nIniciando app_RGBGame...\r\n");
    
    
    
    // --------------------------------------
    // Creo módulo TouchManager
    DEBUG_TRACE("\r\nNew TouchManager... ");    
    touchm = new TouchManager(PB_7, PB_6, PB_1, 0x1ff);
    touchm->setDebugChannel(logger);
    while(!touchm->ready()){
        Thread::yield();
    }
    DEBUG_TRACE("OK!");    
        
    // establezco topic base 'touch'
    DEBUG_TRACE("\r\n    pub_base  = xrinst/rgbgame/stat/touch\r\n");    
    touchm->setPublicationBase("xrinst/rgbgame/stat/touch");
    

    // --------------------------------------
    // Creo driver de control para los leds
    //  - Dirección I2C = 0h
    //  - Número de leds controlables = NUM_LEDS    
    DEBUG_TRACE("\r\nCreando Driver WS281x...");    
    leddrv = new WS281xLedStrip(PA_8, 800000, NUM_LEDS);
    
        
    // elijo un color base, por ejemplo rojo
    DEBUG_TRACE("\r\nElijo color rojo como color base... ");
    WS281xLedStrip::Color_t color_max;
    color_max.red = 0; color_max.green = 0; color_max.blue = 255;            
    WS281xLedStrip::Color_t color_min;
    color_min.red = 0; color_min.green = 0; color_min.blue = 1;   

    int i;
    for(i=0;i<NUM_LEDS;i++){
        strip[i] = color_min;
        leddrv->applyColor(i, strip[i]);
    }
    // --------------------------------------
    // Inicio aplicación
    DEBUG_TRACE("\r\n ------ APPLICATION RUNNING ------- ");
    
    leddrv->start();

    // continuamente, cada 50ms actualizo el wave propagándolo hacia el final de la tira    
    int8_t point = -1;
    
    for(;;){
        // actualiza el punto de cresta
        point++;
        point = (point >= NUM_LEDS+16)? 0 : point;
        // la primera vez espera 2s y luego actualiza cada 100ms
        if(point==0){
            Thread::wait(2000);
        }
        else{
            Thread::wait(50);
        }
        // propaga colores por la tira
        // disco superior
        for(i=NUM_LEDS-1; i>NUM_LEDS-3; i--){
            strip[i] = strip[i-1];
            strip[i-3] = strip[i-4];
            strip[i-6] = strip[i-7];
        }
        strip[51] = strip[39];
        strip[45] = strip[44];
        strip[48] = strip[34];
        
        // disco medio-sup
        for(i=NUM_LEDS-10; i>NUM_LEDS-14; i--){
            strip[i] = strip[i-1];
            strip[i-5] = strip[i-6];
            strip[i-10] = strip[i-11];
        }
        strip[40] = strip[24];
        strip[30] = strip[29];
        strip[35] = strip[19];
        
        // disco medio-inf
        for(i=NUM_LEDS-25; i>NUM_LEDS-29; i--){
            strip[i] = strip[i-1];
            strip[i-5] = strip[i-6];
            strip[i-10] = strip[i-11];
        }
        strip[15] = strip[14];
        strip[20] = strip[4];
        strip[25] = strip[9];
        
        // disco inferior
        for(i=NUM_LEDS-40; i>NUM_LEDS-44; i--){
            strip[i] = strip[i-1];
            strip[i-5] = strip[i-6];
            strip[i-10] = strip[i-11];
        }
        // calcula el nuevo valor al principio de la tira
        strip[0].red = wavePoint(point, color_max.red, color_min.red);
        strip[0].green = wavePoint(point, color_max.green, color_min.green);
        strip[0].blue = wavePoint(point, color_max.blue, color_min.blue);
        strip[5].red = strip[0].red;
        strip[5].green = strip[0].green;
        strip[5].blue = strip[0].blue;
        strip[10].red = strip[0].red;
        strip[10].green = strip[0].green;
        strip[10].blue = strip[0].blue;
        
        // actualiza la tira
        for(i=0;i<NUM_LEDS;i++){        
            leddrv->applyColor(i, strip[i]);
        }       
    }    
    
}

