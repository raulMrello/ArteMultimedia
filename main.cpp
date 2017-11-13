#include "mbed.h"
#include "MQLib.h"



// **************************************************************************
// *********** TESTS ********************************************************
// **************************************************************************

extern void test_ServoManager();

typedef void(*TestCallback)();
static TestCallback test_list[] = {
    test_ServoManager, 
    NULL};


// **************************************************************************
// *********** OBJETOS ******************************************************
// **************************************************************************


/** Utilidad para conocer el tamaño en número de elementos de cualquier tipo de array */
template <typename T, size_t N> inline size_t SizeOfArray(const T(&)[N]) { return N; }

/** Lista de tokens MQLib */
static MQ::Token token_list[] = {
    "mqserialbridge",
    "cmd",
    "sta",
    "info",
    "test",
    "servo_enable",
    "servo0",
    "servo1",
    "servo2",
    "servo3",
    "servo4",
    "servo5",
    "servo6",
    "servo7",
    "servo8",
    "servo9",
    "servo10",
    "servo11",
    "deg",
    "mov",
    "breathe",
    "move",
    "servo",
    "start",
    "stop",
};




// **************************************************************************
// *********** MAIN *********************************************************
// **************************************************************************

int main() {
    // --------------------------------------
    // Arranca el broker con la siguiente configuración:
    //  - Lista de tokens predefinida
    //  - Número máximo de caracteres para los topics: 64 caracteres incluyendo fin de cadena '\0'
    //  - Espera a que esté operativo
    MQ::MQBroker::start(token_list, SizeOfArray(token_list), 64);
    while(!MQ::MQBroker::ready()){
        Thread::yield();
    }
    
    // --------------------------------------
    // Arranca los tests
    int i=0;
    while(test_list[i] != NULL){
        test_list[i]();
        i++;
    }
    
    for(;;){ Thread::yield(); }
}





// **************************************************************************
// *********** TEST DMA_SPI *************************************************
// **************************************************************************

//#include "DMA_SPI.h"

//DMA_SPI* spi;

//struct Color_t{
//    uint8_t green;
//    uint8_t red;
//    uint8_t blue;
//};
//static const Color_t red_color = {0, 255, 0};
//static const Color_t green_color = {255, 0, 0};
//static const Color_t blue_color = {0, 0, 255};
//static const Color_t white_color = {255, 255, 255};
//static const Color_t test_color = {0, 0, 0};

//__packed struct ColorBits_t{
//    uint32_t green[8];
//    uint32_t red[8];
//    uint32_t blue[8];
//};

//static const uint16_t ROW_COUNT = 2;
//__packed struct ColorBuffer_t{
////    uint32_t resetcode[50];
//    ColorBits_t row[ROW_COUNT];
//};

//static ColorBuffer_t color_buffer __attribute__((aligned(32)));

//void setColorAt(const Color_t* color, uint16_t pos, uint32_t bitAt1, uint32_t bitAt0){    
//    for(uint8_t i=0;i<32;i++){
//        color_buffer.row[pos].green[31-i] = ((color->green & (1 << (31-i))) != 0)? bitAt1 : bitAt0;
//    }
//    for(uint8_t i=0;i<32;i++){
//        color_buffer.row[pos].red[31-i] = ((color->red & (1 << (31-i))) != 0)? bitAt1 : bitAt0;
//    }
//    for(uint8_t i=0;i<32;i++){
//        color_buffer.row[pos].blue[31-i] = ((color->blue & (1 << (31-i))) != 0)? bitAt1 : bitAt0;
//    }
//}

//osThreadId thTestSpi;

//void onDmaHalfIsr(){
//    osSignalSet(thTestSpi, 1);
//}
//void onDmaCpltIsr(){
//    osSignalSet(thTestSpi, 2);
//}
//void onDmaErrIsr(DMA_SPI::ErrorResult err){
//    osSignalSet(thTestSpi, 4);
//}

