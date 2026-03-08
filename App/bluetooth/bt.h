#ifndef _hc06_h
#define _hc06_h
#include "stdint.h"

#define BT_TIMEOUT 5000

#define MAX_Speed 22
#define MAX_Turn 24

typedef struct
{
	uint8_t forward;
    uint8_t backward;
    uint8_t left;
    uint8_t right;
} BT_Command_t;

typedef struct
{
    uint8_t data;
    uint8_t flag;
} RX_DATA_t;

void BT_Init(void);
void BT_Start(void);
void BT_Check_Connect(void);

#endif