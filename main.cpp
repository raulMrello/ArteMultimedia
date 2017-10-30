#include "mbed.h"
#include "MQLib.h"


DigitalOut led1(LED3);



// **************************************************************************
// *********** TEST MQLIB ***************************************************
// **************************************************************************



//-----------------------------------------------------------------------------
/** Utilidad para conocer el tamaño en número de elementos de cualquier tipo de array */
template <typename T, size_t N>
inline
size_t SizeOfArray( const T(&)[ N ] )
{
  return N;
}

//-----------------------------------------------------------------------------
/** Lista de tokens proporcionados */
static MQ::Token token_list[] = {
    "topic0",
    "topic1",
};

//-----------------------------------------------------------------------------
static void onPublished(const char* topic, int32_t result){
    if(strchr(topic, '0') != 0){
        led1 = 0;
    }
    if(strchr(topic, '1') != 0){
        led1 = 1;
    }
}

//-----------------------------------------------------------------------------
class Test{
public:
	Test(){
		strcpy(_topic, "");
		_suscrCb = callback(this, &Test::subscriptionCb);		
	}
	void task(){
        MQ::MQClient::subscribe("topic0", &_suscrCb);
		MQ::MQClient::subscribe("topic1", &_suscrCb);
	}
	MQ::SubscribeCallback _suscrCb;
	
protected:

	void subscriptionCb(const char* topic, void* msg, uint16_t msg_len){
		strncpy(_topic, topic, 15);
	}
	char _topic[16];
};

//-----------------------------------------------------------------------------
static Test* t;

//-----------------------------------------------------------------------------
int test_MQlib(){    
	t = new Test();
	Thread th;
               
    // Arranca el broker con la siguiente configuración:
    //  - Lista de tokens predefinida
    //  - Número máximo de caracteres para los topics: 64 caracteres incluyendo fin de cadena '\0'
    //  - Espera a que esté operativo
    MQ::MQBroker::start(token_list, SizeOfArray(token_list), 64);
    while(!MQ::MQBroker::ready()){
        Thread::yield();
    }

    
	MQ::PublishCallback pc = callback(onPublished);
		
	// Inicia la tarea paralela
	th.start(callback(t, &Test::task));
	
	Thread::wait(1000);
	
	char data = 0;
	MQ::MQClient::publish("topic0", &data, sizeof(char), &pc);
	data = 4;
	while (true) {
        led1 = !led1;
        wait(0.5);
        MQ::MQClient::publish("topic1", &data, sizeof(char), &pc);
    }
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
#include "WS281xLedStrip.h"

static const WS281xLedStrip::Color_t red = { .red   = 255, 
                                             .green = 0, 
                                             .blue  = 0};
static const WS281xLedStrip::Color_t test = {.red   = 0x55, 
                                             .green = 0x55, 
                                             .blue  = 0x55};


static WS281xLedStrip* led_strip;

void test_WS281xLedStrip(){
    led_strip = new WS281xLedStrip(PA_10, 800000, 20);
    led_strip->setRange(0, 19, test);
    led_strip->start();
    for(;;){
        Thread::wait(5000);
        led_strip->stop();
        Thread::wait(5000);
        led_strip->start();
    }    
}




// **************************************************************************
// *********** TEST HCSR04 **************************************************
// **************************************************************************
#include "HCSR04.h"
HCSR04* usound;

void onDistEvent(HCSR04::DistanceEvent ev, uint16_t distance){
}

void test_HCSR04(){
    usound = new HCSR04(PA_8, PA_11);
    usound->setDistRange(20, 20);
    usound->start(callback(onDistEvent), 2000);
    
}

// **************************************************************************
// **************************************************************************
// **************************************************************************


// main() runs in its own thread in the OS
int main() {
//    test_MQlib();
//    test_SpiDma();
//    test_DMA_PwmOut();
//    test_WS281xLedStrip();
    test_HCSR04();
}


