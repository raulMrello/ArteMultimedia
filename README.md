# mbed-os-l432kc

Proyecto plantilla para desarrollos con la tarjeta NUCLEO_L432KC de ST en mbed-os

  
## Changelog


*28.02.2018*
>**"Actualizo TouchManager con eventos HOLD"**
>
- [x] Implementados eventos HOLD en TouchManager
- [x] Implemento actualización de efectos como sigue: Electrodos en posición superior incrementan, en posición inferior decrementan y posición central fijan a valor medio.
- [ ] (opcional) Utilizar NVFlash para guardar y recuperar datos de memoria o utilizar mbed-5.7 para utilizar el sistema key-value.
- [ ] (opcional) Pensar la forma de migrar a submódulos...
  

*28.02.2018*
>**"Verificado funcionamiento correcto"**
>
- [x] Verificado funcionamiento general correcto
- [x] Corrijo heap_overflow
- [x] Actualizo MQLib utilizando MBED_ASSERT en las asignaciones de memoria dinámica
- [ ] Implementar actualización de efectos y pulsaciones largas en Touch.
- [ ] Utilizar NVFlash para guardar y recuperar datos de memoria o utilizar mbed-5.7 para utilizar el sistema key-value.
- [ ] Cablear VCC al módulo para arranque al conectar.
- [ ] Pensar la forma de migrar a submódulos...
  

*28.02.2018*
>**"Depurando funcionamiento"**
>
- [x] El cliente mqtt falla al enviar mensajes continuados
- [x] Veo un heap_overflow, analizarlo
- [ ] Utilizar NVFlash para guardar y recuperar datos de memoria o utilizar mbed-5.7 para utilizar el sistema key-value.
- [ ] Cablear VCC al módulo para arranque al conectar.
- [ ] Pensar la forma de migrar a submódulos...
  

*22.02.2018*
>**"Corregidos bugs en RGBGame"**
>
- [x] Corregida asignación de estados _stXX
- [x] Modificado Init_Handler para que conmute a wait al recibir el evento de fin de efecto en lugar de chequear el flag.
- [x] Actualizado a última versión de MQLib, List, StateMachine
- [x] El cliente MQTT reconecta bien si falla en el arranque.
- [x] Juego RGBGame parece funcinar bien. Conectar y verificar.
- [ ] Utilizar NVFlash para guardar y recuperar datos de memoria o utilizar mbed-5.7 para utilizar el sistema key-value.
- [ ] Cablear VCC al módulo para arranque al conectar.
- [ ] Pensar la forma de migrar a submódulos...
  

*20.02.2018*
>**"Corregido bug en RGBGame"**
>
- [x] Corregida asignación de estados _stXX
- [ ] Modificar Init_Handler para que conmute a wait al recibir el evento de fin de efecto en lugar de chequear el flag.
- [ ] Ver qué ocurre cuando un mensaje es IGNORADO y tampoco se procesa en el parentState.
- [ ] A veces se queda pillado la conexión del cliente mqtt. Hacer un delete y volver  a montarlo.
- [ ] Utilizar NVFlash para guardar y recuperar datos de memoria o utilizar mbed-5.7 para utilizar el sistema key-value.
- [ ] Verificar funcionamiento del juego con trazas de depuración
- [ ] Incluir MQTT
- [ ] Cablear VCC al módulo para arranque al conectar.
- [!] Cambiando topics en cyberribs. REVISAR
- [ ] Definir los diferentes modos de juego mediante comando mq: xrinst/rgbgame/select/cmd.
- [ ] Actualizar todos los repos paralelos.
  

*15.02.2018*
>**"Actualizo paquete MQTT y easy-connect"**
>
- [x] Ya funciona la conexión MQTT tras actualizar las librerías correspondientes.
- [ ] REVISAR HARDFAULT
- [ ] Utilizar NVFlash para guardar y recuperar datos de memoria o utilizar mbed-5.7 para utilizar el sistema key-value.
- [ ] Verificar funcionamiento del juego con trazas de depuración
- [ ] Incluir MQTT
- [ ] Cablear VCC al módulo para arranque al conectar.
- [!] Cambiando topics en cyberribs. REVISAR
- [ ] Definir los diferentes modos de juego mediante comando mq: xrinst/rgbgame/select/cmd.
- [ ] Actualizar todos los repos paralelos.
  

