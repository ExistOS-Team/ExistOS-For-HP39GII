
#include "cdmp.h"

#if ENABLE_MTD_POOL

static uint8_t mtd_buffer[MTD_PAGE_SZ * 2];

#define MTD_RDPG(x) cdmp_mtd_read_page((x), mtd_buffer);
#define MTD_WRPG(x) cdmp_mtd_wrtie_page((x), mtd_buffer);

static uint32_t cdmp_mtdpool_end;
static uint8_t mtd_enable = 0;
#endif

#define CDMP_MEMBLOCK_MAGIC_MEM_USED 0x554D454D // MEMU
#define CDMP_MEMBLOCK_MAGIC_MEM_FREE 0x464D454D // MEMF

#define CDMP_MEMBLOCK_MAGIC_MTD_USED 0x554D544D // MTMU
#define CDMP_MEMBLOCK_MAGIC_MTD_FREE 0x464D544D // MTMF

#define DEV_MASK (0xF0000000UL)
#define ID_MEM (0x20000000UL)
#define ID_MTD (0x10000000UL)

typedef struct cdmp_memblock_header_t {
    uint32_t magic;
    struct cdmp_memblock_header_t *next;
    struct cdmp_memblock_header_t *prev;

} cdmp_memblock_header_t;

static uint32_t mem_hibyte;
static cdmp_memblock_header_t *cdmp_mempool;
static uint32_t cdmp_mempool_end;

static uint32_t alloc_cnt = 0;
static uint32_t free_cnt = 0;
static uint32_t write_cnt = 0;
static uint32_t read_cnt = 0;

int cdmp_mem_init(void *start, uint32_t size) {
    cdmp_mempool = (cdmp_memblock_header_t *)start;
    cdmp_mempool->magic = CDMP_MEMBLOCK_MAGIC_MEM_FREE;
    cdmp_mempool->next = NULL;
    cdmp_mempool->prev = NULL;

    mem_hibyte = (DEV_MASK & ((uint32_t)start));
    cdmp_mempool_end = (uint32_t)(start + size);
    return 0;
}
#if ENABLE_MTD_POOL
int cdmp_mtd_enable() {
    cdmp_mtdpool_end = MTD_PAGE_SZ * MTD_PAGES;
    memset(mtd_buffer, 0xFF, MTD_PAGE_SZ);
    cdmp_memblock_header_t *m = (cdmp_memblock_header_t *)mtd_buffer;
    m->magic = CDMP_MEMBLOCK_MAGIC_MTD_FREE;
    m->next = NULL;
    m->prev = NULL;
    MTD_WRPG(0);
    mtd_enable = 1;
}

void cdmp_mtd_char_read(uint32_t address, void *buf, uint32_t size) {
    uint32_t page = address / MTD_PAGE_SZ;
    uint32_t offset = address % MTD_PAGE_SZ;
    if (offset + size > MTD_PAGE_SZ) {
        cdmp_mtd_read_page(page, &mtd_buffer[0]);
        cdmp_mtd_read_page(page + 1, &mtd_buffer[MTD_PAGE_SZ]);
    } else {
        cdmp_mtd_read_page(page, &mtd_buffer[0]);
    }
    memcpy(buf, &mtd_buffer[offset], size);
}

void cdmp_mtd_char_write(uint32_t address, void *buf, uint32_t size) {
    uint32_t page = address / MTD_PAGE_SZ;
    uint32_t offset = address % MTD_PAGE_SZ;
    if (offset + size > MTD_PAGE_SZ) {
        cdmp_mtd_read_page(page, &mtd_buffer[0]);
        cdmp_mtd_read_page(page + 1, &mtd_buffer[MTD_PAGE_SZ]);
        memcpy(&mtd_buffer[offset], buf, size);
        cdmp_mtd_wrtie_page(page, &mtd_buffer[0]);
        cdmp_mtd_wrtie_page(page + 1, &mtd_buffer[MTD_PAGE_SZ]);
    } else {
        cdmp_mtd_read_page(page, &mtd_buffer[0]);
        memcpy(&mtd_buffer[offset], buf, size);
        cdmp_mtd_wrtie_page(page, &mtd_buffer[0]);
    }
}

