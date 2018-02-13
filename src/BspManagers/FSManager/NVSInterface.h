/*
 * NVSInterface.h
 *
 *  Created on: Ene 2018
 *      Author: raulMrello
 *
 *	NVSInterface es un m�dulo que proporciona un interfaz b�sico para almacenar y recuperar pares KEY-VALUE de una
 *  zona de memoria no vol�til. 
 *
 */
 
#ifndef __NVSInterface__H
#define __NVSInterface__H

#include "mbed.h"


class NVSInterface{
  public:

	/** KeyValueType
	 * 	Tipo de datos almacenables en el sistema KEY-VALUE
	 */
	enum KeyValueType{
		TypeUint8, //!< TypeUint8
		TypeInt8,  //!< TypeInt8
		TypeUint16,//!< TypeUint16
		TypeInt16, //!< TypeInt16
		TypeUint32,//!< TypeUint32
		TypeInt32, //!< TypeInt32
		TypeUint64,//!< TypeUint64
		TypeInt64, //!< TypeInt64
		TypeString,//!< TypeString
		TypeBlob   //!< TypeBlob
	};
              
    /** Constructor
     *  Crea el gestor del sistema NVS asociando un nombre
     *  @param name Nombre del sistema de ficheros
     */
    NVSInterface(const char *name) : _name(name), _error(0) {
    }
  
  
    /** init
     *  Inicializa el sistema de ficheros
     *  @return 0 (correcto), <0 (c�digo de error)
     */
    virtual int init() = 0;  
  
    /** ready
     *  Chequea si el sistema de ficheros est� listo
     *  @return True (si tiene formato) o False (si tiene errores)
     */
    virtual bool ready() = 0;
  
    /** getName
     *  Obtiene el nombre del sistema de ficheros
     *  @return _name Nombre asignado
     */
    const char* getName() { return _name; }


    /** Abre el handle para realizar varias operaciones en bloque
     *
     * @return True: Handle abierto, False: Handle no abierto (error)
     */
    virtual bool open() = 0;


    /** cierra el handle
     *
     */
    virtual void close() = 0;


    /** save
     *  Graba datos en memoria no vol�til de acuerdo a un identificador dado
     *  @param data_id Identificador de los datos a grabar
     *  @param data  Puntero a los datos 
     *  @param size Tama�o de los datos en bytes
     *  @param type tipo de dato
     *  @return N�mero de bytes escritos
     */   
    virtual int save(const char* data_id, void* data, uint32_t size, KeyValueType type) = 0;
  
  
    /** restore
     *  Recupera datos de memoria no vol�til de acuerdo a un identificador dado
     *  @param data_id Identificador de los datos a grabar
     *  @param data  Puntero que recibe los datos recuperados
     *  @param size Tama�o m�ximo de datos a recuperar
     *  @param type tipo de dato
     *  @return N�mero de bytes le�dos.
     */   
    virtual int restore(const char* data_id, void* data, uint32_t size, KeyValueType type) = 0;
    
  protected:

    const char* _name;          /// Nombre del sistema de ficheros
    int _error;                 /// �ltimo error registrado
};
     
#endif /*__FSManager__H */

/**** END OF FILE ****/


