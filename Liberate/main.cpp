#define STRICT
/*--------------------------------------------------------------------------*/
#include <windows.h>
#include <windowsx.h>
#include <tlhelp32.h>
#include <service.h>
#include "dd.h"
#include "zdk.h"

/*--------------------------------------------------------------------------*/
// Function declarations
LRESULT CALLBACK WndProc( HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam );
BOOL             Register( HINSTANCE hInst );
HWND             Create( int nCmdShow, int w, int h );

extern "C" DWORD SetSystemMemoryDivision (DWORD dwStorePages);

#define VERSION 1
#define RELAYINPUT_PATH L"\\Drivers\\BuiltIn\\RelayInput"
//#define DEPLOYSAMPLE 1

// Globals
static HWND      g_hwndMain;
static HINSTANCE g_hInstance;
       bool      g_bRunning;

typedef struct {
	TCHAR *ValueName;
	DWORD Type;
	DWORD DwordValue;
	TCHAR *StringValue;
} RegEntry ;

RegEntry RelayInputEntries[] = {
    {TEXT("Dll"), REG_SZ, 0, TEXT("relayinput_serv.dll") },
    {TEXT("Prefix"), REG_SZ, 0, TEXT("INP") },
    {TEXT("Order"), REG_DWORD, 50, NULL },
    {TEXT("UserProcGroup"), REG_DWORD, 2, NULL },
    {TEXT("Flags"), REG_DWORD, 16, NULL },
    {TEXT("Keep"), REG_DWORD, 1, NULL },
    {TEXT("Index"), REG_DWORD, 0, NULL },
	{NULL, 0, 0, NULL}};
 
RegEntry Cab[] = {
	{TEXT("Default"), REG_SZ, 0, TEXT("Cab File")},
	{NULL, 0, 0, NULL}
};

RegEntry CabFile[] = {
	{TEXT("EditFlags"), REG_DWORD, 0x10000, NULL},
	{NULL, 0, 0, NULL}
};

RegEntry CabFileShellOpenCommand[] = {
	{TEXT("Default"), REG_SZ, NULL, TEXT("cabinstall.exe %1")},
	{NULL, 0, 0, NULL}
};

RegEntry CabInstaller[] = {
	{TEXT("Default"), REG_SZ, NULL, TEXT("Cab Installer")},
	{NULL, 0, 0, NULL}
};

RegEntry CabInstallerCLSID[] = {
	{TEXT("Default"), REG_SZ, NULL, TEXT("{4A7DD4C3-05CF-4c7d-823F-5B5514634AE0}")},
	{NULL, 0, 0, NULL}
};

RegEntry DynamicDelete[] = {
	{TEXT("nDynamicDelete"), REG_DWORD, NULL, NULL},
	{NULL, 0, 0, NULL}
};

void CreateEntries(HKEY key, RegEntry entries[])
{
	int Index = 0;
	DWORD uSize = 0;
	BYTE *Value;

	while( entries[Index].ValueName != NULL )
	{
		if( entries[Index].Type == REG_DWORD )
		{
			Value = (BYTE *)&entries[Index].DwordValue;
			uSize = sizeof( DWORD );
		}
		else
		{
			Value = (BYTE *)entries[Index].StringValue;
			uSize = (wcslen(entries[Index].StringValue ) + 1 ) * sizeof( TCHAR );
		}

		DWORD RetVal = RegSetValueEx(key,
			entries[Index].ValueName,
			0,
			entries[Index].Type,
			(CONST BYTE *)Value,
			uSize
			);

		Index++;
	}
}
 
void AddRelayInputEntries()
{
	HKEY      hTargetKey;
	DWORD                dwDisposition;
	DWORD                RetVal;
	DWORD Index=0;
	BYTE * Value;
	DWORD uSize;

	RetVal = RegCreateKeyEx(           HKEY_LOCAL_MACHINE,
		RELAYINPUT_PATH,
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS ,
		NULL,
		&hTargetKey,
		&dwDisposition
		);
	if (RetVal == ERROR_SUCCESS)
	{
		while( RelayInputEntries[Index].ValueName != NULL )
		{
			if( RelayInputEntries[Index].Type == REG_DWORD )
			{
				Value = (BYTE *)&RelayInputEntries[Index].DwordValue;
				uSize = sizeof( DWORD );
			}
			else
			{
				Value = (BYTE *)RelayInputEntries[Index].StringValue;
				uSize = (wcslen( RelayInputEntries[Index].StringValue ) + 1 ) * sizeof( TCHAR );
			}

			RetVal = RegSetValueEx(hTargetKey,
				RelayInputEntries[Index].ValueName,
				0,
				RelayInputEntries[Index].Type,
				(CONST BYTE *)Value,
				uSize
				);
			Index++;
		}
	}

}

