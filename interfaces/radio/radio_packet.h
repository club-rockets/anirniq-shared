#ifndef RADIO_PACKET_H_
#define RADIO_PACKET_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef union {
    float FLOAT;
    uint32_t UINT;
    int32_t  INT;
    uint8_t BYTES[8];
} radio_packet_payload_t;

typedef struct __attribute__((__packed__)){
    uint8_t node;
    uint8_t message_id;
    radio_packet_payload_t payload;
    uint8_t checksum;
} radio_packet_t;

uint8_t radio_compute_crc(radio_packet_t* packet);

#ifdef __cplusplus
}
#endif

#endif
