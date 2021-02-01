//
// (c) 2021 Graeme Gregory <graeme@xora.org.uk>
//     SPDX-License-Identifier: BSD-3-Clause
//
// All changes that are made by Roger Boesch:
// "You can do whatever you like with it!"

#ifndef SQLUX_BDI_H
#define SQLUX_BDI_H

#include <stdint.h>

void SQLUXBDISelect(uint8_t d);
void SQLUXBDICommand(uint8_t command);
void SQLUXBDIDataWrite(uint8_t command);
uint8_t SQLUXBDIStatus(void);
uint8_t SQLUXBDIDataRead(void);
void SQLUXBDIAddressHigh(uint16_t bdi_addr);
void SQLUXBDIAddressLow(uint16_t bdi_addr);
uint16_t SQLUXBDISizeHigh(void);
uint16_t SQLUXBDISizeLow(void);

#endif /* SQLUX_BDI_H */
