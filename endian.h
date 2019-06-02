/* jrra 2019 */

#ifndef _ENDIAN_H_
#define _ENDIAN_H_

#ifdef _WIN32
#if BYTE_ORDER == LITTLE_ENDIAN
#define ntohl(x) __builtin_bswap32(x)
#define htonl(x) __builtin_bswap32(x)
#elif BYTE_ORDER == BIG_ENDIAN
#define ntohl(x) ((uint32_t)(x))
#define htonl(x) ((uint32_t)(x))
#endif
#else
#include <arpa/inet.h>
#endif

/* _ENDIAN_H_ */
#endif
