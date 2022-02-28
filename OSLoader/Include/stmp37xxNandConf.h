#ifndef __STMP37XX_NAND_CONF__
#define __STMP37XX_NAND_CONF__


#include "stmp_NandControlBlock.h"

void RestoreLDLB2(void);
void mkNCB(uint32_t inBlock);
void mkDBBT(uint32_t inBlock);
void mkLDLB(uint32_t inBlock, uint32_t fwPageOffset, uint32_t fwPageTotal, uint32_t DBBT1_page, uint32_t DBBT2_page);




#endif