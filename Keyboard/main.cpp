
#include "hooks.h"
#include "keybd.h"

TCHAR szAppName[] = L"Keyboard Hook";

LONG FAR PASCAL WndProc (HWND , UINT , UINT , LONG) ;

HINSTANCE g_hInstance		= NULL;							// Handle to app calling the hook (me!)
HINSTANCE  g_hHookApiDLL	= NULL;							// Handle to loaded library (system DLL where the API is located)

BOOL g_HookDeactivate();
BOOL g_HookActivate(HINSTANCE hInstance);

#pragma data_seg(".HOOKDATA")								//	Shared data (memory) among all instances.
	HHOOK g_hInstalledLLKBDhook = NULL;						// Handle to low-level keyboard hook
	HHOOK g_hInstalledCallWndProchook = NULL;
#pragma data_seg()

#pragma comment(linker, "/SECTION:.HOOKDATA,RWS")			//linker directive

uint DownState = KeyStateDownFlag;
uint UpState = 0;

__declspec(dllexport) LRESULT CALLBACK g_LLKeyboardHookCallback(
   int nCode,      // The hook code
   WPARAM wParam,  // The window message (WM_KEYUP, WM_KEYDOWN, etc.)
   LPARAM lParam   // A pointer to a struct with information about the pressed key
) 
{
	static bool caps = false, shift = false;
	static uint syskey = 0;
	static char syscount = 0;
	bool processed_key = false;

	if (nCode == HC_ACTION) 
	{ 
		PKBDLLHOOKSTRUCT pkbhData = (PKBDLLHOOKSTRUCT)lParam;
		
		uint key = (uint)pkbhData->vkCode;

		if(wParam == WM_KEYUP)
		{
			if(key == VK_SHIFT)
				shift = false;

#if DEBUG
			printf("Key up: %c, #=%d\n", key, key);
#endif
		}
		else
		if(wParam == WM_KEYDOWN)
		{
			bool capital = shift != caps;
			UINT vkey = key;

			if(key >= 0x30 && key <= 0x39)
			{
				capital = shift;

				if(!capital)
				{
					PostKeybdMessage((HWND)-1, vkey, KeyStateDownFlag, 1, &DownState, &key);
#if DEBUG
					printf("Lower number down: %c\n", key);
#endif
				}
				else
				{
					switch(key)
					{
						case '1': key = '!'; break;
						case '2': key = '@'; break;
						case '3': key = '#'; break;
						case '4': key = '$'; break;
						case '5': key = '%'; break;
						case '6': key = '^'; break;
						case '7': key = '&'; break;
						case '8': key = '*'; break;
						case '9': key = '('; break;
						case '0': key = ')'; break;
					}

					PostKeybdMessage((HWND)-1, vkey, KeyStateDownFlag, 1, &DownState, &key);
#if DEBUG
					printf("Upper number down: %c\n", key);
#endif
				}

				processed_key = true;
			}
			else
				if(key >= 0x39 && key <= 0x5A)
				{
					if(capital)
					{
						PostKeybdMessage((HWND)-1, vkey, KeyStateDownFlag, 1, &DownState, &key);
#if DEBUG
						printf("Upper letter down: %c\n", key);
#endif
					}
					else
					{
#if DEBUG
						printf("Lower letter down: %c\n", key);
#endif
						key += 32;

						PostKeybdMessage((HWND)-1, vkey, KeyStateDownFlag, 1, &DownState, &key);
					}

					processed_key = true;
				}
				else
				{
					processed_key = true;

					if(!capital)
						switch(key)
						{
							case VK_ADD:		key = '+';	break;
							case VK_APOSTROPHE: key = '\'';	break;
							case VK_BACKQUOTE:	key = '`';	break;
							case VK_BACKSLASH:	key = '\\';	break;
							case VK_DIVIDE:		key = VK_BACKSLASH;	break;
							case VK_EQUAL:		key = '=';	break;
							case VK_COMMA:		key = ',';	break;
								//case VK_DECIMAL:	key = '.';	break;
							case VK_HYPHEN:		key = '-';	break;
							case VK_LBRACKET:	key = '[';	break;
							case VK_PERIOD:		key = '.';	break;
							case VK_RBRACKET:	key = ']';	break;
							case VK_RETURN:		break;
							case VK_SEMICOLON:	key = ';';	break;
							case VK_SLASH:		key = '/';	break;
							case VK_SPACE:		key = ' ';	break;
							case VK_SUBTRACT:	key = '-';	break;
							case VK_TAB:		key = '	';	break;

							default:
								processed_key = false;
								break;
					}
					else
						switch(key)
						{
							case VK_ADD:		key = '+';	break;
							case VK_APOSTROPHE: key = '"';	break;
							case VK_BACKQUOTE:	key = '~';	break;
							case VK_BACKSLASH:  key = '|';	break;
							case VK_COMMA:		key = '<';	break;
							case VK_EQUAL:		key = '+';	break;
								//case VK_DECIMAL:	key = '.';	break;
							case VK_HYPHEN:		key = '_';	break;
							case VK_LBRACKET:	key = '{';	break;
							case VK_PERIOD:		key = '>';	break;
							case VK_RBRACKET:	key = '}';	break;
							case VK_RETURN:		break;
							case VK_SEMICOLON:	key = ':';	break;
							case VK_SLASH:		key = '?';	break;
							case VK_SPACE:		key = ' ';	break;
							case VK_SUBTRACT:	key = '_';	break;
							case VK_TAB:		key = '	';	break;

							default:
								processed_key = false;
								break;
					}

					if(processed_key)
					{
						PostKeybdMessage((HWND)-1, vkey, KeyStateDownFlag, 1, &DownState, &key);
#if DEBUG
						printf("Special down: %c\n", key);
#endif
					}
					else
						if(key == VK_SHIFT)
						{
							shift = true;
#if DEBUG
							printf("Shift down: %c\n", key);
#endif
						}
						else
							if(key == VK_CAPITAL)
							{
								caps = !caps;
#if DEBUG
								if(caps)
									printf("Caps down: %c, ON\n", key);
								else
									printf("Caps down: %c, OFF\n", key);
#endif
							}
				}
		}
		else
			if(wParam == WM_SYSKEYDOWN)
			{
				if(key == 18)
				{
					syscount = 0;
					return true;
				}
				else
					syscount++;

				switch(syscount)
				{
					case 1:
						syskey = 1000 * (key - '0');
						break;
						
					case 2:
						syskey += 100 * (key - '0');
						break;

					case 3:
						syskey += 10 * (key - '0');
						break;
						
					case 4:
						syskey += (key - '0');
						syscount = 0;
						break;
				}

				if(!syscount)
				{
					PostKeybdMessage((HWND)-1, key, KeyStateDownFlag, 1, &DownState, &syskey);
#if DEBUG
					printf("System down: %d, ON\n", key);
#endif
				}
			}
	}
	//shall we forward processed keys?
	if (processed_key)
		return true;
	else
		return CallNextHookEx(g_hInstalledLLKBDhook, nCode, wParam, lParam);
}