inline long CreateKey(HKEY hive, PHKEY key, LPCWSTR path)
{
	DWORD dwDisposition = 0;

	return RegCreateKeyEx(hive,
		path,
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		key,
		&dwDisposition);
}

void AddEntries()
{
	HKEY key = NULL;
	
	if(CreateKey(HKEY_CLASSES_ROOT, &key, L".cab") == ERROR_SUCCESS)
	{
		CreateEntries(key, Cab);

		RegCloseKey(key);
	}

	if(CreateKey(HKEY_CLASSES_ROOT, &key, L"Cab File") == ERROR_SUCCESS)
	{
		CreateEntries(key, CabFile);
		
		RegCloseKey(key);
	}

	if(CreateKey(HKEY_CLASSES_ROOT, &key, L"Cab File\\shell\\open\\command") == ERROR_SUCCESS)
	{
		CreateEntries(key, CabFileShellOpenCommand);

		RegCloseKey(key);
	}
	
	RegCloseKey(key);

	if(CreateKey(HKEY_CLASSES_ROOT, &key, L"CabInstaller") == ERROR_SUCCESS)
	{
		CreateEntries(key, CabInstaller);

		RegCloseKey(key);
	}
	
	if(CreateKey(HKEY_CLASSES_ROOT, &key, L"CabInstallerCLSID") == ERROR_SUCCESS)
	{
		CreateEntries(key, CabInstallerCLSID);

		RegCloseKey(key);
	}

	if(CreateKey(HKEY_LOCAL_MACHINE, &key, L"Software\\Apps\\Microsoft Application Installer") == ERROR_SUCCESS)
	{
		CreateEntries(key, DynamicDelete);

		RegCloseKey(key);
	}
}

bool DestroyEvil()
{
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snapshot == INVALID_HANDLE_VALUE) return FALSE;
	PROCESSENTRY32 process;
	process.dwSize = sizeof(PROCESSENTRY32);
	int successful = Process32First(snapshot, &process);
	while (successful)
	{
		for(unsigned int i = 0; i < wcslen(process.szExeFile); i++)
			process.szExeFile[i] = tolower(process.szExeFile[i]);

		if(!wcscmp(process.szExeFile, L"compositor.exe"))
		{
			TerminateProcess((HANDLE)process.th32ProcessID, 0);
			CloseToolhelp32Snapshot(snapshot);
			return true;
		}

		successful = Process32Next(snapshot, &process);
	}

	CloseToolhelp32Snapshot(snapshot);

	return false;
}