*15.02.2018*
>**"Cambio secuencia de conexión"**
>
- [x] Cambio secuencia de conexión
- [ ] Verificar funcionamiento.
- [ ] Utilizar NVFlash para guardar y recuperar datos de memoria o utilizar mbed-5.7 para utilizar el sistema key-value.
- [ ] Verificar funcionamiento del juego con trazas de depuración
- [ ] Incluir MQTT
- [ ] Cablear VCC al módulo para arranque al conectar.
- [!] Cambiando topics en cyberribs. REVISAR
- [ ] Definir los diferentes modos de juego mediante comando mq: xrinst/rgbgame/select/cmd.
- [ ] Actualizar todos los repos paralelos.
  

*14.02.2018*
>**"Verificando wifi. Falla en conexión a socket. Se queda groggy"**
>
- [x] Actualizo a última versión de ActiveModule
- [x] Completo MQNetBridge.
- [ ] Verificar funcionamiento.
- [ ] Utilizar NVFlash para guardar y recuperar datos de memoria o utilizar mbed-5.7 para utilizar el sistema key-value.
- [ ] Verificar funcionamiento del juego con trazas de depuración
- [ ] Incluir MQTT
- [ ] Cablear VCC al módulo para arranque al conectar.
- [!] Cambiando topics en cyberribs. REVISAR
- [ ] Definir los diferentes modos de juego mediante comando mq: xrinst/rgbgame/select/cmd.
- [ ] Actualizar todos los repos paralelos.
  

*14.02.2018*
>**"Finalizo implementación app_RGBGame"**
>
- [x] Actualizo a última versión de MQLib
- [x] Completo MQNetBridge.
- [ ] Verificar funcionamiento.
- [ ] Utilizar NVFlash para guardar y recuperar datos de memoria o utilizar mbed-5.7 para utilizar el sistema key-value.
- [ ] Verificar funcionamiento del juego con trazas de depuración
- [ ] Incluir MQTT
- [ ] Cablear VCC al módulo para arranque al conectar.
- [!] Cambiando topics en cyberribs. REVISAR
- [ ] Definir los diferentes modos de juego mediante comando mq: xrinst/rgbgame/select/cmd.
- [ ] Actualizar todos los repos paralelos.
  

*13.02.2018*
>**"Compatibilidad MBED y ESP-IDF"**
>
- [x] Actualizo a la última versión de ActiveModule, StateMachine y MQLib compatibles en ambas plataformas.
- [ ] Utilizar NVFlash para guardar y recuperar datos de memoria o utilizar mbed-5.7 para utilizar el sistema key-value.
- [ ] Verificar funcionamiento del juego con trazas de depuración
- [ ] Incluir MQTT
- [ ] Cablear VCC al módulo para arranque al conectar.
- [!] Cambiando topics en cyberribs. REVISAR
- [ ] Definir los diferentes modos de juego mediante comando mq: xrinst/rgbgame/select/cmd.
- [ ] Actualizar todos los repos paralelos.
  

*12.02.2018*
>**"Implementado RGBGame"**
>
- [x] Implementado RGBGame. Aunque falta lo siguiente:
- [ ] Utilizar NVFlash para guardar y recuperar datos de memoria o utilizar mbed-5.7 para utilizar el sistema key-value.
- [ ] Verificar funcionamiento del juego con trazas de depuración
- [ ] Incluir MQTT
- [ ] Cablear VCC al módulo para arranque al conectar.
- [!] Cambiando topics en cyberribs. REVISAR
- [ ] Definir los diferentes modos de juego mediante comando mq: xrinst/rgbgame/select/cmd.
- [ ] Actualizar todos los repos paralelos.
  

-------------------
*12.02.2018*
>**"Rehaciendo RGBGame"**
>
- [x] codificando la clase...
- [ ] Incluir MQTT
- [ ] Cablear VCC al módulo para arranque al conectar.
- [!] Cambiando topics en cyberribs. REVISAR
- [ ] Definir los diferentes modos de juego mediante comando mq: xrinst/rgbgame/select/cmd.
- [ ] Actualizar todos los repos paralelos.
  

