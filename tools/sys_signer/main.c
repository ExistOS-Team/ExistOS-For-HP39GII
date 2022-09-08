#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libelf/elf32.h"
#include "libelf/elf_user.h"

char path_syself[512];
char path_sysbin[512];

FILE *f_sysbin;
FILE *f_syself;

uint8_t *buf_syself;
uint8_t *buf_sysbin;

size_t sz_syself;
size_t sz_sysbin;

elf_t elf_sys;


static Elf32_Sym *sys_symtab;
static const char *sys_symstr;
static size_t syssym_num;

uint32_t calc_sys_sym_hash()
{
    uint32_t hash = 0x5a5a1234;
    for (int i = 0; i < syssym_num; i++)
    {
        hash ^= sys_symtab[i].st_value;
        hash ^= hash << 16;
    }
    return hash;
}

void Usage() {
    printf("sysign sys.elf ExistOS.sys");
    exit(-1);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        Usage();
    }
    strcpy(path_syself, argv[1]);
    strcpy(path_sysbin, argv[2]);

    f_syself = fopen(path_syself, "rb");
    if (!f_syself) {
        fprintf(stderr, "Failed to open syself\n");
        exit(-1);
    }
    f_sysbin = fopen(path_sysbin, "rb+");
    if (!f_sysbin) {
        fprintf(stderr, "Failed to open sysbin\n");
        fclose(f_syself);
        exit(-1);
    }

    fseek(f_syself, 0, SEEK_END);
    sz_syself = ftell(f_syself);
    fseek(f_syself, 0, SEEK_SET);

    //fseek(f_sysbin, 0, SEEK_END);
    sz_sysbin = 4*32;
    fseek(f_sysbin, 0, SEEK_SET);

    buf_syself = malloc(sz_syself);
    if (!buf_syself) {
        fprintf(stderr, "Failed to alloc memory 1\n");
        exit(-1);
    }

    buf_sysbin = malloc(sz_sysbin);
    if (!buf_sysbin) {
        fprintf(stderr, "Failed to alloc memory 2\n");
        exit(-1);
    }

    fread(buf_sysbin, 1, sz_sysbin, f_sysbin);
    fread(buf_syself, 1, sz_syself, f_syself);

    if (
        (((uint32_t *)buf_sysbin)[0] != 0xEF5AE0EF) || ((uint32_t *)buf_sysbin)[1] != 0xFECDAFDE) {
        fprintf(stderr, "Error SysBIN Format!\n");
        exit(-1);
    }

    elf_sys.elfFile = buf_syself;
    elf_sys.elfClass = ELFCLASS32;
    elf_sys.elfSize = sz_syself;

    if (elf_checkFile(&elf_sys)) {
        fprintf(stderr, "Error SysELF Format!\n");
        exit(-1);
    }


        static uint32_t *got_table;
        static Elf32_Sym *dynamic_table;
        static uint32_t *rel_plt;
        static uint32_t rel_plt_num;
        static uint32_t *rel_dyn;
        static uint32_t rel_dyn_num;
        static const char *dynstr;

        const char *sname;

        size_t SysNumSection = elf_getNumSections(&elf_sys);
        for (int i = 0; i < SysNumSection; i++)
        {
            sname = elf_getSectionName(&elf_sys, i);
            if (strcmp(sname, ".symtab") == 0)
            {
                sys_symtab = (Elf32_Sym *)elf_getSectionOffset(&elf_sys, i);
                syssym_num = elf_getSectionSize(&elf_sys, i) / sizeof(Elf32_Sym);
                printf("sys_symtab:%p, item:%ld\n", sys_symtab, syssym_num);
                sys_symtab = (Elf32_Sym *)((uint64_t)sys_symtab + (uint64_t)buf_syself);
            }
            if (strcmp(sname, ".strtab") == 0)
            {
                sys_symstr = (char *)elf_getSectionOffset(&elf_sys, i);
                printf("sys_symstr:%p\n", sys_symstr);
                sys_symstr += (uint64_t)buf_syself;
            }
        }
		uint32_t hash = calc_sys_sym_hash();
		printf("System Symbol Hash:%08x\n", hash);

		((uint32_t *)buf_sysbin)[3] = hash;

		
    	fseek(f_sysbin, 0, SEEK_SET);

		fwrite(buf_sysbin, 1, sz_sysbin, f_sysbin);

		fclose(f_sysbin);
		fclose(f_syself);
		free(buf_sysbin);
		free(buf_syself);

		return 0;
}