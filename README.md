# mbed-os-l432kc

Proyecto plantilla para desarrollos con la tarjeta NUCLEO_L432KC de ST en mbed-os


  
## Changelog

----------------------------------------------------------------------------------------------
##### 31.10.2017 ->commit:"Integrando driver MPR121"
- [ ] Integrando driver de detectores táctiles MPR121. Lista de comandos iniciales.
- [x] Integrados drivers HCSR04 ultrasonidos y PCA9685 servos.
	  

----------------------------------------------------------------------------------------------
##### 30.10.2017 ->commit:"Inctegrando driver HCSR04"
- [x] Integrando driver del sensor de ultrasonidos.	  
	  

----------------------------------------------------------------------------------------------
##### 30.10.2017 ->commit:"Incluyo drivers DMA y WS281xLedStrip"
- [x] Modifico los drivers que utilizan capacidades DMA dentro de ese directorio y los renombro
	  como DMA_SPI y DMA_PwmOut.
- [x] Añado driver WS281xLedStrip con capacidades mínimas. Hay que seguir implementando funciones	  
	  

----------------------------------------------------------------------------------------------
##### 26.10.2017 ->commit:"Incluyo SpiDmaInterface, PwmOutDmaBurst"
- [x] Integro módulos con gestión de DMA así como el módulo común DMA_IRQHandlers que permite 
	  agrupar todas las definiciones comunes relativas a los diferentes canales DMA.
- [ ] Cambiar nombre SpiDmaInterface por SpiDma y PwmOutDmaBurst por PwmDma
- [ ] Actualizar el proyecto BspDrivers cuando esté todo terminado.
	  

----------------------------------------------------------------------------------------------
##### 25.10.2017 ->commit:"Integrando SpiDmaInterface"
- [x] Integrando el driver SpiDma. Ahora estoy integrando todo lo relativo al archivo st..msp.c
	  tengo que integrarlo dentro de la clase.

----------------------------------------------------------------------------------------------
##### 18.09.2017 ->commit:"Actualizo submódulo MQLib"
- [x] Actualizo submódulo MQLib para mbed_api y cmsis_os

----------------------------------------------------------------------------------------------
##### 17.09.2017 ->commit:"Incluyo submódulo MQLib"
- [x] Incluyo submódulo MQLib para hacer pruebas con varios módulos anidados

