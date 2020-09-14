#include "msgQueue.h"
//#include "dcp.h"
#include <stdlib.h>

#define MAX_PRIORITY 3

static MsgBlock *pointer[MAX_PRIORITY+1];

void msg_queue_init()
{
    MsgBlock *addr = malloc(4+4);
    addr->next = 0x0;
    addr->length = 0;
    memset(pointer, addr, MAX_PRIORITY+1);
}

void msg_queue_read(char *dst)
{
    MsgBlock *next = pointer[0]->next;
    if (next != 0x0)
    {
        memcpy(dst, pointer[0]->msg, pointer[0]->length);
        free(pointer[0]);
    }
    pointer[0] = next;
}

void msg_queue_write(char* msg, reg32_t length, reg8_t priority)
{
    MsgBlock *addr = malloc(4 + 4 + length);
    if (priority == 0)
    {
        addr->next = pointer[0];
        pointer[0] = addr;
    }
    else
    {
        addr->next = pointer[priority]->next;
        pointer[priority]->next = addr;
    }
}