#include "mbed.h"
#include "MQLib.h"
#include "MQSerialBridge.h"
#include "Logger.h"
#include "HCSR04.h"


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
static HCSR04* distdrv;


// **************************************************************************
// *********** TEST  ********************************************************
// **************************************************************************



//------------------------------------------------------------------------------------
static osThreadId th;
static HCSR04::DistanceEvent g_ev;
static uint16_t g_dist;

//------------------------------------------------------------------------------------
static void distEventCallback(HCSR04::DistanceEvent ev, int16_t dist){
    g_ev = ev;
    g_dist = dist;
    osSignalSet(th, 1);
}




//------------------------------------------------------------------------------------
void test_HCSR04(){

    th = Thread::gettid();
            
    // --------------------------------------
    // Inicia el canal de comunicación remota
    //  - Pines USBTX, USBRX a 115200bps y 256 bytes para buffers
    //  - Configurado por defecto en modo texto
    qserial = new MQSerialBridge(USBTX, USBRX, 115200, 256);
    

    // --------------------------------------
    // Inicia el canal de depuración (compartiendo salida remota)
    logger = (Logger*)qserial;    
    DEBUG_TRACE("\r\nIniciando test_PCA9685...\r\n");


    // --------------------------------------
    // Creo driver de control para los servos
    //  - Dirección I2C = 0h
    //  - Número de servos controlables = 12 (0 al 11)    
    DEBUG_TRACE("\r\nCreando Driver HCSR04...");    
    distdrv = new HCSR04(PA_0, PA_1);
        
    // situo todos a 0º y doy la orden sincronizada
    DEBUG_TRACE("\r\nAjustando distancias a 10cm... ");
    distdrv->config(10, 10, 3);
    
    // inicio
    DEBUG_TRACE("\r\nSTART... ");
    distdrv->start(callback(&distEventCallback), 2000);
    
    for(;;){
        osSignalWait(0, osWaitForever);
        DEBUG_TRACE("\r\nEvent=%d, Dist=%dcm", g_ev, g_dist);
    }    
}