void cdmp_dump_mtd_layout() {
    printf("--------------------------\n");
    uint32_t use_sum = 0, free_sum = 0;
    cdmp_memblock_header_t m;
    uint32_t m_address = 0;
    /*
    do{
        cdmp_mtd_char_read(m_address, &m, sizeof(m));
        if(m.magic == CDMP_MEMBLOCK_MAGIC_MTD_USED)
        {
            uint32_t start = m_address;
            printf("Use  %p ~ ", start);
            do{
                m_address = (uint32_t)m.next;
                cdmp_mtd_char_read(m_address, &m, sizeof(m));
            }while((m.next) && (m.magic == CDMP_MEMBLOCK_MAGIC_MTD_USED));
            use_sum += m_address - start;
            printf("%p ,sz:%d B\n", m_address, m_address - start);
        }

        if(m.magic == CDMP_MEMBLOCK_MAGIC_MTD_FREE)
        {
            uint32_t start = m_address;
            printf("Free %p ~ ", start);
            do{
                m_address = (uint32_t)m.next;
                cdmp_mtd_char_read(m_address, &m, sizeof(m));
            }while((m.next) && (m.magic == CDMP_MEMBLOCK_MAGIC_MTD_FREE));
            free_sum += m_address - start;
            printf("%p ,sz:%d B\n", m_address, m_address - start);
        }

    }while(m_address);
    */

    do {
        cdmp_mtd_char_read(m_address, &m, sizeof(m));

        if (m.magic == CDMP_MEMBLOCK_MAGIC_MTD_USED) {
            printf("USED: ");
        } else if (m.magic == CDMP_MEMBLOCK_MAGIC_MTD_FREE) {
            printf("FRE : ");
        }
        printf("%08x ~ ", m_address);
        if (m.next) {
            printf("%08x , sz:%d B\n", (uint32_t)m.next, (uint32_t)m.next - m_address);
        } else {
            printf("%08x , sz:%d B\n", MTD_PAGE_SZ * MTD_PAGES, (MTD_PAGE_SZ * MTD_PAGES - m_address));
        }

        m_address = (uint32_t)m.next;
    } while (m_address);

    printf("--------------------------\n");
}

#endif

void cdmp_meminfo(uint32_t *free, uint32_t *total)
{
    uint32_t use_sum = 0, free_sum = 0;
    cdmp_memblock_header_t *m = cdmp_mempool;
    while (m) {
        if (m->magic == CDMP_MEMBLOCK_MAGIC_MEM_USED) {
            uint32_t start = (uint32_t)m;
            while ((m) && (m->magic == CDMP_MEMBLOCK_MAGIC_MEM_USED)) {
                m = m->next;
            }
            use_sum += (uint32_t)m - start;
        }

        if ((m) && (m->magic == CDMP_MEMBLOCK_MAGIC_MEM_FREE)) {
            uint32_t start = (uint32_t)m;
            while ((m) && (m->magic == CDMP_MEMBLOCK_MAGIC_MEM_FREE)) {
                m = m->next;
            }
            uint32_t end = (uint32_t)m;

            if ((uint32_t)m == 0) {
                end = cdmp_mempool_end;
            }
            free_sum += end - start;
        }
        // m=m->next;
    }

    *free = free_sum;
    *total = (uint32_t)cdmp_mempool_end - (uint32_t)cdmp_mempool;

}

