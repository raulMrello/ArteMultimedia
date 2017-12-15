#include "mbed.h"
#include "MQLib.h"
#include "Logger.h"
#include "MPR121_CapTouch.h"


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
static MPR121_CapTouch* touchdrv;


// **************************************************************************
// *********** TEST  ********************************************************
// **************************************************************************



//------------------------------------------------------------------------------------
static osThreadId th;

//------------------------------------------------------------------------------------
static void driverEventCallback(){
    osSignalSet(th, 1);
}


//------------------------------------------------------------------------------------
void test_MPR121(){

    th = Thread::gettid();
            
    // --------------------------------------
    // Inicia el canal de comunicación remota
    logger = new Logger(USBTX, USBRX, 16, 115200);
    DEBUG_TRACE("\r\nIniciando test_MPR121...\r\n");


    // --------------------------------------
    // Creo driver de control para los canales touch 0 y 1
    //  - Dirección I2C = 5Ah
    //  - Número de canales controlables = 2 (0, 1)    
    DEBUG_TRACE("\r\nCreando Driver MPR121...");    
    touchdrv = new MPR121_CapTouch(PB_7, PB_6, PB_1, 0x03);
    MPR121_CapTouch::Status stat = touchdrv->getState();
    if(stat != MPR121_CapTouch::Ready){
        DEBUG_TRACE("\r\nERROR_stat=%d\r\n", stat);    
        while(1){}
    }
    
    touchdrv->attachIrqCb(callback(&driverEventCallback));
        
    // inicio
    DEBUG_TRACE("OK!\r\nSTART... ");
    uint16_t touch_values = touchdrv->touched();
    uint16_t prev_values = touch_values;
    uint16_t filt_data[2];
    uint16_t baseline[2];
    filt_data[0] = touchdrv->getFilteredData(0);
    filt_data[1] = touchdrv->getFilteredData(1);
    baseline[0] = touchdrv->getBaselineData(0);
    baseline[1] = touchdrv->getBaselineData(1);
    DEBUG_TRACE("OK[Touch=%d, filt0=%d, bsln0=%d, filt1=%d, bsln1=%d]", touch_values, filt_data[0], baseline[0], filt_data[1], baseline[1]);
    for(;;){
        osSignalWait(0, osWaitForever);
        touch_values = touchdrv->touched();
        if(touch_values != prev_values){
            prev_values = touch_values;
            filt_data[0] = touchdrv->getFilteredData(0);
            filt_data[1] = touchdrv->getFilteredData(1);
            baseline[0] = touchdrv->getBaselineData(0);
            baseline[1] = touchdrv->getBaselineData(1);
            DEBUG_TRACE("\r\nEvent[Touch=%d, filt0=%d, bsln0=%d, filt1=%d, bsln1=%d]", touch_values, filt_data[0], baseline[0], filt_data[1], baseline[1]);      
        }
    }    
}

