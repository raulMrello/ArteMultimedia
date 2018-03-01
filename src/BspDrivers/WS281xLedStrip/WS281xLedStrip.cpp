/*
 * WS281xLedStrip.cpp
 *
 *  Created on: Oct 2017
 *      Author: raulMrello
 */

#include "WS281xLedStrip.h"



//------------------------------------------------------------------------------------
//- STATIC ---------------------------------------------------------------------------
//------------------------------------------------------------------------------------




//------------------------------------------------------------------------------------
//- PUBLIC CLASS IMPL. ---------------------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
WS281xLedStrip::WS281xLedStrip(PinName pin, uint32_t hz, uint16_t num_leds) : DMA_PwmOut(pin, hz){ 
    _num_leds = num_leds;
    _buffer_size = ((_num_leds * ColorBits) + ResetTimeBits) * sizeof(uint32_t);
    _color_buffer = (uint32_t*)Heap::memAlloc(_buffer_size);
    MBED_ASSERT(_color_buffer);

    memset(_color_buffer, ResetTimeValue, _buffer_size);        
    _bitLow = DMA_PwmOut::getTickPercent(32);
    _bitHigh = DMA_PwmOut::getTickPercent(64);
}


//------------------------------------------------------------------------------------
void WS281xLedStrip::setRange(uint16_t from, uint16_t to, const Color_t& color){
    for(int i = from; i < to; i++){
        applyColor(i, color);
    }
}



//------------------------------------------------------------------------------------
//- PROTECTED CLASS IMPL. ------------------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
void WS281xLedStrip::applyColor(uint16_t led, const Color_t& color){
    // calcula la posición base del led, excluyendo los bits dedicados al tiempo de reset
    uint16_t pos = ResetTimeBits + (led * ColorBits);    
    uint8_t c13 = ColorBits/3;
    uint8_t c23 = (2*ColorBits)/3;
    
    uint8_t mask = 0x80;
    for(int i = pos; i < (pos + c13); i++){
        _color_buffer[i] = ((color.green & mask) != 0)? _bitHigh : _bitLow;
        mask = ((mask >> 1) & 0x7F);
    }
    mask = 0x80;
    for(int i = (pos+c13); i < (pos + c23); i++){
        _color_buffer[i] = ((color.red & mask) != 0)? _bitHigh : _bitLow;
        mask = ((mask >> 1) & 0x7F);
    }
    mask = 0x80;
    for(int i = (pos+c23); i < (pos + ColorBits); i++){
        _color_buffer[i] = ((color.blue & mask) != 0)? _bitHigh : _bitLow;
        mask = ((mask >> 1) & 0x7F);
    }
}

