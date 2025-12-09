#ifndef CAN_TYPES_H

#include <stdint.h>

typedef struct
{
    uint32_t    ID;
    uint8_t     dlc;
    uint8_t     ide;
    uint8_t     rtr;
    uint8_t     pad;
    uint8_t     data[8];
}CAN_FRAME;

#define CAN_TYPES_H

#endif