-------------------
*09.02.2018*
>**"Diseñando RGBGame"**
>
- [x] codificando la clase...
- [ ] Incluir MQTT
- [ ] Cablear VCC al módulo para arranque al conectar.
- [!] Cambiando topics en cyberribs. REVISAR
- [ ] Definir los diferentes modos de juego mediante comando mq: xrinst/rgbgame/select/cmd.
- [ ] Actualizar todos los repos paralelos.
  

-------------------
*31.01.2018*
>**"Funcionamiento de app_RGBGame corregido"**
>
- [x] Ajusto parámetros de propagación de los servos a diferentes velocidades.
- [ ] Modificar el juego para que tenga varios estados
- [ ] Incluir MQTT
- [ ] Cablear VCC al módulo para arranque al conectar.
- [!] Cambiando topics en cyberribs. REVISAR
- [ ] Definir los diferentes modos de juego mediante comando mq: xrinst/rgbgame/select/cmd.
- [ ] Actualizar todos los repos paralelos.
  

-------------------
*31.01.2018*
>**"Corrijo funcionamiento del driver PCA9685"**
>
- [x] Corrijo activación de los servos ya que no estaba bien calculado el duty en setServoAngle.
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
>**"Añado detector touch de cambio de color"**
>
- [x] Añado detector touch (AUNQUE NO VA FINO QUE DIGAMOS), puede valer.
- [ ] Definir los diferentes modos de juego mediante comando mq: xrinst/rgbgame/select/cmd.
- [ ] Actualizar todos los repos paralelos.
  

-------------------
*19.01.2018*
>**"Corregido juego de color en estructura"**
>
- [x] Corrijo los juegos de propagación del color en la estructura por medio del juego app_RGBGame
- [ ] Integrar TouchManager para cambiar el color de la tira, primeros cambios de escala y luego degradados en función del tipo de pulsación.
- [ ] Definir los diferentes modos de juego mediante comando mq: xrinst/rgbgame/select/cmd.
- [ ] Actualizar todos los repos paralelos.
  

-------------------
*18.12.2017*
>**"Intercambio pinout MPR121 y corrijo NVFlash"**
>
- [x] Corrijo pinout en controlador HCSR04 para que no haya solapamiento en las EXTI con el driver MPR121.
- [x] Corrijo recuperación y backup de datos en CyberRibs.
- [ ] Actualizar todos los repos paralelos.
  

-------------------
*14.12.2017*
>**"Corrección de múltiples bugs"**
>
- [x] Creo clase MinLogger para utilizar un logger de capacidades reducidas.
- [x] Modifico MQLib con la posibilidad de crear los topics durante las suscripciones.
- [x] Corrijo bugs en MQNetBridge al publicar duplicados y quito esperas en las trazas de depuración	  
- [x] Añado claves de compilación ESP-IDF en MQLib, StateMachine.
- [!] FALLA al recuperar la memoria no volátil en CyberRibs. Buscar forma alternativa.
- [!] FALLA la notificación de eventos touch cuando ProximityManager está en funcionamiento, seguramente debido a que se queda con mucho % de cpu.
- [ ] Actualizar todos los repos paralelos.
  

-------------------
*14.12.2017*
>**"Repasando MQNetBridge"**
>
- [x] Modifico MQNetBridge para que no borre el topic de la suscripción, que necesita persistir en memoria. Ahora ya funciona bien, aunque siempre publica los mensajes 2 veces, incluso con un retardo de 100ms. REVISARLO.
- [x] Modifico MQLib porque tenía un bug en matchIds. CORREGIR REPOS PARALELOS.
- [ ] Añadir la lógica a los diferentes estados funcionales de CyberRibs.	  
- [ ] Actualizar todos los repos paralelos.	  
  

-------------------
*13.12.2017*
>**"Modificado MQTTNetwork.h"**
>
- [x] Añado a MQTTNetwork el servicio set_blocking para dejar el socket interno como no bloqueante y cambio el servicio read para que si lee el valor WOULD_BLOCK, devuelva 0 y pueda salir del ciclo de consulta.
- [!] He obtenido un HARDFAULT pero parece que funciona mejor. REVISAR DE FORMA AISLADA en test_MQNETBRIDGE.
- [ ] Añadir la lógica a los diferentes estados funcionales de CyberRibs.	  
- [ ] Actualizar todos los repos paralelos.	  
  

