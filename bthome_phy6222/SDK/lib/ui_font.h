/**************************************************************

    Module Name: font
    File name:   ui_font.h
    Brief description:
      UI display drvier
    Author:  Eagle.Lao
    Revision:V0.01

 SDK_LICENSE

****************************************************************/

#ifndef _UI_FONT_HEAD_
#define _UI_FONT_HEAD_

#include <stdint.h>


int utf8_to_unicode(const char* utf8, uint16_t* unicode);
int ui_font_unicode(void* font, uint16_t unicode,uint8_t* bitmap);
void* ui_font_load(uint32_t flash_addr);
const char*  ui_font_version(void);

#endif

