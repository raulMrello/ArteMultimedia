#include "mbed.h"

RawSerial  pc(USBTX, USBRX);
RawSerial  dev(D1, D0);

void dev_recv()
{
    while(dev.readable()) {
        pc.putc(dev.getc());
    }
}

void pc_recv()
{
    while(pc.readable()) {
        dev.putc(pc.getc());
    }
}

void program_ESP8266()
{
    pc.baud(115200);
    dev.baud(115200);

    pc.attach(&pc_recv, Serial::RxIrq);
    dev.attach(&dev_recv, Serial::RxIrq);

    while(1) {
        Thread::yield();        
    }
}