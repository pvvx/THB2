#ifndef _CRC16_H__
#define _CRC16_H__

#include <stdint.h>

uint16_t crc16(uint16_t seed, const volatile void* p_data, uint32_t size);

#endif // _CRC16_H__

