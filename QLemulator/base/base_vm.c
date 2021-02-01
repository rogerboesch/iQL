//
// (c) UQLX - see COPYRIGHT
//
// All changes that are made by Roger Boesch:
// "You can do whatever you like with it!"
//

#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "QL_68000.h"
#include "base_unix.h"
#include "base_xcodes.h"
#include "base_proto.h"

extern int main();
extern int strcasecmp();
extern uw32 rtop_hard;
extern int is_patching;
extern int rtc_emu_on;

char *scrModTable;
int faultaddr;
int vm_ison=0;

uw32 vm_saved_rom_value;
uw32 vm_saved_rom_addr=131072;  /* IMPORTANT */
int vm_saved_nInst=0;

int vmfatalretry=0;

void vm_on(void)
{
  vm_ison=1;
}

void vm_off(void)
{
  vm_ison=0;
}