BOOL g_HookActivate(HINSTANCE hInstance)
{
	// We manually load these standard Win32 API calls (Microsoft says "unsupported in CE")
	SetWindowsHookEx		= NULL;
	CallNextHookEx			= NULL;
	UnhookWindowsHookEx	= NULL;

	// Load the core library. If it's not found, you've got CErious issues :-O
	//TRACE(_T("LoadLibrary(coredll.dll)..."));
	g_hHookApiDLL = LoadLibrary(_T("coredll.dll"));
	if(g_hHookApiDLL == NULL) return false;
	else {
		// Load the SetWindowsHookEx API call (wide-char)
		//TRACE(_T("OK\nGetProcAddress(SetWindowsHookExW)..."));
		SetWindowsHookEx = (_SetWindowsHookExW)GetProcAddress(g_hHookApiDLL, _T("SetWindowsHookExW"));
		if(SetWindowsHookEx == NULL) return false;
		else
		{
			// Load the hook.  Save the handle to the hook for later destruction.
			//TRACE(_T("OK\nCalling SetWindowsHookEx..."));
			g_hInstalledLLKBDhook = SetWindowsHookEx(WH_KEYBOARD_LL, g_LLKeyboardHookCallback, hInstance, 0);
			if(g_hInstalledLLKBDhook == NULL) return false;
		}

		// Get pointer to CallNextHookEx()
		//TRACE(_T("OK\nGetProcAddress(CallNextHookEx)..."));
		CallNextHookEx = (_CallNextHookEx)GetProcAddress(g_hHookApiDLL, _T("CallNextHookEx"));
		if(CallNextHookEx == NULL) return false;

		// Get pointer to UnhookWindowsHookEx()
		//TRACE(_T("OK\nGetProcAddress(UnhookWindowsHookEx)..."));
		UnhookWindowsHookEx = (_UnhookWindowsHookEx)GetProcAddress(g_hHookApiDLL, _T("UnhookWindowsHookEx"));
		if(UnhookWindowsHookEx == NULL) return false;
	}
	//TRACE(_T("OK\nEverything loaded OK\n"));
	return true;
}


