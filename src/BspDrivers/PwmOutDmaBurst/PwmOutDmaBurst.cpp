/*
 * PwmOutDmaBurst.cpp
 *
 *  Created on: Oct 2017
 *      Author: raulMrello
 */

#include "PwmOutDmaBurst.h"



//------------------------------------------------------------------------------------
//- STATIC ---------------------------------------------------------------------------
//------------------------------------------------------------------------------------


extern TIM_HandleTypeDef*  dma_tim_ch1;
extern TIM_HandleTypeDef*  dma_tim_ch2;
extern TIM_HandleTypeDef*  dma_tim_ch3;
extern TIM_HandleTypeDef*  dma_tim_ch4;


//------------------------------------------------------------------------------------
//- WEAK IMPL. -----------------------------------------------------------------------
//------------------------------------------------------------------------------------




//------------------------------------------------------------------------------------
//- PUBLIC CLASS IMPL. ---------------------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
PwmOutDmaBurst::PwmOutDmaBurst(PinName pin, uint32_t period_us){ 
    GPIO_InitTypeDef   GPIO_InitStruct;
    switch(pin){
        case PA_0:{
            _handle.Instance = TIM2;
            _port = GPIOA;
            _channel = TIM_CHANNEL_1;
            _request = DMA_REQUEST_4;
            _dma_channel = DMA1_Channel5;
            _irqn = DMA1_Channel5_IRQn;
            _ccreg = TIM_DMA_ID_CC1;
            dma_tim_ch1 = &_handle;
            GPIO_InitStruct.Pin = GPIO_PIN_0;
            GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
            GPIO_InitStruct.Pull = GPIO_PULLUP;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
            GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
            break;
        }
        case PA_1:{
            _handle.Instance = TIM2;
            _channel = TIM_CHANNEL_2;
            _port = GPIOA;
            _request = DMA_REQUEST_4;
            _dma_channel = DMA1_Channel7;
            _irqn = DMA1_Channel7_IRQn;
            _ccreg = TIM_DMA_ID_CC2;
            dma_tim_ch2 = &_handle;
            GPIO_InitStruct.Pin = GPIO_PIN_1;
            GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
            GPIO_InitStruct.Pull = GPIO_PULLUP;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
            GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
            break;
        }
        case PA_2:{
            _handle.Instance = TIM2;
            _channel = TIM_CHANNEL_3;
            _port = GPIOA;
            _request = DMA_REQUEST_4;
            _dma_channel = DMA1_Channel1;
            _irqn = DMA1_Channel1_IRQn;
            _ccreg = TIM_DMA_ID_CC3;
            dma_tim_ch3 = &_handle;
            GPIO_InitStruct.Pin = GPIO_PIN_2;
            GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
            GPIO_InitStruct.Pull = GPIO_PULLUP;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
            GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
            break;
        }
        case PA_3:{
            _handle.Instance = TIM2;
            _channel = TIM_CHANNEL_4;
            _port = GPIOA;
            _request = DMA_REQUEST_4;
            _dma_channel = DMA1_Channel7;
            _irqn = DMA1_Channel7_IRQn;
            _ccreg = TIM_DMA_ID_CC4;
            dma_tim_ch4 = &_handle;
            GPIO_InitStruct.Pin = GPIO_PIN_3;
            GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
            GPIO_InitStruct.Pull = GPIO_PULLUP;
            GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
            GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
            break;
        }
        default:{
            Error_Handler();
            return;
        }
    }
    
    _handle.Init.RepetitionCounter  = 1;
    _handle.Init.Prescaler          = 1;
    uint32_t tick_ns                = 1000000000/(SystemCoreClock/_handle.Init.Prescaler);    
    _handle.Init.Period             = (period_us * 1000)/tick_ns;
    _handle.Init.ClockDivision      = 0;
    _handle.Init.CounterMode        = TIM_COUNTERMODE_UP;
    if (HAL_TIM_PWM_Init(&_handle) != HAL_OK) {
        /* Initialization Error */
        Error_Handler();
    }    

    /*##-2- Configure the PWM channel 3 ########################################*/
    _sConfig.OCMode       = TIM_OCMODE_PWM1;
    _sConfig.OCPolarity   = TIM_OCPOLARITY_HIGH;
    _sConfig.Pulse        = 0;
    _sConfig.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
    _sConfig.OCFastMode   = TIM_OCFAST_DISABLE;
    _sConfig.OCIdleState  = TIM_OCIDLESTATE_RESET;
    _sConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    if (HAL_TIM_PWM_ConfigChannel(&_handle, &_sConfig, _channel) != HAL_OK)  {
        /* Configuration Error */
        Error_Handler();
    }
    
    /* TIMx clock enable */
    TIM2_CLK_ENABLE();

    /* Enable GPIO  Clocks */
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* Enable DMA clock */
    __HAL_RCC_DMA1_CLK_ENABLE();
    
    HAL_GPIO_Init(_port, &GPIO_InitStruct);


    /* Set the parameters to be configured */
    _hdma_tim.Init.Request  = _request;
    _hdma_tim.Init.Direction = DMA_MEMORY_TO_PERIPH;
    _hdma_tim.Init.PeriphInc = DMA_PINC_DISABLE;
    _hdma_tim.Init.MemInc = DMA_MINC_ENABLE;
    _hdma_tim.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD ;
    _hdma_tim.Init.MemDataAlignment = DMA_MDATAALIGN_WORD ;
    _hdma_tim.Init.Mode = DMA_CIRCULAR;
    _hdma_tim.Init.Priority = DMA_PRIORITY_HIGH;

    /* Set hdma_tim instance */
    _hdma_tim.Instance = _dma_channel;

    /* Link hdma_tim to hdma[TIM_DMA_ID_CC3] (channel3) */
    __HAL_LINKDMA(&_handle, hdma[_ccreg], _hdma_tim);

    /* Initialize TIMx DMA handle */
    HAL_DMA_Init(_handle.hdma[_ccreg]);

    /*##-2- Configure the NVIC for DMA #########################################*/
    /* NVIC configuration for DMA transfer complete interrupt */
    HAL_NVIC_SetPriority(_irqn, 0, 0);
  
    switch(pin){
        case PA_0:{
            HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQHandler);    
            break;
        }
        case PA_1:{
            HAL_NVIC_EnableIRQ(DMA1_Channel7_IRQHandler);    
            break;
        }
        case PA_2:{
            HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQHandler);    
            break;
        }
        case PA_3:{
            HAL_NVIC_EnableIRQ(DMA1_Channel7_IRQHandler);    
            break;
        }
    }
}


//------------------------------------------------------------------------------------
ErrorResult PwmOutDmaBurst::dmaStart(uint32_t* buf, uint16_t bufsize){
    return (ErrorResult)HAL_TIM_PWM_Start_DMA(&_handle, _channel, buf, bufsize);
}


//------------------------------------------------------------------------------------
ErrorResult PwmOutDmaBurst::dmaStop(){
    return (ErrorResult)HAL_TIM_PWM_Stop_DMA(&_handle, _channel);
}



//------------------------------------------------------------------------------------
//- PROTECTED CLASS IMPL. ------------------------------------------------------------
//------------------------------------------------------------------------------------

