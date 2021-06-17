#ifndef RADIO_PACKET_H_
#define RADIO_PACKET_H_

#include <stdint.h>

#ifdef __GNUC__
#define PACK(__Declaration__) __Declaration__ __attribute__((__packed__))
#endif

#ifdef _MSC_VER
#define PACK(__Declaration__) __pragma(pack(push, 1)) __Declaration__ __pragma(pack(pop))
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef union {
    float FLOAT;
    uint32_t UINT;
    int32_t  INT;
    uint8_t BYTES[8];
} radio_packet_payload_t;

PACK(struct radio_packet {
    uint8_t node;
    uint8_t message_id;
    radio_packet_payload_t payload;
    uint8_t checksum;
});
typedef struct radio_packet radio_packet_t;

uint8_t radio_compute_crc(radio_packet_t* packet);

#ifdef __cplusplus
}
#endif

#endif
