#include "mbed.h"
#include "MQLib.h"
#include "MQSerialBridge.h"
#include "Logger.h"
#include "PCA9685_ServoDrv.h"


// **************************************************************************
// *********** DEFINICIONES *************************************************
// **************************************************************************


/** Macro de impresión de trazas de depuración */
#define DEBUG_TRACE(format, ...)    if(logger){logger->printf(format, ##__VA_ARGS__);}


// **************************************************************************
// *********** OBJETOS  *****************************************************
// **************************************************************************

/** Canal de comunicación remota */
static MQSerialBridge* qserial;
static Logger* logger;

/** Driver control de servos */
static PCA9685_ServoDrv* servodrv;
/** Número de servos máximo */
static const uint8_t SERVO_COUNT = 3;



// **************************************************************************
// *********** TEST  ********************************************************
// **************************************************************************




//------------------------------------------------------------------------------------
void test_PCA9685(){
            
    DEBUG_TRACE("\r\nTest PCA9685_ServoDrv...\r\n");
    DEBUG_TRACE("\r\nFunciones:");
    DEBUG_TRACE("\r\n\t[ ] - Constructor");
    DEBUG_TRACE("\r\n\t[ ] - getState");
    DEBUG_TRACE("\r\n\t[ ] - setServoRanges");
    DEBUG_TRACE("\r\n\t[ ] - getServoRanges");
    DEBUG_TRACE("\r\n\t[ ] - setServoAngle");
    DEBUG_TRACE("\r\n\t[ ] - getServoAngle");
    DEBUG_TRACE("\r\n\t[ ] - setServoDuty");
    DEBUG_TRACE("\r\n\t[ ] - getServoDuty");
    DEBUG_TRACE("\r\n\t[ ] - updateAll");
    DEBUG_TRACE("\r\n\t[ ] - readServoDuty");
    DEBUG_TRACE("\r\n\t[ ] - getDutyFromAngle");
    DEBUG_TRACE("\r\n\t[ ] - getAngleFromDuty");
    DEBUG_TRACE("\r\n\t[ ] - getNVDataSize");
    DEBUG_TRACE("\r\n\t[ ] - setNVData");
    DEBUG_TRACE("\r\n\t[ ] - getNVData");

    DEBUG_TRACE("\r\n\r\nIniciando test_PCA9685...\r\n");
    
    // --------------------------------------
    // Inicia el canal de comunicación remota
    //  - Pines USBTX, USBRX a 115200bps y 256 bytes para buffers
    //  - Configurado por defecto en modo texto
    qserial = new MQSerialBridge(USBTX, USBRX, 115200, 256);
    logger = (Logger*)qserial;


    // --------------------------------------
    // Creo driver de control para los servos
    //  - Dirección I2C = 0h
    //  - Número de servos controlables = 12 (0 al 11)    
    DEBUG_TRACE("\r\nCreando Driver PCA9685...");    
    servodrv = new PCA9685_ServoDrv(PB_4, PA_7, SERVO_COUNT, 0);
    MBED_ASSERT(servodrv);
    DEBUG_TRACE("\r\n\t[x] - Constructor");
    
    
    // espero a que esté listo
    DEBUG_TRACE("\r\n¿Listo?... ");
    do{
		Thread::wait(1);
    }while(servodrv->getState() != PCA9685_ServoDrv::Ready);
    DEBUG_TRACE("\r\n\t[x] - getState");
   
    
    // establezco rangos de funcionamiento y marco como deshabilitaos
    DEBUG_TRACE("\r\nAjustando rangos... ");
    for(uint8_t i=0;i<SERVO_COUNT;i++){
        if(servodrv->setServoRanges(i, 0, 180, 1000, 2000) != PCA9685_ServoDrv::Success){
            DEBUG_TRACE("\r\nERR_servo_%d", i);
        }            
        else{
            int16_t min_angle, max_angle;
            uint16_t min_duty, max_duty;
            servodrv->getServoRanges(i, &min_angle, &max_angle, &min_duty, &max_duty);
            DEBUG_TRACE("\r\nSRV_RANGE %d, minAng=%d, maxAng=%d, minDuty=%d, maxDuty=%d", i, min_angle, max_angle, min_duty, max_duty);
        }
    }
    DEBUG_TRACE("\r\nOK");
    DEBUG_TRACE("\r\n\t[x] - setServoRanges");
    DEBUG_TRACE("\r\n\t[x] - getServoRanges");
        
    
    // situo todos a 0º y doy la orden sincronizada
    DEBUG_TRACE("\r\nGirando servos a 10º cada segundo... ");
    for(int s=0;s<=180;s=s+45){
        for(uint8_t i=0;i<SERVO_COUNT;i++){
            if(servodrv->setServoAngle(i, s) != PCA9685_ServoDrv::Success){
                DEBUG_TRACE("\r\nERR_servo_%d", i);
            }   
            else{
                DEBUG_TRACE("\r\nSRV_ANGLE %d angle=%dº", i, servodrv->getServoAngle(i));
                DEBUG_TRACE("\r\nSRV_ANGLE %d duty=%dº", i, servodrv->getServoDuty(i));   
                DEBUG_TRACE("\r\nSRV_AfromD %d angle=%dº from duty=%d", i, servodrv->getAngleFromDuty(i, servodrv->getServoDuty(i)), servodrv->getServoDuty(i));                
                DEBUG_TRACE("\r\nSRV_DfromA %d duty=%d from angle=%dº", i, servodrv->getDutyFromAngle(i, servodrv->getServoAngle(i)), servodrv->getServoAngle(i));                
                DEBUG_TRACE("\r\n\t[x] - setServoAngle");
                DEBUG_TRACE("\r\n\t[x] - getServoAngle"); 
                DEBUG_TRACE("\r\n\t[x] - getServoDuty");                    
                DEBUG_TRACE("\r\n\t[x] - getDutyFromAngle");
                DEBUG_TRACE("\r\n\t[x] - getAngleFromDuty");
            }
        }
        if(servodrv->updateAll() != PCA9685_ServoDrv::Success){
            DEBUG_TRACE("\r\nERR_update");
        }                
        else{
            DEBUG_TRACE("\r\n\t[x] - updateAll");
            for(uint8_t i=0;i<SERVO_COUNT;i++){
                uint16_t duty;
                servodrv->readServoDuty(i, &duty);
                DEBUG_TRACE("\r\nREAD_DUTY %d duty=%dº", i, duty);
                DEBUG_TRACE("\r\n\t[x] - readServoDuty");
            }
        }       
     }
    DEBUG_TRACE("\r\nNV_DATASIZE = %d", servodrv->getNVDataSize()); 
    DEBUG_TRACE("\r\n\t[x] - getNVDataSize");        
    
    PCA9685_ServoDrv::NVData_t* nvdata = (PCA9685_ServoDrv::NVData_t*)Heap::memAlloc(sizeof(PCA9685_ServoDrv::NVData_t*));
    MBED_ASSERT(nvdata);
    servodrv->getNVData(nvdata);
    for(uint8_t i=0;i<SERVO_COUNT;i++){
        DEBUG_TRACE("\r\nNV_GET servo %d, minAng=%d, maxAng=%d, minDuty=%d, maxDuty=%d, crc=%d", i, nvdata->minAngle[i], nvdata->maxAngle[i], nvdata->minDuty[i], nvdata->maxDuty[i]);
    }
    DEBUG_TRACE("\r\n\t[x] - getNVData"); 
    if(servodrv->setNVData(nvdata) == 0){
        DEBUG_TRACE("\r\n\t[x] - setNVData"); 
    }	
   
    DEBUG_TRACE("\r\n...FIN DEL TEST...\r\n");    
}

