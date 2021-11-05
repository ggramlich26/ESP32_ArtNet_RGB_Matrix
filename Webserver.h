/*
 * webserver.h
 *
 *  Created on: Feb 9, 2020
 *      Author: tsugua
 */

#ifndef WEBSERVER_H_
#define WEBSERVER_H_

#ifdef __cplusplus
extern "C" {
#endif

void webserver_init_for_wifi();
void webserver_init_for_ethernet();
void webserver_update_for_ethernet();

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* WEBSERVER_H_ */
