/* Hello World Example
	Ejemplo que hace uso de la plataforma ESP32 con el porting MBED-OS v5x a ESP-IDF+FreeRtos
*/
#include "mbed.h"
#include "PushButton.h"

/** Gestor de eventos del thread de test */

static volatile int event = 0;
osThreadId tid;
static void pushEvtCallback(uint32_t id){
	//printf("PUSH!\r\n");
	osSignalSet(tid, 1);
}
static void releaseEvtCallback(uint32_t id){
	//printf("RELEASE!\r\n");
	osSignalSet(tid, 2);
}
static void holdEvtCallback(uint32_t id){
	//printf("HOLD\r\n");
	osSignalSet(tid, 4);
}

//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
void test_PushButton(){
	printf("[Test]\t Iniciando test... \n");
	tid = Thread::gettid();

	printf("[Test]\t Creando PushButton ... \n");
	PushButton *pb = new PushButton(PA_0, 0, PushButton::PressIsLowLevel, PullUp, true);

	printf("[Test]\t Instala callbacks PUSH, HOLD, RELEASE... \n");

	pb->enablePressEvents(callback(&pushEvtCallback));
	pb->enableHoldEvents(callback(&holdEvtCallback), 1000);
	pb->enableReleaseEvents(callback(&releaseEvtCallback));

	for(;;){
		printf("[Test]\t Esperando eventos... \n");
		osEvent oe = Thread::signal_wait(0, osWaitForever);
		printf("[Test]\t EVENTO %d \n", oe.value.signals);
		
	}

	printf("*** FIN DEL TEST *** \n");
}