//int test_SpiDma(){
//    Callback<void()> half_isr = callback(onDmaHalfIsr);
//    Callback<void()> cplt_isr = callback(onDmaCpltIsr);
//    Callback<void(DMA_SPI::ErrorResult)> err_isr = callback(onDmaErrIsr);
//    Color_t* curr_color = (Color_t*)&test_color;
//    // Inicio el puerto spi1 a 6.4MHz (= 800Kbps)
//    spi = new DMA_SPI(6400000, PA_7, PA_6, PA_5);    
//    
//    // obtengo referencia al hilo de ejecución principal
//    thTestSpi = Thread::gettid();

//    // Preparo buffer de color
//    memset(&color_buffer, 0, sizeof(ColorBuffer_t));
//    
//    // Relleno el array de 100 leds a rojo
//    for(int i=0;i<ROW_COUNT;i++){
//        setColorAt(curr_color, i, 64, 32);
//    }

//    
//    // Inicio la operación vía DMA
//    spi->transmit((uint8_t*)&color_buffer, sizeof(ColorBuffer_t), half_isr, cplt_isr, err_isr);
//    
//    for(;;){
//        // espero resultados...
//        osEvent evt = osSignalWait(0, osWaitForever);
//        if(evt.status == osEventSignal){   
//            uint32_t sig = evt.value.signals;
//            // repinto la primera mitad
//            if((sig & 1)!=0){
////                if(curr_color == &red_color){
////                    curr_color = (Color_t*)&green_color;
////                }
////                else if(curr_color == &green_color){
////                    curr_color = (Color_t*)&blue_color;
////                }
////                else if(curr_color == &blue_color){
////                    curr_color = (Color_t*)&red_color;
////                }
////                for(int i=0;i<(ROW_COUNT/2);i++){                    
////                    setColorAt(curr_color, i);
////                }                
//            }
//            // repinto la segunda mitad e inicio de nuevo
//            if((sig & 2)!=0){
////                for(int i=(ROW_COUNT/2);i<ROW_COUNT;i++){
////                    setColorAt(curr_color, i);
////                }                
//                Thread::wait(1);
//                spi->transmit((uint8_t*)&color_buffer, sizeof(ColorBuffer_t), half_isr, cplt_isr, err_isr);
//            }
//            // pinto de blanco
//            if((sig & 4)!=0){
////                for(int i=0;i<ROW_COUNT;i++){
////                    setColorAt(&white_color, i);
////                }                
//                Thread::wait(1);
//                spi->transmit((uint8_t*)&color_buffer, sizeof(ColorBuffer_t), half_isr, cplt_isr, err_isr);
//            }
//        }
//    }
//}


// **************************************************************************
// *********** TEST DMA_PwmOut **********************************************
// **************************************************************************

//#include "DMA_PwmOut.h"

//DMA_PwmOut* pwm;


//osThreadId thTestPwm; 
//uint32_t data_buffer[2400];

//int test_DMA_PwmOut(){
//    
//    // Inicio el puerto pwm en PA_10 (TIM1_CH3, DMA1_Ch7) con periodo de a 800KHz (25ns * 1250)
//    pwm = new DMA_PwmOut(PA_10, 800000);    
//    
//    // obtengo referencia al hilo de ejecución principal
//    thTestPwm = Thread::gettid(); 
//    
//    // Preparo el duty para enviar bits a 0 y a 1.
//    uint32_t ws2812_bit1 = pwm->getTickPercent(64);
//    uint32_t ws2812_bit0 = pwm->getTickPercent(32);
//    
//    // Relleno el array de 100 leds a rojo
//    for(int i=0;i<50;i++){
//        data_buffer[i] = 0;
//    }
//    for(int i=50;i<2400;i++){
//        data_buffer[i] = ws2812_bit0;
//    }
//    
//    // Inicio la operación vía DMA
//    pwm->dmaStart(data_buffer, 2400);
//    
//    for(;;){
//        // Tras 10seg la detengo
//        Thread::wait(10000);
//        pwm->dmaStop();
//        // Tras 10seg la reinicio
//        Thread::wait(10000);
//        pwm->dmaStart(data_buffer, 2400);
//    }
//}



// **************************************************************************
// *********** TEST WS281xLedStrip ******************************************
// **************************************************************************
//#include "WS281xLedStrip.h"

