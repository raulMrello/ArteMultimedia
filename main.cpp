#include "mbed.h"
#include "MQLib.h"


DigitalOut led1(LED3);


static void onPublished(uint16_t topic, int32_t result){
	led1 = topic;
}


class Test{
public:
	Test(){
		_topic = 5;
		Topic0Cb = callback(this, &Test::onTopic_0);
		Topic1Cb = callback(this, &Test::onTopic_1);
		Topic01Cb = callback(this, &Test::onTopic_01);
	}
	void task(){
		while(!MQ::MQBroker::ready()){
			Thread::wait(100);
		}
		MQ::MQClient::subscribe(0, &Topic0Cb);
		MQ::MQClient::subscribe(1, &Topic1Cb);
		MQ::MQClient::subscribe(0, &Topic01Cb);
		MQ::MQClient::subscribe(1, &Topic01Cb);
	}
	MQ::SubscribeCallback Topic0Cb, Topic1Cb, Topic01Cb;
	
protected:

	void onTopic_0(uint16_t topic, void* msg, uint16_t msg_len){
		_topic = 0;
	}
	void onTopic_1(uint16_t topic, void* msg, uint16_t msg_len){
		_topic = 1;
	}
	void onTopic_01(uint16_t topic, void* msg, uint16_t msg_len){
		_topic = 2;
	}
	void onPublished(uint16_t topic , int32_t result){
		_topic = 2;
	}
	uint16_t _topic;
};


static Test* t;


// main() runs in its own thread in the OS
int main() {
	t = new Test();
	Thread th;
	MQ::PublishCallback pc = callback(onPublished);
	
	// Inicia la ejecución del broker con un tope de 10 topics
	MQ::MQBroker::start(10);
	
	// Inicia la tarea paralela
	th.start(callback(t, &Test::task));
	
	Thread::wait(1000);
	
	char data = 0;
	MQ::MQClient::publish(0, &data, sizeof(char), &pc);
	data = 4;
	MQ::MQClient::publish(1, &data, sizeof(char), &pc);
    while (true) {
        led1 = !led1;
        wait(0.5);
    }
}