-------------------
*13.12.2017*
>**"Depurando app_Countdown"**
>
- [x] Verificando los pasos de arranque de app_Countdown.
- [x] Incluyo cambio en MQLib para copiar internamente el mensaje publicado, por si algún suscriptor lo modifica via strtok o similar.
- [ ] Probando MQTT ya que no funciona bien en multihilo. Volver a reactivar los mensajes para hacerlo todo en el hilo propio.
- [ ] Añadir la lógica a los diferentes estados funcionales de CyberRibs.	  
- [ ] Actualizar todos los repos paralelos.	  
  

-------------------
*12.12.2017*
>**"Incorporo NVData a driver PCA"**
>
- [x] Incorporo NVData al driver PCA de los servos, para que se pueda recuperar y salvar la calibración directamente desde el driver y así lo pueda utilizar también CyberRibs.
- [x] Incluyo topics a CyberRibs para que pueda realizar la calibración de los servos.	  
- [x] Modifico los topics para que todos escuchen en descendientes de xrinst/countdown/cmd/... y publiquen en descendientes de xrinst/countdown/stat/.
- [x] Modifico MQLib para que dé soporte al chequeo de tokens durante la suscripción mediante el servicio MQ::MQClient::isTopicToken.
- [ ] Añadir la lógica a los diferentes estados funcionales de CyberRibs.	  
- [ ] Actualizar todos los repos paralelos.	  
  

-------------------
*01.12.2017*
>**"Integrando CyberRibs"**
>
- [x] Actualizo StateMachine para que sea capaz de utilizar únicamente señales o también poder ser utilizado con un Mail o un Queue en el objeto base, para ello proporciono callbacks de publicación de señales en ese tipo de objetos. FALTA ACTUALIZAR REPO PARALELO
- [x] Diseñando la aplicación Countdown. Cambio ServoManager y DriverLed por el nuevo módulo CyberRibs que permite controlar de forma sincronizada ambos drivers.	  
- [ ] Hay que ver la forma de poder recuperar los datos de calibración en el módulo CyberRibs, lo mismo tiene que utilizar el ServoManager en lugar del PCAdriver.
- [ ] Modificar app_Countdown, ya que CyberRibs es capaz de procesar mensajes en su topic cmd/mode que se corresponden con los que emitirá appXR, con lo que ya app_Countdown no necesita implementar la lógica de actuación, todo recaería en CyberRibs. Cuando lo haga, app_Countdown únicamente se encargará de arrancar y configurar los módulos necesarios y asociar los topics a los que responderá cada uno.
- [ ] Hay que pensar la forma de conmutar la aplicación a modo de ejecución y a modo de calibración o test, o como opción alternativa, poder enviar los parámetros de calibración, desde un script en remoto via mqtt.
- [ ] Añadir la lógica a los diferentes estados funcionales de CyberRibs.	  
- [ ] Actualizar todos los repos paralelos.	  
  

-------------------
*30.11.2017*
>**"Diseñando aplicación app_Countdown"**
>
- [x] Diseñando la aplicación Countdown.
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
- [x] Corregido bug, al registrar el topic stop, se duplican los envíos mqtt ???	  
- [ ] Actualizar todos los repos paralelos.	  
  

-------------------
*22.11.2017*
>**"Mejoro módulo MQNetBridge"**
>
- [x] Mejoro funcionalidad módulo MQNetBridge, es más estable, pero sigo con algún problema a la hora de recibir los topics remotos, seguramente por el maldito yield, que no me funciona como espero. De todas formas, la publicación es buena, con lo que para la funcionalidad de enviar datos es de momento suficiente.
- [ ] REVISAR BUG, al registrar el topic stop, se duplican los envíos mqtt ???	  
- [ ] Actualizar repos paralelos.	  
  

-------------------
*21.11.2017*
>**"Incluyo módulo MQNetBridge"**
>
- [x] Añado módulo MQNetBridge y ahora estoy realizando pruebas, que tengo problemas de suscripción, hay varios threads que intentan leer.
- [ ] Actualizar repos paralelos.	  
  

-------------------
*20.11.2017*
>**"Verifico funcionamiento MQTT-ESP8266"**
>
- [x] Añado test_MQTT para verificar el funcionamiento de las comunicaciones de red MQTT. 
- [ ] FALTA integrar un manager que se encargue de las configuraciones y proporcione callbacks de envío y recepción de datos.
- [x] Añado módulo "program_ESP8266" para volcar la última versión de firmware compatible con mbed.
  

