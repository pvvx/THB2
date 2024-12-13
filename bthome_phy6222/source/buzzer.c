/*
 * buzzer.c
 *
 *  Created on: 13 дек. 2024 г.
 *      Author: pvvx
 */
#include "rom_sym_def.h"
#include "types.h"
#include "config.h"

#if defined(GPIO_BUZZER) && defined(PWM_CHL_BUZZER)

#include "pwm.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"
#include "pwrmgr.h"
#include "thb2_main.h"

#define NOTE_VOLUME0 0x00 // min x1
#define NOTE_VOLUME1 0x40 // x2
#define NOTE_VOLUME2 0x80 // x4
#define NOTE_VOLUME3 0xC0 // max x8

#define NOTE_VOLUME NOTE_VOLUME3

#define NOTE_REST 0xff


#define NOTE_C4  0x20+NOTE_VOLUME
#define NOTE_CS4 0x21+NOTE_VOLUME
#define NOTE_D4  0x22+NOTE_VOLUME
#define NOTE_DS4 0x23+NOTE_VOLUME
#define NOTE_E4  0x24+NOTE_VOLUME
#define NOTE_F4  0x25+NOTE_VOLUME
#define NOTE_FS4 0x26+NOTE_VOLUME
#define NOTE_G4  0x27+NOTE_VOLUME
#define NOTE_GS4 0x28+NOTE_VOLUME
#define NOTE_A4  0x29+NOTE_VOLUME
#define NOTE_AS4 0x2A+NOTE_VOLUME
#define NOTE_B4  0x2B+NOTE_VOLUME

#define NOTE_C5  0x10+NOTE_VOLUME
#define NOTE_CS5 0x11+NOTE_VOLUME
#define NOTE_D5  0x12+NOTE_VOLUME
#define NOTE_DS5 0x13+NOTE_VOLUME
#define NOTE_E5  0x14+NOTE_VOLUME
#define NOTE_F5  0x15+NOTE_VOLUME
#define NOTE_FS5 0x16+NOTE_VOLUME
#define NOTE_G5  0x17+NOTE_VOLUME
#define NOTE_GS5 0x18+NOTE_VOLUME
#define NOTE_A5  0x19+NOTE_VOLUME
#define NOTE_AS5 0x1A+NOTE_VOLUME
#define NOTE_B5  0x1B+NOTE_VOLUME

#define NOTE_C6  0x00+NOTE_VOLUME
#define NOTE_CS6 0x01+NOTE_VOLUME
#define NOTE_D6  0x02+NOTE_VOLUME
#define NOTE_DS6 0x03+NOTE_VOLUME
#define NOTE_E6  0x04+NOTE_VOLUME
#define NOTE_F6  0x05+NOTE_VOLUME
#define NOTE_FS6 0x06+NOTE_VOLUME
#define NOTE_G6  0x07+NOTE_VOLUME
#define NOTE_GS6 0x08+NOTE_VOLUME
#define NOTE_A6  0x09+NOTE_VOLUME
#define NOTE_AS6 0x0A+NOTE_VOLUME
#define NOTE_B6  0x0B+NOTE_VOLUME

#define NOTE_C7  0x0C+NOTE_VOLUME
#define NOTE_CS7 0x0D+NOTE_VOLUME
#define NOTE_D7  0x0E+NOTE_VOLUME
#define NOTE_DS7 0x0F+NOTE_VOLUME


#define N_T32 	1 // 1/16
#define N_T16 	2 // 1/16
#define N_T16N  3 // 1.5/16
#define N_T8 	4 // 1/8
#define N_T8N   6 // 1.5/8
#define N_T4 	8 // 1/4
#define N_T4N   12 // 1.5/4

#define NOTE_TEMP 55

