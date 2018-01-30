#include "mbed.h"
#include "MQLib.h"
#include "MQSerialBridge.h"
#include "Logger.h"
#include "PCA9685_ServoDrv.h"


// **************************************************************************
// *********** DEFINICIONES *************************************************
// **************************************************************************


/** Macro de impresi�n de trazas de depuraci�n */
#define DEBUG_TRACE(format, ...)    if(logger){logger->printf(format, ##__VA_ARGS__);}


// **************************************************************************
// *********** OBJETOS  *****************************************************
// **************************************************************************

/** Canal de comunicaci�n remota */
static MQSerialBridge* qserial;
static Logger* logger;

/** Driver control de servos */
static PCA9685_ServoDrv* servodrv;
/** N�mero de servos m�ximo */
static const uint8_t SERVO_COUNT = 3;



// **************************************************************************
// *********** TEST  ********************************************************
// **************************************************************************




//------------------------------------------------------------------------------------
void test_PCA9685(){
            
    // --------------------------------------
    // Inicia el canal de comunicaci�n remota
    //  - Pines USBTX, USBRX a 115200bps y 256 bytes para buffers
    //  - Configurado por defecto en modo texto
    qserial = new MQSerialBridge(USBTX, USBRX, 115200, 256);
    logger = (Logger*)qserial;
    DEBUG_TRACE("\r\nIniciando test_PCA9685...\r\n");



    // --------------------------------------
    // Creo driver de control para los servos
    //  - Direcci�n I2C = 0h
    //  - N�mero de servos controlables = 12 (0 al 11)    
    DEBUG_TRACE("\r\nCreando Driver PCA9685...");    
    servodrv = new PCA9685_ServoDrv(PB_4, PA_7, SERVO_COUNT, 0);
    
    // espero a que est� listo
    DEBUG_TRACE("\r\n�Listo?... ");
    while(servodrv->getState() != PCA9685_ServoDrv::Ready){
		DEBUG_TRACE("\r\n ERR_STAT %d", servodrv->getState());
        servodrv->init();
    }
    DEBUG_TRACE(" OK");
    
    // establezco rangos de funcionamiento y marco como deshabilitaos
    DEBUG_TRACE("\r\nAjustando rangos... ");
    for(uint8_t i=0;i<SERVO_COUNT;i++){
        if(servodrv->setServoRanges(i, 0, 180, 1000, 2000) != PCA9685_ServoDrv::Success){
            DEBUG_TRACE("ERR_servo_%d\r\n...", i);
        }            
    }
    DEBUG_TRACE("OK");
    
    // situo todos a 0� y doy la orden sincronizada
    DEBUG_TRACE("\r\nGirando servos a 0�... ");
    for(uint8_t i=0;i<SERVO_COUNT;i++){
        if(servodrv->setServoAngle(i, 0) != PCA9685_ServoDrv::Success){
            DEBUG_TRACE("ERR_servo_%d\r\n...", i);
        }               
    }
    if(servodrv->updateAll() != PCA9685_ServoDrv::Success){
        DEBUG_TRACE("ERR_update");
    }                   
    DEBUG_TRACE("OK");
	
	Thread::wait(2000);
	
    // situo todos a 40� y doy la orden sincronizada
    DEBUG_TRACE("\r\nGirando servos a 40�... ");
    for(uint8_t i=0;i<SERVO_COUNT;i++){
        if(servodrv->setServoAngle(i, 40) != PCA9685_ServoDrv::Success){
            DEBUG_TRACE("ERR_servo_%d\r\n...", i);
        }               
    }
    if(servodrv->updateAll() != PCA9685_ServoDrv::Success){
        DEBUG_TRACE("ERR_update");
    }                   
    DEBUG_TRACE("OK");
	
	Thread::wait(2000);
	
    // situo todos a 80� y doy la orden sincronizada
    DEBUG_TRACE("\r\nGirando servos a 80�... ");
    for(uint8_t i=0;i<SERVO_COUNT;i++){
        if(servodrv->setServoAngle(i, 80) != PCA9685_ServoDrv::Success){
            DEBUG_TRACE("ERR_servo_%d\r\n...", i);
        }               
    }
    if(servodrv->updateAll() != PCA9685_ServoDrv::Success){
        DEBUG_TRACE("ERR_update");
    }                   
    DEBUG_TRACE("OK");
	
	Thread::wait(2000);
	
    // situo todos a 0� y doy la orden sincronizada
    DEBUG_TRACE("\r\nGirando servos a 0�... ");
    for(uint8_t i=0;i<SERVO_COUNT;i++){
        if(servodrv->setServoAngle(i, 0) != PCA9685_ServoDrv::Success){
            DEBUG_TRACE("ERR_servo_%d\r\n...", i);
        }               
    }
    if(servodrv->updateAll() != PCA9685_ServoDrv::Success){
        DEBUG_TRACE("ERR_update");
    }                   
    DEBUG_TRACE("OK");
   
    DEBUG_TRACE("\r\n...FIN DEL TEST...\r\n");    
}

