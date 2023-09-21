
#include <Windows.h>
//#include <commdlg.h>
#include <SDL.h>
#include <SDL_syswm.h>

#include "resource.h"
#include "SDLInterface.h"
//#include "../../src/faux86/Config.h"

//extern Faux86::VM* vm86;
extern void SetScreenSize(uint16_t _scrw, uint16_t _scrh);
extern void SetRenderQuality(uint8_t _mode);
extern void SetCpuSpeed(uint32_t _speed);
extern void SetMonitorType(uint8_t _type);
extern void SendSoftReset();
extern void SendHardReset();

struct
{
	uint8_t running;
	uint8_t bootdrive;
	uint8_t dohardreset;
	uint8_t dosoftreset;
	uint8_t renderquality;
	uint8_t cpuspeed;
	uint8_t monitortype;
	uint16_t scrwidth;
	uint16_t scrheight;
} menu_settings;


//uint8_t insertdisk (uint8_t drivenum, char *filename);
//uint8_t ejectdisk (uint8_t drivenum);

HWND myWindow;
HINSTANCE myInstance;
HMENU myMenu;
WNDPROC oldProc;
HWND GetHwnd();
HICON myIcon;
void SetWndProc();
void MenuItemClick(WPARAM wParam);

void ShowMenu() {
	SetMenu(myWindow, myMenu);
}

void HideMenu() {
	SetMenu(myWindow, NULL);
}

void initmenus(HWND hwnd) {
	//myWindow = GetHwnd();
	myWindow = hwnd;
	myInstance = GetModuleHandle(NULL);
	myMenu = LoadMenu(NULL, MAKEINTRESOURCE(IDR_MENU1));
	ShowMenu();
	SetWndProc();
	//myIcon = LoadIcon(myInstance, MAKEINTRESOURCE(IDI_ICON1));
	//SetClassLong(myWindow, GCLP_HICON, (LONG)(uint64_t)myIcon);
	//SetClassLong(myWindow, GCLP_HICONSM, (LONG)(uint64_t)myIcon);

	return;
}

HWND GetHwnd() {
	SDL_SysWMinfo wmi;
	SDL_VERSION(&wmi.version);

	//if (!SDL_GetWMInfo(&wmi)) return(NULL); //SDL1
	if (!SDL_GetWindowWMInfo(0, &wmi)) return(NULL); //appWindow
	//return(wmi.window); //SDL1
	return(wmi.info.win.window);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_COMMAND:
			MenuItemClick(wParam);
			return(TRUE);
	}

	return(CallWindowProc(oldProc, hwnd, msg, wParam, lParam));
}

void SetWndProc() {
	oldProc = (WNDPROC)SetWindowLong(myWindow, GWL_WNDPROC, (LONG_PTR)WndProc);
}

