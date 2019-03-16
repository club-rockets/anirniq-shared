#ifndef CANSTUFF_H
#define CANSTUFF_H

#include "stdint.h"
#include "stm32f407xx.h"

// defines the software receive fifo size for fifo 0 anf fifo 1
#define CAN_SF_FIFO0_SIZE 10
#define CAN_SF_FIFO1_SIZE 10

// error codes for rx sf fifo
#define RX_SF_FIFO_OVERRUN (1 << 0)

enum canOpMode { normal, loopback, silent, silentLoopback };

enum canFilterMode { id11Bit, mask11Bit, id29Bit, mask29Bit };

typedef struct {
    CAN_TypeDef *instance;
    uint32_t baudPrescaler;
    uint32_t timeQuanta1;
    uint32_t timeQuanta2;
    uint32_t timeReSync;
    enum canOpMode opMode;
    uint32_t debugFreeze; // true to freese can peripheral during debug

} canInstance_t;

typedef struct __attribute__((__packed__)) {
    // header (id register)
    unsigned TXRQ : 1;  // transmit request
    unsigned RTR : 1;   // remote transmit request
    unsigned IDE : 1;   // extended id flag
    unsigned EXID : 18; // extended id
    unsigned STID : 11; // standad id

    // data length and control
    unsigned DLC : 4;   // Data length
    unsigned : 4;       // reserved
    unsigned TGT : 1;   // timestamp flag
    unsigned : 7;       // reserved
    unsigned TIME : 16; // timestamp

    // data
    uint8_t data[8]; // data

} canTXpacket_t;

typedef struct __attribute__((__packed__)) {
    // header (id register)
    unsigned : 1;       // reserved
    unsigned RTR : 1;   // remote transmit request
    unsigned IDE : 1;   // extended id flag
    unsigned EXID : 18; // extended id
    unsigned STID : 11; // standad id

    // length control and time stamp
    unsigned DLC : 4;   // Data length
    unsigned : 4;       // reserved
    unsigned FMI : 8;   // filter match index
    unsigned TIME : 16; // timestamp

    // data
    uint8_t data[8]; // data

} canRXpacket_t;

// can mailbox struct 11 bit mode for now
// same typedef for mask and list
typedef union {
    // ID1_1 and 1_2 will show as same index in filter match index
    struct __attribute__((__packed__)) {
        unsigned : 5;
        unsigned ID0 : 11;
        unsigned : 5;
        unsigned ID1 : 11;
        unsigned : 5;
        unsigned ID2 : 11;
        unsigned : 5;
        unsigned ID3 : 11;
    } list11;

    struct __attribute__((__packed__)) {
        unsigned : 5;
        unsigned ID0 : 11;
        unsigned : 5;
        unsigned mask0 : 11;
        unsigned : 5;
        unsigned ID1 : 11;
        unsigned : 5;
        unsigned mask1 : 11;
    } mask11;
} canFilter_t;

// initialise the can1 peripheral using the settings in the canInstance_t typedef
// IO's are not initialised by this fuction
void canInit(canInstance_t *instance);

// set the filter to the rignt mode
void canSetFilter(canInstance_t *instance, canFilter_t *filter, enum canFilterMode mode, uint32_t filterId,
                  uint32_t fifoId);

// init the fifo0 for interrupt use
void can1Fifo0InitIt(canInstance_t *instance);
// De init the fifo 0 interrupt
void can1Fifo0DeInitIt(canInstance_t *instance);
// can1 fifo0 register Rx Callback (0 as parameter to unregeister)
void can1Fifo0RegisterCallback(void (*callback)(void));
// get message from sf fifo return positive when new message available
uint32_t can1SfFifo0Get(canRXpacket_t *packet);

// init the fifo1 for interrupt use
void can1Fifo1InitIt(canInstance_t *instance);
// De init the fifo 1 interrupt
void can1Fifo1DeInitIt(canInstance_t *instance);
// can1 fifo1 register Rx Callback (0 as parameter to unregeister)
void can1Fifo1RegisterCallback(void (*callback)(void));
// get message from sf fifo return positive when new message available
uint32_t can1SfFifo1Get(canRXpacket_t *packet);
// enable the interrupt on wakeup and automatic wakeup
void can1WakeItInit(void);
// enable the interrupt on wakeup and automatic wakeup
void can1WakeItDeInit(void);
// puts can1 to sleep
void can1Sleep(void);
// register a callback for the buss activity wake event
void can1WakeRegisterCallback(void (*callback)(void));
// send a packet return true if success
uint32_t canSendPacket(canInstance_t *instance, uint32_t id, uint32_t remote, uint32_t dataL, void *data);

// enable transmit complete interrupt
void can1TXItInit(void);
void can1TXItDeInit(void);
void can1TXRegisterCallback(void (*callback)(void));

// fifo id 0 or 1 return positive if success
uint32_t canPollFifo(canInstance_t *instance, canRXpacket_t *packet, uint32_t fifoId);

void canPrintRxPacket(canRXpacket_t *packet);

#endif // CANSTUFF_H
