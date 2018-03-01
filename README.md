# mbed-os-l432kc

Proyecto plantilla para desarrollos con la tarjeta NUCLEO_L432KC de ST en mbed-os

  
## Changelog


*28.02.2018*
>**"Actualizo TouchManager con eventos HOLD"**
>
- [x] Implementados eventos HOLD en TouchManager
- [x] Implemento actualizaci�n de efectos como sigue: Electrodos en posici�n superior incrementan, en posici�n inferior decrementan y posici�n central fijan a valor medio.
- [ ] (opcional) Utilizar NVFlash para guardar y recuperar datos de memoria o utilizar mbed-5.7 para utilizar el sistema key-value.
- [ ] (opcional) Pensar la forma de migrar a subm�dulos...
  

*28.02.2018*
>**"Verificado funcionamiento correcto"**
>
- [x] Verificado funcionamiento general correcto
- [x] Corrijo heap_overflow
- [x] Actualizo MQLib utilizando MBED_ASSERT en las asignaciones de memoria din�mica
- [ ] Implementar actualizaci�n de efectos y pulsaciones largas en Touch.
- [ ] Utilizar NVFlash para guardar y recuperar datos de memoria o utilizar mbed-5.7 para utilizar el sistema key-value.
- [ ] Cablear VCC al m�dulo para arranque al conectar.
- [ ] Pensar la forma de migrar a subm�dulos...
  

*28.02.2018*
>**"Depurando funcionamiento"**
>
- [x] El cliente mqtt falla al enviar mensajes continuados
- [x] Veo un heap_overflow, analizarlo
- [ ] Utilizar NVFlash para guardar y recuperar datos de memoria o utilizar mbed-5.7 para utilizar el sistema key-value.
- [ ] Cablear VCC al m�dulo para arranque al conectar.
- [ ] Pensar la forma de migrar a subm�dulos...
  

*22.02.2018*
>**"Corregidos bugs en RGBGame"**
>
- [x] Corregida asignaci�n de estados _stXX
- [x] Modificado Init_Handler para que conmute a wait al recibir el evento de fin de efecto en lugar de chequear el flag.
- [x] Actualizado a �ltima versi�n de MQLib, List, StateMachine
- [x] El cliente MQTT reconecta bien si falla en el arranque.
- [x] Juego RGBGame parece funcinar bien. Conectar y verificar.
- [ ] Utilizar NVFlash para guardar y recuperar datos de memoria o utilizar mbed-5.7 para utilizar el sistema key-value.
- [ ] Cablear VCC al m�dulo para arranque al conectar.
- [ ] Pensar la forma de migrar a subm�dulos...
  

*20.02.2018*
>**"Corregido bug en RGBGame"**
>
- [x] Corregida asignaci�n de estados _stXX
- [ ] Modificar Init_Handler para que conmute a wait al recibir el evento de fin de efecto en lugar de chequear el flag.
- [ ] Ver qu� ocurre cuando un mensaje es IGNORADO y tampoco se procesa en el parentState.
- [ ] A veces se queda pillado la conexi�n del cliente mqtt. Hacer un delete y volver  a montarlo.
- [ ] Utilizar NVFlash para guardar y recuperar datos de memoria o utilizar mbed-5.7 para utilizar el sistema key-value.
- [ ] Verificar funcionamiento del juego con trazas de depuraci�n
- [ ] Incluir MQTT
- [ ] Cablear VCC al m�dulo para arranque al conectar.
- [!] Cambiando topics en cyberribs. REVISAR
- [ ] Definir los diferentes modos de juego mediante comando mq: xrinst/rgbgame/select/cmd.
- [ ] Actualizar todos los repos paralelos.
  

*15.02.2018*
>**"Actualizo paquete MQTT y easy-connect"**
>
- [x] Ya funciona la conexi�n MQTT tras actualizar las librer�as correspondientes.
- [ ] REVISAR HARDFAULT
- [ ] Utilizar NVFlash para guardar y recuperar datos de memoria o utilizar mbed-5.7 para utilizar el sistema key-value.
- [ ] Verificar funcionamiento del juego con trazas de depuraci�n
- [ ] Incluir MQTT
- [ ] Cablear VCC al m�dulo para arranque al conectar.
- [!] Cambiando topics en cyberribs. REVISAR
- [ ] Definir los diferentes modos de juego mediante comando mq: xrinst/rgbgame/select/cmd.
- [ ] Actualizar todos los repos paralelos.
  