void cdmp_dump_layout() {
    printf("--------------------------\n");
    uint32_t use_sum = 0, free_sum = 0;
    cdmp_memblock_header_t *m = cdmp_mempool;
    printf("mempool: %p ~ %08lx, %ld B\n", cdmp_mempool, cdmp_mempool_end, ((uint32_t)cdmp_mempool_end - (uint32_t)cdmp_mempool));
    while (m) {
        if (m->magic == CDMP_MEMBLOCK_MAGIC_MEM_USED) {
            uint32_t start = (uint32_t)m;
            printf("Use  %p ~ ", m);
            while ((m) && (m->magic == CDMP_MEMBLOCK_MAGIC_MEM_USED)) {
                m = m->next;
            }
            use_sum += (uint32_t)m - start;
            printf("%p ,sz:%ld B\n", m, (uint32_t)m - start);
        }

        if ((m) && (m->magic == CDMP_MEMBLOCK_MAGIC_MEM_FREE)) {
            uint32_t start = (uint32_t)m;
            printf("Free %p ~ ", m);
            while ((m) && (m->magic == CDMP_MEMBLOCK_MAGIC_MEM_FREE)) {
                m = m->next;
            }
            uint32_t end = (uint32_t)m;

            if ((uint32_t)m == 0) {
                end = cdmp_mempool_end;
            }
            free_sum += end - start;
            printf("%08lx ,sz:%ld B\n", end, end - start);
        }
        // m=m->next;
    }

    printf("use:%ld B, free:%ld B, total:%ld B\n", use_sum, free_sum, use_sum + free_sum);
    printf("alloc:%ld, free:%ld, write:%ld, read:%ld\n", alloc_cnt, free_cnt, write_cnt, read_cnt);
    /*
    while (m)
    {
        if (m->magic == CDMP_MEMBLOCK_MAGIC_MEM_USED)
        {
            printf("USED: ");
        }
        else if (m->magic == CDMP_MEMBLOCK_MAGIC_MEM_FREE)
        {
            printf("FRE : ");
        }
        printf("%p ~ ", m);
        if (m->next)
        {
            printf("%p , sz:%d B\n", m->next, ((uint32_t)m->next - (uint32_t)m));
        }
        else
        {
            printf("%08x , sz:%d B\n", cdmp_mempool_end, (cdmp_mempool_end - (uint32_t)m));
        }

        m = m->next;
    }*/
    printf("--------------------------\n");
}

#if ENABLE_MTD_POOL
uint32_t cdmp_alloc_mtd(uint32_t size) {
    // cdmp_memblock_header_t *m = (cdmp_memblock_header_t *)mtd_buffer;
    cdmp_memblock_header_t m;
    cdmp_memblock_header_t n;
    cdmp_memblock_header_t n_next;
    uint32_t m_address = 0;
    uint32_t n_address = 0;
    uint32_t n_next_address = 0;
    uint32_t sz;

    if (!size) {
        return 0;
    }

    while (size % 4) {
        size++;
    }

    do {
        cdmp_mtd_char_read(m_address, &m, sizeof(m));
        if (m.magic == CDMP_MEMBLOCK_MAGIC_MTD_FREE) {
            if (m.next) {
                sz = (uint32_t)m.next - m_address - sizeof(cdmp_memblock_header_t);
                if (sz >= size) {
                    m.magic = CDMP_MEMBLOCK_MAGIC_MTD_USED;
                    n_address = m_address + sizeof(cdmp_memblock_header_t) + size;
                    if ((uint32_t)m.next - sizeof(cdmp_memblock_header_t) > n_address) {
                        n.magic = CDMP_MEMBLOCK_MAGIC_MTD_FREE;
                        n.next = m.next;
                        n_next_address = (uint32_t)n.next;
                        cdmp_mtd_char_read(n_next_address, &n_next, sizeof(n_next));
                        n_next.prev = (cdmp_memblock_header_t *)n_address;
                        n.prev = (cdmp_memblock_header_t *)m_address;
                        m.next = (cdmp_memblock_header_t *)n_address;

                        cdmp_mtd_char_write(n_next_address, &n_next, sizeof(n_next));
                        cdmp_mtd_char_write(n_address, &n, sizeof(n));
                    }
                    cdmp_mtd_char_write(m_address, &m, sizeof(m));

                    return ((~DEV_MASK) & (uint32_t)m_address) | ID_MTD;
                } else {
                }
            } else {
                sz = cdmp_mempool_end - m_address - sizeof(cdmp_memblock_header_t);
                if (sz >= size) {
                    m.magic = CDMP_MEMBLOCK_MAGIC_MTD_USED;
                    n_address = m_address + sizeof(cdmp_memblock_header_t) + size;
                    if ((uint32_t)m.next - sizeof(cdmp_memblock_header_t) > n_address) {
                        n.magic = CDMP_MEMBLOCK_MAGIC_MTD_FREE;
                        n.next = NULL;
                        n.prev = (cdmp_memblock_header_t *)m_address;
                        m.next = (cdmp_memblock_header_t *)n_address;
                        cdmp_mtd_char_write(n_address, &n, sizeof(n));
                    }

                    cdmp_mtd_char_write(m_address, &m, sizeof(m));

                    return ((~DEV_MASK) & (uint32_t)m_address) | ID_MTD;
                } else {
                    printf("NO MTD MEM\n");
                    return 0;
                }
            }
        }
        m_address = (uint32_t)m.next;
    } while (m_address);
}

