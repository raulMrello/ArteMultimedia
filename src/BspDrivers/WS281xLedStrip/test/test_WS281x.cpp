#include "mbed.h"
#include "MQLib.h"
#include "Logger.h"
#include "WS281xLedStrip.h"


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
/** Driver control detector */
static WS281xLedStrip* leddrv;


// **************************************************************************
// *********** TEST  ********************************************************
// **************************************************************************


#define NUM_LEDS    150

static WS281xLedStrip::Color_t strip[NUM_LEDS];

//------------------------------------------------------------------------------------
uint8_t wavePoint(uint8_t i, uint8_t max, uint8_t min){
    static const float values[] = {0, 0.08f, 0.12f, 0.25f, 0.33f, 0.65f, 0.75f, 1.0f, 0.75f, 0.65f, 0.33f, 0.25f, 0.12f, 0.08f, 0}; 
    i = (i > 14)? 14 : i;
    int16_t result = (int16_t)(((float)(max - min) * values[i]) + min);
    result = (result > max)? max : result;
    result = (result < min)? min : result;
    return (uint8_t)result;
}



//------------------------------------------------------------------------------------
void test_WS281x(){

    uint8_t i;
    
    // --------------------------------------
    // Inicia el canal de comunicación remota
    //  - Pines USBTX, USBRX a 115200bps 
    logger = new Logger(USBTX, USBRX, 16, 115200);
    DEBUG_TRACE("\r\nIniciando test_WS281x...\r\n");


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
    color_min.red = 0; color_min.green = 0; color_min.blue = 4;   

    for(i=0;i<NUM_LEDS;i++){
        strip[i] = color_min;
        leddrv->applyColor(i, strip[i]);
    }
    
    // inicio
    DEBUG_TRACE("\r\nSTART... ");
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
        for(i=NUM_LEDS-1; i>0; i--){
            strip[i] = strip[i-1];
        }
        // calcula el nuevo valor al principio de la tira
        strip[0].red = wavePoint(point, color_max.red, color_min.red);
        strip[0].green = wavePoint(point, color_max.green, color_min.green);
        strip[0].blue = wavePoint(point, color_max.blue, color_min.blue);
        
        // actualiza la tira
        for(i=0;i<NUM_LEDS;i++){        
            leddrv->applyColor(i, strip[i]);
        }       
    }    
}

