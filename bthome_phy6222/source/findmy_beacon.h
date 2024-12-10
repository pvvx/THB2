/*
 * findmy_beacon.h
 *
 *  Created on: 8 дек. 2024 г.
 *      Author: pvvx
 */

#ifndef SOURCE_FINDMY_BEACON_H_
#define SOURCE_FINDMY_BEACON_H_

#define SIZE_FINDMY_KEY		28

extern uint8_t findmy_key[SIZE_FINDMY_KEY];
extern uint8_t findmy_key_new[SIZE_FINDMY_KEY];

uint8_t findmy_beacon(void * padbuf);

#endif /* SOURCE_FINDMY_BEACON_H_ */
