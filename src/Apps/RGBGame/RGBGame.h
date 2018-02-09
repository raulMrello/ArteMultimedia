/*
 * RGBGame.h
 *
 *  Created on: Ene 2018
 *      Author: raulMrello
 *
 *	RGBGame es el m�dulo activo encargado de gestionar el juego RGB. Consta de los siguientes estados:
 *
 *	stInit: 	Realiza un efecto wave en color azul y conmuta a stWait.
 *
 *	stWait: 	En 10s sin eventos, apaga leds y servos.
 *				En Touch conmuta a stConfig
 *
 *	stConfig: 	En Entry hace un efecto wave con la configuraci�n por defecto y publica xrinst/rgbgame/config/stat -m[Color,Speed] 
 *					para que la app ajuste el tipo de pompa a generar y actualice la simulaci�n.
 *			  	En 30s sin eventos vuelve a stWait.
 *			  	En Touch cambia la configuraci�n y vuelve a reentrar
 *			  	En 10s sin eventos (tras touch) conmuta a stGame
 *
 *	stGame:		En Entry arranca el efecto en bucle
 *				En 30s sin eventos vuelve a stWait
 *				En Touch vuelve a stConfig
 *				En EfectStart publica mensaje en xrinst/rgbgame/value/stat -m[NULL] para que la app simule la generaci�n de pompa.
 *
 *  Los eventos Touch los recibe mediante suscripci�n al topic xrinst/rgbgame/elec/stat, el mensaje recibido es un stream 'ELEC,1' o 'ELEC,0'
 */
 
#ifndef __RGBGame__H
#define __RGBGame__H

#include "mbed.h"
#include "ActiveModule.h"


   
class RGBGame : public ActiveModule {
  public:

    static const uint32_t MaxNumMessages = 16;		//!< M�ximo n�mero de mensajes procesables en el Mailbox del componente
              
    /** Constructor por defecto
     * 	@param fs Objeto FSManager para operaciones de backup
     * 	@param defdbg Flag para habilitar depuraci�n por defecto
     */
    RGBGame(FSManager* fs, bool defdbg = false);


    /** Destructor
     */
    virtual ~RGBGame(){}

  protected:

    /** M�ximo n�mero de mensajes alojables en la cola asociada a la m�quina de estados */
    static const uint32_t MaxQueueMessages = 16;


    /** Flags de operaciones a realizar por la tarea */
    enum MsgEventFlags{
    	TouchEvt = (State::EV_RESERVED_USER << 0),  /// Flag al recibir un evento touch
    };

    /** Flag para la activaci�n de la depuraci�n por defecto */
    bool _defdbg;

    /** Cola de mensajes de la m�quina de estados */
    Queue<State::Msg, MaxQueueMessages> _queue;

    /** Datos de configuraci�n */
	struct Config {
		uint8_t color[3];	// Color RGB
		uint8_t speed;		// Velocidad de 0..100
	};
	Config _cfg;


    /** Interfaz para postear un mensaje de la m�quina de estados en el Mailbox de la clase heredera
     *  @param msg Mensaje a postear
     */
    virtual void putMessage(State::Msg *msg);


    /** Interfaz para obtener un evento osEvent de la clase heredera
     *  @param msg Mensaje a postear
     */
    virtual osEvent getOsEvent();


 	/** Interfaz para manejar los eventos en la m�quina de estados por defecto
      *  @param se Evento a manejar
      *  @return State::StateResult Resultado del manejo del evento
      */
    virtual State::StateResult Init_EventHandler(State::StateEvent* se);


 	/** Callback invocada al recibir una actualizaci�n de un topic local al que est� suscrito
      *  @param topic Identificador del topic
      *  @param msg Mensaje recibido
      *  @param msg_len Tama�o del mensaje
      */
    virtual void subscriptionCb(const char* topic, void* msg, uint16_t msg_len);


 	/** Callback invocada al finalizar una publicaci�n local
      *  @param topic Identificador del topic
      *  @param result Resultado de la publicaci�n
      */
    virtual void publicationCb(const char* topic, int32_t result);


   	/** Chequea la integridad de los datos de configuraci�n <_cfg>. En caso de que algo no sea
   	 * 	coherente, restaura a los valores por defecto y graba en memoria NV.
   	 * 	@return True si la integridad es correcta, False si es incorrecta
	 */
	bool checkIntegrity();


   	/** Establece la configuraci�n por defecto grab�ndola en memoria NV
	 */
	void setDefaultConfig();

};
     
#endif /*__RGBGame__H */

/**** END OF FILE ****/


