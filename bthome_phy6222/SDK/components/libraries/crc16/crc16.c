/*
    crc16
*/

#include "crc16.h"



static uint16_t crc16_byte(uint16_t crc, uint8_t byte)
{
    static const uint16_t crc16_table[16] =
    {
        0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401,
        0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400
    };
    uint16_t temp;
    // Compute checksum of lower four bits of a byte.
    temp         = crc16_table[crc & 0xF];
    crc  = (crc >> 4u) & 0x0FFFu;
    crc  = crc ^ temp ^ crc16_table[byte & 0xF];
    // Now compute checksum of upper four bits of a byte.
    temp         = crc16_table[crc & 0xF];
    crc  = (crc >> 4u) & 0x0FFFu;
    crc  = crc ^ temp ^ crc16_table[(byte >> 4u) & 0xF];
    return crc;
}


uint16_t crc16(uint16_t seed, const volatile void* p_data, uint32_t size)
{
    uint8_t* p_block = (uint8_t*)p_data;

    while (size != 0)
    {
        seed = crc16_byte(seed, *p_block);
        p_block++;
        size--;
    }

    return seed;
}