void cdmp_free_mtd(uint32_t address) {
    cdmp_memblock_header_t j, m, k, k_next;
    uint32_t j_address = 0,
             m_address = 0,
             k_address = 0,
             k_next_address = 0;
    if ((address & DEV_MASK) == ID_MTD) {
        address &= ~(DEV_MASK);
        if (address < MTD_PAGES * MTD_PAGE_SZ) {
            m_address = address;
            cdmp_mtd_char_read(m_address, &m, sizeof(m));
            if (m.magic == CDMP_MEMBLOCK_MAGIC_MTD_USED) {
                m.magic = CDMP_MEMBLOCK_MAGIC_MTD_FREE;
                j_address = (uint32_t)m.prev;
                k_address = (uint32_t)m.next;
                uint8_t j_merge = 0;

                if (j_address) {
                    cdmp_mtd_char_read(j_address, &j, sizeof(j));
                }

                if (k_address) {
                    cdmp_mtd_char_read(k_address, &k, sizeof(k));
                    if (k.next) {
                        k_next_address = (uint32_t)k.next;
                        cdmp_mtd_char_read(k_next_address, &k_next, sizeof(k_next));
                    }
                }

                if (j_address) {
                    if (j.magic == CDMP_MEMBLOCK_MAGIC_MTD_FREE) {
                        m.magic = 0;
                        j.next = (cdmp_memblock_header_t *)k_address;
                        if (k_address) {
                            k.prev = (cdmp_memblock_header_t *)j_address;
                        }
                        j_merge = 1;
                    }
                }

                if (k_address) {
                    if (k.magic == CDMP_MEMBLOCK_MAGIC_MTD_FREE) {
                        if (j_merge) {
                            j.next = k.next;
                            if (k_next_address) {
                                k_next.prev = (cdmp_memblock_header_t *)j_address;
                            }
                            k.magic = 0;
                        } else {
                            k.magic = CDMP_MEMBLOCK_MAGIC_MTD_FREE;
                            m.next = k.next;
                            if (k_next_address) {
                                k_next.prev = (cdmp_memblock_header_t *)m_address;
                            }
                        }
                    }
                }

                cdmp_mtd_char_write(m_address, &m, sizeof(m));

                if (j_address) {
                    cdmp_mtd_char_write(j_address, &j, sizeof(j));
                }

                if (k_address) {
                    cdmp_mtd_char_write(k_address, &k, sizeof(k));
                    if (k_next_address) {
                        cdmp_mtd_char_write(k_next_address, &k_next, sizeof(k_next));
                    }
                }
            }
        }
    }
}

#endif

void cdmp_free(uint32_t address) {
    //[ ... , j , m (to free) , k , ... ]
    cdmp_memblock_header_t *m;
    cdmp_memblock_header_t *j, *k;
    if (!address) {
        //printf("Err Address\n");
        return;
    }

    if ((address & DEV_MASK) == ID_MEM) {
        address &= ~DEV_MASK;
        address |= mem_hibyte;
        if ((address >= (uint32_t)cdmp_mempool) && (address < (uint32_t)cdmp_mempool_end)) {
            m = (cdmp_memblock_header_t *)address;
            if (m->magic == CDMP_MEMBLOCK_MAGIC_MEM_USED) {
                m->magic = CDMP_MEMBLOCK_MAGIC_MEM_FREE;
                j = m->prev;
                k = m->next;
                uint8_t j_merge = 0;

                if (j) {
                    if (j->magic == CDMP_MEMBLOCK_MAGIC_MEM_FREE) {
                        m->magic = 0;
                        j->next = k;
                        if (k) {
                            k->prev = j;
                        }
                        j_merge = 1;
                    }
                }

                if (k) {
                    if (k->magic == CDMP_MEMBLOCK_MAGIC_MEM_FREE) {
                        if (j_merge) {
                            j->next = k->next;
                            if (k->next) {
                                k->next->prev = j;
                            }
                            k->magic = 0;
                        } else {
                            k->magic = CDMP_MEMBLOCK_MAGIC_MEM_FREE;
                            m->next = k->next;
                            if (k->next) {
                                k->next->prev = m;
                            }
                        }
                    }
                }
                free_cnt++;
            } else {
                printf("Err Address\n");
                return;
            }
        }
    } else

#if ENABLE_MTD_POOL
        if (mtd_enable) {
        cdmp_free_mtd(address);
    }
#else
    {
        printf("Err Address\n");
        return;
    }
#endif
}

