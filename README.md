# mbed-os-l432kc

Proyecto plantilla para desarrollos con la tarjeta NUCLEO_L432KC de ST en mbed-os


  
## Changelog

----------------------------------------------------------------------------------------------
##### 22.11.2017 ->commit:"Mejoro m�dulo MQNetBridge"
- [x] Mejoro funcionalidad m�dulo MQNetBridge, es m�s estable, pero sigo con alg�n problema a la hora de
	  recibir los topics remotos, seguramente por el maldito yield, que no me funciona como espero. De todas
	  formas, la publicaci�n es buena, con lo que para la funcionalidad de enviar datos es de momento suficiente.
- [ ] REVISAR BUG, al registrar el topic stop, se duplican los env�os mqtt ???	  
- [ ] Actualizar repos paralelos.	  


	  

----------------------------------------------------------------------------------------------
##### 21.11.2017 ->commit:"Incluyo m�dulo MQNetBridge"
- [x] A�ado m�dulo MQNetBridge y ahora estoy realizando pruebas, que tengo problemas de suscripci�n,
	  hay varios threads que intentan leer.
- [ ] Actualizar repos paralelos.	  


	  

----------------------------------------------------------------------------------------------
##### 20.11.2017 ->commit:"Verifico funcionamiento MQTT-ESP8266"
- [x] A�ado test_MQTT para verificar el funcionamiento de las comunicaciones de red MQTT. 
- [ ] FALTA integrar un manager que se encargue de las configuraciones y proporcione callbacks 
      de env�o y recepci�n de datos.
- [x] A�ado m�dulo "program_ESP8266" para volcar la �ltima versi�n de firmware compatible con mbed.


	  

----------------------------------------------------------------------------------------------
##### 17.11.2017 ->commit:"Corrjio bug en c�lculo de buffer en WS281xLedStrip"
- [x] Verificando funcionamiento

	  

----------------------------------------------------------------------------------------------
##### 17.11.2017 ->commit:"Realizando pruebas de test con driver WS281xLedStrip"
- [x] Verificando funcionamiento

	  

----------------------------------------------------------------------------------------------
##### 16.11.2017 ->commit:"Actualizo TouchManager"
- [x] Verificado funcionamiento de TouchManager.

	  

----------------------------------------------------------------------------------------------
##### 16.11.2017 ->commit:"Actualizo driver MPR121"
- [x] Verificado funcionamiento de driver MPR121.
- [x] Falta implementar el TouchManager
	  

----------------------------------------------------------------------------------------------
##### 16.11.2017 ->commit:"Finalizado m�dulo ProximityManager"
- [x] Verificado funcionamiento de ProximityManager y driver HCSR04.
- [x] Genero directorio /hex con las versiones generadas.
	  

----------------------------------------------------------------------------------------------
##### 15.11.2017 ->commit:"Incluyo ProximityManager"
- [x] Verificando funcionamiento de ProximityManager. 
- [x] Retoco driver HCSR04.
	  

----------------------------------------------------------------------------------------------
##### 14.11.2017 ->commit:"Mejoro funcionalidad ServoManager y a�ado NVFlash"
- [x] A�ado nuevos casos de uso para activar a un duty, establecer rangos de calibraci�n, etc...
- [x] A�ado funcionalidades al driver PCA9685.
- [x] A�ado driver NVFlash que permite guardar datos de backup en flash. AUNQUE CON FUNCIONALIDAD
	  LIMITADA, FUNCIONA.
	  

----------------------------------------------------------------------------------------------
##### 13.11.2017 ->commit:"Incluyo ServoManager"
- [x] Modifico ServoManager y a�ado test dedicado para pruebas reales.
	  

----------------------------------------------------------------------------------------------
##### 08.11.2017 ->commit:"A�ado tests dedicados"
- [x] Modifico MQSerialBridge para que acepte publicaciones de este estilo:
	    "topic/0 55"
- [x] Incluyo test dedicado para el Servo. QUIZAS HAYA QUE PENSAR EN USAR DMA PARA MOVIMIENTO
	  REPETITIVO.
	  