void MenuItemClick(WPARAM wParam) {
	OPENFILENAME of_dlg;
	//uint8_t filename[MAX_PATH] = { 0 };
	char filename[MAX_PATH] = { 0 };
	
	uint8_t screenchange = 0;
	uint8_t speedchange = 0;
	uint8_t renderchange = 0;
	uint8_t monitorchange = 0;

	switch (LOWORD(wParam)) {
		//file menu
		case ID_MAIN_EXIT:
			menu_settings.running = 0;
			break;

		//emulation menu
		case ID_EMULATION_SOFTRESET:
			menu_settings.dosoftreset = 1;
			SendSoftReset();
			break;
		case ID_EMULATION_HARDRESET:
			menu_settings.dohardreset = 1;
			SendHardReset();
			break;
			
		case ID_EMULATION_SPEED_AUTOMATIC:
			menu_settings.cpuspeed = 0;
			speedchange = 1;
			break;
		case ID_EMULATION_SPEED_5MHZ:
			menu_settings.cpuspeed = 5;
			speedchange = 1;
			break;
		case ID_EMULATION_SPEED_10MHZ:
			menu_settings.cpuspeed = 10;
			speedchange = 1;
			break;
		case ID_EMULATION_SPEED_16MHZ:
			menu_settings.cpuspeed = 16;
			speedchange = 1;
			break;
		case ID_EMULATION_SPEED_20MHZ:
			menu_settings.cpuspeed = 20;
			speedchange = 1;
			break;
		case ID_EMULATION_SPEED_33MHZ:
			menu_settings.cpuspeed = 33;
			speedchange = 1;
			break;
		case ID_EMULATION_SPEED_50MHZ:
			menu_settings.cpuspeed = 50;
			speedchange = 1;
			break;
		case ID_EMULATION_SPEED_100MHZ:
			menu_settings.cpuspeed = 100;
			speedchange = 1;
			break;

		case ID_FLOPPY0_INSERTDISK:
		case ID_FLOPPY1_INSERTDISK:
			break; //TODO
			memset(&of_dlg, 0, sizeof(of_dlg));
			of_dlg.lStructSize = sizeof(of_dlg);
			of_dlg.lpstrTitle = "Open disk image";
			of_dlg.hInstance = NULL;
			of_dlg.lpstrFile = filename;
			of_dlg.lpstrFilter = "Floppy disk images (*.img)\0*.img\0All files (*.*)\0*.*\0\0";
			of_dlg.nMaxFile = MAX_PATH;
			of_dlg.Flags = OFN_FILEMUSTEXIST | OFN_LONGNAMES;
			if (GetOpenFileName(&of_dlg)) {
				if (LOWORD(wParam) == ID_FLOPPY0_INSERTDISK) {
					//insertdisk(0, (char *)of_dlg.lpstrFile);
					if (menu_settings.bootdrive == 255) menu_settings.bootdrive = 0;
				} else {
					//insertdisk(1, (char *)of_dlg.lpstrFile);		
				}
			}
			break;
		case ID_FLOPPY0_EJECTDISK:
			//ejectdisk(0); //TODO
			break;
		case ID_FLOPPY1_EJECTDISK:
			//ejectdisk(1); //TODO
			break;

		case ID_HARDDRIVE0_INSERTDISK:
		case ID_HARDDRIVE1_INSERTDISK:
			break; ////TODO
			memset(&of_dlg, 0, sizeof(of_dlg));
			of_dlg.lStructSize = sizeof(of_dlg);
			of_dlg.lpstrTitle = "Open disk image";
			of_dlg.hInstance = NULL;
			of_dlg.lpstrFile = filename;
			of_dlg.lpstrFilter = "Raw disk images (*.raw, *.img)\0*.raw;*.img\0All files (*.*)\0*.*\0\0";
			of_dlg.nMaxFile = MAX_PATH;
			of_dlg.Flags = OFN_FILEMUSTEXIST | OFN_LONGNAMES;
			if (GetOpenFileName(&of_dlg)) {
				if (LOWORD(wParam) == ID_HARDDRIVE0_INSERTDISK) {
					//insertdisk(128, (char *)of_dlg.lpstrFile);
					if (menu_settings.bootdrive == 255) menu_settings.bootdrive = 128;
				} else {
					//insertdisk(129, (char *)of_dlg.lpstrFile);	
				}
			}
			break;
		case ID_HARDDRIVE0_EJECTDISK:
			//ejectdisk(128); //TODO
			break;
		case ID_HARDDRIVE1_EJECTDISK:
			//ejectdisk(129); //TODO
			break;

		case ID_SETBOOTDRIVE_FLOPPY0:
			menu_settings.bootdrive = 0;
			break;
		case ID_SETBOOTDRIVE_HARDDRIVE0:
			menu_settings.bootdrive = 128;
			break;

		//video menu
		case ID_VIDEO_RESOLUTION_AUTOMATIC:
			menu_settings.scrwidth = 0;
			menu_settings.scrheight = 0;
			screenchange = 1;
			break;
		case ID_VIDEO_RESOLUTION_320X200:
			menu_settings.scrwidth = 320;
			menu_settings.scrheight = 200;
			screenchange = 1;
			break;
		case ID_VIDEO_RESOLUTION_640X350:
			menu_settings.scrwidth = 640;
			menu_settings.scrheight = 350;
			screenchange = 1;
			break;
		case ID_VIDEO_RESOLUTION_640X400:
			menu_settings.scrwidth = 640;
			menu_settings.scrheight = 400;
			screenchange = 1;
			break;
		case ID_VIDEO_RESOLUTION_640X480:
			menu_settings.scrwidth = 640;
			menu_settings.scrheight = 480;
			screenchange = 1;
			break;
		case ID_VIDEO_RESOLUTION_720X400:
			menu_settings.scrwidth = 720;
			menu_settings.scrheight = 400;
			screenchange = 1;
			break;
			
		case ID_VIDEO_RENDER_LINEAR:
			menu_settings.renderquality = 0;
			renderchange = 1;
			break;
		case ID_VIDEO_RENDER_NEAREST:
			menu_settings.renderquality = 1;
			renderchange = 1;
			break;
		case ID_VIDEO_RENDER_BEST:
			menu_settings.renderquality = 2;
			renderchange = 1;
			break;
			
		case ID_VIDEO_MONITOR_COLOR:
			menu_settings.monitortype = 0;
			monitorchange = 1;
			break;
		case ID_VIDEO_MONITOR_AMBER:
			menu_settings.monitortype = 1;
			monitorchange = 1;
			break;
		case ID_VIDEO_MONITOR_GREEN:
			menu_settings.monitortype = 2;
			monitorchange = 1;
			break;
		case ID_VIDEO_MONITOR_BLUE:
			menu_settings.monitortype = 3;
			monitorchange = 1;
			break;
		case ID_VIDEO_MONITOR_BLUE_TOSH:
			menu_settings.monitortype = 4;
			monitorchange = 1;
			break;
		case ID_VIDEO_MONITOR_TEAL_TERM:
			menu_settings.monitortype = 5;
			monitorchange = 1;
			break;
		case ID_VIDEO_MONITOR_GREEN_TERM:
			menu_settings.monitortype = 6;
			monitorchange = 1;
			break;
		case ID_VIDEO_MONITOR_AMBER_TERM:
			menu_settings.monitortype = 7;
			monitorchange = 1;
			break;
	}
	if (screenchange) {
		SetScreenSize(menu_settings.scrwidth, menu_settings.scrheight);
	}
	if (renderchange) {
		SetRenderQuality(menu_settings.renderquality);
	}
	if (speedchange) {
		SetCpuSpeed(menu_settings.cpuspeed);
	}
	if (monitorchange) {
		SetMonitorType(menu_settings.monitortype);
	}
}
