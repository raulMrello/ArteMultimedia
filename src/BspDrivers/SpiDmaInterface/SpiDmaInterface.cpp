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


static SpiDmaInterface* spi1Dma;    /// Manejador estático del periférico SPI1
static SpiDmaInterface* spi3Dma;    /// Manejador estático del periférico SPI3


//------------------------------------------------------------------------------------
//- WEAK IMPL. -----------------------------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
/**
  * @brief  This function handles DMA Rx interrupt request.
  * @param  None
  * @retval None
  */
extern "C" void SPIx_DMA_RX_IRQHandler(void)
{
  HAL_DMA_IRQHandler(SpiHandle.hdmarx);
}


//------------------------------------------------------------------------------------
/**
  * @brief  This function handles DMA Tx interrupt request.
  * @param  None
  * @retval None
  */
extern "C" void SPIx_DMA_TX_IRQHandler(void)
{
  HAL_DMA_IRQHandler(SpiHandle.hdmatx);
}


//------------------------------------------------------------------------------------
/**
  * @brief Tx Transfer completed callback.
  * @param  hspi: pointer to a SPI_HandleTypeDef structure that contains
  *               the configuration information for SPI module.
  * @retval None
  */
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi){
    if(spi1Dma->getHandler() == hspi){
        spi1Dma->dmaCpltIsrCb.call();
        return;
    }
    if(spi3Dma->getHandler() == hspi){
        spi3Dma->dmaCpltIsrCb.call();
        return;
    }
}


//------------------------------------------------------------------------------------
/**
  * @brief Rx Transfer completed callback.
  * @param  hspi: pointer to a SPI_HandleTypeDef structure that contains
  *               the configuration information for SPI module.
  * @retval None
  */
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi){
    if(spi1Dma->getHandler() == hspi){
        spi1Dma->dmaCpltIsrCb.call();
        return;
    }
    if(spi3Dma->getHandler() == hspi){
        spi3Dma->dmaCpltIsrCb.call();
        return;
    }
}


//------------------------------------------------------------------------------------
/**
  * @brief Tx and Rx Transfer completed callback.
  * @param  hspi: pointer to a SPI_HandleTypeDef structure that contains
  *               the configuration information for SPI module.
  * @retval None
  */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi){
    if(spi1Dma->getHandler() == hspi){
        spi1Dma->dmaCpltIsrCb.call();
        return;
    }
    if(spi3Dma->getHandler() == hspi){
        spi3Dma->dmaCpltIsrCb.call();
        return;
    }
}


//------------------------------------------------------------------------------------
/**
  * @brief Tx Half Transfer completed callback.
  * @param  hspi: pointer to a SPI_HandleTypeDef structure that contains
  *               the configuration information for SPI module.
  * @retval None
  */
void HAL_SPI_TxHalfCpltCallback(SPI_HandleTypeDef *hspi){
    if(spi1Dma->getHandler() == hspi){
        spi1Dma->dmaHalfIsrCb.call();
        return;
    }
    if(spi3Dma->getHandler() == hspi){
        spi3Dma->dmaHalfIsrCb.call();
        return;
    }
}


//------------------------------------------------------------------------------------
/**
  * @brief Rx Half Transfer completed callback.
  * @param  hspi: pointer to a SPI_HandleTypeDef structure that contains
  *               the configuration information for SPI module.
  * @retval None
  */
void HAL_SPI_RxHalfCpltCallback(SPI_HandleTypeDef *hspi){
    if(spi1Dma->getHandler() == hspi){
        spi1Dma->dmaHalfIsrCb.call();
        return;
    }
    if(spi3Dma->getHandler() == hspi){
        spi3Dma->dmaHalfIsrCb.call();
        return;
    }
}


//------------------------------------------------------------------------------------
/**
  * @brief Tx and Rx Half Transfer callback.
  * @param  hspi: pointer to a SPI_HandleTypeDef structure that contains
  *               the configuration information for SPI module.
  * @retval None
  */
void HAL_SPI_TxRxHalfCpltCallback(SPI_HandleTypeDef *hspi){
    if(spi1Dma->getHandler() == hspi){
        spi1Dma->dmaHalfIsrCb.call();
        return;
    }
    if(spi3Dma->getHandler() == hspi){
        spi3Dma->dmaHalfIsrCb.call();
        return;
    }
}


//------------------------------------------------------------------------------------
/**
  * @brief SPI error callback.
  * @param  hspi: pointer to a SPI_HandleTypeDef structure that contains
  *               the configuration information for SPI module.
  * @retval None
  */
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi){
    if(spi1Dma->getHandler() == hspi){
        spi1Dma->dmaErrIsrCb.call(SpiDmaInterface::TRANSFER_ERROR);
        return;
    }
    if(spi3Dma->getHandler() == hspi){
        spi3Dma->dmaErrIsrCb.call(SpiDmaInterface::TRANSFER_ERROR);
        return;
    }
}


//------------------------------------------------------------------------------------
/**
  * @brief  SPI Abort Complete callback.
  * @param  hspi SPI handle.
  * @retval None
  */
void HAL_SPI_AbortCpltCallback(SPI_HandleTypeDef *hspi){
    if(spi1Dma->getHandler() == hspi){
        spi1Dma->dmaErrIsrCb.call(SpiDmaInterface::ABORT_ERROR);
        return;
    }
    if(spi3Dma->getHandler() == hspi){
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
        spi1Dma = this;
    }
    else if(_handle->Instance == SPI3){
        spi3Dma = this;
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