//static const WS281xLedStrip::Color_t red = { .red   = 255, 
//                                             .green = 0, 
//                                             .blue  = 0};
//static const WS281xLedStrip::Color_t test = {.red   = 0x55, 
//                                             .green = 0x55, 
//                                             .blue  = 0x55};


//static WS281xLedStrip* led_strip;

//void test_WS281xLedStrip(){
//    led_strip = new WS281xLedStrip(PA_10, 800000, 20);
//    led_strip->setRange(0, 19, test);
//    led_strip->start();
//    for(;;){
//        Thread::wait(5000);
//        led_strip->stop();
//        Thread::wait(5000);
//        led_strip->start();
//    }    
//}




// **************************************************************************
// *********** TEST HCSR04 **************************************************
// **************************************************************************
//#include "HCSR04.h"
//HCSR04* usound;

//void onDistEvent(HCSR04::DistanceEvent ev, uint16_t distance){
//    switch(ev){
//        case HCSR04::Approaching:{
//            DEBUG_TRACE("Acercándose a %d cm\r\n", distance);
//            break;
//        }        
//        case HCSR04::MovingAway:{
//            DEBUG_TRACE("Alejándose a %d cm\r\n", distance);
//            break;
//        }
//        case HCSR04::MeasureError:{
//            DEBUG_TRACE("Error en la medida\r\n");
//            break;
//        }
//        
//    }
//}

//void test_HCSR04(){
//    usound = new HCSR04(PA_8, PA_11);
//    usound->setDistRange(20, 20);
//    usound->start(callback(onDistEvent), 2000);
//    
//}




// **************************************************************************
// *********** TEST PCA9685 *************************************************
// **************************************************************************
//#include "PCA9685_ServoDrv.h"
//PCA9685_ServoDrv* servodrv;

//void test_PCA9685(){
//    uint8_t SERVO_COUNT = 12;
//    uint8_t ADDR = 0;
//    // creo driver
//    servodrv = new PCA9685_ServoDrv(PB_7, PB_6, SERVO_COUNT, ADDR);
//    
//    // espero a que esté listo
//    do{
//        Thread::yield();
//        }while(servodrv->getState() != PCA9685_ServoDrv::Ready);
//    
//    // establezco rangos de funcionamiento
//    for(uint8_t i=0;i<SERVO_COUNT;i++){
//        servodrv->setServoRanges(i, 0, 180, 1000, 2000);    
//    }
//    
//    // situo todos a 45º y doy la orden sincronizada
//    for(uint8_t i=0;i<SERVO_COUNT;i++){
//        servodrv->setServoAngle(i, 45);    
//    }
//    servodrv->updateAll();
//    
//    // situo el 2º servo a 90º
//    servodrv->setServoAngle(1, 90, true);    

//    for(;;){
//    }
//}





// **************************************************************************
// *********** TEST MPR121 *************************************************
// **************************************************************************
//#include "TouchManager.h"
//TouchManager* touch;

///** Procesa eventos touch mediante callback dedicada */
//void onTouchEvent(TouchManager::TouchMsg* msg){
//    DEBUG_TRACE("\r\nTouchEvt elec=%d, evt=%d", msg->elec, msg->evt);
//}

///** Procesa eventos touch mediante suscripción al topic "touch" */
//void onTouchTopic(const char* topic, void* msg, uint16_t msg_len){    
//    DEBUG_TRACE("\r\nTouchMsg elec=%d, evt=%d", ((TouchManager::TouchMsg*)msg)->elec, ((TouchManager::TouchMsg*)msg)->evt);
//}

//void test_MPR121(){
//    // Arranca el broker con la siguiente configuración:
//    //  - Lista de tokens predefinida
//    //  - Número máximo de caracteres para los topics: 64 caracteres incluyendo fin de cadena '\0'
//    //  - Espera a que esté operativo
//    MQ::MQBroker::start(token_list, SizeOfArray(token_list), 64);
//    while(!MQ::MQBroker::ready()){
//        Thread::yield();
//    }
//    
//    // Crea suscripción a mensajes del topic "touch"
//    MQ::MQClient::subscribe("touch", new MQ::SubscribeCallback(onTouchTopic));
//    
//    // Crea el manejador del driver MPR121 de alto nivel e instala callback y publicación de topics
//    touch = new TouchManager(PB_7, PB_6, PB_0);
//    touch->attachCallback(callback(onTouchEvent));
//    touch->attachTopics("touch");
//    
//    // Espero eventos...
//    DEBUG_TRACE("\r\nEsperando eventos ...");
//    for(;;){
//        Thread::yield();
//    }
//}