void CopyFiles()
{
	CopyFile(L"\\gametitle\\584E07D1\\Content\\Liberate.exe", L"\\flash2\\Liberate\\Liberate.exe", FALSE);
	CopyFile(L"\\gametitle\\584E07D1\\Content\\RelayInput.exe", L"\\flash2\\Liberate\\RelayInput.exe", FALSE);
	CopyFile(L"\\gametitle\\584E07D1\\Content\\Keyboard.exe", L"\\flash2\\Liberate\\Keyboard.exe", FALSE);
	CopyFile(L"\\gametitle\\584E07D1\\Content\\JOTKBD.exe", L"\\flash2\\Liberate\\JOTKBD.exe", FALSE);
	CopyFile(L"\\gametitle\\584E07D1\\Content\\Dummy.exe", L"\\Windows\\gemstone.exe", FALSE);
	CopyFile(L"\\gametitle\\584E07D1\\Content\\Reboot.exe", L"\\flash2\\Liberate\\Reboot.exe", FALSE);
	CopyFile(L"\\gametitle\\584E07D1\\Content\\Reboot.lnk", L"\\Windows\\Desktop\\Reboot.lnk", FALSE);
	CopyFile(L"\\gametitle\\584E07D1\\Content\\JOTKBD.lnk", L"\\Windows\\Desktop\\JOTKBD.lnk", FALSE);
	CopyFile(L"\\gametitle\\584E07D1\\Content\\explorer.exe", L"\\flash2\\Liberate\\explorer.exe", FALSE);
	CopyFile(L"\\gametitle\\584E07D1\\Content\\wceload.exe", L"\\Windows\\wceload.exe", FALSE);
	CopyFile(L"\\gametitle\\584E07D1\\Content\\cabinstall.exe", L"\\Windows\\cabinstall.exe", FALSE);

	CopyFile(L"\\gametitle\\584E07D1\\Content\\Libraries\\aygshell.dll", L"\\Windows\\aygshell.dll", FALSE);
	CopyFile(L"\\gametitle\\584E07D1\\Content\\Libraries\\gx.dll", L"\\Windows\\gx.dll", FALSE);
	CopyFile(L"\\gametitle\\584E07D1\\Content\\Libraries\\imgdecmp.dll", L"\\Windows\\imgdecmp.dll", FALSE);
	CopyFile(L"\\gametitle\\584E07D1\\Content\\Libraries\\tgetfile.dll", L"\\Windows\\tgetfile.dll", FALSE);
	CopyFile(L"\\gametitle\\584E07D1\\Content\\Libraries\\RelayInput.dll", L"\\Windows\\relayinput_serv.dll", FALSE);

#ifdef DEPLOYSAMPLE
	CopyFile(L"\\gametitle\\584E07D1\\Content\\Sample.exe", L"\\Windows\\Desktop\\Sample.exe", FALSE);
#endif

	WIN32_FIND_DATA finddata;
	HANDLE file = FindFirstFile(L"\\flash2\\Files", &finddata);

	if(file != INVALID_HANDLE_VALUE)
	{
		do
		{
			if(finddata.dwFileAttributes == 16)
			{
				CopyFile(L"\\gametitle\\584E07D1\\Content\\Files.lnk", L"\\Windows\\Desktop\\Files.lnk", FALSE);
				break;
			}
		}
		while(FindNextFile(file, &finddata));
		
		FindClose(file);
	}
}

void SetReboot(bool reboot)
{
	HKEY key = NULL;
	HRESULT hr = S_OK;
	DWORD value;

	if(reboot)
	{
		if (SUCCEEDED(hr))
			hr = HRESULT_FROM_WIN32(RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"System\\CurrentControlSet\\Control\\Power\\State\\Reboot", 0, 0, &key));
		if (SUCCEEDED(hr))
			hr = HRESULT_FROM_WIN32(RegSetValueEx(key, L"Flags", 0, REG_DWORD, (BYTE *)&(value = 0x800000), sizeof(DWORD)));
		if (SUCCEEDED(hr))
			hr = HRESULT_FROM_WIN32(RegSetValueEx(key, L"Default", 0, REG_DWORD, (BYTE *)&(value = 4), sizeof(DWORD)));
		if (key)
			RegCloseKey(key);
	}
	else
	{
		if (SUCCEEDED(hr))
			hr = HRESULT_FROM_WIN32(RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"System\\CurrentControlSet\\Control\\Power\\State\\Reboot", 0, 0, &key));
		if (SUCCEEDED(hr))
			hr = HRESULT_FROM_WIN32(RegSetValueEx(key, L"Flags", 0, REG_DWORD, (BYTE *)&(value = 0x10000), sizeof(DWORD)));
		if (SUCCEEDED(hr))
			hr = HRESULT_FROM_WIN32(RegSetValueEx(key, L"Default", 0, REG_DWORD, (BYTE *)&(value = 0), sizeof(DWORD)));
		if (SUCCEEDED(hr))
			hr = HRESULT_FROM_WIN32(RegFlushKey(key));
		if (key)
			RegCloseKey(key);
	}
}

void SearchWindows(HWND window, LPARAM processID)
{
	wchar_t buffer[64];
	DWORD ID = NULL;

	GetWindowText(window, buffer, 64);

	if(!wcscmp(buffer, L"Gemstone"))
	{
		GetWindowThreadProcessId(window, &ID);
		TerminateProcess((HANDLE)ID, 0);
	}
	else
		if(!wcscmp(buffer, L"ZHudService"))
			SendMessage(window, WM_CLOSE, NULL, NULL);
}

