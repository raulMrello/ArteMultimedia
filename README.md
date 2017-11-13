# mbed-os-l432kc

Proyecto plantilla para desarrollos con la tarjeta NUCLEO_L432KC de ST en mbed-os


  
## Changelog

----------------------------------------------------------------------------------------------
##### 13.11.2017 ->commit:"Incluyo ServoManager"
- [x] Modifico ServoManager y añado test dedicado para pruebas reales.
	  

----------------------------------------------------------------------------------------------
##### 08.11.2017 ->commit:"Añado tests dedicados"
- [x] Modifico MQSerialBridge para que acepte publicaciones de este estilo:
	    "topic/0 55"
- [x] Incluyo test dedicado para el Servo. QUIZAS HAYA QUE PENSAR EN USAR DMA PARA MOVIMIENTO
	  REPETITIVO.
	  

----------------------------------------------------------------------------------------------
##### 07.11.2017 ->commit:"Reorganizo código en Middleware, Drivers y Managers"
- [x] Reorganizo módulos de acuerdo a su repo.
	  

----------------------------------------------------------------------------------------------
##### 07.11.2017 ->commit:"Incluyo módulo MQSerialBridge en Middleware"
- [x] Integrado módulo MQSerialBridge. Verificado funcionamiento!.
- [x] Actualizo funcinamiento de SerialTerminal
	  

----------------------------------------------------------------------------------------------
##### 07.11.2017 ->commit:"Incluyo módulo StateMachine en Middleware"
- [x] Integrado módulo StateMachine. Verificado funcionamiento!.
	  

----------------------------------------------------------------------------------------------
##### 2.11.2017 ->commit:"Creo rama dev y actualizo a mbed-os-5.6"
- [x] Integrado módulos de comunicaciones wifi y mqtt, con actualización necesaria a mbed-os-5.6
	  para solucionar compatibilidad con ATCmdParser.
	  

----------------------------------------------------------------------------------------------
##### 2.11.2017 ->commit:"Integro MQTT y ESP8266"
- [x] Integrando módulos de comunicaciones wifi y mqtt
- [ ] Nota: ATCmdParser me da un error. Voy a crear la rama dev para importar mbed-os a su última
	  versión a ver si lo soluciono. De todas formas he hecho un backup de mbed-os en kk
	  

----------------------------------------------------------------------------------------------
##### 2.11.2017 ->commit:"Integrado driver MPR121 y TouchManager"
- [x] Integrado driver de detectores táctiles MPR121 y módulo de gestión de alto nivel TouchManager.
- [ ] Nota: Sería posible hacer que MQLib utilizara las callbacks directamente en lugar de punteros
	  pero para ello, habría que retocar la librería "List" ya que guarda objetos tipo T* y por lo 
	  tanto no se puede. Habría que retocar "List" para que utilizara objetos T directamente.
	  

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

