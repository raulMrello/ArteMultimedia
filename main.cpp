#include "mbed.h"
#include "MQLib.h"



// **************************************************************************
// *********** APPS *********************************************************
// **************************************************************************

extern void app_Countdown();
extern void app_RGBGame();


// **************************************************************************
// *********** TESTS ********************************************************
// **************************************************************************

extern void test_StateMachine();
extern void test_MQNetBridge();
extern void test_MQLib();
extern void test_TouchManager();
extern void test_ProximityManager();
extern void test_ServoManager();
extern void test_WS281x();
extern void test_HCSR04();
extern void test_PCA9685();
extern void test_MPR121();
//extern void program_ESP8266();

typedef void(*CapabilityCallback)();
static CapabilityCallback capability_list[] = {
	app_RGBGame,
    NULL,
    test_WS281x,
    test_PCA9685,
    app_Countdown,
    test_TouchManager,
    test_MQNetBridge,
    test_StateMachine,
    test_MQLib,
    test_ServoManager,    
    test_ProximityManager,
    test_MPR121,
    test_HCSR04, 
//    program_ESP8266,
    NULL};


// **************************************************************************
// *********** OBJETOS ******************************************************
// **************************************************************************


/** Utilidad para conocer el tamaño en número de elementos de cualquier tipo de array */
template <typename T, size_t N> inline size_t SizeOfArray(const T(&)[N]) { return N; }

/** Lista de tokens MQLib */
//static MQ::Token token_list[] = {
//    "cyber_ribs",
//    "countdown",
//    "breathe",
//    "cmd",
//    "config",
//    "conn",
//    "deg",
//    "disc",
//    "dist",
//    "echo",
//    "ERROR",
//    "info",
//    "invalid",
//    "listen",
//    "lsub",
//    "mov",
//    "move",
//    "mqnetbridge",
//    "mqserialbridge",
//    "mqtt",
//    "prox",
//    "range",
//    "rsub",
//    "runs",
//    "servo",
//    "servo_enable",
//    "servo0",
//    "servo1",
//    "servo2",
//    "servo3",
//    "servo4",
//    "servo5",
//    "servo6",
//    "servo7",
//    "servo8",
//    "servo9",
//    "servo10",
//    "servo11",
//    "sta",
//    "stat",
//    "start",
//    "stop",
//    "test",
//    "touch",
//    "xrinst",
//};





// **************************************************************************
// *********** MAIN *********************************************************
// **************************************************************************

int main() {
        
    // --------------------------------------
    // Arranca el broker con la siguiente configuración:
    //  - Lista de tokens predefinida
    //  - Número máximo de caracteres para los topics: 64 caracteres incluyendo fin de cadena '\0'
    //  - Espera a que esté operativo
    MQ::MQBroker::start(64);
    while(!MQ::MQBroker::ready()){
        Thread::wait(1);
    }
    
    // --------------------------------------
    // Arranca los programas de la lista
    int i=0;
    while(capability_list[i] != NULL){
        capability_list[i]();
        i++;
    }
    
    for(;;){ 
        Thread::wait(osWaitForever);
    }
}

