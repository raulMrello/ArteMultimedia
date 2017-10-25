/*
 * SpiDmaInterface.h
 *
 *  Created on: Oct 2017
 *      Author: raulMrello
 *
 *  SpiDmaInterface es un módulo C++ que proporciona un mecanismo de transmisión/recepción SPI utilizando los
 *  canales DMA asociados para enviar y/o recibir datos desde un buffer de memoria.
 *  Se pueden realizar operaciones de envío, de recepción o de envío+recepción.
 *  La notificación de eventos se delega a callbacks dedicadas: dmaHalfIsrCb, dmaCpltIsrCb y dmaErrIsrCb que
 *  deberán ser instaladas al inicio del proceso.
 *
 *  NOTA: Esta librería es compatible con procesadores STM32L4xx
 */
 
 
#ifndef SPIDMAINTERFACE_H
#define SPIDMAINTERFACE_H


#ifdef __cplusplus
extern "C" {
#endif
void SPIx_DMA_RX_IRQHandler(void);
void SPIx_DMA_TX_IRQHandler(void);
#ifdef __cplusplus
}
#endif
 
#include "mbed.h"
#include "spi_api.h"
#include "stm32l4xx_hal_def.h" 
#include "stm32l4xx_hal_dma.h"    

class SpiDmaInterface : public SPI {
  public:

    enum ErrorResult{
        NO_ERRORS = HAL_OK,
        UNKNOWN_ERROR = HAL_ERROR,
        BUSY_ERROR = HAL_BUSY,
        TIMEOUT_ERROR = HAL_TIMEOUT,
        TRANSFER_ERROR,        
        ABORT_ERROR,
    };
	
    /** @fn SpiDmaInterface()
     *  @brief Constructor, que asocia un manejador SPI (SPI_x)
     *  @param hz Velocidad bus spi
     *  @param mosi SPI Master Out, Slave In pin
     *  @param miso SPI Master In, Slave Out pin
     *  @param sclk SPI Clock pin
     *  @param ssel SPI chip select pin
     */
    SpiDmaInterface(int hz, PinName mosi, PinName miso, PinName sclk, PinName ssel=NC);

	
    /** @fn ~SpiDmaInterface()
     *  @brief Destructor por defecto
     */
    virtual ~SpiDmaInterface(){}

	
    /** @fn transmit()
     *  @brief Envía un buffer por medio de DMA
     *  @param txbuf Datos de origen
     *  @param bufsize Tamaño de los datos a enviar
     *  @param dmaHalfIsrCb Callback para recibir eventos halfDma
     *  @param dmaCpltIsrCb Callback para recibir eventos CpltDma
     *  @param dmaErrIsrCb Callback para recibir eventos ErrorDma
     */
    void transmit(uint8_t* txbuf, uint16_t bufsize, Callback<void()>& dmaHalfIsrCb, 
                        Callback<void()>& dmaCpltIsrCb, Callback<void(ErrorResult)>& dmaErrIsrCb);

	
    /** @fn receive()
     *  @brief Recibe un buffer por medio de DMA
     *  @param rxbuf Datos de destino
     *  @param bufsize Tamaño de los datos a enviar
     *  @param dmaHalfIsrCb Callback para recibir eventos halfDma
     *  @param dmaCpltIsrCb Callback para recibir eventos CpltDma
     *  @param dmaErrIsrCb Callback para recibir eventos ErrorDma
     */
    void receive(uint8_t* rxbuf, uint16_t bufsize, Callback<void()>& dmaHalfIsrCb, 
                        Callback<void()>& dmaCpltIsrCb, Callback<void(ErrorResult)>& dmaErrIsrCb);

	
    /** @fn transmitAndReceive()
     *  @brief Envía y recibe un buffer por medio de DMA
     *  @param txbuf Datos de origen para la escritura
     *  @param rxbuf Datos de destino tras la lectura
     *  @param bufsize Tamaño de los datos a enviar
     *  @param dmaHalfIsrCb Callback para recibir eventos halfDma
     *  @param dmaCpltIsrCb Callback para recibir eventos CpltDma
     *  @param dmaErrIsrCb Callback para recibir eventos ErrorDma
     */
    void transmitAndReceive(uint8_t* txbuf, uint8_t* rxbuf, uint16_t bufsize, 
                        Callback<void()>& dmaHalfIsrCb, Callback<void()>& dmaCpltIsrCb, Callback<void(ErrorResult)>& dmaErrIsrCb);

	
    /** @fn getHandler()
     *  @brief Obtiene la referencia al manejador SPI
     *  @return Manejador spi
     */
    SPI_HandleTypeDef* getHandler(){ return _handle; }

    /** Callbacks de notificación de interrupción dma */
    Callback<void()> dmaHalfIsrCb;
    Callback<void()> dmaCpltIsrCb;
    Callback<void(ErrorResult)> dmaErrIsrCb;  
    
  protected:        
    SPI_HandleTypeDef* _handle;
};    

#endif   /* SPIDMAINTERFACE_H */
