/*
 * =====================================================================================
 *
 *       Filename:  pubsub.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/30/2018 10:17:04
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef PUBSUB_H_
#define PUBSUB_H_

void subscribe(const std::string event, QueueHandle_t *queue);

#endif
