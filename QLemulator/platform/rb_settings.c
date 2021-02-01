//
//  rb_settings.c
//
//  Created by Roger Boesch on Jan. 10, 2021.
//  "You can do whatever you like with it"
//

#include <stdio.h>

extern char* rb_get_platform_system_path(void);
extern char* rb_get_platform_resource_path(void);

char* SYSTEM_ROM = "min198.rom";
char* TOOLKIT_ROM = "tk232.rom";

// Root path for all system files (as drives, roms, etc.)
char* rb_get_system_path() {
    return rb_get_platform_system_path();
}

// Root path for all resource files
char* rb_get_resource_path() {
    return rb_get_platform_resource_path();
}

// Path of system rom
char* rb_get_system_rom() {
    return SYSTEM_ROM;
}

// Path of toolkit rom
char* rb_get_toolkit_rom() {
    return TOOLKIT_ROM;
}

// Path of mdv1 device
char* rb_get_mdv1_path() {
    char temp[256];
    sprintf(temp, "%s/mdv1", rb_get_system_path());
    return temp;
}

// Path of mdv2 device
char* rb_get_mdv2_path() {
    char temp[256];
    sprintf(temp, "%s/mdv2", rb_get_system_path());
    return temp;
}

// Path of win device
char* rb_get_win_path() {
    char temp[256];
    sprintf(temp, "%s/win", rb_get_system_path());
    return temp;
}

// Name of printer device
char* rb_get_device_printer() {
    return "lpt";
}

// Name of printer device
char* rb_get_printer_device() {
    return "lpt";
}

int rb_get_ramtop() {
    return 4096;
}

int rb_get_fast_startup() {
    return 1;
}

int rb_get_cpu_hog() {
    return 0;
}

int rb_get_color() {
    return 0;
}
