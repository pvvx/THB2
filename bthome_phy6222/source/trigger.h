/*
 * trigger.h
 *
 *  Created on: 5 февр. 2024 г.
 *      Author: pvvx
 */

#ifndef _TRIGGER_H_
#define _TRIGGER_H_
#include "config.h"
#if (DEV_SERVICES & SERVICE_THS)

typedef struct _trigger_t {
#if (DEV_SERVICES & SERVICE_SCREEN)
	int16_t temp_min; // x0.01°,
	int16_t temp_max; // x0.01°,
	int16_t humi_min; // x0.01%,
	int16_t humi_max; // x0.01%,
#endif
#if (DEV_SERVICES & SERVICE_TH_TRG)
	int16_t temp_threshold; // x0.01°, temp threshold
	int16_t humi_threshold; // x0.01%, humi threshold
	int16_t temp_hysteresis; // temp hysteresis, -327.67..327.67 °
	int16_t humi_hysteresis; // humi hysteresis, -327.67..327.67 %
	uint8_t	cfg;
#endif
}trigger_t;
#if (DEV_SERVICES & SERVICE_SCREEN)
 #if (DEV_SERVICES & SERVICE_TH_TRG)
	#define trigger_send_size		17
 #else
	#define trigger_send_size		9
 #endif
#else
 #if (DEV_SERVICES & SERVICE_TH_TRG)
	#define trigger_send_size		9
 #else
	#define trigger_send_size		0
 #endif
#endif
extern trigger_t trg;
extern const trigger_t def_trg;

#define TRG_CFG_OUT_INV		1 // инверсия вывода trg

void set_trigger_out(void);

#endif // SERVICE_THS

#endif /* _TRIGGER_H_ */