uint32_t cdmp_alloc(uint32_t size) {
    cdmp_memblock_header_t *m = cdmp_mempool, *n;
    uint32_t sz;

    if (!size) {
        return 0;
    }

    while (size % 4) {
        size++;
    }

    while (m) {
        // printf("sr:%p\n", m);
        if (m->magic == CDMP_MEMBLOCK_MAGIC_MEM_FREE) {
            if (m->next) {
                sz = (uint32_t)m->next - (uint32_t)m - sizeof(cdmp_memblock_header_t);
                if (sz >= size) {
                    m->magic = CDMP_MEMBLOCK_MAGIC_MEM_USED;

                    n = (cdmp_memblock_header_t *)(((uint32_t)m) + sizeof(cdmp_memblock_header_t) + size);
                    if ((uint32_t)m->next - sizeof(cdmp_memblock_header_t) > (uint32_t)n) {
                        n->magic = CDMP_MEMBLOCK_MAGIC_MEM_FREE;
                        n->next = m->next;
                        n->next->prev = n;
                        n->prev = m;
                        m->next = n;
                    }

                    alloc_cnt++;
                    return ((~DEV_MASK) & (uint32_t)m) | ID_MEM;
                } else {
                }
            } else {
                sz = cdmp_mempool_end - (uint32_t)m - sizeof(cdmp_memblock_header_t);
                if (sz >= size) {
                    m->magic = CDMP_MEMBLOCK_MAGIC_MEM_USED;

                    n = (cdmp_memblock_header_t *)(((uint32_t)m) + sizeof(cdmp_memblock_header_t) + size);
                    if ((uint32_t)m->next - sizeof(cdmp_memblock_header_t) > (uint32_t)n) {
                        n->magic = CDMP_MEMBLOCK_MAGIC_MEM_FREE;
                        n->next = NULL;
                        n->prev = m;
                        m->next = n;
                    }
                    alloc_cnt++;
                    return ((~DEV_MASK) & (uint32_t)m) | ID_MEM;
                } else {
#if ENABLE_MTD_POOL
                    if (mtd_enable) {
                        return cdmp_alloc_mtd(size);
                    }
#endif
                    printf("NO MEM\n");
                    return 0;
                }
            }
        }

        m = m->next;
    }

    printf("NO MEM\n");
    return 0;
}

uint32_t cdmp_memblock_size(uint32_t address)
{
    if ((address & DEV_MASK) == ID_MEM) {
        address &= ~DEV_MASK;
        address |= mem_hibyte;
        cdmp_memblock_header_t *m = (cdmp_memblock_header_t *)address;
        if (m->magic != CDMP_MEMBLOCK_MAGIC_MEM_USED) {
            printf("ERROR try to read a free block.\n");
            return NULL;
        }
        if(m->next)
        {
            return ((uint32_t)m->next) - (uint32_t)m - sizeof(cdmp_memblock_header_t);
        }else{
            return cdmp_mempool_end - (uint32_t)m - sizeof(cdmp_memblock_header_t);
        }
    }
}

void *cdmp_get_memblock(uint32_t address)
{
    if ((address & DEV_MASK) == ID_MEM) {
        address &= ~DEV_MASK;
        address |= mem_hibyte;
        cdmp_memblock_header_t *m = (cdmp_memblock_header_t *)address;
        if (m->magic != CDMP_MEMBLOCK_MAGIC_MEM_USED) {
            printf("ERROR try to read a free block.\n");
            return NULL;
        }
        read_cnt++;
        void *ptr = (void *)((uint32_t)m + sizeof(cdmp_memblock_header_t));
        return ptr;
    }
}