*15.02.2018*
>**"Cambio secuencia de conexi�n"**
>
- [x] Cambio secuencia de conexi�n
- [ ] Verificar funcionamiento.
- [ ] Utilizar NVFlash para guardar y recuperar datos de memoria o utilizar mbed-5.7 para utilizar el sistema key-value.
- [ ] Verificar funcionamiento del juego con trazas de depuraci�n
- [ ] Incluir MQTT
- [ ] Cablear VCC al m�dulo para arranque al conectar.
- [!] Cambiando topics en cyberribs. REVISAR
- [ ] Definir los diferentes modos de juego mediante comando mq: xrinst/rgbgame/select/cmd.
- [ ] Actualizar todos los repos paralelos.
  

*14.02.2018*
>**"Verificando wifi. Falla en conexi�n a socket. Se queda groggy"**
>
- [x] Actualizo a �ltima versi�n de ActiveModule
- [x] Completo MQNetBridge.
- [ ] Verificar funcionamiento.
- [ ] Utilizar NVFlash para guardar y recuperar datos de memoria o utilizar mbed-5.7 para utilizar el sistema key-value.
- [ ] Verificar funcionamiento del juego con trazas de depuraci�n
- [ ] Incluir MQTT
- [ ] Cablear VCC al m�dulo para arranque al conectar.
- [!] Cambiando topics en cyberribs. REVISAR
- [ ] Definir los diferentes modos de juego mediante comando mq: xrinst/rgbgame/select/cmd.
- [ ] Actualizar todos los repos paralelos.
  

*14.02.2018*
>**"Finalizo implementaci�n app_RGBGame"**
>
- [x] Actualizo a �ltima versi�n de MQLib
- [x] Completo MQNetBridge.
- [ ] Verificar funcionamiento.
- [ ] Utilizar NVFlash para guardar y recuperar datos de memoria o utilizar mbed-5.7 para utilizar el sistema key-value.
- [ ] Verificar funcionamiento del juego con trazas de depuraci�n
- [ ] Incluir MQTT
- [ ] Cablear VCC al m�dulo para arranque al conectar.
- [!] Cambiando topics en cyberribs. REVISAR
- [ ] Definir los diferentes modos de juego mediante comando mq: xrinst/rgbgame/select/cmd.
- [ ] Actualizar todos los repos paralelos.
  

*13.02.2018*
>**"Compatibilidad MBED y ESP-IDF"**
>
- [x] Actualizo a la �ltima versi�n de ActiveModule, StateMachine y MQLib compatibles en ambas plataformas.
- [ ] Utilizar NVFlash para guardar y recuperar datos de memoria o utilizar mbed-5.7 para utilizar el sistema key-value.
- [ ] Verificar funcionamiento del juego con trazas de depuraci�n
- [ ] Incluir MQTT
- [ ] Cablear VCC al m�dulo para arranque al conectar.
- [!] Cambiando topics en cyberribs. REVISAR
- [ ] Definir los diferentes modos de juego mediante comando mq: xrinst/rgbgame/select/cmd.
- [ ] Actualizar todos los repos paralelos.
  

*12.02.2018*
>**"Implementado RGBGame"**
>
- [x] Implementado RGBGame. Aunque falta lo siguiente:
- [ ] Utilizar NVFlash para guardar y recuperar datos de memoria o utilizar mbed-5.7 para utilizar el sistema key-value.
- [ ] Verificar funcionamiento del juego con trazas de depuraci�n
- [ ] Incluir MQTT
- [ ] Cablear VCC al m�dulo para arranque al conectar.
- [!] Cambiando topics en cyberribs. REVISAR
- [ ] Definir los diferentes modos de juego mediante comando mq: xrinst/rgbgame/select/cmd.
- [ ] Actualizar todos los repos paralelos.
  

