/*
 * PwmOutDmaBurst.cpp
 *
 *  Created on: Oct 2017
 *      Author: raulMrello
 */

#include "DMA_IRQHandlers.h"



//------------------------------------------------------------------------------------
//- STATIC ---------------------------------------------------------------------------
//------------------------------------------------------------------------------------


TIM_HandleTypeDef*  dma_tim_ch1 = 0;
TIM_HandleTypeDef*  dma_tim_ch2 = 0;
TIM_HandleTypeDef*  dma_tim_ch3 = 0;
TIM_HandleTypeDef*  dma_tim_ch4 = 0;
SPI_HandleTypeDef*  dma_spi1 = 0;    
SPI_HandleTypeDef*  dma_spi3 = 0; 

//------------------------------------------------------------------------------------
//- WEAK IMPL. -----------------------------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
void DMA1_Channel1_IRQHandler(void){
    if(dma_tim_ch3){
        HAL_DMA_IRQHandler(dma_tim_ch3->hdma[TIM_DMA_ID_UPDATE]);
    }
}

//------------------------------------------------------------------------------------
void DMA1_Channel2_IRQHandler(void){
    if(dma_spi1){
        HAL_DMA_IRQHandler(dma_spi1->hdmarx);
    }  
}

//------------------------------------------------------------------------------------
void DMA1_Channel3_IRQHandler(void){
    if(dma_spi1){
        HAL_DMA_IRQHandler(dma_spi1->hdmatx);
    }    
}

//------------------------------------------------------------------------------------
void DMA1_Channel5_IRQHandler(void){
    if(dma_tim_ch1){
        HAL_DMA_IRQHandler(dma_tim_ch1->hdma[TIM_DMA_ID_UPDATE]);
    }
}

//------------------------------------------------------------------------------------
void DMA1_Channel7_IRQHandler(void){
    if(dma_tim_ch2){
        HAL_DMA_IRQHandler(dma_tim_ch2->hdma[TIM_DMA_ID_UPDATE]);
    }
    if(dma_tim_ch4){
        HAL_DMA_IRQHandler(dma_tim_ch4->hdma[TIM_DMA_ID_UPDATE]);
    }
}

//------------------------------------------------------------------------------------
void DMA2_Channel1_IRQHandler(void){
    if(dma_spi3){
        HAL_DMA_IRQHandler(dma_spi3->hdmarx);
    }  
}

//------------------------------------------------------------------------------------
void DMA2_Channel2_IRQHandler(void){
    if(dma_spi3){
        HAL_DMA_IRQHandler(dma_spi3->hdmatx);
    }    
}

