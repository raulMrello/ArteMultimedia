/*
 * DMA_IRQHandlers.h
 *
 *  Created on: Oct 2017
 *      Author: raulMrello
 *
 *  DMA_IRQHandlers es un módulo C++ que proporciona acceso a las definiciones comunes de los 
 *  manejadores de interrupción de los diversos canales DMA, cuya implementación se realiza en
 *  el archivo .cpp
 */
 
 
#ifndef DMA_IRQHANDLERS_H
#define DMA_IRQHANDLERS_H

 
#include "stm32l4xx_hal.h"
  

//------------------------------------------------------------------------------------
//- DMA ISR's ------------------------------------------------------------------------
//------------------------------------------------------------------------------------


/** Manejadores de interrupción para los canales DMA asociados a los interfaces SPI (SPI1 y SPI3)*/

#ifdef __cplusplus
extern "C" {
#endif

void DMA1_Channel1_IRQHandler(void);
void DMA1_Channel2_IRQHandler(void);
void DMA1_Channel3_IRQHandler(void);
void DMA1_Channel5_IRQHandler(void);
void DMA1_Channel7_IRQHandler(void);
void DMA2_Channel1_IRQHandler(void);
void DMA2_Channel2_IRQHandler(void);
    
#ifdef __cplusplus
}
#endif


#endif   /* DMA_IRQHANDLERS_H */