const uint8_t melody[] = {  // Note, Time in 10 ms
    NOTE_A5,  N_T16, NOTE_D5,  N_T16, NOTE_AS5,  N_T16, NOTE_D5,  N_T16, NOTE_A5,  N_T16, NOTE_D5,  N_T16, NOTE_G5,  N_T16, NOTE_D5,  N_T16,
    NOTE_A5,  N_T16, NOTE_D5,  N_T16, NOTE_AS5,  N_T16, NOTE_D5,  N_T16, NOTE_C6,  N_T16, NOTE_D5,  N_T16, NOTE_AS5,  N_T16, NOTE_D5,  N_T16,
    NOTE_A5,  N_T16, NOTE_D5,  N_T16, NOTE_F5,  N_T16, NOTE_D5,  N_T16, NOTE_A5,  N_T16, NOTE_D5,  N_T16, NOTE_G5,  N_T16, NOTE_D5,  N_T16,
    NOTE_C6,  N_T16, NOTE_C6,  N_T16, NOTE_F6,  N_T16, NOTE_D6,  N_T8, NOTE_REST,  N_T16, NOTE_REST,  N_T8,

	NOTE_REST, 30,

	NOTE_FS5,N_T8, NOTE_FS5,N_T8,NOTE_D5,N_T8, NOTE_B4,N_T8, NOTE_REST,N_T8, NOTE_B4,N_T8, NOTE_REST,N_T8, NOTE_E5,N_T8,
	NOTE_REST,N_T8, NOTE_E5,N_T8, NOTE_REST,N_T8, NOTE_E5,N_T8, NOTE_GS5,N_T4, NOTE_A5,N_T8, NOTE_B5,N_T8,
	NOTE_A5,N_T8, NOTE_A5,N_T8, NOTE_A5,N_T8, NOTE_E5,N_T8, NOTE_REST,N_T8, NOTE_D5,N_T8, NOTE_REST,N_T8, NOTE_FS5,N_T8,
	NOTE_REST,N_T8, NOTE_FS5,N_T8, NOTE_REST,N_T8, NOTE_FS5,N_T8, NOTE_E5,N_T4, NOTE_FS5,N_T8, NOTE_E5,N_T8,
	NOTE_FS5,N_T4,NOTE_D5,N_T8, NOTE_B4,N_T8, NOTE_REST,N_T8, NOTE_B4,N_T8, NOTE_REST,N_T8, NOTE_E5,N_T8,

	NOTE_REST,N_T8, NOTE_E5,N_T8, NOTE_REST,N_T8, NOTE_E5,N_T8, NOTE_GS5,N_T4, NOTE_A5,N_T8, NOTE_B5,N_T8,
	NOTE_A5,N_T4N, NOTE_E5,N_T8, NOTE_REST,N_T8, NOTE_D5,N_T8, NOTE_REST,N_T8, NOTE_FS5,N_T8,
	NOTE_REST,N_T8, NOTE_FS5,N_T8, NOTE_REST,N_T8, NOTE_FS5,N_T8, NOTE_E5,N_T4, NOTE_FS5,N_T8, NOTE_E5,N_T8,
	NOTE_FS5,N_T4, NOTE_D5,N_T8, NOTE_B4,N_T8, NOTE_REST,N_T8, NOTE_B4,N_T8, NOTE_REST,N_T8, NOTE_E5,N_T8,
	NOTE_REST,N_T8, NOTE_E5,N_T8, NOTE_REST,N_T8, NOTE_E5,N_T8, NOTE_GS5,N_T4, NOTE_A5,N_T8, NOTE_B5,N_T8,

	NOTE_A5,N_T4N, NOTE_E5,N_T8, NOTE_REST,N_T8, NOTE_D5,N_T8, NOTE_REST,N_T8, NOTE_FS5,N_T8,
	NOTE_REST,N_T8, NOTE_FS5,N_T8, NOTE_REST,N_T8, NOTE_FS5,N_T8, NOTE_E5,N_T4, NOTE_FS5,N_T8, NOTE_E5,N_T8,

	NOTE_REST, 30
};


static const uint16_t buzzer_tone[] = {
		160000000/20930, //0 C7 2093.0 Hz 10465
		160000000/22174, //1 C#7          11087
		160000000/23492, //2 D7           11746
		160000000/24890, //3 D#7          12445
		160000000/26370, //4 E7           13185
		160000000/27938, //5 F7           13969
		160000000/29600, //6 F#7          14800
		160000000/31360, //7 G7           15680
		160000000/33324, //8 G#7          16662
		160000000/34400, //9 A7           17200
		160000000/37292, //A A#7          18646
		160000000/39510, //B B7           19755
		160000000/41860, //C C8           20930
		160000000/44348, //D C#8          22174
		160000000/46984, //E D8           23492
		160000000/49780  //F D#8          24890
};