//// **************************************************************************
//// *********** TEST ESP8266MQTT *********************************************
//// **************************************************************************


//#define logMessage printf
//#define MQTTCLIENT_QOS2 1

//#include "easy-connect.h"
//#include "MQTTNetwork.h"
//#include "MQTTmbed.h"
//#include "MQTTClient.h"

//int arrivedcount = 0;

//void messageArrived(MQTT::MessageData& md){
//    MQTT::Message &message = md.message;
//    logMessage("Message arrived: qos %d, retained %d, dup %d, packetid %d\r\n", message.qos, message.retained, message.dup, message.id);
//    logMessage("Payload %.*s\r\n", message.payloadlen, (char*)message.payload);
//    ++arrivedcount;
//}


//int test_ESP8266MQTT(){
//    float version = 0.6;
//    char* topic = "mbed-sample";

//    logMessage("HelloMQTT: version is %.2f\r\n", version);

//    NetworkInterface* network = easy_connect(true);
//    if (!network) {
//        return -1;
//    }

//    MQTTNetwork mqttNetwork(network);

//    MQTT::Client<MQTTNetwork, Countdown> client(mqttNetwork);

//    const char* hostname = "m2m.eclipse.org";
//    int port = 1883;
//    logMessage("Connecting to %s:%d\r\n", hostname, port);
//    int rc = mqttNetwork.connect(hostname, port);
//    if (rc != 0)
//        logMessage("rc from TCP connect is %d\r\n", rc);

//    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
//    data.MQTTVersion = 3;
//    data.clientID.cstring = "mbed-sample";
//    data.username.cstring = "testuser";
//    data.password.cstring = "testpassword";
//    if ((rc = client.connect(data)) != 0)
//        logMessage("rc from MQTT connect is %d\r\n", rc);

//    if ((rc = client.subscribe(topic, MQTT::QOS2, messageArrived)) != 0)
//        logMessage("rc from MQTT subscribe is %d\r\n", rc);

//    MQTT::Message message;

//    // QoS 0
//    char buf[100];
//    sprintf(buf, "Hello World!  QoS 0 message from app version %f\r\n", version);
//    message.qos = MQTT::QOS0;
//    message.retained = false;
//    message.dup = false;
//    message.payload = (void*)buf;
//    message.payloadlen = strlen(buf)+1;
//    rc = client.publish(topic, message);
//    while (arrivedcount < 1)
//        client.yield(100);

//    // QoS 1
//    sprintf(buf, "Hello World!  QoS 1 message from app version %f\r\n", version);
//    message.qos = MQTT::QOS1;
//    message.payloadlen = strlen(buf)+1;
//    rc = client.publish(topic, message);
//    while (arrivedcount < 2)
//        client.yield(100);

//    // QoS 2
//    sprintf(buf, "Hello World!  QoS 2 message from app version %f\r\n", version);
//    message.qos = MQTT::QOS2;
//    message.payloadlen = strlen(buf)+1;
//    rc = client.publish(topic, message);
//    while (arrivedcount < 3)
//        client.yield(100);

//    if ((rc = client.unsubscribe(topic)) != 0)
//        logMessage("rc from unsubscribe was %d\r\n", rc);

//    if ((rc = client.disconnect()) != 0)
//        logMessage("rc from disconnect was %d\r\n", rc);

//    mqttNetwork.disconnect();

//    logMessage("Version %.2f: finish %d msgs\r\n", version, arrivedcount);

//    return 0;
//}







// **************************************************************************
// *********** TEST StateMachine ********************************************
// **************************************************************************

//#include "StateMachine.h"

