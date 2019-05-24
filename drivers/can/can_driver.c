

#include "string.h"
#include "can/can_driver.h"
#include "stm32f407xx.h"
#include "stdio.h"

#if CAN_SF_FIFO0_SIZE > 0
static volatile struct {
    canRXpacket_t rx[CAN_SF_FIFO0_SIZE];
    uint32_t st, end;
    uint32_t error;
} can1SfRxFifo0 = {0};
#endif

#if CAN_SF_FIFO1_SIZE > 0
static volatile struct {
    canRXpacket_t rx[CAN_SF_FIFO1_SIZE];
    uint32_t st, end;
    uint32_t error;
} can1SfRxFifo1 = {0};
#endif

//callbacks pointers
static void(*can1Fifo0Callback)(void) = 0;
static void(*can1Fifo1Callback)(void) = 0;
static void(*can1WakeCallback)(void) = 0;
static void(*can1TXCallback)(void) = 0;

void canInit(canInstance_t *instance)
{
    //activate can1 clock
    if(instance->instance == CAN1) {
        RCC->APB1ENR |= RCC_APB1ENR_CAN1EN;
    }

    //put can peripheral in initialisation mode
    instance->instance->MCR = CAN_MCR_INRQ;
    //wait for initialisation ack
    while(!(instance->instance->MSR & CAN_MSR_INAK));

    //setup can bit timing register
    instance->instance->BTR = 0;
    switch (instance->opMode) {
        case normal :
            break;
        case loopback :
            instance->instance->BTR |= CAN_BTR_LBKM;
            break;
        case silent :
            instance->instance->BTR |= CAN_BTR_SILM;
            break;
        case silentLoopback :
            instance->instance->BTR |= CAN_BTR_SILM;
            instance->instance->BTR |= CAN_BTR_LBKM;
    }
    //baudrate prescaler
    instance->instance->BTR |= (instance->baudPrescaler - 1);
    //timeQuanta1
    instance->instance->BTR |= (instance->timeQuanta1 - 1) << 16;
    //timeQuanta2
    instance->instance->BTR |= (instance->timeQuanta2 - 1) << 20;
    //reset jump width
    instance->instance->BTR |= (instance->timeReSync - 1) << 24;

    //setup master control register
    if(instance->debugFreeze) {
        instance->instance->MCR |= CAN_MCR_DBF;
    }

    //exit initialisation mode -> run mode
    instance->instance->MCR &=~ (CAN_MCR_INRQ | CAN_MCR_SLEEP);
    //WAIT FOR RUN MODE
    while((instance->instance->MSR & CAN_MSR_INAK) && (instance->instance->MSR & CAN_MSR_SLAK));

}

void canSetFilter(canInstance_t *instance,canFilter_t *filter, enum canFilterMode mode, uint32_t filterId, uint32_t fifoId)
{
    canFilter_t *filterPtr = (void*)(instance->instance->sFilterRegister);

    //set filter in init mode
    instance->instance->FMR |= CAN_FMR_FINIT;
    //copy the filter
    filterPtr[filterId] = *filter;
    //activate filter
    instance->instance->FA1R |= (1<<filterId);
    //set filter mode
    switch (mode)
    {
        case id11Bit:
            instance->instance->FM1R |= (1<<filterId);
            instance->instance->FS1R &=~(1<<filterId);
            break;
        case mask11Bit:
            instance->instance->FM1R &=~(1<<filterId);
            instance->instance->FS1R &=~(1<<filterId);
            break;
        case id29Bit:
            instance->instance->FM1R |= (1<<filterId);
            instance->instance->FS1R |= (1<<filterId);
            break;
        case mask29Bit:
            instance->instance->FM1R &=~(1<<filterId);
            instance->instance->FS1R |= (1<<filterId);
            break;
    }
    //fifo assignment
    if (fifoId) {
        instance->instance->FFA1R |= (1<<filterId);
    }
    else {
        instance->instance->FFA1R &=~(1<<filterId);
    }

    //leave init mode
    instance->instance->FMR &=~ CAN_FMR_FINIT;
}

void can1Fifo0InitIt(canInstance_t *instance)
{
    //Enable fifo 0 interrupt in NVIC
    if (instance->instance == CAN1) {
        NVIC->ISER[0] = 1<<20;
    }

    //enable fifo 0 message pending interrupt
    instance->instance->IER |= CAN_IER_FMPIE0;
}

void can1Fifo0DeInitIt(canInstance_t *instance)
{
    if (instance->instance == CAN1) {
        NVIC->ICER[0] = 1<<20;
    }

    instance->instance->IER &=~ (CAN_IER_FMPIE0);
}