-------------------
*12.02.2018*
>**"Rehaciendo RGBGame"**
>
- [x] codificando la clase...
- [ ] Incluir MQTT
- [ ] Cablear VCC al m�dulo para arranque al conectar.
- [!] Cambiando topics en cyberribs. REVISAR
- [ ] Definir los diferentes modos de juego mediante comando mq: xrinst/rgbgame/select/cmd.
- [ ] Actualizar todos los repos paralelos.
  

-------------------
*09.02.2018*
>**"Dise�ando RGBGame"**
>
- [x] codificando la clase...
- [ ] Incluir MQTT
- [ ] Cablear VCC al m�dulo para arranque al conectar.
- [!] Cambiando topics en cyberribs. REVISAR
- [ ] Definir los diferentes modos de juego mediante comando mq: xrinst/rgbgame/select/cmd.
- [ ] Actualizar todos los repos paralelos.
  

-------------------
*31.01.2018*
>**"Funcionamiento de app_RGBGame corregido"**
>
- [x] Ajusto par�metros de propagaci�n de los servos a diferentes velocidades.
- [ ] Modificar el juego para que tenga varios estados
- [ ] Incluir MQTT
- [ ] Cablear VCC al m�dulo para arranque al conectar.
- [!] Cambiando topics en cyberribs. REVISAR
- [ ] Definir los diferentes modos de juego mediante comando mq: xrinst/rgbgame/select/cmd.
- [ ] Actualizar todos los repos paralelos.
  

-------------------
*31.01.2018*
>**"Corrijo funcionamiento del driver PCA9685"**
>
- [x] Corrijo activaci�n de los servos ya que no estaba bien calculado el duty en setServoAngle.
- [!] Cambiando topics en cyberribs. REVISAR
- [ ] Definir los diferentes modos de juego mediante comando mq: xrinst/rgbgame/select/cmd.
- [ ] Actualizar todos los repos paralelos.
  

-------------------
*30.01.2018*
>**"Modificando app_RGBGame con servos"**
>
- [x] Cambiando cyberribs
- [ ] Definir los diferentes modos de juego mediante comando mq: xrinst/rgbgame/select/cmd.
- [ ] Actualizar todos los repos paralelos.
  

-------------------
*29.01.2018*
>**"Modificando topics"**
>
- [x] Cambiando cyberribs
- [ ] Definir los diferentes modos de juego mediante comando mq: xrinst/rgbgame/select/cmd.
- [ ] Actualizar todos los repos paralelos.
  

-------------------
*21.01.2018*
>**"A�ado detector touch de cambio de color"**
>
- [x] A�ado detector touch (AUNQUE NO VA FINO QUE DIGAMOS), puede valer.
- [ ] Definir los diferentes modos de juego mediante comando mq: xrinst/rgbgame/select/cmd.
- [ ] Actualizar todos los repos paralelos.
  

-------------------
*19.01.2018*
>**"Corregido juego de color en estructura"**
>
- [x] Corrijo los juegos de propagaci�n del color en la estructura por medio del juego app_RGBGame
- [ ] Integrar TouchManager para cambiar el color de la tira, primeros cambios de escala y luego degradados en funci�n del tipo de pulsaci�n.
- [ ] Definir los diferentes modos de juego mediante comando mq: xrinst/rgbgame/select/cmd.
- [ ] Actualizar todos los repos paralelos.
  

-------------------
*18.12.2017*
>**"Intercambio pinout MPR121 y corrijo NVFlash"**
>
- [x] Corrijo pinout en controlador HCSR04 para que no haya solapamiento en las EXTI con el driver MPR121.
- [x] Corrijo recuperaci�n y backup de datos en CyberRibs.
- [ ] Actualizar todos los repos paralelos.
  

-------------------
*14.12.2017*
>**"Correcci�n de m�ltiples bugs"**
>
- [x] Creo clase MinLogger para utilizar un logger de capacidades reducidas.
- [x] Modifico MQLib con la posibilidad de crear los topics durante las suscripciones.
- [x] Corrijo bugs en MQNetBridge al publicar duplicados y quito esperas en las trazas de depuraci�n	  
- [x] A�ado claves de compilaci�n ESP-IDF en MQLib, StateMachine.
- [!] FALLA al recuperar la memoria no vol�til en CyberRibs. Buscar forma alternativa.
- [!] FALLA la notificaci�n de eventos touch cuando ProximityManager est� en funcionamiento, seguramente debido a que se queda con mucho % de cpu.
- [ ] Actualizar todos los repos paralelos.
  

