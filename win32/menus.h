
#ifndef _MENUS_H_
#define _MENUS_H_

#ifdef _WIN32
#include <Windows.h>
#include "machine.h"

// Microsoft Visual C++ generated include file.
// Used by menus.rc
//
#define IDR_MENUBAR                     101
#define ID_FILE_SOFTRESET               40001
#define ID_FILE_EXIT                    40002
#define ID_EMULATION_SETCPUSPEEDTO4     40003
#define ID_EMULATION_SETCPUSPEEDTO8MHZ  40004
#define ID_EMULATION_SETCPUSPEEDTO10MHZ 40005
#define ID_EMULATION_SETCPUSPEEDTO16MHZ 40006
#define ID_EMULATION_SETCPUSPEEDTO25MHZ 40007
#define ID_EMULATION_SETCPUSPEEDTO50MHZ 40008
#define ID_EMULATION_SETCPUSPEEDTOUNLIMITED 40009
#define ID_DISK_CHANGEFLOPPY0           40010
#define ID_DISK_CHANGEFLOPPY1           40011
#define ID_DISK_EJECTFLOPPY0            40012
#define ID_DISK_EJECTFLOPPY1            40013
#define ID_DISK_INSERTHARDDISK0         40014
#define ID_DISK_INSERTHARDDISK1         40015
#define ID_DISK_SETBOOTDRIVETOFD0       40016
#define ID_DISK_SETBOOTDRIVETOHD0       40017

// Next default values for new objects
//
#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS
#define _APS_NEXT_RESOURCE_VALUE        102
#define _APS_NEXT_COMMAND_VALUE         40018
#define _APS_NEXT_CONTROL_VALUE         1001
#define _APS_NEXT_SYMED_VALUE           101
#endif
#endif

int menus_init(HWND hwnd);

#endif //_WIN32

#endif //_MENUS_H_