//can1 fifo0 register Rx Callback (0 as parameter to unregeister)
void can1Fifo0RegisterCallback(void (*callback)(void))
{
    can1Fifo0Callback = callback;
}

#if CAN_SF_FIFO0_SIZE > 0
//get message from sf fifo return positive when new message available
uint32_t can1SfFifo0Get(canRXpacket_t *packet)
{
    if (can1SfRxFifo0.st != can1SfRxFifo0.end) {
        can1SfRxFifo0.st++;
        can1SfRxFifo0.st %= CAN_SF_FIFO0_SIZE;
        *packet = can1SfRxFifo0.rx[can1SfRxFifo0.st];
    }
    else {
        return 0;
    }
    return 1;
}
#endif

void can1Fifo1InitIt(canInstance_t *instance)
{
    //Enable fifo 1 interrupt in NVIC
    if(instance->instance == CAN1) {
        NVIC->ISER[0] = 1<<21;
    }

    //enable fifo 1 message pending interrupt
    instance->instance->IER |= CAN_IER_FMPIE1;
}

void can1Fifo1DeInitIt(canInstance_t *instance)
{
    if(instance->instance == CAN1) {
        NVIC->ICER[0] = 1 << 21;
    }

    instance->instance->IER &=~ (CAN_IER_FMPIE1);
}
//can1 fifo1 register Rx Callback (0 as parameter to unregeister)
void can1Fifo1RegisterCallback(void (*callback)(void))
{
    can1Fifo1Callback = callback;
}

#if CAN_SF_FIFO1_SIZE > 0
//get message from sf fifo return positive when new message available
uint32_t can1SfFifo1Get(canRXpacket_t *packet)
{
    if(can1SfRxFifo1.st != can1SfRxFifo1.end) {
        can1SfRxFifo1.st++;
        can1SfRxFifo1.st %= CAN_SF_FIFO0_SIZE;
        *packet = can1SfRxFifo1.rx[can1SfRxFifo1.st];
    }
    else {
        return 0;
    }
    return 1;
}
#endif

void can1WakeItInit(void)
{
    //enable interrupt in NVIC
    NVIC->ISER[0] |= 1 << 22;
    //enable wake up on bus activity
    CAN1->MCR |= CAN_MCR_AWUM;
    //enable wui interrupt mask
    CAN1->IER |= CAN_IER_WKUIE;
}

void can1WakeItDeInit(void)
{
    //disable interrupt in NVIC
    NVIC->ICER[0] |= 1 << 22;
    //disable wake up on bus activity
    CAN1->MCR &=~ CAN_MCR_AWUM;
    //disable wui interrupt mask
    CAN1->IER &=~ CAN_IER_WKUIE;
}

void can1Sleep(void)
{
    //generate sleep request
    CAN1->MCR |= CAN_MCR_SLEEP;
    //wait for sleep
    while(!(CAN1->MSR & CAN_MSR_SLAK));
}

void can1TXItInit(void)
{
    //enable interrupt in nvic
    NVIC->ISER[0] |= 1 << 19;
    //enable transmit mailbox empty interrupt mask
    CAN1->IER |= CAN_IER_TMEIE;
}

void can1TXItDeInit(void)
{
    //enable interrupt in nvic
    NVIC->ICER[0] |= 1 << 19;
    //enable transmit mailbox empty interrupt mask
    CAN1->IER &=~ CAN_IER_TMEIE;
}

void can1TXRegisterCallback(void (*callback)(void))
{
    can1TXCallback = callback;
}

void can1WakeRegisterCallback(void (*callback)(void))
{
    can1WakeCallback = callback;
}

uint32_t canSendPacket(canInstance_t *instance, uint32_t id, uint32_t remote, uint32_t dataL, void *data)
{
    uint32_t mailBoxNb = 0;

    //look if all the mailbox are full
    if ((instance->instance->TSR & CAN_TSR_TME_Msk) == 0) {
        return 0;
    }
    //look for valid size
    if (dataL > 8) {
        return 0;
    }

    mailBoxNb = (instance->instance->TSR & CAN_TSR_CODE_Msk) >> CAN_TSR_CODE_Pos;

    canTXpacket_t *mailbox;
    canTXpacket_t packet = {0};

    mailbox = (canTXpacket_t*) &(instance->instance->sTxMailBox[mailBoxNb]);

    packet.STID = id;
    packet.DLC = dataL;
    packet.RTR = remote;
    memcpy(packet.data, data, dataL);

    *mailbox = packet;

    //transmit request
    instance->instance->sTxMailBox[mailBoxNb].TIR |= CAN_TI0R_TXRQ;

    return 1;
}