-------------------
*14.12.2017*
>**"Repasando MQNetBridge"**
>
- [x] Modifico MQNetBridge para que no borre el topic de la suscripci�n, que necesita persistir en memoria. Ahora ya funciona bien, aunque siempre publica los mensajes 2 veces, incluso con un retardo de 100ms. REVISARLO.
- [x] Modifico MQLib porque ten�a un bug en matchIds. CORREGIR REPOS PARALELOS.
- [ ] A�adir la l�gica a los diferentes estados funcionales de CyberRibs.	  
- [ ] Actualizar todos los repos paralelos.	  
  

-------------------
*13.12.2017*
>**"Modificado MQTTNetwork.h"**
>
- [x] A�ado a MQTTNetwork el servicio set_blocking para dejar el socket interno como no bloqueante y cambio el servicio read para que si lee el valor WOULD_BLOCK, devuelva 0 y pueda salir del ciclo de consulta.
- [!] He obtenido un HARDFAULT pero parece que funciona mejor. REVISAR DE FORMA AISLADA en test_MQNETBRIDGE.
- [ ] A�adir la l�gica a los diferentes estados funcionales de CyberRibs.	  
- [ ] Actualizar todos los repos paralelos.	  
  

-------------------
*13.12.2017*
>**"Depurando app_Countdown"**
>
- [x] Verificando los pasos de arranque de app_Countdown.
- [x] Incluyo cambio en MQLib para copiar internamente el mensaje publicado, por si alg�n suscriptor lo modifica via strtok o similar.
- [ ] Probando MQTT ya que no funciona bien en multihilo. Volver a reactivar los mensajes para hacerlo todo en el hilo propio.
- [ ] A�adir la l�gica a los diferentes estados funcionales de CyberRibs.	  
- [ ] Actualizar todos los repos paralelos.	  
  

-------------------
*12.12.2017*
>**"Incorporo NVData a driver PCA"**
>
- [x] Incorporo NVData al driver PCA de los servos, para que se pueda recuperar y salvar la calibraci�n directamente desde el driver y as� lo pueda utilizar tambi�n CyberRibs.
- [x] Incluyo topics a CyberRibs para que pueda realizar la calibraci�n de los servos.	  
- [x] Modifico los topics para que todos escuchen en descendientes de xrinst/countdown/cmd/... y publiquen en descendientes de xrinst/countdown/stat/.
- [x] Modifico MQLib para que d� soporte al chequeo de tokens durante la suscripci�n mediante el servicio MQ::MQClient::isTopicToken.
- [ ] A�adir la l�gica a los diferentes estados funcionales de CyberRibs.	  
- [ ] Actualizar todos los repos paralelos.	  
  

-------------------
*01.12.2017*
>**"Integrando CyberRibs"**
>
- [x] Actualizo StateMachine para que sea capaz de utilizar �nicamente se�ales o tambi�n poder ser utilizado con un Mail o un Queue en el objeto base, para ello proporciono callbacks de publicaci�n de se�ales en ese tipo de objetos. FALTA ACTUALIZAR REPO PARALELO
- [x] Dise�ando la aplicaci�n Countdown. Cambio ServoManager y DriverLed por el nuevo m�dulo CyberRibs que permite controlar de forma sincronizada ambos drivers.	  
- [ ] Hay que ver la forma de poder recuperar los datos de calibraci�n en el m�dulo CyberRibs, lo mismo tiene que utilizar el ServoManager en lugar del PCAdriver.
- [ ] Modificar app_Countdown, ya que CyberRibs es capaz de procesar mensajes en su topic cmd/mode que se corresponden con los que emitir� appXR, con lo que ya app_Countdown no necesita implementar la l�gica de actuaci�n, todo recaer�a en CyberRibs. Cuando lo haga, app_Countdown �nicamente se encargar� de arrancar y configurar los m�dulos necesarios y asociar los topics a los que responder� cada uno.
- [ ] Hay que pensar la forma de conmutar la aplicaci�n a modo de ejecuci�n y a modo de calibraci�n o test, o como opci�n alternativa, poder enviar los par�metros de calibraci�n, desde un script en remoto via mqtt.
- [ ] A�adir la l�gica a los diferentes estados funcionales de CyberRibs.	  
- [ ] Actualizar todos los repos paralelos.	  
  