void cdmp_read(uint32_t address, uint32_t offset, uint32_t len, void *buffer) {
    if ((address & DEV_MASK) == ID_MEM) {
        address &= ~DEV_MASK;
        address |= mem_hibyte;
        cdmp_memblock_header_t *m = (cdmp_memblock_header_t *)address;
        if (m->magic != CDMP_MEMBLOCK_MAGIC_MEM_USED) {
            printf("ERROR try to read a free block.\n");
            return;
        }
        read_cnt++;
        void *ptr = (void *)((uint32_t)m + sizeof(cdmp_memblock_header_t));
        ptr += offset;
        if (len) {
            memcpy(buffer, ptr, len);
        } else {
            if (m->next) {
                memcpy(buffer, ptr, (uint32_t)m->next - (uint32_t)ptr);
            } else {
                memcpy(buffer, ptr, cdmp_mempool_end - (uint32_t)ptr);
            }
        }
    }
#if ENABLE_MTD_POOL
    else if ((address & DEV_MASK) == ID_MTD) {
        cdmp_memblock_header_t m;
        address &= ~DEV_MASK;
        cdmp_mtd_char_read(address, &m, sizeof(m));
        if (m.magic != CDMP_MEMBLOCK_MAGIC_MTD_USED) {
            printf("ERROR try to read a free block.\n");
            return;
        }
        uint32_t ptr = address + sizeof(cdmp_memblock_header_t);
        ptr += offset;
        if (len) {
            cdmp_mtd_char_read(ptr, buffer, len);
        } else {
            if (m.next) {
                cdmp_mtd_char_read(ptr, buffer, (uint32_t)m.next - ptr);
            } else {
                cdmp_mtd_char_read(ptr, buffer, MTD_PAGE_SZ * MTD_PAGES - ptr > MTD_PAGE_SZ ? MTD_PAGE_SZ : MTD_PAGE_SZ * MTD_PAGES - ptr);
            }
        }
    }
#endif
    else {
        printf("bad address\n");
    }
}

void cdmp_wrtie(uint32_t address, uint32_t offset, uint32_t len, void *buffer) {

    if ((address & DEV_MASK) == ID_MEM) {
        address &= ~DEV_MASK;
        address |= mem_hibyte;
        cdmp_memblock_header_t *m = (cdmp_memblock_header_t *)address;
        if (m->magic != CDMP_MEMBLOCK_MAGIC_MEM_USED) {
            printf("ERROR try to write a free block.\n");
            return;
        }
        write_cnt++;
        void *ptr = (void *)((uint32_t)m + sizeof(cdmp_memblock_header_t));
        ptr += offset;
        if (len) {
            memcpy(ptr, buffer, len);
        } else {
            if (m->next) {
                memcpy(ptr, buffer, (uint32_t)m->next - (uint32_t)ptr);
            } else {
                memcpy(ptr, buffer, cdmp_mempool_end - (uint32_t)ptr);
            }
        }
    }
#if ENABLE_MTD_POOL
    else if ((address & DEV_MASK) == ID_MTD) {
        cdmp_memblock_header_t m;
        address &= ~DEV_MASK;
        cdmp_mtd_char_read(address, &m, sizeof(m));
        if (m.magic != CDMP_MEMBLOCK_MAGIC_MTD_USED) {
            printf("ERROR try to write a free block.\n");
            return;
        }
        uint32_t ptr = address + sizeof(cdmp_memblock_header_t);
        ptr += offset;
        if (len) {
            cdmp_mtd_char_write(ptr, buffer, len);
        } else {
            if (m.next) {
                cdmp_mtd_char_write(ptr, buffer, (uint32_t)m.next - ptr);
            } else {
                cdmp_mtd_char_write(ptr, buffer, MTD_PAGE_SZ * MTD_PAGES - ptr > MTD_PAGE_SZ ? MTD_PAGE_SZ : MTD_PAGE_SZ * MTD_PAGES - ptr);
            }
        }
    }
#endif
    else {
        printf("bad address\n");
    }
}