-------------------
*17.11.2017*
>**"Corrjio bug en cálculo de buffer en WS281xLedStrip"**
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
>**"Finalizado módulo ProximityManager"**
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
>**"Mejoro funcionalidad ServoManager y añado NVFlash"**
>
- [x] Añado nuevos casos de uso para activar a un duty, establecer rangos de calibración, etc...
- [x] Añado funcionalidades al driver PCA9685.
- [x] Añado driver NVFlash que permite guardar datos de backup en flash. AUNQUE CON FUNCIONALIDAD
	  LIMITADA, FUNCIONA.
  

-------------------
*13.11.2017*
>**"Incluyo ServoManager"**
>
- [x] Modifico ServoManager y añado test dedicado para pruebas reales.
  

-------------------
*08.11.2017*
>**"Añado tests dedicados"**
>
- [x] Modifico MQSerialBridge para que acepte publicaciones de este estilo: "topic/0 55"
- [x] Incluyo test dedicado para el Servo. QUIZAS HAYA QUE PENSAR EN USAR DMA PARA MOVIMIENTO REPETITIVO.
  

-------------------
*07.11.2017*
>**"Reorganizo código en Middleware, Drivers y Managers"**
>
- [x] Reorganizo módulos de acuerdo a su repo.
  

-------------------
*07.11.2017*
>**"Incluyo módulo MQSerialBridge en Middleware"**
>
- [x] Integrado módulo MQSerialBridge. Verificado funcionamiento!.
- [x] Actualizo funcinamiento de SerialTerminal
  

-------------------
*07.11.2017*
>**"Incluyo módulo StateMachine en Middleware"**
>
- [x] Integrado módulo StateMachine. Verificado funcionamiento!.
  

-------------------
*2.11.2017*
>**"Creo rama dev y actualizo a mbed-os-5.6"**
>
- [x] Integrado módulos de comunicaciones wifi y mqtt, con actualización necesaria a mbed-os-5.6 para solucionar compatibilidad con ATCmdParser.
  

-------------------
*2.11.2017*
>**"Integro MQTT y ESP8266"**
>
- [x] Integrando módulos de comunicaciones wifi y mqtt
- [ ] Nota: ATCmdParser me da un error. Voy a crear la rama dev para importar mbed-os a su última versión a ver si lo soluciono. De todas formas he hecho un backup de mbed-os en kk
  

-------------------
*2.11.2017*
>**"Integrado driver MPR121 y TouchManager"**
>
- [x] Integrado driver de detectores táctiles MPR121 y módulo de gestión de alto nivel TouchManager.
- [ ] Nota: Sería posible hacer que MQLib utilizara las callbacks directamente en lugar de punteros pero para ello, habría que retocar la librería "List" ya que guarda objetos tipo T* y por lo tanto no se puede. Habría que retocar "List" para que utilizara objetos T directamente.
  

-------------------
*31.10.2017*
>**"Integrando driver MPR121"**
>
- [ ] Integrando driver de detectores táctiles MPR121. Lista de comandos iniciales.
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
- [x] Añado driver WS281xLedStrip con capacidades mínimas. Hay que seguir implementando funciones	  
  

-------------------
*26.10.2017*
>**"Incluyo SpiDmaInterface, PwmOutDmaBurst"**
>
- [x] Integro módulos con gestión de DMA así como el módulo común DMA_IRQHandlers que permite agrupar todas las definiciones comunes relativas a los diferentes canales DMA.
- [ ] Cambiar nombre SpiDmaInterface por SpiDma y PwmOutDmaBurst por PwmDma
- [ ] Actualizar el proyecto BspDrivers cuando esté todo terminado.
  

-------------------
*25.10.2017*
>**"Integrando SpiDmaInterface"**
>
- [x] Integrando el driver SpiDma. Ahora estoy integrando todo lo relativo al archivo st..msp.c tengo que integrarlo dentro de la clase.
  

-------------------
*18.09.2017*
>**"Actualizo submódulo MQLib"**
>
- [x] Actualizo submódulo MQLib para mbed_api y cmsis_os
  

-------------------
*17.09.2017*
>**"Incluyo submódulo MQLib"**
>
- [x] Incluyo submódulo MQLib para hacer pruebas con varios módulos anidados

