/*
 * DMA_PwmOut.cpp
 *
 *  Created on: Oct 2017
 *      Author: raulMrello
 */

#include "DMA_PwmOut.h"
#include "stm32l432xx.h"


//------------------------------------------------------------------------------------
//- STATIC ---------------------------------------------------------------------------
//------------------------------------------------------------------------------------

static DMA_PwmOut* pwm_tim16_ch1 = 0;
static DMA_PwmOut* pwm_tim1_ch1 = 0;
static DMA_PwmOut* pwm_tim1_ch2 = 0;
static DMA_PwmOut* pwm_tim1_ch3 = 0;
static DMA_PwmOut* pwm_tim1_ch4 = 0;


//------------------------------------------------------------------------------------
//- WEAK IMPL. -----------------------------------------------------------------------
//------------------------------------------------------------------------------------

/** @defgroup HAL_MSP_Private_Functions
  * @{
  */


/**
  * @brief TIM MSP Initialization
  *        This function configures the hardware resources used in this example:
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration
  *           - DMA configuration for transmission request by peripheral
  * @param htim: TIM handle pointer
  * @retval None
  */
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim){
    GPIO_InitTypeDef*   GPIO_InitStruct;
    GPIO_TypeDef* port = GPIOA;    
    DMA_HandleTypeDef* hdma_tim;
    uint16_t ccreg;
    IRQn_Type irqn;
    uint32_t request;
    DMA_Channel_TypeDef* dma_channel;
    
    if(pwm_tim16_ch1->getHandler() == htim){
        GPIO_InitStruct = pwm_tim16_ch1->getGPIOTypeDef();
        hdma_tim = pwm_tim16_ch1->getDMAHandle();
        request = DMA_REQUEST_4;
        dma_channel = DMA1_Channel3;
        irqn = DMA1_Channel3_IRQn;
        ccreg = TIM_DMA_ID_CC1;

    }
    else if(pwm_tim1_ch1->getHandler() == htim){
        GPIO_InitStruct = pwm_tim1_ch1->getGPIOTypeDef();
        hdma_tim = pwm_tim1_ch1->getDMAHandle();
        request = DMA_REQUEST_7;
        dma_channel = DMA1_Channel2;
        irqn = DMA1_Channel2_IRQn;
        ccreg = TIM_DMA_ID_CC1;
    }
    else if(pwm_tim1_ch2->getHandler() == htim){
        GPIO_InitStruct = pwm_tim1_ch2->getGPIOTypeDef();
        hdma_tim = pwm_tim1_ch2->getDMAHandle();
        request = DMA_REQUEST_7;
        dma_channel = DMA1_Channel3;
        irqn = DMA1_Channel3_IRQn;
        ccreg = TIM_DMA_ID_CC2;
    }
    else if(pwm_tim1_ch3->getHandler() == htim){
        GPIO_InitStruct = pwm_tim1_ch3->getGPIOTypeDef();
        hdma_tim = pwm_tim1_ch3->getDMAHandle();
        request = DMA_REQUEST_7;
        dma_channel = DMA1_Channel7;
        irqn = DMA1_Channel7_IRQn;
        ccreg = TIM_DMA_ID_CC3;
    }
    else if(pwm_tim1_ch4->getHandler() == htim){
        GPIO_InitStruct = pwm_tim1_ch4->getGPIOTypeDef();
        hdma_tim = pwm_tim1_ch4->getDMAHandle();
        request = DMA_REQUEST_7;
        dma_channel = DMA1_Channel4;
        irqn = DMA1_Channel4_IRQn;
        ccreg = TIM_DMA_ID_CC4;
    }    

    
    /* TIMx clock enable */
    if(htim->Instance == TIM1){
        __HAL_RCC_TIM1_CLK_ENABLE();
    }
    if(htim->Instance == TIM16){
        __HAL_RCC_TIM16_CLK_ENABLE();
    }

    /* Enable GPIO  Clocks */
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* Enable DMA clock */
    __HAL_RCC_DMA1_CLK_ENABLE();
    
    HAL_GPIO_Init(port, GPIO_InitStruct);


    /* Set the parameters to be configured */
    hdma_tim->Init.Request  = request;
    hdma_tim->Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_tim->Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_tim->Init.MemInc = DMA_MINC_ENABLE;
    hdma_tim->Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_tim->Init.MemDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_tim->Init.Mode = DMA_CIRCULAR;
    hdma_tim->Init.Priority = DMA_PRIORITY_HIGH;

    /* Set hdma_tim instance */
    hdma_tim->Instance = dma_channel;

    /* Link hdma_tim to hdma[TIM_DMA_ID_CC3] (channel3) */
    __HAL_LINKDMA(htim, hdma[ccreg], (*hdma_tim));

    /* Initialize TIMx DMA handle */
    HAL_DMA_Init(htim->hdma[ccreg]);

    /*##-2- Configure the NVIC for DMA #########################################*/
    /* NVIC configuration for DMA transfer complete interrupt */
    HAL_NVIC_SetPriority(irqn, 0, 0);
    HAL_NVIC_EnableIRQ(irqn); 
}