----------------------------------------------------------------------------------------------
##### 07.11.2017 ->commit:"Reorganizo c�digo en Middleware, Drivers y Managers"
- [x] Reorganizo m�dulos de acuerdo a su repo.
	  

----------------------------------------------------------------------------------------------
##### 07.11.2017 ->commit:"Incluyo m�dulo MQSerialBridge en Middleware"
- [x] Integrado m�dulo MQSerialBridge. Verificado funcionamiento!.
- [x] Actualizo funcinamiento de SerialTerminal
	  

----------------------------------------------------------------------------------------------
##### 07.11.2017 ->commit:"Incluyo m�dulo StateMachine en Middleware"
- [x] Integrado m�dulo StateMachine. Verificado funcionamiento!.
	  

----------------------------------------------------------------------------------------------
##### 2.11.2017 ->commit:"Creo rama dev y actualizo a mbed-os-5.6"
- [x] Integrado m�dulos de comunicaciones wifi y mqtt, con actualizaci�n necesaria a mbed-os-5.6
	  para solucionar compatibilidad con ATCmdParser.
	  

----------------------------------------------------------------------------------------------
##### 2.11.2017 ->commit:"Integro MQTT y ESP8266"
- [x] Integrando m�dulos de comunicaciones wifi y mqtt
- [ ] Nota: ATCmdParser me da un error. Voy a crear la rama dev para importar mbed-os a su �ltima
	  versi�n a ver si lo soluciono. De todas formas he hecho un backup de mbed-os en kk
	  

----------------------------------------------------------------------------------------------
##### 2.11.2017 ->commit:"Integrado driver MPR121 y TouchManager"
- [x] Integrado driver de detectores t�ctiles MPR121 y m�dulo de gesti�n de alto nivel TouchManager.
- [ ] Nota: Ser�a posible hacer que MQLib utilizara las callbacks directamente en lugar de punteros
	  pero para ello, habr�a que retocar la librer�a "List" ya que guarda objetos tipo T* y por lo 
	  tanto no se puede. Habr�a que retocar "List" para que utilizara objetos T directamente.
	  

----------------------------------------------------------------------------------------------
##### 31.10.2017 ->commit:"Integrando driver MPR121"
- [ ] Integrando driver de detectores t�ctiles MPR121. Lista de comandos iniciales.
- [x] Integrados drivers HCSR04 ultrasonidos y PCA9685 servos.
	  

----------------------------------------------------------------------------------------------
##### 30.10.2017 ->commit:"Inctegrando driver HCSR04"
- [x] Integrando driver del sensor de ultrasonidos.	  
	  

----------------------------------------------------------------------------------------------
##### 30.10.2017 ->commit:"Incluyo drivers DMA y WS281xLedStrip"
- [x] Modifico los drivers que utilizan capacidades DMA dentro de ese directorio y los renombro
	  como DMA_SPI y DMA_PwmOut.
- [x] A�ado driver WS281xLedStrip con capacidades m�nimas. Hay que seguir implementando funciones	  
	  

----------------------------------------------------------------------------------------------
##### 26.10.2017 ->commit:"Incluyo SpiDmaInterface, PwmOutDmaBurst"
- [x] Integro m�dulos con gesti�n de DMA as� como el m�dulo com�n DMA_IRQHandlers que permite 
	  agrupar todas las definiciones comunes relativas a los diferentes canales DMA.
- [ ] Cambiar nombre SpiDmaInterface por SpiDma y PwmOutDmaBurst por PwmDma
- [ ] Actualizar el proyecto BspDrivers cuando est� todo terminado.
	  

----------------------------------------------------------------------------------------------
##### 25.10.2017 ->commit:"Integrando SpiDmaInterface"
- [x] Integrando el driver SpiDma. Ahora estoy integrando todo lo relativo al archivo st..msp.c
	  tengo que integrarlo dentro de la clase.

----------------------------------------------------------------------------------------------
##### 18.09.2017 ->commit:"Actualizo subm�dulo MQLib"
- [x] Actualizo subm�dulo MQLib para mbed_api y cmsis_os

----------------------------------------------------------------------------------------------
##### 17.09.2017 ->commit:"Incluyo subm�dulo MQLib"
- [x] Incluyo subm�dulo MQLib para hacer pruebas con varios m�dulos anidados

