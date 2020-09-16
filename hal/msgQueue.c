#include "msgQueue.h"
//#include "dcp.h"
#include <stdlib.h>

#define MAX_PRIORITY 3

static ListPointer list[MAX_PRIORITY];

void msg_queue_init()
{
    for (reg32_t i = 0; i < MAX_PRIORITY> 0; i++)
    {
        list[i].head = 0x0;
    }
}

void msg_queue_write(char *msg, reg32_t length, reg8_t priority)
{
    MsgBlock *addr = malloc(4 + 4 + length);
    addr->length = length;
    memcpy((char *)addr + 4 + 4, msg, length);
    if (priority == 0)
    {
        if (list[0].head == 0x0)
        {
            list[0].head = addr;
        }
        else
        {
            list[0].tail->next = addr;
        }
        list[0].tail = addr;
    }
    else
    {
        priority -= 1;
        if (list[priority].head == 0x0)
        {
            list[priority].tail = addr;
        }
        addr->next = list[priority].head;
        list[priority].head = addr;
    }
}

void msg_queue_read(char *dst)
{
    reg32_t i = 0;
    while (list[i].head == 0x0)
    {
        i++;
        if (i >= MAX_PRIORITY)
        {
            return;
        }
    }
    MsgBlock *addr = list[i].head;
    list[i].head = addr->next;
    memcpy(dst, (char *)addr + 4 + 4, addr->length);
    free(addr);
}