/* tone:
 * bit[0:3] - Note: 0..15 (C..D#)
 * bit[4:5] - Octave: 0..3
 * bit[6:7] - Volume: 0..3
*/
uint8_t pwm_buzzer_enable = 0;
uint16_t pwm_buzzer_note_idx;

static void set_buzzer_pwm(uint8_t tone) {
    PWM_DISABLE_CH(PWM_CHL_BUZZER);
   	uint32_t pwmDiv = ((tone >> 4) & 0x3);
	uint32_t cmpVal = (64 << (tone >> 6)) >> pwmDiv;

    PWM_SET_DIV(PWM_CHL_BUZZER, pwmDiv);
	// hal_pwm_set_count_val(PWM_CHL_BUZZER, cmpVal, cntTopVal);
    PWM_NO_LOAD_CH(PWM_CHL_BUZZER);
    PWM_SET_CMP_VAL(PWM_CHL_BUZZER, cmpVal);
    PWM_SET_TOP_VAL(PWM_CHL_BUZZER, buzzer_tone[tone & 0x0f]);
    PWM_LOAD_CH(PWM_CHL_BUZZER);
	PWM_ENABLE_CH(PWM_CHL_BUZZER);
}

static void pwm_buzzer_note(uint8_t tone) {
	if(tone == NOTE_REST) {
	    hal_gpio_pull_set(GPIO_BUZZER, GPIO_PULL_UP | BUZZER_ON);
	    hal_gpio_fmux(GPIO_BUZZER, Bit_DISABLE);
		PWM_DISABLE_ALL;
	    hal_clk_gate_disable(MOD_PWM);
	    hal_pwrmgr_unlock(MOD_PWM);
	    pwm_buzzer_enable = 0;
	} else {
		if(!pwm_buzzer_enable) {
		    hal_pwrmgr_register(MOD_PWM, NULL, NULL);
			hal_pwrmgr_lock(MOD_PWM);
		    hal_clk_gate_enable(MOD_PWM);
		    set_buzzer_pwm(tone);
		    PWM_SET_MODE(PWM_CHL_BUZZER, PWM_CNT_UP);
		    PWM_SET_POL(PWM_CHL_BUZZER, BUZZER_ON);
		    PWM_INSTANT_LOAD_CH(PWM_CHL_BUZZER);

			hal_gpio_pull_set(GPIO_BUZZER, GPIO_FLOATING);
			hal_gpio_fmux_set(GPIO_BUZZER, (gpio_fmux_e)(FMUX_PWM0 + PWM_CHL_BUZZER));

			PWM_ENABLE_ALL;
			pwm_buzzer_enable = 1;
		} else {
		    set_buzzer_pwm(tone);
		}
	}
}

void pwm_buzzer_event(void) {
	pwm_buzzer_note(melody[pwm_buzzer_note_idx*2]);
	osal_start_timerEx(simpleBLEPeripheral_TaskID, BUZZER_TONE_EVT, melody[pwm_buzzer_note_idx*2 + 1] * NOTE_TEMP);
	if(++pwm_buzzer_note_idx >= sizeof(melody)/2)
		pwm_buzzer_note_idx = 0;
}


void pwm_buzzer_start(void) {
	osal_stop_timerEx(simpleBLEPeripheral_TaskID, BUZZER_TONE_EVT);
	pwm_buzzer_note_idx = 0;
	pwm_buzzer_event();
}

void pwm_buzzer_stop(void) {
	PWM_DISABLE_ALL;
    hal_pwrmgr_unlock(MOD_PWM);
    hal_clk_gate_disable(MOD_PWM);
    pwm_buzzer_enable = 0;
    hal_gpio_pull_set(GPIO_BUZZER, GPIO_PULL_UP | BUZZER_ON);
	osal_stop_timerEx(simpleBLEPeripheral_TaskID, BUZZER_TONE_EVT);
}



#endif // GPIO_BUZZER