-------------------
*30.11.2017*
>**"Dise�ando aplicaci�n app_Countdown"**
>
- [x] Dise�ando la aplicaci�n Countdown.
- [ ] Crear Manager que controle de forma sincronizada el driver servo y el driver led. Para crear animaciones, modos, transiciones, etc...
- [ ] Actualizar todos los repos paralelos.	  
  

-------------------
*22.11.2017*
>**"Verificado driver led WS281xLedStrip"**
>
- [x] Verificado driver y cambio pin a PA_8 (D9).  
- [ ] Actualizar todos los repos paralelos.	  
  

-------------------
*22.11.2017*
>**"Corregido pub arg en MQSerialBridge"**
>
- [x] Corregido bug, al registrar el topic stop, se duplican los env�os mqtt ???	  
- [ ] Actualizar todos los repos paralelos.	  
  

-------------------
*22.11.2017*
>**"Mejoro m�dulo MQNetBridge"**
>
- [x] Mejoro funcionalidad m�dulo MQNetBridge, es m�s estable, pero sigo con alg�n problema a la hora de recibir los topics remotos, seguramente por el maldito yield, que no me funciona como espero. De todas formas, la publicaci�n es buena, con lo que para la funcionalidad de enviar datos es de momento suficiente.
- [ ] REVISAR BUG, al registrar el topic stop, se duplican los env�os mqtt ???	  
- [ ] Actualizar repos paralelos.	  
  

-------------------
*21.11.2017*
>**"Incluyo m�dulo MQNetBridge"**
>
- [x] A�ado m�dulo MQNetBridge y ahora estoy realizando pruebas, que tengo problemas de suscripci�n, hay varios threads que intentan leer.
- [ ] Actualizar repos paralelos.	  
  

-------------------
*20.11.2017*
>**"Verifico funcionamiento MQTT-ESP8266"**
>
- [x] A�ado test_MQTT para verificar el funcionamiento de las comunicaciones de red MQTT. 
- [ ] FALTA integrar un manager que se encargue de las configuraciones y proporcione callbacks de env�o y recepci�n de datos.
- [x] A�ado m�dulo "program_ESP8266" para volcar la �ltima versi�n de firmware compatible con mbed.
  

-------------------
*17.11.2017*
>**"Corrjio bug en c�lculo de buffer en WS281xLedStrip"**
>
- [x] Verificando funcionamiento
  

-------------------
*17.11.2017*
>**"Realizando pruebas de test con driver WS281xLedStrip"**
>
- [x] Verificando funcionamiento
  

-------------------
*16.11.2017*
>**"Actualizo TouchManager"**
>
- [x] Verificado funcionamiento de TouchManager.
  

-------------------
*16.11.2017*
>**"Actualizo driver MPR121"**
>
- [x] Verificado funcionamiento de driver MPR121.
- [x] Falta implementar el TouchManager
  

-------------------
*16.11.2017*
>**"Finalizado m�dulo ProximityManager"**
>
- [x] Verificado funcionamiento de ProximityManager y driver HCSR04.
- [x] Genero directorio /hex con las versiones generadas.
  

-------------------
*15.11.2017*
>**"Incluyo ProximityManager"**
>
- [x] Verificando funcionamiento de ProximityManager. 
- [x] Retoco driver HCSR04.
  

-------------------
*14.11.2017*
>**"Mejoro funcionalidad ServoManager y a�ado NVFlash"**
>
- [x] A�ado nuevos casos de uso para activar a un duty, establecer rangos de calibraci�n, etc...
- [x] A�ado funcionalidades al driver PCA9685.
- [x] A�ado driver NVFlash que permite guardar datos de backup en flash. AUNQUE CON FUNCIONALIDAD
	  LIMITADA, FUNCIONA.
  

-------------------
*13.11.2017*
>**"Incluyo ServoManager"**
>
- [x] Modifico ServoManager y a�ado test dedicado para pruebas reales.
  

