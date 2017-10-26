/*
 * PwmOutDmaBurst.h
 *
 *  Created on: Oct 2017
 *      Author: raulMrello
 *
 *  PwmOutDmaBurst es un módulo C++ que proporciona un mecanismo de recarga del duty cycle de una salida PWM
 *  basado en la funcionalidad DMA Burst. Esta funcionalidad permite cargar desde un buffer de datos en los
 *  que se almacenan diferentes configuraciones del duty cycle, en el evento TIM2_UP del timer TIM2, de forma que
 *  se pueda modificar la salida PWM asociada, sin intervención de la CPU, cuando las temporizaciones son 
 *  rápidas. Por ejemplo al manejar tiras de leds del tipo WS2812 o similares.
 *
 *  NOTA: Este módulo está portado a la plataforma NUCLEO_L432KC aunque se puede portar a otras plataformas
 *  tipo stm32f4xx. Si se utiliza otro timer distinto a TIM2 habría que modificar los manejadores IRQ del canal
 *  dma utilizado.
 *
 *  La configuración por defecto es TIM_clk = SYS_CLK/2 (40MHz) lo que da un tick=25ns, siendo TIM2 un timer de 32bit
 *  permite un amplio rango de temporizaciones desde centenas de ns hasta centenas de segundo.
 */
 
 
#ifndef PWMOUTDMABURST_H
#define PWMOUTDMABURST_H

 
#include "mbed.h"
#include "pwmout_api.h"
#include "stm32l4xx_hal_def.h" 
#include "stm32l4xx_hal_dma.h"    
#include "stm32l4xx_hal_tim.h"  



//------------------------------------------------------------------------------------
//- CLASS PwmOutDmaBurst ------------------------------------------------------------
//------------------------------------------------------------------------------------


class PwmOutDmaBurst {
  public:

    enum ErrorResult{
        NO_ERRORS = HAL_OK,
        UNKNOWN_ERROR = HAL_ERROR,
        BUSY_ERROR = HAL_BUSY,
        TIMEOUT_ERROR = HAL_TIMEOUT,
        TRANSFER_ERROR,        
        ABORT_ERROR,
    };
	
    /** @fn PwmOutDmaBurst()
     *  @brief Constructor, que asocia un manejador PwmOut
     *  @param pin Pin de salida
     *  @param hz Frecuencia pwm
     */
    PwmOutDmaBurst(PinName pin, uint32_t period_us);

	
    /** @fn ~PwmOutDmaBurst()
     *  @brief Destructor por defecto
     */
    virtual ~PwmOutDmaBurst(){}

	
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

        
  protected:        
    TIM_HandleTypeDef _handle;
    TIM_OC_InitTypeDef _sConfig;
    DMA_HandleTypeDef  _hdma_tim;
    uint32_t _channel;
    GPIO_TypeDef* _port;
    DMA_Channel_TypeDef* _dma_channel;
    uint16_t _ccreg;
    IRQn_Type _irqn;
    uint32_t _request;
};    




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


#endif   /* PWMOUTDMABURST_H */
