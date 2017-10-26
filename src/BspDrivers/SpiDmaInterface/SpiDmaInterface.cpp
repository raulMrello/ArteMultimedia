/*
 * SpiDmaInterface.cpp
 *
 *  Created on: Oct 2017
 *      Author: raulMrello
 */

#include "SpiDmaInterface.h"



//------------------------------------------------------------------------------------
//- STATIC ---------------------------------------------------------------------------
//------------------------------------------------------------------------------------

/** Referencias de DMA_IRQHandlers.cpp */
extern SPI_HandleTypeDef*  dma_spi1;    
extern SPI_HandleTypeDef*  dma_spi3; 
static SpiDmaInterface* spi1Dma;
static SpiDmaInterface* spi3Dma;



//------------------------------------------------------------------------------------
//- WEAK IMPL. -----------------------------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
/** Callback de interrupción dma_transmit_complete */
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi){
    if(dma_spi1 == hspi){
        spi1Dma->dmaCpltIsrCb.call();
        return;
    }
    if(dma_spi3 == hspi){
        spi3Dma->dmaCpltIsrCb.call();
        return;
    }
}


//------------------------------------------------------------------------------------
/** Callback de interrupción dma_receive_complete */
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi){
    if(dma_spi1 == hspi){
        spi1Dma->dmaCpltIsrCb.call();
        return;
    }
    if(dma_spi3 == hspi){
        spi3Dma->dmaCpltIsrCb.call();
        return;
    }
}


//------------------------------------------------------------------------------------
/** Callback de interrupción dma_transmit_receive_complete */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi){
    if(dma_spi1 == hspi){
        spi1Dma->dmaCpltIsrCb.call();
        return;
    }
    if(dma_spi3 == hspi){
        spi3Dma->dmaCpltIsrCb.call();
        return;
    }
}


//------------------------------------------------------------------------------------
/** Callback de interrupción dma_transmit_halfcomplete */
void HAL_SPI_TxHalfCpltCallback(SPI_HandleTypeDef *hspi){
    if(dma_spi1 == hspi){
        spi1Dma->dmaHalfIsrCb.call();
        return;
    }
    if(dma_spi3 == hspi){
        spi3Dma->dmaHalfIsrCb.call();
        return;
    }
}


//------------------------------------------------------------------------------------
/** Callback de interrupción dma_receive_halfcomplete */
void HAL_SPI_RxHalfCpltCallback(SPI_HandleTypeDef *hspi){
    if(dma_spi1 == hspi){
        spi1Dma->dmaHalfIsrCb.call();
        return;
    }
    if(dma_spi3 == hspi){
        spi3Dma->dmaHalfIsrCb.call();
        return;
    }
}


//------------------------------------------------------------------------------------
/** Callback de interrupción dma_transmit_receive_halfcomplete */
void HAL_SPI_TxRxHalfCpltCallback(SPI_HandleTypeDef *hspi){
    if(dma_spi1 == hspi){
        spi1Dma->dmaHalfIsrCb.call();
        return;
    }
    if(dma_spi3 == hspi){
        spi3Dma->dmaHalfIsrCb.call();
        return;
    }
}


//------------------------------------------------------------------------------------
/** Callback de interrupción dma_error */
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi){
    if(dma_spi1 == hspi){
        spi1Dma->dmaErrIsrCb.call(SpiDmaInterface::TRANSFER_ERROR);
        return;
    }
    if(dma_spi3 == hspi){
        spi3Dma->dmaErrIsrCb.call(SpiDmaInterface::TRANSFER_ERROR);
        return;
    }
}


//------------------------------------------------------------------------------------
/** Callback de interrupción dma_abort */
void HAL_SPI_AbortCpltCallback(SPI_HandleTypeDef *hspi){
    if(dma_spi1 == hspi){
        spi1Dma->dmaErrIsrCb.call(SpiDmaInterface::ABORT_ERROR);
        return;
    }
    if(dma_spi3 == hspi){
        spi3Dma->dmaErrIsrCb.call(SpiDmaInterface::ABORT_ERROR);
        return;
    }
}



