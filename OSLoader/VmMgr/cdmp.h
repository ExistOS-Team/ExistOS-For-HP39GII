#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define ENABLE_MTD_POOL     (0)
#define MTD_PAGE_SZ 2048
#define MTD_PAGES 1024

int cdmp_mem_init(void *start, uint32_t size);
uint32_t cdmp_alloc(uint32_t size);
void cdmp_free(uint32_t address);

void *cdmp_get_memblock(uint32_t address);
uint32_t cdmp_memblock_size(uint32_t address);
void cdmp_read(uint32_t address, uint32_t offset, uint32_t len, void *buffer);
void cdmp_wrtie(uint32_t address, uint32_t offset, uint32_t len, void *buffer);

void cdmp_meminfo(uint32_t *free, uint32_t *total);

#if ENABLE_MTD_POOL
void cdmp_mtd_wrtie_page(uint32_t page, void *buf);
void cdmp_mtd_read_page(uint32_t page, void *buf);
int cdmp_mtd_enable();

uint32_t cdmp_alloc_mtd(uint32_t size);
void cdmp_free_mtd(uint32_t address);
void cdmp_dump_mtd_layout();
#endif
void cdmp_dump_layout();