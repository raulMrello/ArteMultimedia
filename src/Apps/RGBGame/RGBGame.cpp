/*
 * RGBGame.cpp
 *
 *  Created on: Ene 2018
 *      Author: raulMrello
 */

#include "RGBGame.h"

//------------------------------------------------------------------------------------
//-- PRIVATE TYPEDEFS ----------------------------------------------------------------
//------------------------------------------------------------------------------------

/** Macro para imprimir trazas de depuración, siempre que se haya configurado un objeto
 *	Logger válido (ej: _debug)
 */

#define DEBUG_TRACE(format, ...)			\
if(_defdbg){								\
	printf(format, ##__VA_ARGS__);			\
}											\
 
//------------------------------------------------------------------------------------
//-- PUBLIC METHODS IMPLEMENTATION ---------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
RGBGame::RGBGame(FSManager* fs, bool defdbg) : ActiveModule("RGBGame", osPriorityNormal, OS_STACK_SIZE) {
	_defdbg = defdbg;
	ActiveModule::attachFSManager(fs);
}


//------------------------------------------------------------------------------------
//-- PROTECTED METHODS IMPLEMENTATION ------------------------------------------------
//------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------
void RGBGame::subscriptionCb(const char* topic, void* msg, uint16_t msg_len){
    // si es un comando para actualizar los flags de notificación o los flags de evento...
    if(MQ::MQClient::isTopicToken(topic, "/elec/stat")){
        DEBUG_TRACE("\r\nRGBGame\t Recibido topic %s", topic);

        // el mensaje es un stream 'ELEC,0'
		char* elec = strtok((char*)msg, ",");
		MBED_ASSERT(elec);
		char* value = strtok(NULL, ",");
		MBED_ASSERT(value);
		        
        // crea mensaje para publicar en la máquina de estados
        State::Msg* op = (State::Msg*)Heap::memAlloc(sizeof(State::Msg));
        MBED_ASSERT(op);
		// sig contiene el evento, en función del valor
        op->sig = (atoi(value))? TouchOnEvt : TouchOffEvt;
        // msg contiene el valor del electrodo que genera el evento
        op->msg = (void*)atoi(elec);
        // postea en la cola de la máquina de estados
        _queue.put(op);
        return;
    }
    DEBUG_TRACE("\r\nRGBGame\t ERR_TOPIC. No se puede procesar el topic [%s]", topic);
}

//------------------------------------------------------------------------------------
State::StateResult RGBGame::Init_EventHandler(State::StateEvent* se){
	State::Msg* st_msg = (State::Msg*)se->oe->value.p;
    switch((int)se->evt){
        case State::EV_ENTRY:{
        	int err = osOK;
        	DEBUG_TRACE("\r\nRGBGame\t Iniciando recuperación de datos...");
        	// recupera los datos de memoria NV
        	err = _fs->restore("RGBGameCfg", &_cfg, sizeof(Config), NVSInterface::TypeBlob);
        	if(err == osOK){
            	// chequea la coherencia de los datos y en caso de algo no esté bien, establece los datos por defecto
            	// almacenándolos de nuevo en memoria NV.
            	if(!checkIntegrity()){
            		DEBUG_TRACE("\r\nRGBGame\t ERR_CFG. Ha fallado el check de integridad. Establece configuración por defecto.");
					setDefaultConfig();
            	}
				else{
					DEBUG_TRACE("\r\nRGBGame\t Recuperación de datos OK!");
				}
        	}
			else{
				DEBUG_TRACE("\r\nRGBGame\t ERR_FS. Error en la recuperación de datos. Establece configuración por defecto");
				setDefaultConfig();
			}
        	
            return State::HANDLED;
        }

        case State::EV_TIMED:{
            return State::HANDLED;
        }

        // Procesa datos recibidos de la publicación en $BASE/notif/cmd
        case RecvNotifCmd:{
        	// actualiza los flags upd
        	_cfg.updFlagMask = (Blob::AstCalUpdFlags)*((uint32_t*)st_msg->msg);

        	// almacena en el sistema de ficheros
        	_fs->save("AstCalUpdFlags", &_cfg.updFlagMask, sizeof(uint32_t), NVSInterface::TypeUint32);

        	DEBUG_TRACE("\r\n[AstCal]\t AstCalUpdFlags actualizados [%d]", _cfg.updFlagMask);

            // libera el mensaje (tanto el contenedor de datos como el propio mensaje)
        	Heap::memFree(st_msg->msg);
            Heap::memFree(st_msg);

            return State::HANDLED;
        }

        // Procesa datos recibidos de la publicación en $BASE/iv/cmd
        case RecvIVCmd:{
        	// actualiza los cambios de estación de verano
        	_cfg.ivHeader = ((Blob::Season_t*)st_msg->msg)->header;
        	// recalcula la tabla de datos
        	memset(&_cfg.data[0], -1, sizeof(_cfg.data)/2);
        	memcpy(&_cfg.data[0], ((Blob::Season_t*)st_msg->msg)->content.data, _cfg.ivHeader.numRecords * sizeof(uint32_t));

        	// almacena en el sistema de ficheros
        	_fs->save("AstCalIVHeader", &_cfg.ivHeader, sizeof(_cfg.ivHeader), NVSInterface::TypeBlob);
        	_fs->save("AstCalIVData", &_cfg.data[0], _cfg.ivHeader.numRecords * sizeof(uint32_t), NVSInterface::TypeBlob);

        	DEBUG_TRACE("\r\n[AstCal]\t IVConfig actualizada");

        	// si está habilitada la notificación de actualización, lo notifica
        	if((_cfg.updFlagMask & Blob::EnableIVUpdNotif) != 0){
				char* pub_topic = (char*)Heap::memAlloc(MQ::MQClient::getMaxTopicLen());
				MBED_ASSERT(pub_topic);
				sprintf(pub_topic, "%s/iv/stat", _pub_topic_base);
				Blob::Season_t* season = (Blob::Season_t*)Heap::memAlloc(sizeof(Blob::Season_t));
				MBED_ASSERT(season);
				season->header = _cfg.ivHeader;
				memset(season->content.data, -1, sizeof(season->content.data));
				memcpy(season->content.data, &_cfg.data[0], _cfg.ivHeader.numRecords * sizeof(uint32_t));
				MQ::MQClient::publish(pub_topic, season, sizeof(Blob::Season_t), &_publicationCb);
				Heap::memFree(season);
				Heap::memFree(pub_topic);
        	}

            // libera el mensaje (tanto el contenedor de datos como el propio mensaje)
        	Heap::memFree(st_msg->msg);
            Heap::memFree(st_msg);
			executeEffect("wave");
			tranState(stWait);
            return State::HANDLED;
        }

        // Procesa datos recibidos de la publicación en $BASE/vi/cmd
        case RecvVICmd:{
        	// actualiza los cambios de estación de invierno
        	_cfg.viHeader = ((Blob::Season_t*)st_msg->msg)->header;

        	// recalcula la tabla de datos
        	memset(&_cfg.data[MaxAllowedSeasonDataInArray], -1, sizeof(_cfg.data)/2);
        	memcpy(&_cfg.data[MaxAllowedSeasonDataInArray], ((Blob::Season_t*)st_msg->msg)->content.data, _cfg.viHeader.numRecords * sizeof(uint32_t));

        	// almacena en el sistema de ficheros
        	_fs->save("AstCalVIHeader", &_cfg.viHeader, sizeof(_cfg.viHeader), NVSInterface::TypeBlob);
        	_fs->save("AstCalVIData", &_cfg.data[MaxAllowedSeasonDataInArray], _cfg.viHeader.numRecords * sizeof(uint32_t), NVSInterface::TypeBlob);

        	DEBUG_TRACE("\r\n[AstCal]\t VIConfig actualizada");

        	// si está habilitada la notificación de actualización, lo notifica
        	if((_cfg.updFlagMask & Blob::EnableVIUpdNotif) != 0){
				char* pub_topic = (char*)Heap::memAlloc(MQ::MQClient::getMaxTopicLen());
				MBED_ASSERT(pub_topic);
				sprintf(pub_topic, "%s/vi/stat", _pub_topic_base);
				Blob::Season_t* season = (Blob::Season_t*)Heap::memAlloc(sizeof(Blob::Season_t));
				MBED_ASSERT(season);
				season->header = _cfg.viHeader;
				memset(season->content.data, -1, sizeof(season->content.data));
				memcpy(season->content.data, &_cfg.data[MaxAllowedSeasonDataInArray], _cfg.viHeader.numRecords * sizeof(uint32_t));
				MQ::MQClient::publish(pub_topic, season, sizeof(Blob::Season_t), &_publicationCb);
				Heap::memFree(season);
				Heap::memFree(pub_topic);
        	}

            // libera el mensaje (tanto el contenedor de datos como el propio mensaje)
        	Heap::memFree(st_msg->msg);
            Heap::memFree(st_msg);

            return State::HANDLED;
        }

        // Procesa datos recibidos de la publicación en $BASE/evt/cmd
        case RecvEvtCmd:{
        	// actualiza los flags evt
			_cfg.evtFlagMask = (Blob::AstCalEvtFlags)*((uint32_t*)st_msg->msg);

        	// almacena en el sistema de ficheros
        	_fs->save("AstCalEvtFlags", &_cfg.evtFlagMask, sizeof(uint32_t), NVSInterface::TypeUint32);

			DEBUG_TRACE("\r\n[AstCal]\t AstCalEvtFlags actualizados [%d]", _cfg.evtFlagMask);

			// si está habilitada la notificación de actualización, lo notifica
        	if((_cfg.updFlagMask & Blob::EnableEvtUpdNotif) != 0){
				char* pub_topic = (char*)Heap::memAlloc(MQ::MQClient::getMaxTopicLen());
				MBED_ASSERT(pub_topic);
				sprintf(pub_topic, "%s/evt/stat", _pub_topic_base);
				MQ::MQClient::publish(pub_topic, &_cfg.evtFlagMask, sizeof(uint32_t), &_publicationCb);
				Heap::memFree(pub_topic);
        	}

            // libera el mensaje (tanto el contenedor de datos como el propio mensaje)
        	Heap::memFree(st_msg->msg);
            Heap::memFree(st_msg);

            return State::HANDLED;
        }

        // Procesa datos recibidos de la publicación en $BASE/ast/cmd
        case RecvAstCmd:{
        	// actualiza la configuración completa
        	_cfg.astCfg = *((Blob::AstCalAstData_t*)st_msg->msg);

        	// almacena en el sistema de ficheros
        	_fs->save("AstCalAstData", &_cfg.astCfg, sizeof(Blob::AstCalAstData_t), NVSInterface::TypeBlob);

        	DEBUG_TRACE("\r\n[AstCal]\t AstCalAstData actualizado");

        	// si está habilitada la notificación de actualización, lo notifica
        	if((_cfg.updFlagMask & Blob::EnableAstUpdNotif) != 0){
				char* pub_topic = (char*)Heap::memAlloc(MQ::MQClient::getMaxTopicLen());
				MBED_ASSERT(pub_topic);
				sprintf(pub_topic, "%s/ast/stat", _pub_topic_base);
				MQ::MQClient::publish(pub_topic, &_cfg.astCfg, sizeof(Blob::AstCalAstData_t), &_publicationCb);
				Heap::memFree(pub_topic);
        	}

            // libera el mensaje (tanto el contenedor de datos como el propio mensaje)
        	Heap::memFree(st_msg->msg);
            Heap::memFree(st_msg);

            return State::HANDLED;
        }

        // Procesa datos recibidos de la publicación en $BASE/cfg/cmd
        case RecvCfgCmd:{
        	// actualiza la configuración completa
        	_cfg.updFlagMask = ((Blob::AstCalCfgData_t*)st_msg->msg)->updFlagMask;
        	_cfg.evtFlagMask = ((Blob::AstCalCfgData_t*)st_msg->msg)->evtFlagMask;
        	_cfg.ivHeader = ((Blob::AstCalCfgData_t*)st_msg->msg)->ivHeader;
        	_cfg.viHeader = ((Blob::AstCalCfgData_t*)st_msg->msg)->viHeader;
        	_cfg.astCfg = ((Blob::AstCalCfgData_t*)st_msg->msg)->astCfg;
        	// recalcula la tabla de datos
        	memset(_cfg.data, -1, sizeof(_cfg.data));
        	memcpy(&_cfg.data[0], ((Blob::AstCalCfgData_t*)st_msg->msg)->data, _cfg.ivHeader.numRecords * sizeof(uint32_t));
        	memcpy(&_cfg.data[MaxAllowedSeasonDataInArray], &((Blob::AstCalCfgData_t*)st_msg->msg)->data[_cfg.ivHeader.numRecords], _cfg.viHeader.numRecords * sizeof(uint32_t));

        	// almacena en el sistema de ficheros
        	_fs->save("AstCalUpdFlags", &_cfg.updFlagMask, sizeof(uint32_t), NVSInterface::TypeUint32);
        	_fs->save("AstCalEvtFlags", &_cfg.evtFlagMask, sizeof(uint32_t), NVSInterface::TypeUint32);
        	_fs->save("AstCalAstData", &_cfg.astCfg, sizeof(Blob::AstCalAstData_t), NVSInterface::TypeBlob);
        	_fs->save("AstCalIVHeader", &_cfg.ivHeader, sizeof(_cfg.ivHeader), NVSInterface::TypeBlob);
        	_fs->save("AstCalIVData", &_cfg.data[0], _cfg.ivHeader.numRecords * sizeof(uint32_t), NVSInterface::TypeBlob);
        	_fs->save("AstCalVIHeader", &_cfg.viHeader, sizeof(_cfg.viHeader), NVSInterface::TypeBlob);
        	_fs->save("AstCalVIData", &_cfg.data[MaxAllowedSeasonDataInArray], _cfg.viHeader.numRecords * sizeof(uint32_t), NVSInterface::TypeBlob);

        	DEBUG_TRACE("\r\n[AstCal]\t AstCalConfig actualizada");

        	// si está habilitada la notificación de actualización, lo notifica
        	if((_cfg.updFlagMask & Blob::EnableCfgUpdNotif) != 0){
				char* pub_topic = (char*)Heap::memAlloc(MQ::MQClient::getMaxTopicLen());
				MBED_ASSERT(pub_topic);
				sprintf(pub_topic, "%s/cfg/stat", _pub_topic_base);
				MQ::MQClient::publish(pub_topic, &_cfg, sizeof(Blob::AstCalCfgData_t), &_publicationCb);
				Heap::memFree(pub_topic);
        	}

            // libera el mensaje (tanto el contenedor de datos como el propio mensaje)
        	Heap::memFree(st_msg->msg);
            Heap::memFree(st_msg);

            return State::HANDLED;
        }

        case State::EV_EXIT:{
            nextState();
            return State::HANDLED;
        }

        default:{
        	return State::IGNORED;
        }

     }
}


//------------------------------------------------------------------------------------
void RGBGame::putMessage(State::Msg *msg){
    _queue.put(msg);
}


//------------------------------------------------------------------------------------
osEvent RGBGame:: getOsEvent(){
	return _queue.get();
}



//------------------------------------------------------------------------------------
void RGBGame::publicationCb(const char* topic, int32_t result){

}


//------------------------------------------------------------------------------------
bool RGBGame::checkIntegrity(){
	// verifico rango gmt
	if(_cfg.ivHeader.gmt < -12 || _cfg.ivHeader.gmt > 12 || _cfg.viHeader.gmt < -12 || _cfg.viHeader.gmt > 12){
		setDefaultConfig();
		return false;
	}
	// verifico iyear
	if(_cfg.ivHeader.iyear < 2018  || _cfg.viHeader.iyear < 2018){
		setDefaultConfig();
		return false;
	}
	// verifico tamaño de datos
	if(_cfg.ivHeader.numRecords > MaxAllowedSeasonDataInArray  || _cfg.viHeader.numRecords > MaxAllowedSeasonDataInArray){
		setDefaultConfig();
		return false;
	}
	// verifico tablas de datos (el timestamp irá en segundos de 0 (1 ene 00:00:00) a 31622400 (31 dic 23:59:59)
	for(uint32_t i=0; i<_cfg.ivHeader.numRecords; i++){
		if(_cfg.data[i] >= Blob::TimestampSecondsYearLimit){
			setDefaultConfig();
			return false;
		}
	}
	for(uint32_t i=0; i<_cfg.viHeader.numRecords; i++){
		if(_cfg.data[MaxAllowedSeasonDataInArray+i] >= Blob::TimestampSecondsYearLimit){
			setDefaultConfig();
			return false;
		}
	}
	// verifico rangos astCfg
	if(_cfg.astCfg.reductionStart > Blob::TimestampMinutesDayLimit || _cfg.astCfg.reductionStop > Blob::TimestampMinutesDayLimit ||
	   _cfg.astCfg.wdowDawnStart > Blob::TimestampMinutesDayLimit || _cfg.astCfg.wdowDawnStop > Blob::TimestampMinutesDayLimit ||
	   _cfg.astCfg.wdowDuskStart > Blob::TimestampMinutesDayLimit || _cfg.astCfg.wdowDuskStop > Blob::TimestampMinutesDayLimit){
		setDefaultConfig();
		return false;
	}

	return true;
}


//------------------------------------------------------------------------------------
void RGBGame::setDefaultConfig(){
	_cfg.updFlagMask = (Blob::AstCalUpdFlags)(Blob::EnableIVUpdNotif | Blob::EnableVIUpdNotif | Blob::EnableEvtUpdNotif | Blob::EnableCfgUpdNotif);
	_cfg.evtFlagMask = Blob::AstCalEvtFlags(Blob::IVEvt | Blob::VIEvt | Blob::DayEvt | Blob::DawnEvt | Blob::DuskEvt | Blob::MinEvt | Blob::SecEvt);
	_cfg.astCfg = {0, 0, 0, 0, 0, 0};
	_cfg.ivHeader = {0, 2018, MaxAllowedSeasonDataInArray};
	_cfg.viHeader = {0, 2018, MaxAllowedSeasonDataInArray};
	memset(&_cfg.data[0], 0, MaxAllowedSeasonDataInArray * sizeof(uint32_t));
	memset(&_cfg.data[MaxAllowedSeasonDataInArray], 0, MaxAllowedSeasonDataInArray * sizeof(uint32_t));
	// almacena en el sistema de ficheros
	_fs->save("AstCalUpdFlags", &_cfg.updFlagMask, sizeof(uint32_t), NVSInterface::TypeUint32);
	_fs->save("AstCalEvtFlags", &_cfg.evtFlagMask, sizeof(uint32_t), NVSInterface::TypeUint32);
	_fs->save("AstCalAstData", &_cfg.astCfg, sizeof(Blob::AstCalAstData_t), NVSInterface::TypeBlob);
	_fs->save("AstCalIVHeader", &_cfg.ivHeader, sizeof(_cfg.ivHeader), NVSInterface::TypeBlob);
	_fs->save("AstCalIVData", &_cfg.data[0], _cfg.ivHeader.numRecords * sizeof(uint32_t), NVSInterface::TypeBlob);
	_fs->save("AstCalVIHeader", &_cfg.viHeader, sizeof(_cfg.viHeader), NVSInterface::TypeBlob);
	_fs->save("AstCalVIData", &_cfg.data[MaxAllowedSeasonDataInArray], _cfg.viHeader.numRecords * sizeof(uint32_t), NVSInterface::TypeBlob);
}

