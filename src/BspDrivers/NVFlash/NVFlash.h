/*
 * NVFlash.h
 *
 *  Created on: Nov 2017
 *      Author: raulMrello
 *
 *  NVFlash es el módulo C++ que proporciona acceso a zonas de memoria FLASH para almacenar datos no volátiles
 *  como configuraciones, parámetros de calibración, etc...
 *  Por regla general, son zonas de memoria situadas al final de la flash de programa
 * 
 */
 
 
#ifndef _NVFlash_H
#define _NVFlash_H


#include "mbed.h"  
  
  
//---------------------------------------------------------------------------------
//- class NVFlash ----------------------------------------------------------
//---------------------------------------------------------------------------------


class NVFlash {

public:
        
    /** Lista de erores devueltos por el módulo */
    enum ErrorResult{
        Success,
        Locked,
        Busy,
        OutOfRange,
        InvalidArguments,
        ReadError,
        WriteError,
    };
    
    /** init()
     *  Inicializa reservando una zona de la flash
     *  @param base_addr Páginas a reservar
     *  @param size Tamaño de página.
     *  @return Código de error
     */
    static ErrorResult init(uint32_t base_addr = 0x0803F800, uint32_t size = 2048);
    
    
    /** getPageSize()
     *  Obtiene el tamaño de una página
     *  @return Tamaño de página
     */
    static uint32_t getPageSize() { return _page_size; }


    /** readPage()
     *  Lee una página
     *  @param page Página a leer
     *  @param dest Recibe los datos leídos
     *  @param size Tamaño a leer.
     *  @return Código de error
     */
    static ErrorResult readPage(uint8_t page, uint32_t* dest, uint32_t size = 2048);


    /** writePage()
     *  Escribe datos a una posición de memoria
     *  @param page Página a programar
     *  @param data Datos a escribir
     *  @return Código de error
     */
    static ErrorResult writePage(uint8_t page, uint32_t* data);


    /** erasePage()
     *  Borra una página
     *  @param page Página a borrar
     */
    static ErrorResult erasePage(uint8_t page);

      
protected:

    static FlashIAP _flash;         /// Controlador de memoria
    static uint32_t _start_addr;    /// Dirección de origen
    static uint8_t _num_pages;      /// Número de páginas reservadas
    static uint32_t _page_size;     /// Tamaño de la página

};

#endif  /** _NVFlash_H */

