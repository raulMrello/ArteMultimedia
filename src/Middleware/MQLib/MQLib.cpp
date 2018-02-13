/*
 * MQLib.cpp
 *
 *  Versión: 13 Feb 2018
 *  Author: raulMrello
 *
 */

#include "MQLib.h"

/** Mutex para MQBroker */

Mutex MQ::MQBroker::_mutex;

/** Lista de topics */
List<MQ::Topic> * MQ::MQBroker::_topic_list = 0;

/** Variables para el control de tokens y topics */
bool MQ::MQBroker::_tokenlist_internal = false;
const char** MQ::MQBroker::_token_provider = 0;
uint32_t MQ::MQBroker::_token_provider_count = 0;
uint8_t MQ::MQBroker::_token_bits = 0;
uint8_t MQ::MQBroker::_max_name_len = 0;
bool MQ::MQBroker::_defdbg = false;

