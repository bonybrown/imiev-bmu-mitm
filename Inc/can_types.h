#ifndef CAN_TYPES_H

#include <stdint.h>


const int QUEUE_CAPACITY = 64;


typedef struct
{
    uint32_t    ID;
    uint8_t     dlc;
    uint8_t     ide;
    uint8_t     rtr;
    union {                     // union used here so code makes sense in both tx and rx contexts
        uint8_t     rx_channel;
        uint8_t     tx_channel; // Used to store CAN channel index
    };
    uint8_t     data[8];
}CAN_FRAME;

#define CAN_TYPES_H

#endif
