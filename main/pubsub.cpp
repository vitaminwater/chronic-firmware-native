/*
 * =====================================================================================
 *
 *       Filename:  pubsub.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/30/2018 10:16:55
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#include <iostream>
#include <map>
#include <list>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

std::map<const std::string, std::list<QueueHandle_t *> *> subs; 

void subscribe(const std::string event, QueueHandle_t *queue) {
  if (!subs[event]) {
    subs[event] = new std::list<QueueHandle_t *>();
  }
  subs[event]->push_back(queue);
}

void publish(const std::string event, void *param) {
  if (!subs[event]) {
    return;
  }
  for (std::list<QueueHandle_t *>::iterator it = subs[event]->begin(); it != subs[event]->end(); ++it) {
    xQueueSend(*it, param, 0);
  }
}
