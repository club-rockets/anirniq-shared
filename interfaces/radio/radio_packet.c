#include "radio_packet.h"
#include "stdio.h"

uint8_t radio_compute_crc(radio_packet_t* packet)
{
	uint8_t sum = 0xFF + packet->node + packet->message_id;

	for (uint8_t i = 0; i < 8; i++) {
		sum += packet->payload.BYTES[i];
    }

    return ((uint8_t)~sum) + 1;
}