BOOL g_HookDeactivate()
{
	//TRACE(_T("Uninstalling hook..."));
	if(g_hInstalledLLKBDhook != NULL)
	{
		UnhookWindowsHookEx(g_hInstalledLLKBDhook);		// Note: May not unload immediately because other apps may have me loaded
		g_hInstalledLLKBDhook = NULL;
	}

	//TRACE(_T("OK\nUnloading coredll.dll..."));
	if(g_hHookApiDLL != NULL)
	{
		FreeLibrary(g_hHookApiDLL);
		g_hHookApiDLL = NULL;
	}
	//TRACE(_T("OK\nEverything unloaded OK\n"));
	return true;
}

LONG FAR PASCAL WndProc (HWND hwnd   , UINT message , 
                         UINT wParam , LONG lParam)                
                            
{
	switch (message)         
	{
	case WM_CREATE:
		if (!g_HookActivate(g_hInstance))
		{
			MessageBox(hwnd, L"Could not hook!", L"Error", MB_OK | MB_ICONEXCLAMATION);
			PostQuitMessage(-1);
		}
		return 0;

	case WM_DESTROY:

		g_HookDeactivate();
		PostQuitMessage(0); 
		return 0;
		break;
	}

	return DefWindowProc (hwnd , message , wParam , lParam) ;
}

int WINAPI WinMain(	HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPTSTR    lpCmdLine,
					int       nCmdShow)
{
	MSG      msg      ;
	HWND     hwnd     ;
	WNDCLASS wndclass ;

	wndclass.style         = CS_HREDRAW | CS_VREDRAW; 
	wndclass.lpfnWndProc   = WndProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = hInstance;
	wndclass.hIcon         = NULL;
	wndclass.hCursor       = NULL; 
	wndclass.hbrBackground = NULL;
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = szAppName; 

	RegisterClass (&wndclass);

	g_hInstance=hInstance; 

	hwnd = CreateWindow (szAppName , L"" ,   
		NULL,          // Style flags                         
		0,       // x position                         
		0,       // y position                         
		50,       // Initial width                         
		50,       // Initial height                         
		NULL,                // Parent                         
		NULL,                // Menu, must be null                         
		hInstance,           // Application instance                         
		NULL);               // Pointer to create
	// parameters
	if (!IsWindow (hwnd)) 
		return 0; // Fail if not created.

	while (GetMessage (&msg , NULL , 0 , 0))   
	{
		TranslateMessage (&msg) ;         
		DispatchMessage  (&msg) ;         
	}

	return msg.wParam;
}