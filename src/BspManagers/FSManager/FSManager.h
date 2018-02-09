/*
 * NVSInterface.h
 *
 *  Created on: Ene 2018
 *      Author: raulMrello
 *
 *	NVSInterface es un módulo que proporciona un interfaz básico para almacenar y recuperar pares KEY-VALUE de una
 *  zona de memoria no volátil. 
 *
 */
 
#ifndef __NVSInterface__H
#define __NVSInterface__H

#include "mbed.h"
#include "NVSInterface.h"

class FSManager : public NVSInterface {
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
    FSManager(const char *name) : NVSInterface(name){
		_ready = true;
    }
  
  
    /** init
     *  Inicializa el sistema de ficheros
     *  @return 0 (correcto), <0 (código de error)
     */
    virtual int init(){
		return 0;
	}
  
    /** ready
     *  Chequea si el sistema de ficheros está listo
     *  @return True (si tiene formato) o False (si tiene errores)
     */
    virtual bool ready() {
		return _ready;
	}
  
  
    /** save
     *  Graba datos en memoria no volátil de acuerdo a un identificador dado
     *  @param data_id Identificador de los datos a grabar
     *  @param data  Puntero a los datos 
     *  @param size Tamaño de los datos en bytes
     *  @param type tipo de dato
     *  @return Número de bytes escritos
     */   
    virtual int save(const char* data_id, void* data, uint32_t size, KeyValueType type){
		return 0;
	}
  
  
    /** restore
     *  Recupera datos de memoria no volátil de acuerdo a un identificador dado
     *  @param data_id Identificador de los datos a grabar
     *  @param data  Puntero que recibe los datos recuperados
     *  @param size Tamaño máximo de datos a recuperar
     *  @param type tipo de dato
     *  @return Número de bytes leídos.
     */   
    virtual int restore(const char* data_id, void* data, uint32_t size, KeyValueType type){
		return 0;
	}
    
  protected:

    bool _ready;
};
     
#endif /*__FSManager__H */

/**** END OF FILE ****/


