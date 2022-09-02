#include <stdio.h>

#include "SystemUI.h"
#include "VROMLoader.h"

#include "dll_table.h"

#include "KLib/libelf/elf32.h"
#include "KLib/libelf/elf_user.h"


#define VROM_ADDR (0x03000000)
#define TMP_LOAD_ADDR (0x03000000 + 2 * 1048576)

static elf_t elf_file;

static uint32_t *got_table;
static Elf32_Sym *dynamic_table;
static uint32_t *rel_plt;
static uint32_t *rel_dyn;
static uint32_t rel_dyn_num;
static const char *dynstr;



void dl_nop()
{


}


uint32_t _dl_fixup(uint32_t linker_map,  uint32_t reloc_arg)
{
    Elf32_Sym *sym;
    Elf32_Rel *reloc;

    //printf("call dls:%08x, %08x\n", linker_map, reloc_arg);
    //printf("rel_plt + reloc_arg: %08x\n", (uint32_t)rel_plt + reloc_arg);

    reloc = (Elf32_Rel *)((uint32_t)rel_plt + reloc_arg);

    //printf("reloc->offset: %08x\n", reloc->r_offset);
    //printf("reloc->info: %08x\n", reloc->r_info);

    sym = dynamic_table + (reloc->r_info >> 8);

    //printf("sym:%s, at:%08x\n",&dynstr[sym->st_name], sym->st_value   );
        
    if(sym->st_value)
    {
        *((uint32_t *)reloc->r_offset) = sym->st_value;
        return sym->st_value;
    }else{
        void *func = dll_table_find((char *)&dynstr[sym->st_name]);
        if(func)
        {
            *((uint32_t *)reloc->r_offset) = (uint32_t)func;
            return (uint32_t)func;
        }

        printf("unresolve symbol:%s\n", &dynstr[sym->st_name]);
        return (uint32_t)dl_nop;
    }

    //printf("sym:%08x, %08x, %08x\n", sym->st_name, sym->st_value, sym->st_size);


    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void __attribute__((naked)) __attribute__((target("arm"))) dl_runtime_resolve() 
{

    asm volatile("push    {r0-r4}");

    asm volatile("ldr     r0, [lr, #-4]");

    asm volatile("sub    r1, ip, lr");
    asm volatile("sub    r1, r1, #4");
    asm volatile("add    r1, r1, r1");  //change &GOT[n+3] into 8*n        NOTE: reloc are 8 bytes each

    asm volatile("bl    _dl_fixup");

    asm volatile("mov    ip, r0");
    asm volatile("pop    {r0-r4,lr}");
    asm volatile("BX   ip");

}


static void resolve_rel_dyn()
{
    Elf32_Sym *sym;
    Elf32_Rel *rel_d = (Elf32_Rel *)rel_dyn;
    for(int i = 0; i < rel_dyn_num; i++)
    {
        if(rel_d[i].r_offset)
        {
            sym = dynamic_table + (rel_d[i].r_info >> 8);
            *((uint32_t *)rel_d[i].r_offset) = sym->st_value;

            //printf("sym:%s, at:%08x\n",&dynstr[sym->st_name], sym->st_value   );
        }
    }
}



void elf_exec(void *par)
{
    FILINFO finfo;
    FRESULT fr;
    int res;
    void (*app_entry)();

    FIL *f = pvPortMalloc(sizeof(FIL));
    if(!f)
    {
        vTaskDelete(NULL);
    }
    fr = f_open(f, par, FA_OPEN_EXISTING | FA_READ);
    if(fr)
    {
        vPortFree(f);
        vTaskDelete(NULL);
    }

    f_stat(par, &finfo);
    res = VROMLoaderCreateFileMap(f, 0, TMP_LOAD_ADDR, finfo.fsize);
    if(res)
    {
        vTaskDelete(NULL);
    }

    elf_file.elfFile = (void *)TMP_LOAD_ADDR;
    elf_file.elfClass = ELFCLASS32;
    elf_file.elfSize = finfo.fsize;

    res = elf_checkFile(&elf_file);
    if(res == 0)
    {
        app_entry = (void (*)())elf_getEntryPoint(&elf_file);
        size_t numPH = elf_getNumProgramHeaders(&elf_file);
        size_t fileOffset;
        uint32_t vaddr;
        uint32_t paddr;
        uint32_t sz;
        for(int i = 0; i<numPH; i++)
        {
            if(elf_getProgramHeaderFlags(&elf_file, i) == (PF_R | PF_X))
            {
                fileOffset = elf_getProgramHeaderOffset(&elf_file, i);
                vaddr = elf_getProgramHeaderVaddr(&elf_file, i);
                paddr = elf_getProgramHeaderPaddr(&elf_file, i);
                sz = elf_getProgramHeaderFileSize(&elf_file, i);
                printf("(text) Executable:%08x,%08x,%08x,%d\n",fileOffset, vaddr, paddr, sz);
                res = VROMLoaderCreateFileMap(f, fileOffset, vaddr, sz);
                if(res)
                {
                    printf("Load Error!\n");
                }
            }

            if(elf_getProgramHeaderFlags(&elf_file, i) == (PF_R | PF_X | PF_W))
            {
                fileOffset = elf_getProgramHeaderOffset(&elf_file, i);
                vaddr = elf_getProgramHeaderVaddr(&elf_file, i);
                paddr = elf_getProgramHeaderPaddr(&elf_file, i);
                sz = elf_getProgramHeaderFileSize(&elf_file, i);
                printf("(Dyn ) Relocate:%08x,%08x,%08x,%d\n",fileOffset, vaddr, paddr, sz);
                memcpy((void *)vaddr, (void *)(TMP_LOAD_ADDR + fileOffset), sz);
            }
        }

        size_t numSection = elf_getNumSections(&elf_file);
        const char *sname;
        for(int i = 0; i < numSection; i++)
        {
            sname = elf_getSectionName(&elf_file, i);
            if(strcmp(sname, ".got") == 0)
            {
                got_table = (uint32_t *)elf_getSectionAddr(&elf_file, i);
                printf("got_table:%08x\n", got_table);
            }
            if(strcmp(sname, ".dynsym") == 0)
            {
                dynamic_table = (Elf32_Sym *)elf_getSectionAddr(&elf_file, i);
                printf("dynamic_table:%08x\n", dynamic_table);
            }
            
            if(strcmp(sname, ".rel.plt") == 0)
            {
                rel_plt = (uint32_t *)elf_getSectionAddr(&elf_file, i);
                printf("rel_plt:%08x\n", rel_plt);
            }
            
            if(strcmp(sname, ".rel.dyn") == 0)
            {
                rel_dyn = (uint32_t *)elf_getSectionAddr(&elf_file, i);
                rel_dyn_num = elf_getSectionSize(&elf_file, i) / sizeof(Elf32_Rel);
                printf("rel_dyn:%08x\n", rel_dyn);
            }

            if(strcmp(sname, ".dynstr") == 0)
            {
                dynstr = (const char *)elf_getSectionAddr(&elf_file, i);
                printf("dynstr:%08x\n", dynstr);
            }
               
        }

        
        got_table[1] = 0x88997766;
        got_table[2] = (uint32_t)dl_runtime_resolve;

        resolve_rel_dyn();


    }
    SystemUISuspend();


    app_entry();
    
    lv_obj_invalidate(lv_scr_act());

    SystemUIResume();

    VROMLoaderDeleteMap(TMP_LOAD_ADDR);

    vTaskDelete(NULL);
}


