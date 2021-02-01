//
// (c) 2021 Graeme Gregory <graeme@xora.org.uk>
//     SPDX-License-Identifier: BSD-3-Clause
//
// All changes that are made by Roger Boesch:
// "You can do whatever you like with it!"

#include <fcntl.h>
#include <stdint.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "base_cfg.h"

static int bdi_files[8];
static uint32_t bdi_address;
static int bdi_ctr;
static int bdi_unit;
uint8_t bdi_buffer[512];

#ifdef DEBUG_QL
#define bdi_debug(fmt, a...) printf(fmt, ##a)
#else
#define bdi_debug(fmt, a...)                                                   \
	do {                                                                   \
	} while (0)
#endif

void SQLUXBDISelect(uint8_t d)
{
	int bdi_file;

	/* Currently only supporting 1 unit */
	if ((d == 1) && (!bdi_files[d - 1])) {
		bdi_debug("BDI: Opening %s\n", QMD.bdi1);
		if (strlen(QMD.bdi1)) {
			bdi_file = open(QMD.bdi1, O_RDWR);
			if (bdi_file < 0) {
				perror("BDI: Select Open File");
				printf("BDI: ERROR Opening %s\n", QMD.bdi1);
			} else {
				bdi_files[d - 1] = bdi_file;
			}
		}
	}

	if (d == 0) {
		close(bdi_files[bdi_unit - 1]);
		bdi_files[bdi_unit - 1] = 0;
	}

	bdi_unit = d;

	return;
}

void SQLUXBDICommand(uint8_t command)
{
	switch (command) {
	case 2:
		bdi_debug("BDI: Read Command\n");
		bdi_ctr = 0;
		break;
	case 3:
		bdi_debug("BDI: Write Command\n");
		bdi_ctr = 0;
		break;
	default:
		bdi_debug("BDI: Uknown Command 0x%2x\n", command);
	}
}

uint8_t SQLUXBDIStatus()
{
	if (!bdi_files[bdi_unit - 1]) {
		bdi_debug("BDI: ERROR File Not Open\n");
		return 1;
	}

	return 0;
}

uint8_t SQLUXBDIDataRead()
{
	if (!bdi_files[bdi_unit - 1]) {
		bdi_debug("BDI: ERROR File Not Open\n");
		return 0;
	}
	if (bdi_ctr == 0) {
		bdi_debug("BDI: seeking to 0x%8x\n", bdi_address * 512);
		lseek(bdi_files[bdi_unit - 1], bdi_address * 512, SEEK_SET);
		read(bdi_files[bdi_unit - 1], bdi_buffer, 512);
	}

	bdi_debug("BDI: Read %d\n", bdi_ctr);

	if (bdi_ctr < 512)
		return bdi_buffer[bdi_ctr++];

	return 0;
}

void SQLUXBDIDataWrite(uint8_t d)
{
	if (!bdi_files[bdi_unit - 1]) {
		bdi_debug("BDI: ERROR File Not Open\n");
		return;
	}
	if (bdi_ctr == 0) {
		bdi_debug("BDI: seeking to 0x%8x\n", bdi_address * 512);
		lseek(bdi_files[bdi_unit - 1], bdi_address * 512, SEEK_SET);
	}

	printf("BDI: Write %d\n", bdi_ctr);

	if (bdi_ctr < 512)
		bdi_buffer[bdi_ctr++] = d;

	if (bdi_ctr == 512) {
		write(bdi_files[bdi_unit - 1], bdi_buffer, 512);
		fsync(bdi_files[bdi_unit - 1]);
	}
}

void SQLUXBDIAddressHigh(uint16_t bdi_addr)
{
	bdi_debug("BDI: Address 0x%4x\n", bdi_addr);
	bdi_address = (bdi_address & 0x0000FFFF) | ((uint32_t)bdi_addr << 16);
}

void SQLUXBDIAddressLow(uint16_t bdi_addr)
{
	bdi_debug("BDI: Address 0x%4x\n", bdi_addr);
	bdi_address = (bdi_address & 0xFFFF0000) | ((uint32_t)bdi_addr);
}

uint16_t SQLUXBDISizeHigh()
{
	struct stat bdi_stat;

	if (!bdi_files[bdi_unit - 1]) {
		bdi_debug("BDI: ERROR File Not Open\n");
		return 0;
	}

	bdi_debug("BDI: SizeHigh\n");

	fstat(bdi_files[bdi_unit - 1], &bdi_stat);

	return (uint16_t)(bdi_stat.st_size / 512) >> 16;
}

uint16_t SQLUXBDISizeLow()
{
	struct stat bdi_stat;

	if (!bdi_files[bdi_unit - 1]) {
		bdi_debug("BDI: ERROR File Not Open\n");
		return 0;
	}

	bdi_debug("BDI: SizeLow\n");

	fstat(bdi_files[bdi_unit - 1], &bdi_stat);

	return (uint16_t)(bdi_stat.st_size / 512) & 0xFFFF;
}