//------------------------------------------------------------------------------------
//- PUBLIC CLASS IMPL. ---------------------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
DMA_PwmOut::DMA_PwmOut(PinName pin, uint32_t hz){ 
    
    switch(pin){
        case PA_6:{
            DMA::tim16_ch1 = &_handle;
            pwm_tim16_ch1 = this;
            _handle.Instance = TIM16;
            _channel = TIM_CHANNEL_1;
            _GPIO_InitStruct.Pin = GPIO_PIN_6;
            _GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
            _GPIO_InitStruct.Pull = GPIO_PULLUP;
            _GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
            _GPIO_InitStruct.Alternate = GPIO_AF14_TIM16;
            break;
        }
        case PA_8:{
            DMA::tim1_ch1 = &_handle;
            pwm_tim1_ch1 = this;
            _handle.Instance = TIM1;
            _channel = TIM_CHANNEL_1;
            _GPIO_InitStruct.Pin = GPIO_PIN_8;
            _GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
            _GPIO_InitStruct.Pull = GPIO_PULLUP;
            _GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
            _GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
            break;
        }
        case PA_9:{
            DMA::tim1_ch2 = &_handle;
            pwm_tim1_ch2 = this;
            _handle.Instance = TIM1;
            _channel = TIM_CHANNEL_2;
            _GPIO_InitStruct.Pin = GPIO_PIN_9;
            _GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
            _GPIO_InitStruct.Pull = GPIO_PULLUP;
            _GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
            _GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
            break;
        }
        case PA_10:{
            DMA::tim1_ch3 = &_handle;
            pwm_tim1_ch3 = this;
            _handle.Instance = TIM1;
            _channel = TIM_CHANNEL_3;
            _GPIO_InitStruct.Pin = GPIO_PIN_10;
            _GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
            _GPIO_InitStruct.Pull = GPIO_PULLUP;
            _GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
            _GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
            break;
        }
        case PA_11:{
            DMA::tim1_ch4 = &_handle;
            pwm_tim1_ch4 = this;
            _handle.Instance = TIM1;
            _channel = TIM_CHANNEL_4;
            _GPIO_InitStruct.Pin = GPIO_PIN_11;
            _GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
            _GPIO_InitStruct.Pull = GPIO_PULLUP;
            _GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
            _GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
            break;
        }
        default:{
            return;
        }
    }
    do{
        _handle.Init.RepetitionCounter  = 1;
        _handle.Init.Prescaler          = 0; 
        _handle.Init.Period             = (uint32_t)((SystemCoreClock / hz) - 1);
        _handle.Init.ClockDivision      = 0;
        _handle.Init.CounterMode        = TIM_COUNTERMODE_UP;
        HAL_TIM_PWM_DeInit(&_handle);
        
        if (HAL_TIM_PWM_Init(&_handle) != HAL_OK) {
            /* Initialization Error */
            return;
        }
    }while(_handle.State != HAL_TIM_STATE_READY);
    
    _period_ticks = _handle.Init.Period;
    
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
        return;
    }    
}


//------------------------------------------------------------------------------------
DMA_PwmOut::ErrorResult DMA_PwmOut::dmaStart(uint32_t* buf, uint16_t bufsize){
    return (DMA_PwmOut::ErrorResult)HAL_TIM_PWM_Start_DMA(&_handle, _channel, buf, bufsize);
}


//------------------------------------------------------------------------------------
DMA_PwmOut::ErrorResult DMA_PwmOut::dmaStop(){
    return (DMA_PwmOut::ErrorResult)HAL_TIM_PWM_Stop_DMA(&_handle, _channel);
}



//------------------------------------------------------------------------------------
//- PROTECTED CLASS IMPL. ------------------------------------------------------------
//------------------------------------------------------------------------------------