//class MySM : public StateMachine{
//public:    
//    MySM() : StateMachine(){
//        _stInit.setHandler(callback(this, &MySM::Init_EventHandler));
//        _stNext.setHandler(callback(this, &MySM::Next_EventHandler));        
//    }
//    
//    void start(){
//        initState(&_stInit, Thread::gettid());        
//        // Ejecuta máquinas de estados
//        for(;;){
//            osEvent oe = Thread::signal_wait(0, osWaitForever);        
//            run(&oe);
//        }
//    }

//protected:
//    State _stInit;
//    State::StateResult Init_EventHandler(State::StateEvent* se){
//        switch(se->evt){          
//            case State::EV_ENTRY:{
//                DEBUG_TRACE("\r\nInit ENTRY");
//                Thread::wait(1000);
//                raiseEvent(State::EV_RESERVED_USER, Thread::gettid());
//                raiseEvent((State::EV_RESERVED_USER+1), Thread::gettid());
//                return State::HANDLED;                
//            }
//            case State::EV_RESERVED_USER:{
//                DEBUG_TRACE("\r\nEvt USER_0");
//                return State::HANDLED;                
//            }
//            case (State::EV_RESERVED_USER+1):{
//                DEBUG_TRACE("\r\nEvt USER_1");
//                Thread::wait(1000);
//                // Conmuta a modo Next
//                tranState(&_stNext, Thread::gettid());
//                return State::HANDLED;                
//            }
//            case State::EV_EXIT:{
//                DEBUG_TRACE("\r\nInit EXIT");
//                nextState();
//                return State::HANDLED;
//            }
//        }
//        return State::IGNORED; 
//    }

//    State _stNext;
//    State::StateResult Next_EventHandler(State::StateEvent* se){
//        switch(se->evt){          
//            case State::EV_ENTRY:{
//                DEBUG_TRACE("\r\nNext ENTRY");
//                // Conmuta a modo Init
//                tranState(&_stInit, Thread::gettid());
//                return State::HANDLED;                
//            }
//            
//            case State::EV_EXIT:{
//                DEBUG_TRACE("\r\nNext EXIT");
//                nextState();
//                return State::HANDLED;
//            }
//        }
//        return State::IGNORED; 
//    }      
//};

//void test_StateMachine(){
//    MySM* sm = new MySM();
//    sm->start();
//    for(;;){
//        Thread::yield();
//    }
//}




// **************************************************************************
// *********** TEST MQSerialBridge ******************************************
// **************************************************************************

//#include "MQSerialBridge.h"
//MQSerialBridge* qserial;

//static MQ::SubscribeCallback _sc;
//static MQ::PublishCallback _pc;

//static void pubCb(const char* topic, int32_t result){
//    DEBUG_TRACE("\r\nPublicado en %s", topic);
//}
//static void subscCb(const char* topic, void* msg, uint16_t msg_len){
//    if(strncmp(topic, "topic0", strlen("topic0"))==0){
//        MQ::MQClient::publish("topic1", msg, msg_len, &_pc);
//    }
//    if(strncmp(topic, "topic1", strlen("topic1"))==0){
//        MQ::MQClient::publish("topic0", msg, msg_len, &_pc);
//    }
//}

//static void test_MQSerialBridge(){
//    
//    
//    // Arranca el broker con la siguiente configuración:
//    //  - Lista de tokens predefinida
//    //  - Número máximo de caracteres para los topics: 64 caracteres incluyendo fin de cadena '\0'
//    //  - Espera a que esté operativo
//    MQ::MQBroker::start(token_list, SizeOfArray(token_list), 64);
//    while(!MQ::MQBroker::ready()){
//        Thread::yield();
//    }
//    
//    _sc = callback(subscCb);
//    _pc = callback(pubCb);
//    qserial = new MQSerialBridge(USBTX, USBRX, 115200, 256);
//    logger = (Logger*)qserial;
//    
//    DEBUG_TRACE("\r\nIniciando test...\r\n");

//    
//    DEBUG_TRACE("\r\nMQBroker listo!");
//    
//    // se suscribe a los topics "topic0" en los que publicará a topic1
//    MQ::MQClient::subscribe("topic0", &_sc);
//    
//    DEBUG_TRACE("\r\nSuscripciones hechas!");
//    
//    for(;;){
//        Thread::yield();
//    }
//}



