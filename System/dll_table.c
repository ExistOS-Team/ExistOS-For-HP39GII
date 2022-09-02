#include "dll_table.h"


#include "FreeRTOS.h"
#include "task.h"

typedef struct dll_table_t
{
    struct dll_table_t *next;
    void *func;
    const char *name;
    uint32_t elfHash;
}dll_table_t;


dll_table_t *dll_tab = NULL;


static uint32_t elf_hash(const uint8_t* name) 
{
    uint32_t h = 0, g;
    for (; *name; name++) {
        h = (h << 4) + *name;
        if (g = h & 0xf0000000) {
            h ^= g >> 24;
        }
        h &= ~g;
    }
    return h;
}

void *dll_table_find(char *fname)
{
    if(!dll_tab)
    {
        return NULL;
    }
    uint32_t hash = elf_hash(fname);
    dll_table_t *tab;
    tab = dll_tab;

    while(tab)
    {
        if(tab->elfHash == hash)
        {
            if(strcmp(tab->name, fname) == 0)
            {
                return tab->func;
            }
        }
        tab = tab->next;
    }
    return NULL;
}


int dll_table_add(const char *fname, void *func)
{
    if(dll_table_find((char *)fname))
    {
        return 1;
    }

    if(!dll_tab)
    {
        dll_tab = pvPortMalloc(sizeof(dll_table_t));
        if(!dll_tab)
        {
            return -1;
        }
    }

    dll_table_t *tab;
    tab = dll_tab;

    while(tab && tab->next)
    {
        tab = tab->next;
    }

    tab->next = pvPortMalloc(sizeof(dll_table_t));
    if(!tab->next)
    {
        return -1;
    }
    tab = tab->next;
    
    tab->next = NULL;
    tab->func = func;
    tab->name = fname;
    tab->elfHash = elf_hash(fname);
    return 0;
}





