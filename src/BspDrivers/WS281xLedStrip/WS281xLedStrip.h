/*
 * WS281xLedStrip.h
 *
 *  Created on: Oct 2017
 *      Author: raulMrello
 *
 *  WS281xLedStrip es un módulo C++ que gestiona la iluminación de un array de Leds direccionables tipo WS2811, WS2812...
 *  El módulo permite operaciones de alto nivel como degradados, fundidos, propagaciones de color, grupos, etc...
 *
 *  La velocidad del puerto es de 800Kbps con lo que se requiere el uso del driver DMA_PwmOut para dar soporte a la alta
 *  velocidad de actuación.
 *
 *  El protocolo establece un tiempo de reset de >50us y después se envían datos en formato 24bit para cada led, con esta
 *  estructura: g7g6..g1g0r7r6..r1r0b7b6...b1b0 siendo el MSB el primer bit enviado.
 *                                                         ____
 *  Los bits a 0 se codifican como una señal de este tipo /    \________ donde el tON=400ns(32%) y tOFF=850ns(68%)
 *                                                         ________
 *  Los bits a 1 se codifican como una señal de este tipo /        \____ donde el tON=800ns(64%) y tOFF=450ns(36%)
 *
 */
 
 
#ifndef WS281XLEDSTRIP_H
#define WS281XLEDSTRIP_H

 
#include "mbed.h"
#include "DMA_PwmOut.h"
#include "Heap.h"

//------------------------------------------------------------------------------------
//- CLASS WS281xLedStrip ------------------------------------------------------------
//------------------------------------------------------------------------------------


class WS281xLedStrip : public DMA_PwmOut {
  public:
	
    /** @struct Color_t
     *  @brief Definición de un color RGB de 24-bit
     */
    struct Color_t{
        uint8_t red;
        uint8_t green;
        uint8_t blue;
    };

	
    /** @fn WS281xLedStrip()
     *  @brief Constructor, que asocia un manejador DMA_PwmOut y un nº de leds
     *  @param pin Pin de salida
     *  @param hz Frecuencia del ciclo pwm
     *  @param num_leds Número de leds en la tira
     */
    WS281xLedStrip(PinName pin, uint32_t hz, uint16_t num_leds);

	
    /** @fn ~WS281xLedStrip()
     *  @brief Destructor por defecto
     */
    virtual ~WS281xLedStrip(){}

	
    /** @fn start()
     *  @brief Inicia la salida vía dma
     */
    void start() { DMA_PwmOut::dmaStart(_color_buffer, _buffer_size/sizeof(uint32_t)); }

	
    /** @fn stop()
     *  @brief Detiene la salida vía dma
     */
    void stop() { DMA_PwmOut::dmaStop(); }

	
    /** @fn setRange()
     *  @brief Establece el color de un rango de leds
     *  @param from Led desde el que se cambiará el color (incluido 0)
     *  @param to Led hasta el que se cambiará el color (incluido _num_leds-1)
     *  @param color Referencia al color a cambiar
     */
    void setRange(uint16_t from, uint16_t to, const Color_t& color);
    
        
  protected:       
    static const uint32_t ResetTimeValue = 0;   /// Valor del tiempo de reset.
    static const uint8_t ResetTimeBits = 50;    /// Tiempo de reset >50us. Como cada ciclo es de 1.25us. Asignado 50 = 62.5us
    static const uint8_t ColorBits = 24;        /// Número de bits a enviar por color R, G, B
    uint16_t _num_leds;                         /// Número de leds de la tira
    uint32_t _buffer_size;                      /// Tamaño del buffer reservado
    uint32_t * _color_buffer;                   /// Buffer para envío de colores
    uint32_t  _bitLow;                          /// Valor para enviar un bit a 0
    uint32_t  _bitHigh;                         /// Valor para enviar un bit a 1
  
	
    /** @fn applyColor()
     *  @brief Establece el color de un led
     *  @param led Led al que cambiar el color
     *  @param color Referencia al color a cambiar
     */
    void applyColor(uint16_t led, const Color_t& color);  
};    



#endif   /* WS281XLEDSTRIP_H */