static int DisableTimeout()
{
	HANDLE hservice;

	hservice = GetServiceHandle(TEXT("MTP0:"), NULL, NULL);
	if (hservice != INVALID_HANDLE_VALUE) {
		if (DeregisterService(hservice) != TRUE) {
			fprintf(stderr, "Failed to unregister MTP0:\n");
			return -1;
		}
	}

	Sleep(1000);

	hservice = ActivateService(TEXT("MtpSvc"), 0);
	if (hservice == NULL) {
		fprintf(stderr, "Couldn't register MPT0:n");
		return -1;
	}

	return 0;
}

/*--------------------------------------------------------------------------*/
int APIENTRY wWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPWSTR     lpCmdLine,
                     int       nCmdShow)
{
	DWORD result = SetSystemMemoryDivision(4500);

	PROCESS_INFORMATION pinfo;

#ifdef NDEBUG
	if(wcscmp(lpCmdLine, L"Start"))
	{
		SetReboot(false);
		
		FILE *file = fopen("\\gametitle\\584E07D1\\Content\\Liberate.exe", "rb");

		if(file)
		{
			fclose(file);

			CreateDirectory(L"\\flash2\\Liberate", NULL);

			CopyFiles();
		}
		
		CreateProcess(L"\\flash2\\Liberate\\RelayInput.exe", L"Init", NULL, NULL, NULL, NULL, NULL, NULL, NULL, &pinfo);

		return 0;
	}
#else
	if(wcscmp(lpCmdLine, L"Start"))
		CreateProcess(L"\\flash2\\Liberate\\RelayInput.exe", L"", NULL, NULL, NULL, NULL, NULL, NULL, NULL, &pinfo);
	else
		CopyFiles();
#endif

	CreateProcess(L"\\flash2\\Liberate\\explorer.exe", L"", NULL, NULL, NULL, NULL, NULL, NULL, NULL, &pinfo);

	AddEntries();
	AddRelayInputEntries();
	
	HANDLE hand = ActivateDevice(RELAYINPUT_PATH, 0);

	DWORD last = GetLastError();

	g_hInstance = hInstance;
	
	if (!hPrevInstance) {
		if (!Register( g_hInstance ))
			return FALSE;
	}
	
	// Create the main window
	g_hwndMain = Create( nCmdShow, 272, 480 );
	if (!g_hwndMain)
		return FALSE;  
	
	// Initialize DirectDraw
	if (!DDInit( g_hwndMain ))
	{
		MessageBox( g_hwndMain, L"Failed to initialize DirectDraw", L"Error", MB_OK );
		return 0;
	}
	
	// Create DirectDraw surfaces
	if (!DDCreateSurfaces( false ))
	{
		MessageBox( g_hwndMain, L"Failed to create surfaces", L"Error", MB_OK );
		return 0;
	}

	DDDone();

	DestroyWindow(g_hwndMain);

	EnumWindows((WNDENUMPROC)SearchWindows, (LPARAM)NULL);

	while(DestroyEvil())
		Sleep(10);

	DisableTimeout();

	return 0;
}

/*--------------------------------------------------------------------------*/
// Handle WM_DESTROY
void OnDestroy(HWND )
{
	g_bRunning = false;
	PostQuitMessage(0);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch(Message)
	{
		HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
		//HANDLE_MSG(hwnd, WM_PAINT, OnPaint);
	}
	return DefWindowProc(hwnd, Message, wParam, lParam);
}
/*--------------------------------------------------------------------------*/
//
// Register the window class
//
BOOL Register(HINSTANCE hInst)
{
	WNDCLASS wndclass;

	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hIcon = NULL;
	wndclass.hInstance = hInst;
	wndclass.hCursor = NULL;
	wndclass.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = L"Liberate";

	return RegisterClass (&wndclass);
}
/*--------------------------------------------------------------------------*/
//
// Create the window
//
HWND Create( int nCmdShow, int w, int h )
{
	RECT rc;
	
	// Calculate size of window based on desired client window size
	rc.left = 0;
	rc.top = 0;
	rc.right = w;
	rc.bottom = h;
	AdjustWindowRectEx(&rc, WS_OVERLAPPED, FALSE, NULL);
	
	HWND hwnd = CreateWindow(L"Liberate", L"Liberating... Stand by.",
		WS_OVERLAPPED,
		CW_USEDEFAULT, CW_USEDEFAULT,
		rc.right-rc.left, rc.bottom-rc.top,
		NULL, NULL, g_hInstance, NULL);
	
	if (hwnd == NULL)
		return hwnd;
	
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);
	
	return hwnd;
}