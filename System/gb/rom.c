#ifdef _WIN32
#include <windows.h>
#else
//#include <sys/mman.h>

#include "ff.h"
#include "FreeRTOS.h"

#include <sys/stat.h>
#include <fcntl.h>
#endif
#include <stdio.h>
#include <string.h>
#include "rom.h"

unsigned char *bytes;
unsigned int mapper;

static long rom_size;

static char *carts[] = {
	[0x00] = "ROM ONLY",
	[0x01] = "MBC1",
	[0x02] = "MBC1+RAM",
	[0x03] = "MBC1+RAM+BATTERY",
	[0x05] = "MBC2",
	[0x06] = "MBC2+BATTERY",
	[0x08] = "ROM+RAM",
	[0x09] = "ROM+RAM+BATTERY",
	[0x0B] = "MMM01",
	[0x0C] = "MMM01+RAM",
	[0x0D] = "MMM01+RAM+BATTERY",
	[0x0F] = "MBC3+TIMER+BATTERY",
	[0x10] = "MBC3+TIMER+RAM+BATTERY",
	[0x11] = "MBC3",
	[0x12] = "MBC3+RAM",
	[0x13] = "MBC3+RAM+BATTERY",
	[0x15] = "MBC4",
	[0x16] = "MBC4+RAM",
	[0x17] = "MBC4+RAM+BATTERY",
	[0x19] = "MBC5",
	[0x1A] = "MBC5+RAM",
	[0x1B] = "MBC5+RAM+BATTERY",
	[0x1C] = "MBC5+RUMBLE",
	[0x1D] = "MBC5+RUMBLE+RAM",
	[0x1E] = "MBC5+RUMBLE+RAM+BATTERY",
	[0xFC] = "POCKET CAMERA",
	[0xFD] = "BANDAI TAMA5",
	[0xFE] = "HuC3",
	[0xFF] = "HuC1+RAM+BATTERY",
};

static char *banks[] = {
	" 32KiB",
	" 64KiB",
	"128KiB",
	"256KiB",
	"512KiB",
	"  1MiB",
	"  2MiB",
	"  4MiB",
	/* 0x52 */
	"1.1MiB",
	"1.2MiB",
	"1.5MiB",
	"Unknown"
};

static const int bank_sizes[] = {
	32*1024,
	64*1024,
	128*1024,
	256*1024,
	512*1024,
	1024*1024,
	2048*1024,
	4096*1024,
	1152*1024,
	1280*1024,
	1536*1024
};

static char *rams[] = {
	"None",
	"  2KiB",
	"  8KiB",
	" 32KiB",
	"Unknown"
};

static char *regions[] = {
	"Japan",
	"Non-Japan",
	"Unknown"
};

static unsigned char header[] = {
	0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
	0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
	0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
	0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
	0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
	0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E
};

static int rom_init(unsigned char *rombytes, off_t filesize)
{
	char buf[17];
	int type, bank_index, ram, region, version, i, pass;
	unsigned char checksum = 0;

	if(memcmp(&rombytes[0x104], header, sizeof(header)) != 0)
		return 0;

	memcpy(buf, &rombytes[0x134], 16);
	buf[16] = '\0';
	printf("Rom title: %s\n", buf);

	type = rombytes[0x147];

	printf("Cartridge type: %s (%02X)\n", carts[type], type);

	bank_index = rombytes[0x148];
	/* Adjust for the gap in the bank indicies */
	if(bank_index >= 0x52 && bank_index <= 0x54)
		bank_index -= 74;
	else if(bank_index > 7)
		bank_index = 11;

	if(bank_index >= 10)
	{
		printf("Illegal ROM size in header\n");
		return 0;
	}

	printf("Rom size: %s\n", banks[bank_index]);

	rom_size = bank_sizes[bank_index];

	if(rom_size < filesize)
	{
		printf("File not big enough for ROM size.\n");
		return 0;
	}

	ram = rombytes[0x149];
	if(ram > 3)
		ram = 4;

	printf("RAM size: %s\n", rams[ram]);

	region = rombytes[0x14A];
	if(region > 2)
		region = 2;
	printf("Region: %s\n", regions[region]);

	version = rombytes[0x14C];
	printf("Version: %02X\n", version);

	for(i = 0x134; i <= 0x14C; i++)
		checksum = checksum - rombytes[i] - 1;

	pass = rombytes[0x14D] == checksum;

	printf("Checksum: %s (%02X)\n", pass ? "OK" : "FAIL", checksum);
	if(!pass)
		return 0;

	bytes = rombytes;

	switch(type)
	{
		case 0x00:
		case 0x08:
		case 0x09:
			mapper = NROM;
		break;
		case 0x01:
		case 0x02:
		case 0x03:
			mapper = MBC1;
		break;
		case 0x05:
		case 0x06:
			mapper = MBC2;
		break;
		case 0x0B:
		case 0x0C:
			mapper = MMM01;
		break;
		case 0x0F:
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
			mapper = MBC3;
		break;
		case 0x15:
		case 0x16:
		case 0x17:
			mapper = MBC4;
		break;
		case 0x19:
		case 0x1A:
		case 0x1B:
		case 0x1C:
		case 0x1D:
		case 0x1E:
			mapper = MBC5;
		break;
	}

	return 1;
}

unsigned int rom_get_mapper(void)
{
	return mapper;
}

int rom_load(const char *filename)
{
#ifdef _WIN32
	HANDLE f, map;
#else
	int f;
	struct stat st;
#endif
	unsigned char *bytes;
	size_t rom_size;

#ifdef _WIN32
	f = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if(f == INVALID_HANDLE_VALUE)
		return 0;

	rom_size = GetFileSize(f, NULL);

	map = CreateFileMapping(f, NULL, PAGE_READONLY, 0, 0, NULL);
	if(!map)
		return 0;

	bytes = MapViewOfFile(map, FILE_MAP_READ, 0, 0, 0);
	if(!bytes)
		return 0;
#else

	FIL *fil = pvPortMalloc(sizeof(FIL));
	FRESULT fres;
	fres = f_open(fil, filename, FA_READ);
	if(fres){
		return 0;
	}

	FILINFO finfo;
	f_stat(filename, &finfo);
	rom_size = finfo.fsize;

	bytes = pvPortMalloc(rom_size);
	if(!bytes)
	{
		return 0;
	}
	UINT br;
	fres = f_read(fil, bytes, rom_size, &br);
	if(fres)
	{
		return 0;
	}
/*
	f = open(filename, O_RDONLY);
	if(f == -1)
		return 0;
	if(fstat(f, &st) == -1)
		return 0;

	rom_size = st.st_size;

	bytes = mmap(NULL, rom_size, PROT_READ, MAP_PRIVATE, f, 0);
	if(!bytes)
		return 0;
	*/
#endif
	return rom_init(bytes, rom_size);
}

unsigned char *rom_getbytes(void)
{
	return bytes;
}

int rom_bank_valid(int bank)
{
	if(bank * 0x4000 > rom_size)
		return 0;
	return 1;
}
