/*
 * crc32.h
 *
 *  Created on: Jun 22, 2020
 *      Author: BJones
 */

#ifndef __CRC32_H_
#define __CRC32_H_

#include <stdint.h>

namespace icsneo {
/*
 * When any data/buffer is run through calCRC(), then the resulting CRC value
 *  is appended to the end of the data/buffer and the data/buffer is rerun
 *  through calCRC(), the result will be CRC32_IDENT
 */
#define CRC32_IDENT (0x2144DF1C) /**< CRC check value */

#define CRC32_ISVALID(crc) ((crc) == CRC32_IDENT)

uint32_t crc32(uint32_t crc, const unsigned char* buf, uint32_t len);
uint32_t revcrc32(uint32_t crc, const unsigned char* buf, uint32_t len);

}

#endif // CRC32_H_
