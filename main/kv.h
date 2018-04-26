/*
 * =====================================================================================
 *
 *       Filename:  kv.h
 *
 *    Description: 
 *
 *        Version:  1.0
 *        Created:  04/16/2018 15:47:38
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef KV_H_
#define KV_H_

#include <string>

void init_kv();

int geti(std::string key);
void seti(std::string key, int value);
bool hasi(std::string key);
void defaulti(std::string key, int value);

std::string getstr(std::string key);
void setstr(std::string key, std::string value);
bool hasstr(std::string key);
void defaultstr(std::string key, std::string value);

void bleSynci(std::string service, std::string uuid, std::string key);
void bleSyncstr(std::string service, std::string uuid, std::string key);

#endif
