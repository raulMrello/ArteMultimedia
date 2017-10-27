/*
 * DMA.cpp
 *
 *  Created on: Oct 2017
 *      Author: raulMrello
 */

#include "DMA.h"



//------------------------------------------------------------------------------------
//- STATIC ---------------------------------------------------------------------------
//------------------------------------------------------------------------------------


/** Manejadores DMA para periféricos TIMx */
TIM_HandleTypeDef*  DMA::tim1_ch1 = 0;
TIM_HandleTypeDef*  DMA::tim1_ch2 = 0;
TIM_HandleTypeDef*  DMA::tim1_ch3 = 0;
TIM_HandleTypeDef*  DMA::tim1_ch4 = 0;
TIM_HandleTypeDef*  DMA::tim16_ch1 = 0;

/** Manejadores DMA para periféricos SPIx */
SPI_HandleTypeDef*  DMA::spi1 = 0;    
SPI_HandleTypeDef*  DMA::spi3 = 0;

/** Manejadores DMA para periféricos I2Cx */
I2C_HandleTypeDef*  DMA::i2c1 = 0;    
I2C_HandleTypeDef*  DMA::i2c3 = 0;        

/** Manejadores DMA para periféricos USARTx */
USART_HandleTypeDef*  DMA::usart1 = 0;    
USART_HandleTypeDef*  DMA::usart2 = 0;        
USART_HandleTypeDef*  DMA::usart3 = 0;        

/** Manejadores DMA para periféricos CANx */
CAN_HandleTypeDef*  DMA::can1 = 0;    

/** Manejadores DMA para periféricos ADCx */
ADC_HandleTypeDef*  DMA::adc1 = 0;    

/** Manejadores DMA para periféricos DACx */
DAC_HandleTypeDef*  DMA::dac1 = 0;    
DAC_HandleTypeDef*  DMA::dac2 = 0;        

/** Manejadores DMA para periféricos SAIx */
SAI_HandleTypeDef*  DMA::sai1 = 0;    
SAI_HandleTypeDef*  DMA::sai2 = 0;   

//------------------------------------------------------------------------------------
//- WEAK IMPL. -----------------------------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
void DMA1_Channel1_IRQHandler(void){
}

//------------------------------------------------------------------------------------
void DMA1_Channel2_IRQHandler(void){
    if(DMA::spi1){
        HAL_DMA_IRQHandler(DMA::spi1->hdmarx);
    }
    if(DMA::tim1_ch1){
        HAL_DMA_IRQHandler(DMA::tim1_ch1->hdma[TIM_DMA_ID_CC1]);
    }    
}

//------------------------------------------------------------------------------------
void DMA1_Channel3_IRQHandler(void){
    if(DMA::spi1){
        HAL_DMA_IRQHandler(DMA::spi1->hdmatx);
    }    
    if(DMA::tim16_ch1){
        HAL_DMA_IRQHandler(DMA::tim16_ch1->hdma[TIM_DMA_ID_CC1]);
    }
    if(DMA::tim1_ch2){
        HAL_DMA_IRQHandler(DMA::tim1_ch2->hdma[TIM_DMA_ID_CC2]);
    }    
}

//------------------------------------------------------------------------------------
void DMA1_Channel4_IRQHandler(void){
    if(DMA::tim1_ch4){
        HAL_DMA_IRQHandler(DMA::tim1_ch4->hdma[TIM_DMA_ID_CC4]);
    }
}

//------------------------------------------------------------------------------------
void DMA1_Channel5_IRQHandler(void){
}

//------------------------------------------------------------------------------------
void DMA1_Channel6_IRQHandler(void){

}
//------------------------------------------------------------------------------------
void DMA1_Channel7_IRQHandler(void){
    if(DMA::tim1_ch3){
        HAL_DMA_IRQHandler(DMA::tim1_ch3->hdma[TIM_DMA_ID_CC3]);
    }
}


//------------------------------------------------------------------------------------
void DMA2_Channel1_IRQHandler(void){
    if(DMA::spi3){
        HAL_DMA_IRQHandler(DMA::spi3->hdmarx);
    }  
}

//------------------------------------------------------------------------------------
void DMA2_Channel2_IRQHandler(void){
    if(DMA::spi3){
        HAL_DMA_IRQHandler(DMA::spi3->hdmatx);
    }    
}

//------------------------------------------------------------------------------------
void DMA2_Channel3_IRQHandler(void){

}

//------------------------------------------------------------------------------------
void DMA2_Channel4_IRQHandler(void){

}

//------------------------------------------------------------------------------------
void DMA2_Channel5_IRQHandler(void){

}

//------------------------------------------------------------------------------------
void DMA2_Channel6_IRQHandler(void){

}

//------------------------------------------------------------------------------------
void DMA2_Channel7_IRQHandler(void){

}
