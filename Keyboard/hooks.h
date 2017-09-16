// Undocumented though operational though unsupported Hooks: types and structs

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <windows.h>
#include <winuser.h>

#define WH_KEYBOARD_LL   		20

#define HC_ACTION           	0
#define HC_GETNEXT          	1
#define HC_SKIP             	2
#define HC_NOREMOVE         	3
#define HC_SYSMODALON       	4
#define HC_SYSMODALOFF      	5

#define HC_NOREM            	HC_NOREMOVE

// Used by WH_KEYBOARD_LL
#define LLKHF_EXTENDED       	(KF_EXTENDED >> 8)
#define LLKHF_INJECTED       	0x00000010
#define LLKHF_ALTDOWN        	(KF_ALTDOWN >> 8)
#define LLKHF_UP             	(KF_UP >> 8)
#define LLMHF_INJECTED       	0x00000001

// Define the function types used by hooks
typedef LRESULT	(CALLBACK* HOOKPROC)(int code, WPARAM wParam, LPARAM lParam);
typedef HHOOK 	(WINAPI *_SetWindowsHookExW)(int, HOOKPROC, HINSTANCE, DWORD);
typedef LRESULT	(WINAPI *_CallNextHookEx)(HHOOK, int, WPARAM, LPARAM);
typedef LRESULT	(WINAPI *_UnhookWindowsHookEx)(HHOOK);

// For the low level keyboard hook, you are passed a pointer to one of these
typedef struct {
    DWORD vkCode;
    DWORD scanCode;
    DWORD flags;
    DWORD time;
    ULONG_PTR dwExtraInfo;
} KBDLLHOOKSTRUCT, *PKBDLLHOOKSTRUCT;

// Win32 Hook APIs (manually loaded)
static _SetWindowsHookExW 		SetWindowsHookEx;
static _UnhookWindowsHookEx	UnhookWindowsHookEx;
static _CallNextHookEx  		CallNextHookEx;	