//------------------------------------------------------------------------------------
//- PUBLIC CLASS IMPL. ---------------------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
SpiDmaInterface::SpiDmaInterface(int hz, PinName mosi, PinName miso, PinName sclk, PinName ssel) : SPI(mosi, miso, sclk, ssel){
    SPI::frequency(hz);
    _handle = &_spi.spi.handle;
    if(_handle->Instance == SPI1){
        dma_spi1 = _handle;
        spi1Dma = this;
        // Activo clock DMA1
        __HAL_RCC_DMA1_CLK_ENABLE();
        /* Configure the DMA handler for Transmission process */
        _hdma_tx.Instance                 = DMA1_Channel3;
        _hdma_tx.Init.Request             = DMA_REQUEST_1;
        _hdma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
        _hdma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
        _hdma_tx.Init.MemInc              = DMA_MINC_ENABLE;
        _hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        _hdma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
        _hdma_tx.Init.Mode                = DMA_NORMAL;
        _hdma_tx.Init.Priority            = DMA_PRIORITY_HIGH;

        HAL_DMA_Init(&_hdma_tx);

        /* Associate the initialized DMA handle to the the SPI handle */
        __HAL_LINKDMA(_handle, hdmatx, _hdma_tx);

        /* Configure the DMA handler for Reception process */
        _hdma_rx.Instance                 = DMA1_Channel2;
        _hdma_rx.Init.Request             = DMA_REQUEST_1;
        _hdma_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
        _hdma_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
        _hdma_rx.Init.MemInc              = DMA_MINC_ENABLE;
        _hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        _hdma_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
        _hdma_rx.Init.Mode                = DMA_NORMAL;
        _hdma_rx.Init.Priority            = DMA_PRIORITY_HIGH;

        HAL_DMA_Init(&_hdma_rx);

        /* Associate the initialized DMA handle to the the SPI handle */
        __HAL_LINKDMA(_handle, hdmarx, _hdma_rx);
        
        /*##-4- Configure the NVIC for DMA #########################################*/ 
        /* NVIC configuration for DMA transfer complete interrupt (SPI1_TX) */
        HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 1, 1);
        HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);
        
        /* NVIC configuration for DMA transfer complete interrupt (SPI1_RX) */
        HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 1, 0);
        HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);            
    }
    else if(_handle->Instance == SPI3){
        dma_spi3 = _handle;
        spi3Dma = this;
        // Activo clock DMA2
        __HAL_RCC_DMA2_CLK_ENABLE();
        /* Configure the DMA handler for Transmission process */
        _hdma_tx.Instance                 = DMA2_Channel2;
        _hdma_tx.Init.Request             = DMA_REQUEST_3;
        _hdma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
        _hdma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
        _hdma_tx.Init.MemInc              = DMA_MINC_ENABLE;
        _hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        _hdma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
        _hdma_tx.Init.Mode                = DMA_NORMAL;
        _hdma_tx.Init.Priority            = DMA_PRIORITY_HIGH;

        HAL_DMA_Init(&_hdma_tx);

        /* Associate the initialized DMA handle to the the SPI handle */
        __HAL_LINKDMA(_handle, hdmatx, _hdma_tx);

        /* Configure the DMA handler for Reception process */
        _hdma_rx.Instance                 = DMA2_Channel1;
        _hdma_rx.Init.Request             = DMA_REQUEST_3;
        _hdma_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
        _hdma_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
        _hdma_rx.Init.MemInc              = DMA_MINC_ENABLE;
        _hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        _hdma_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
        _hdma_rx.Init.Mode                = DMA_NORMAL;
        _hdma_rx.Init.Priority            = DMA_PRIORITY_HIGH;

        /* Associate the initialized DMA handle to the the SPI handle */
        __HAL_LINKDMA(_handle, hdmarx, _hdma_rx);
        
        /*##-4- Configure the NVIC for DMA #########################################*/ 
        /* NVIC configuration for DMA transfer complete interrupt (SPI1_TX) */
        HAL_NVIC_SetPriority(DMA2_Channel2_IRQn, 1, 1);
        HAL_NVIC_EnableIRQ(DMA2_Channel2_IRQn);
        
        /* NVIC configuration for DMA transfer complete interrupt (SPI1_RX) */
        HAL_NVIC_SetPriority(DMA2_Channel1_IRQn, 1, 0);
        HAL_NVIC_EnableIRQ(DMA2_Channel1_IRQn);    
    }      
}


//------------------------------------------------------------------------------------
void SpiDmaInterface::transmit(uint8_t* txbuf, uint16_t bufsize, Callback<void()>& xdmaHalfIsrCb, 
                                Callback<void()>& xdmaCpltIsrCb, Callback<void(ErrorResult)>& xdmaErrIsrCb){
    HAL_StatusTypeDef err = HAL_OK;
    dmaHalfIsrCb = xdmaHalfIsrCb;
    dmaCpltIsrCb = xdmaCpltIsrCb;
    dmaErrIsrCb = xdmaErrIsrCb;
    if((err = HAL_SPI_Transmit_DMA(_handle, txbuf, bufsize)) != HAL_OK){
        dmaErrIsrCb.call((ErrorResult)err);
    }
}


//------------------------------------------------------------------------------------
void SpiDmaInterface::receive(uint8_t* rxbuf, uint16_t bufsize, Callback<void()>& xdmaHalfIsrCb, 
                                Callback<void()>& xdmaCpltIsrCb, Callback<void(ErrorResult)>& xdmaErrIsrCb){
    HAL_StatusTypeDef err = HAL_OK;
    dmaHalfIsrCb = xdmaHalfIsrCb;
    dmaCpltIsrCb = xdmaCpltIsrCb;
    dmaErrIsrCb = xdmaErrIsrCb;
    if((err = HAL_SPI_Receive_DMA(_handle, rxbuf, bufsize)) != HAL_OK){
        dmaErrIsrCb.call((ErrorResult)err);
    }
}


//------------------------------------------------------------------------------------
void SpiDmaInterface::transmitAndReceive(uint8_t* txbuf, uint8_t* rxbuf, uint16_t bufsize, 
                Callback<void()>& xdmaHalfIsrCb, Callback<void()>& xdmaCpltIsrCb, Callback<void(ErrorResult)>& xdmaErrIsrCb){
                    
    HAL_StatusTypeDef err = HAL_OK;
    dmaHalfIsrCb = xdmaHalfIsrCb;
    dmaCpltIsrCb = xdmaCpltIsrCb;
    dmaErrIsrCb = xdmaErrIsrCb;
    if((err = HAL_SPI_TransmitReceive_DMA(_handle, txbuf, rxbuf, bufsize)) != HAL_OK){
        dmaErrIsrCb.call((ErrorResult)err);
    }
}



//------------------------------------------------------------------------------------
//- PROTECTED CLASS IMPL. ------------------------------------------------------------
//------------------------------------------------------------------------------------

