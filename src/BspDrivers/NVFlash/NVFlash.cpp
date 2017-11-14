/*
 * NVFlash.cpp
 *
 *  Created on: Nov 2017
 *      Author: raulMrello
 */


#include "NVFlash.h"


//------------------------------------------------------------------------------------
//- STATIC ---------------------------------------------------------------------------
//------------------------------------------------------------------------------------

FlashIAP NVFlash::_flash;
uint32_t NVFlash::_start_addr;
uint8_t NVFlash::_num_pages;
uint32_t NVFlash::_page_size = 0;

    
//------------------------------------------------------------------------------------
//-- PUBLIC METHODS IMPLEMENTATION ---------------------------------------------------
//------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------
NVFlash::ErrorResult NVFlash::init(uint32_t base_addr, uint32_t page_size) {  
    _flash.init();   
    _start_addr = base_addr;
    _page_size = page_size;
    _num_pages = 1;
    return Success;
}


//---------------------------------------------------------------------------------
NVFlash::ErrorResult NVFlash::readPage(uint8_t page, uint32_t* dest){    
    if(_flash.read(dest, _start_addr + (page * _page_size), _page_size)!=0){
        return ReadError;
    }
    return Success;
}


//---------------------------------------------------------------------------------
NVFlash::ErrorResult NVFlash::writePage(uint8_t page, uint32_t* data){
    if(_flash.program(data, _start_addr + (page * _page_size), _page_size)!=0){
        return WriteError;
    }
    return Success;
} 


//---------------------------------------------------------------------------------
NVFlash::ErrorResult NVFlash::erasePage(uint8_t page){
    if(_flash.erase(_start_addr + (page * _page_size), _page_size)!=0){
        return WriteError;
    }
    return Success;
} 
    
//------------------------------------------------------------------------------------
//-- PROTECTED METHODS IMPLEMENTATION ------------------------------------------------
//------------------------------------------------------------------------------------


