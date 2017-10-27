/*
 * DMA_PwmOut.h
 *
 *  Created on: Oct 2017
 *      Author: raulMrello
 *
 *  DMA_PwmOut es un módulo C++ que proporciona acceso al periférico TIM1 para implementar clases PwmOut con 
 *  capacidades de DMA, para cambiar el ciclo de trabajo en tiempo real, a partir de un buffer de valores DUTYCYCLE.
 *
 *  NOTA: Sólo se permite TIM1, TIM15 y TIM16. TIM2 mbed lo usa como base de tiempos para el us_ticker y no se puede usar.
 *  Por lo tanto las posibles configuraciones son:
 *
 *  PA_6 (TIM16_CH1) DMA1_Channel3
 *  PA_8 (TIM1_CH1)  DMA1_Channel2
 *  PA_9 (TIM1_CH2)  DMA1_Channel3
 *  PA_10 (TIM1_CH3) DMA1_Channel7
 *  PA_11 (TIM1_CH4) DMA1_Channel4
 *
 */
 
 
#ifndef DMA_PWMOUT_H
#define DMA_PWMOUT_H

 
#include "mbed.h"
#include "pwmout_api.h"
#include "stm32l4xx_hal_def.h" 
#include "stm32l4xx_hal_dma.h"    
#include "stm32l4xx_hal_tim.h" 
#include "DMA.h"


//------------------------------------------------------------------------------------
//- CLASS DMA_PwmOut ------------------------------------------------------------
//------------------------------------------------------------------------------------


class DMA_PwmOut : public DMA {
  public:

    enum ErrorResult{
        NO_ERRORS = HAL_OK,
        UNKNOWN_ERROR = HAL_ERROR,
        BUSY_ERROR = HAL_BUSY,
        TIMEOUT_ERROR = HAL_TIMEOUT,
        TRANSFER_ERROR,        
        ABORT_ERROR,
    };
	
    /** @fn DMA_PwmOut()
     *  @brief Constructor, que asocia un manejador PwmOut
     *  @param pin Pin de salida
     *  @param hz Frecuencia del ciclo pwm
     */
    DMA_PwmOut(PinName pin, uint32_t hz);

	
    /** @fn ~DMA_PwmOut()
     *  @brief Destructor por defecto
     */
    virtual ~DMA_PwmOut(){}

	
    /** @fn dmaStart()
     *  @brief Inicia la escritura del duty cycle via dma
     *  @param buf Datos de origen (configuraciones del duty cycle)
     *  @param bufsize Tamaño de los datos a enviar
     */
    ErrorResult dmaStart(uint32_t* buf, uint16_t bufsize);

	
    /** @fn dmaStop()
     *  @brief Detiene la salida vía dma
     */
    ErrorResult dmaStop();

	
    /** @fn getHandler()
     *  @brief Obtiene la referencia al manejador TIM
     *  @return Manejador tim
     */
    TIM_HandleTypeDef* getHandler(){
        return &_handle; 
    }

	
    /** @fn getGPIOTypeDef()
     *  @brief Obtiene la referencia al manejador GPIO
     *  @return Manejador gpio
     */
    GPIO_InitTypeDef* getGPIOTypeDef(){
        return &_GPIO_InitStruct; 
    }

	
    /** @fn getDMAHandle()
     *  @brief Obtiene la referencia al manejador DMA
     *  @return Manejador dma
     */
    DMA_HandleTypeDef* getDMAHandle(){
        return &_hdma_tim; 
    }

	
    /** @fn getTickPercent()
     *  @brief Obtiene el número de ticks para un porcentaje del duty cycle dado
     *  @return Ticks correspondientes a un porcentaje 0..100%
     */
    uint32_t getTickPercent(uint8_t percent){
        return ((uint32_t)(((uint32_t) percent * (_period_ticks - 1)) / 100)); 
    }
    
        
  protected:              
  
    TIM_HandleTypeDef _handle;
    TIM_OC_InitTypeDef _sConfig;
    DMA_HandleTypeDef  _hdma_tim;
    GPIO_InitTypeDef   _GPIO_InitStruct;
    
    uint32_t _channel;
    uint32_t _period_ticks;
};    



#endif   /* DMA_PWMOUT_H */