uint32_t canPollFifo(canInstance_t *instance, canRXpacket_t *packet, uint32_t fifoId)
{

    if(instance->instance->RF0R & CAN_RF0R_FMP0_Msk && !fifoId) {
        memcpy(packet, &(instance->instance->sFIFOMailBox[0]), sizeof(instance->instance->sFIFOMailBox[0]));
        //clear received frame from fifo
        instance->instance->RF0R |= CAN_RF0R_RFOM0;
        return 1;
    }
    else if(instance->instance->RF1R & CAN_RF1R_FMP1_Msk && fifoId) {
        memcpy(packet, &(instance->instance->sFIFOMailBox[1]), sizeof(instance->instance->sFIFOMailBox[1]));
        //clear received frame from fifo
        instance->instance->RF1R |= CAN_RF1R_RFOM1;
        return 1;
    }
    return 0;
}

void canPrintRxPacket(canRXpacket_t *packet)
{
    int i;
    printf("id : %#03x \t rmt : %#02x \t fId : %#02x \t size : %#02x \t data:",
        packet->STID,
        packet->RTR,
        packet->FMI,
        packet->DLC);

    for(i = 0; i < packet->DLC; i++) {
        printf(" %#02x",packet->data[i]);
    }

    printf("\n");
}

//RX MAILBOX 0 IRQ HANDLER FOR CAN1
void CAN1_RX0_IRQHandler(void)
{
    if (CAN1->RF0R & CAN_RF0R_FMP0_Msk) {
#if CAN_SF_FIFO0_SIZE > 0
        can1SfRxFifo0.end++;
        can1SfRxFifo0.end %= CAN_SF_FIFO0_SIZE;
        //check for overrun error
        if (can1SfRxFifo0.end == can1SfRxFifo0.st) {
            can1SfRxFifo0.error |= RX_SF_FIFO_OVERRUN;
        }
        //read frame from fifo
        memcpy(&(can1SfRxFifo0.rx[can1SfRxFifo0.end]), &(CAN1->sFIFOMailBox[0]), sizeof(can1SfRxFifo0.rx[can1SfRxFifo0.end]));
        //clear received frame from fifo
        CAN1->RF0R |= CAN_RF0R_RFOM0;
#endif
    }
    //call callback
    if (can1Fifo0Callback) {
        can1Fifo0Callback();
    }
}

//RX MAILBOX 1 IRQ HANDLER FOR CAN1
void CAN1_RX1_IRQHandler(void){
    if(CAN1->RF1R & CAN_RF0R_FMP0_Msk){
#if CAN_SF_FIFO1_SIZE > 0
        can1SfRxFifo1.end++;
        can1SfRxFifo1.end %= CAN_SF_FIFO1_SIZE;
        //check for overrun error
        if(can1SfRxFifo1.end == can1SfRxFifo1.st)
            {
            can1SfRxFifo1.error |= RX_SF_FIFO_OVERRUN;
            }
        //read frame from fifo
        memcpy(&(can1SfRxFifo1.rx[can1SfRxFifo1.end]),&(CAN1->sFIFOMailBox[1]),sizeof(can1SfRxFifo1.rx[can1SfRxFifo1.end]));
        //clear received frame from fifo
        CAN1->RF0R |= CAN_RF0R_RFOM0;
#endif
    }
        if(can1Fifo1Callback){
        can1Fifo1Callback();
    }
}

//CAN1 TRANSMIT EVENT IRQ
void CAN1_TX_IRQHandler(void){
    uint32_t freeMailbox = 0;
    if(CAN1->TSR & CAN_TSR_RQCP0){
        //clear flag
        CAN1->TSR = CAN_TSR_RQCP0;
        freeMailbox = 1;
    }
    else if(CAN1->TSR & CAN_TSR_RQCP1){
        //clear flag
        CAN1->TSR = CAN_TSR_RQCP1;
        freeMailbox = 1;
    }
    else if(CAN1->TSR & CAN_TSR_RQCP2){
        //clear flag
        CAN1->TSR = CAN_TSR_RQCP2;
        freeMailbox = 1;
    }
    if(freeMailbox && can1TXCallback){
        can1TXCallback();
    }
}

//CAN1 STATUS CHANGE IRQ
void CAN1_SCE_IRQHandler()
{
    if (CAN1->MSR & CAN_MSR_WKUI) {
        //clear the flag
        CAN1->MSR = CAN_MSR_WKUI;
        //call the callback
        if (can1WakeCallback) {
            can1WakeCallback();
        }
    }
}


