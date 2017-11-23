#include "mbed.h"
#include "MQLib.h"
#include "MQSerialBridge.h"
#include "Logger.h"
#include "WS281xLedStrip.h"


// **************************************************************************
// *********** DEFINICIONES *************************************************
// **************************************************************************


/** Macro de impresión de trazas de depuración */
#define DEBUG_TRACE(format, ...)    if(logger){Thread::wait(10); logger->printf(format, ##__VA_ARGS__);}


// **************************************************************************
// *********** OBJETOS  *****************************************************
// **************************************************************************

/** Canal de comunicación remota */
static MQSerialBridge* qserial;
/** Canal de depuración */
static Logger* logger;
/** Driver control detector */
static WS281xLedStrip* leddrv;


// **************************************************************************
// *********** TEST  ********************************************************
// **************************************************************************



//------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------
void test_WS281x(){

            
    // --------------------------------------
    // Inicia el canal de comunicación remota
    //  - Pines USBTX, USBRX a 115200bps y 256 bytes para buffers
    //  - Configurado por defecto en modo texto
    qserial = new MQSerialBridge(USBTX, USBRX, 115200, 256);
    

    // --------------------------------------
    // Inicia el canal de depuración (compartiendo salida remota)
    logger = (Logger*)qserial;    
    DEBUG_TRACE("\r\nIniciando test_WS281x...\r\n");


    // --------------------------------------
    // Creo driver de control para los servos
    //  - Dirección I2C = 0h
    //  - Número de servos controlables = 12 (0 al 11)    
    DEBUG_TRACE("\r\nCreando Driver WS281x...");    
    leddrv = new WS281xLedStrip(PA_8, 800000, 3);
        
    // situo todos a 0º y doy la orden sincronizada
    DEBUG_TRACE("\r\nAjustando colores rojo, verde, azul... ");
    WS281xLedStrip::Color_t color;
    color.red = 255; color.green = 0; color.blue = 0;            
    leddrv->setRange(0, 1, color);
    color.red = 0; color.green = 255; color.blue = 0;            
    leddrv->setRange(1, 2, color);
    color.red = 0; color.green = 0; color.blue = 255;            
    leddrv->setRange(2, 3, color);
    // inicio
    DEBUG_TRACE("\r\nSTART... ");
    leddrv->start();
    for(;;){
        // espero 5 segundos
        Thread::wait(5000);
        DEBUG_TRACE("\r\nCambio a verde mínimo... ");
        color.red = 0; color.green = 1; color.blue = 0;
        leddrv->setRange(0, 3, color);
        Thread::wait(5000);
        DEBUG_TRACE("\r\nCambio a azul cuarto... ");
        color.red = 0; color.green = 0; color.blue = 64;
        leddrv->setRange(0, 3, color);
        Thread::wait(5000);
        DEBUG_TRACE("\r\nCambio a rojo medio... ");
        color.red = 128; color.green = 0; color.blue = 0;     
        leddrv->setRange(0, 3, color);        
    }    
}