-------------------
*08.11.2017*
>**"A�ado tests dedicados"**
>
- [x] Modifico MQSerialBridge para que acepte publicaciones de este estilo: "topic/0 55"
- [x] Incluyo test dedicado para el Servo. QUIZAS HAYA QUE PENSAR EN USAR DMA PARA MOVIMIENTO REPETITIVO.
  

-------------------
*07.11.2017*
>**"Reorganizo c�digo en Middleware, Drivers y Managers"**
>
- [x] Reorganizo m�dulos de acuerdo a su repo.
  

-------------------
*07.11.2017*
>**"Incluyo m�dulo MQSerialBridge en Middleware"**
>
- [x] Integrado m�dulo MQSerialBridge. Verificado funcionamiento!.
- [x] Actualizo funcinamiento de SerialTerminal
  

-------------------
*07.11.2017*
>**"Incluyo m�dulo StateMachine en Middleware"**
>
- [x] Integrado m�dulo StateMachine. Verificado funcionamiento!.
  

-------------------
*2.11.2017*
>**"Creo rama dev y actualizo a mbed-os-5.6"**
>
- [x] Integrado m�dulos de comunicaciones wifi y mqtt, con actualizaci�n necesaria a mbed-os-5.6 para solucionar compatibilidad con ATCmdParser.
  

-------------------
*2.11.2017*
>**"Integro MQTT y ESP8266"**
>
- [x] Integrando m�dulos de comunicaciones wifi y mqtt
- [ ] Nota: ATCmdParser me da un error. Voy a crear la rama dev para importar mbed-os a su �ltima versi�n a ver si lo soluciono. De todas formas he hecho un backup de mbed-os en kk
  

-------------------
*2.11.2017*
>**"Integrado driver MPR121 y TouchManager"**
>
- [x] Integrado driver de detectores t�ctiles MPR121 y m�dulo de gesti�n de alto nivel TouchManager.
- [ ] Nota: Ser�a posible hacer que MQLib utilizara las callbacks directamente en lugar de punteros pero para ello, habr�a que retocar la librer�a "List" ya que guarda objetos tipo T* y por lo tanto no se puede. Habr�a que retocar "List" para que utilizara objetos T directamente.
  

-------------------
*31.10.2017*
>**"Integrando driver MPR121"**
>
- [ ] Integrando driver de detectores t�ctiles MPR121. Lista de comandos iniciales.
- [x] Integrados drivers HCSR04 ultrasonidos y PCA9685 servos.
  

-------------------
*30.10.2017*
>**"Inctegrando driver HCSR04"**
>
- [x] Integrando driver del sensor de ultrasonidos.	  
  

-------------------
*30.10.2017*
>**"Incluyo drivers DMA y WS281xLedStrip"**
>
- [x] Modifico los drivers que utilizan capacidades DMA dentro de ese directorio y los renombro como DMA_SPI y DMA_PwmOut.
- [x] A�ado driver WS281xLedStrip con capacidades m�nimas. Hay que seguir implementando funciones	  
  

-------------------
*26.10.2017*
>**"Incluyo SpiDmaInterface, PwmOutDmaBurst"**
>
- [x] Integro m�dulos con gesti�n de DMA as� como el m�dulo com�n DMA_IRQHandlers que permite agrupar todas las definiciones comunes relativas a los diferentes canales DMA.
- [ ] Cambiar nombre SpiDmaInterface por SpiDma y PwmOutDmaBurst por PwmDma
- [ ] Actualizar el proyecto BspDrivers cuando est� todo terminado.
  

-------------------
*25.10.2017*
>**"Integrando SpiDmaInterface"**
>
- [x] Integrando el driver SpiDma. Ahora estoy integrando todo lo relativo al archivo st..msp.c tengo que integrarlo dentro de la clase.
  

-------------------
*18.09.2017*
>**"Actualizo subm�dulo MQLib"**
>
- [x] Actualizo subm�dulo MQLib para mbed_api y cmsis_os
  

-------------------
*17.09.2017*
>**"Incluyo subm�dulo MQLib"**
>
- [x] Incluyo subm�dulo MQLib para hacer pruebas con varios m�dulos anidados

