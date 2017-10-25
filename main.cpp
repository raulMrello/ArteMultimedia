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
// *********** TEST SPIDMA **************************************************
// **************************************************************************

#include "SpiDmaInterface.h"

SpiDmaInterface* spi;

struct Color_t{
    uint8_t green;
    uint8_t red;
    uint8_t blue;
};
static const Color_t red_color = {0, 255, 0};
static const Color_t green_color = {255, 0, 0};
static const Color_t blue_color = {0, 0, 255};
static const Color_t white_color = {255, 255, 255};

__packed struct ColorBits_t{
    uint8_t green[8];
    uint8_t red[8];
    uint8_t blue[8];
};

static const uint16_t ROW_COUNT = 100;
__packed struct ColorBuffer_t{
    ColorBits_t row[ROW_COUNT];
    uint8_t resetcode[50];
};

static ColorBuffer_t color_buffer;

void setColorAt(const Color_t* color, uint16_t pos){    
    const uint8_t bitAt0 = 82;
    const uint8_t bitAt1 = 164;
    for(uint8_t i=0;i<8;i++){
        color_buffer.row[pos].green[7-i] = ((color->green & (1 << (7-i))) != 0)? bitAt1 : bitAt0;
    }
    for(uint8_t i=0;i<8;i++){
        color_buffer.row[pos].red[7-i] = ((color->red & (1 << (7-i))) != 0)? bitAt1 : bitAt0;
    }
    for(uint8_t i=0;i<8;i++){
        color_buffer.row[pos].blue[7-i] = ((color->blue & (1 << (7-i))) != 0)? bitAt1 : bitAt0;
    }
}

osThreadId thTestSpi;

void onDmaHalfIsr(){
    osSignalSet(thTestSpi, 1);
}
void onDmaCpltIsr(){
    osSignalSet(thTestSpi, 2);
}
void onDmaErrIsr(SpiDmaInterface::ErrorResult err){
    osSignalSet(thTestSpi, 4);
}

int test_SpiDma(){
    Callback<void()> half_isr = callback(onDmaHalfIsr);
    Callback<void()> cplt_isr = callback(onDmaCpltIsr);
    Callback<void(SpiDmaInterface::ErrorResult)> err_isr = callback(onDmaErrIsr);
    Color_t* curr_color = (Color_t*)&red_color;
    // Inicio el puerto spi1 a 6.4MHz (= 800Kbps)
    spi = new SpiDmaInterface(6400000, PA_7, PA_6, PA_5);    
    
    // obtengo referencia al hilo de ejecución principal
    thTestSpi = Thread::gettid();

    // Preparo buffer de color
    memset(&color_buffer, 0, sizeof(ColorBuffer_t));
    
    // Relleno el array de 100 leds a rojo
    for(int i=0;i<ROW_COUNT;i++){
        setColorAt(&red_color, i);
    }

    
    // Inicio la operación vía DMA
    spi->transmit((uint8_t*)&color_buffer, sizeof(ColorBuffer_t), half_isr, cplt_isr, err_isr);
    
    for(;;){
        // espero resultados...
        osEvent evt = osSignalWait(0, osWaitForever);
        if(evt.status == osEventSignal){   
            uint32_t sig = evt.value.signals;
            // repinto la primera mitad
            if((sig & 1)!=0){
                if(curr_color == &red_color){
                    curr_color = (Color_t*)&green_color;
                }
                else if(curr_color == &green_color){
                    curr_color = (Color_t*)&blue_color;
                }
                else if(curr_color == &blue_color){
                    curr_color = (Color_t*)&red_color;
                }
                for(int i=0;i<(ROW_COUNT/2);i++){                    
                    setColorAt(curr_color, i);
                }                
            }
            // repinto la segunda mitad e inicio de nuevo
            if((sig & 2)!=0){
                for(int i=(ROW_COUNT/2);i<ROW_COUNT;i++){
                    setColorAt(curr_color, i);
                }                
                Thread::wait(1);
                spi->transmit((uint8_t*)&color_buffer, sizeof(ColorBuffer_t), half_isr, cplt_isr, err_isr);
            }
            // pinto de blanco
            if((sig & 2)!=0){
                for(int i=0;i<ROW_COUNT;i++){
                    setColorAt(&white_color, i);
                }                
                Thread::wait(1);
                spi->transmit((uint8_t*)&color_buffer, sizeof(ColorBuffer_t), half_isr, cplt_isr, err_isr);
            }
        }
    }
}


// **************************************************************************
// **************************************************************************
// **************************************************************************


// main() runs in its own thread in the OS
int main() {
//    test_MQlib();
    test_SpiDma();
}


