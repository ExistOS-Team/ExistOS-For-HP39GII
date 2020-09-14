#include "regs.h"

typedef struct _msg_block
{
    struct _msg_block *next;
    reg32_t length;
    char *msg;
} MsgBlock;

typedef struct
{
    MsgBlock *head;
    MsgBlock *tail;
} ListPointer;