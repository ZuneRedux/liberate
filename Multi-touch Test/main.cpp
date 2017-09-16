#include <Windows.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string>
#include <zdkinput.h>

using namespace std;

string relativepath;
HANDLE context;

#define GETRELAYINPUT CreateFile(L"INP0:", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)
#define GETINPUT(x, y, z) DeviceIoControl(x, NULL, NULL, NULL, (LPVOID)y, sizeof(ZDKINPUT_STATE), z, NULL);

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{          
    switch (message)
    {
		/*case WM_CHAR:
			printf("WM_CHAR: %c, %d\n", wParam, wParam);
			break;

		case WM_KEYDOWN:
			printf("WM_KEYDOWN: %c, %d\n", wParam, wParam);
			break;*/

        case WM_LBUTTONDOWN:
			{
				MessageBox(hWnd, L"Touch the screen with your fingers after pressing \"OK\".", L"Confucious say:", MB_OK);

				RECT rect;
				GetClientRect(hWnd, &rect);
				InvalidateRect(hWnd, &rect, TRUE);

				Sleep(2000);

				OVERLAPPED overlapped;
				DWORD returned = NULL;
				ZDKINPUT_STATE state = {0}; 

				DWORD result = GETINPUT(context, &state, &returned);

				WCHAR buffer[64];
				wsprintf(buffer, L"%d touches.", state.TouchState.Count);

				MessageBox(hWnd, buffer, L"Success", MB_OK);
			}
			break;

        case WM_DESTROY:                                                    
            PostQuitMessage(0);                                             
            break;

        default:                                                            
            return DefWindowProc(hWnd, message, wParam, lParam);            
    }                                                                   
    return 0;                                                           
}    


ATOM MyRegisterClass()
{
    WNDCLASS wndclass;

#if _WIN32_WCE
    wndclass.style          = 0;
#else
    wndclass.style          = CS_OWNDC;
#endif
    wndclass.lpfnWndProc    = WndProc;
    wndclass.cbClsExtra     = 0;
    wndclass.cbWndExtra     = 0;
    wndclass.hInstance      = (HINSTANCE)GetModuleHandle(NULL);
    wndclass.hIcon          = NULL;
    wndclass.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground  = (HBRUSH) GetStockObject (BLACK_BRUSH);
    wndclass.lpszMenuName   = NULL;
    wndclass.lpszClassName  = TEXT("EGLWndClass");
    return RegisterClass(&wndclass);
}

void GetRelativePath(const char *argv)
{
	for(int i = strlen(argv) - 1; i >= 0; i--)
	{
		if(argv[i] != '\\')
			continue;

		relativepath = string(argv).substr(0, i + 1);
		break;
	}
}

#ifndef _WIN32_WCE
int WINAPI WinMain( HINSTANCE,
                    HINSTANCE,
                    LPTSTR,
                    int)
#else
int main (int argc, const char **argv)
#endif
{
	context = GETRELAYINPUT;

	GetRelativePath(argv[0]);

    MyRegisterClass();

    HWND hwnd = CreateWindow(
        TEXT("EGLWndClass"),
        TEXT("Multi-touch test"),
        WS_OVERLAPPED|WS_SYSMENU|WS_DISABLED,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL,
        NULL,
        (HINSTANCE)GetModuleHandle(NULL),
        NULL);

    if (!hwnd)
        return -1;

    EnableWindow(hwnd, TRUE);

    RECT area;
    area.left = 30;
    area.top = 30;
    area.right = 212;
    area.bottom = 212;

    SetWindowPos(hwnd, HWND_TOPMOST, area.left, area.top, area.right, area.bottom, SWP_FRAMECHANGED);

    /* set as foreground window to give this app focus in case it doesn't have it */
    SetForegroundWindow(hwnd);
    ShowWindow(hwnd, SW_SHOWNORMAL);
    
    bool quit = false;

    // Main message loop:
    while (!quit)
    {
        /* relay message queue messages to windowproc's */
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);

            if (msg.message == WM_QUIT)
            {
                quit = true;
                break;
            }

            DispatchMessage(&msg);
        }
    }

	CloseHandle(context);

    return 0;
}
