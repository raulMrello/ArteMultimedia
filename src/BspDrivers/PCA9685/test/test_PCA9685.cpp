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
/** Callbacks de publicación-suscripción */
static MQ::PublishCallback _pc;
static MQ::SubscribeCallback _sc;
/** Número de servos máximo */
static const uint8_t SERVO_COUNT = 12;
/** Control de servos habilitados */
bool enabled[SERVO_COUNT];



// **************************************************************************
// *********** TEST  ********************************************************
// **************************************************************************



//------------------------------------------------------------------------------------
static void pubCb(const char* topic, int32_t result){    
}


//------------------------------------------------------------------------------------
static void subscCb(const char* topic, void* msg, uint16_t msg_len){
    // Procesa comandos...
    if(strcmp(topic, "test/servo_enable/cmd")==0){
        DEBUG_TRACE("\r\nSe solicita activar servos: %s", msg);
        char* servo_id = strtok((char*)msg, ",");
        while(servo_id){
            uint8_t id = atoi(servo_id);
            if(id < SERVO_COUNT && !enabled[id]){
                char topic[16];
                snprintf(topic, 15, "servo%d/#", id);
                MQ::MQClient::subscribe(topic, &_sc);
                DEBUG_TRACE("\r\nSuscrito a topics: %s", topic);
                enabled[id] = true;
            }
            servo_id = strtok(0, ",");
        }
        return;        
    }
    
    if(strstr(topic, "/deg/cmd")!=0){
        char id[3];
        int num = (int)(strchr(topic, '/') - (int)&topic[4]) - 1;
        if(num == 1 || num == 2){
            memcpy(id, &topic[5], num);
            id[num] = 0;
            DEBUG_TRACE("\r\nGirando servo %s a %sº...", id, (char*)msg);        
            if(servodrv->setServoAngle(atoi(id), atoi((char*)msg), true) != PCA9685_ServoDrv::Success){
                DEBUG_TRACE("ERROR!!");
                return;
            }
            DEBUG_TRACE("OK!!");
        }
        else{
            DEBUG_TRACE("\r\nError en el topic %s", topic);        
        }        
        return;
    }
    
    if(strstr(topic, "/mov/cmd")!=0){
        DEBUG_TRACE("\r\nTBD Iniciando trayecto por defecto 180º en 1s");   
        uint8_t duty_servo = 0;
        uint16_t duty_from = servodrv->getDutyFromAngle(duty_servo, 0);
        uint16_t duty_to = servodrv->getDutyFromAngle(duty_servo, 180);
        uint16_t dif = (duty_from < duty_to)? (duty_to - duty_from) : (duty_from - duty_to);
        uint32_t flytime = 1000000/dif;
        do{            
            servodrv->setServoDuty(duty_servo, duty_from, true);
            if(duty_from < duty_to){
                duty_from++;
            }
            else if(duty_from > duty_to){
                duty_from--;
            }
            Thread::wait(flytime/1000);
        }while(duty_from != duty_to);
        servodrv->setServoDuty(duty_servo, duty_from, true);
        DEBUG_TRACE("\r\nServo ha finalizado trayectoria");
        return;
    }
    
    // Procesa solicitud de estados...
    if(strstr(topic, "/deg/sta")!=0){
        char id[3];
        int num = (int)(strchr(topic, '/') - (int)&topic[4]) - 1;
        if(num == 1 || num == 2){
            memcpy(id, &topic[5], num);
            id[num] = 0;
            uint8_t deg = servodrv->getServoAngle(atoi(id));
            DEBUG_TRACE("\r\nServo %s girado a %dº", id, deg);        
        }
        else{
            DEBUG_TRACE("\r\nError en el topic %s", topic);        
        }
        return;
    }
    
    if(strstr(topic, "/servo_enable/sta")!=0){
        bool any = false;
        for(int i=0;i<SERVO_COUNT;i++){
            if(enabled[i]){
                any = true;
                DEBUG_TRACE("\r\nServo %d activado.", i);
            }
        }
        if(!any){
            DEBUG_TRACE("\r\nNingún Servo activado.",);
        }
        return;
    }
}




//------------------------------------------------------------------------------------
void test_PCA9685(){
            
    // --------------------------------------
    // Inicia el canal de comunicación remota
    //  - Pines USBTX, USBRX a 115200bps y 256 bytes para buffers
    //  - Configurado por defecto en modo texto
    qserial = new MQSerialBridge(USBTX, USBRX, 115200, 256);
    logger = (Logger*)qserial;
    DEBUG_TRACE("\r\nIniciando test_PCA9685...\r\n");


    // --------------------------------------
    // Crea suscripciones a los topics para realizar el test
    //  - test/servo_enable/cmd 0,2,3..  Habilita servos para el test (separados por comas del 0 al 11).
    //  - test/servo_enable/sta          Solicita conocer los servos habilitados
    _sc = callback(subscCb);
    _pc = callback(pubCb);    
    MQ::MQClient::subscribe("test/#", &_sc);


    // --------------------------------------
    // Creo driver de control para los servos
    //  - Dirección I2C = 0h
    //  - Número de servos controlables = 12 (0 al 11)    
    DEBUG_TRACE("\r\nCreando Driver PCA9685...");    
    servodrv = new PCA9685_ServoDrv(PB_7, PB_6, SERVO_COUNT, 0);
    
    // espero a que esté listo
    DEBUG_TRACE("\r\n¿Listo?... ");
    do{
        Thread::yield();
    }while(servodrv->getState() != PCA9685_ServoDrv::Ready);
    DEBUG_TRACE(" OK");
    
    // establezco rangos de funcionamiento y marco como deshabilitaos
    DEBUG_TRACE("\r\nAjustando rangos... ");
    for(uint8_t i=0;i<SERVO_COUNT;i++){
        if(servodrv->setServoRanges(i, 0, 180, 1000, 2000) != PCA9685_ServoDrv::Success){
            DEBUG_TRACE("ERR_servo_%d\r\n...", i);
        }            
        enabled[i] = false;
    }
    DEBUG_TRACE("OK");
    
    // situo todos a 0º y doy la orden sincronizada
    DEBUG_TRACE("\r\nGirando servos a 0º... ");
    for(uint8_t i=0;i<SERVO_COUNT;i++){
        if(servodrv->setServoAngle(i, 0) != PCA9685_ServoDrv::Success){
            DEBUG_TRACE("ERR_servo_%d\r\n...", i);
        }               
    }
    if(servodrv->updateAll() != PCA9685_ServoDrv::Success){
        DEBUG_TRACE("ERR_update");
    }                   
    DEBUG_TRACE("OK");
    
    
    // --------------------------------------
    // Arranca el test
    DEBUG_TRACE("\r\n...INICIO DEL TEST...\r\n");    
    DEBUG_TRACE("\r\nComandos MQTT disponibles:");    
    DEBUG_TRACE("\r\n- Activar topic-bridge: mqserialbridge/suscr TOPIC");    
    DEBUG_TRACE("\r\n- Activar servoX:       test/servo_enable/cmd X");    
    DEBUG_TRACE("\r\n- Ver servos activos:   test/servo_enable/sta 0");        
    DEBUG_TRACE("\r\n- Mover servoX Dº:      servoX/deg/cmd D");    
    DEBUG_TRACE("\r\n- Trayecto servoX:      servoX/mov/cmd Di,Df,Tus");    
    DEBUG_TRACE("\r\n- Ver ángulo de servoX: servoX/deg/sta 0");    
    
}

