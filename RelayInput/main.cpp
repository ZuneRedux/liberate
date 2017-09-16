/*  _____                         ________   ____    __  __
 * /\  __`\                      /\_____  \ /\  _`\ /\ \/\ \
 * \ \ \/\ \  _____     __    ___\/____//'/'\ \ \/\ \ \ \/'/'
 *  \ \ \ \ \/\ '__`\ /'__`\/' _ `\   //'/'  \ \ \ \ \ \ , <
 *   \ \ \_\ \ \ \L\ \\  __//\ \/\ \ //'/'___ \ \ \_\ \ \ \\`\
 *    \ \_____\ \ ,__/ \____\ \_\ \_\/\_______\\ \____/\ \_\ \_\
 *     \/_____/\ \ \/ \/____/\/_/\/_/\/_______/ \/___/  \/_/\/_/
 *              \ \_\
 *               \/_/ OpenZDK Release 1 | 2010-04-14
 *
 * main.cpp
 * Copyright (c) 2010 itsnotabigtruck.
 * No rights reserved.
 * 
 * All rights are waived to the maximum extent possible; see
 * http://creativecommons.org/publicdomain/zero/1.0/ for more information
 *
 * Replace this banner when writing your own applications
 */

#include <windows.h>
#include <zdk.h>
#include "ping.h"

#define ZWIDTH 272
#define ZHEIGHT 480
#define TOLERANCE 350

#define ROT_0 0
#define ROT_90 1
#define ROT_180 2
#define ROT_270 4

int rotation = ROT_0;

DWORD KeepWiFiAlive(PVOID unused)
{
	while(true)
	{
		for(int i = 0; i < 80; i++)
		{
			ZDKSystem_SignalUserActivityEx(TRUE);
			Sleep(4000);
		}

		doping("yahoo.com");
		doping("google.com");
	}
}

void MouseInput(DWORD event, float iInputX, float iInputY)
{
	switch(rotation)
	{
		case ROT_0:
			break;

		case ROT_90:
			{
				float tempX = iInputX;
				iInputX = ZHEIGHT - iInputY;
				iInputY = tempX;
			}
			break;
			
		case ROT_180:
			iInputX = ZWIDTH - iInputX;
			iInputY = ZHEIGHT - iInputY;
			break;
			
		case ROT_270:
			{
				float tempX = iInputX;
				iInputX = iInputY;
				iInputY = ZWIDTH - tempX;
			}
			break;
	}

	int iScaleX = 65535 / (GetSystemMetrics(SM_CXSCREEN) - 1);
	int iScaleY = 65535 / (GetSystemMetrics(SM_CYSCREEN) - 1);

	mouse_event(MOUSEEVENTF_ABSOLUTE | event, (int)iInputX * iScaleX, (int)iInputY * iScaleY, 0, 0);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd)
{
	if(!wcscmp(lpCmdLine, L"Init"))
		while(true)
		{
			Sleep(1000);

			ZDKSystem_SignalUserActivityEx(true);

			WIN32_FIND_DATA finddata;

			HANDLE file = FindFirstFile(L"\\gamert", &finddata);

			if(file == INVALID_HANDLE_VALUE)
			{
				PROCESS_INFORMATION pinfo;
				CreateProcess(L"\\flash2\\Liberate\\Liberate.exe", L"Start", NULL, NULL, NULL, NULL, NULL, NULL, NULL, &pinfo);
				CreateProcess(L"\\flash2\\Liberate\\Keyboard.exe", L"", NULL, NULL, NULL, NULL, NULL, NULL, NULL, &pinfo);
				break;
			}

			FindClose(file);
		}

	Sleep(1000);

	ZDKINPUT_STATE_HD state, laststate;
	int lastrotation = ROT_0;
	bool rightclickenabled = true, nextrightclick = false, rightpressed = false, rightchanged = false;

	ZDKInput_GetState(&laststate);

	BOOL connected = FALSE;

	ZDKCloud_IsConnected(&connected);

	if(connected)
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)KeepWiFiAlive, NULL, NULL, NULL);

	while(true)
	{
		ZDKInput_GetState(&state);

		if(abs(state.AccelerometerState.Y) < TOLERANCE)
		{
			if(state.AccelerometerState.X < -TOLERANCE && rotation != ROT_270)
				rotation = ROT_270;

			if(state.AccelerometerState.X > TOLERANCE && rotation != ROT_90)
				rotation = ROT_90;
		}

		if(abs(state.AccelerometerState.X) < TOLERANCE)
		{
			if(state.AccelerometerState.Y > TOLERANCE && rotation != ROT_0)
				rotation = ROT_0;

			if(state.AccelerometerState.Y < -TOLERANCE && rotation != ROT_180)
				rotation = ROT_180;
		}

		if(rotation != lastrotation)
		{
			DEVMODE mode;
			mode.dmSize = sizeof(DEVMODE);
			mode.dmDisplayOrientation = rotation;
			mode.dmFields = DM_DISPLAYORIENTATION;
			ChangeDisplaySettingsEx(NULL, &mode, NULL, NULL, NULL);
			lastrotation = rotation;
		}

		if(!state.TouchState.Count)
		{
			if(laststate.TouchState.Count)
			{
				MouseInput(rightpressed ? MOUSEEVENTF_RIGHTUP : MOUSEEVENTF_LEFTUP, laststate.TouchState.Locations[0].X, laststate.TouchState.Locations[0].Y);

				if(rightchanged)
					rightchanged = false;
				else
					if(rightpressed)
						nextrightclick = rightpressed = false;
			}
		}
		else
			if(state.TouchState.Count == 1)
				if(!laststate.TouchState.Count)
				{
					MouseInput(nextrightclick ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_LEFTDOWN, state.TouchState.Locations[0].X, state.TouchState.Locations[0].Y);

					if(nextrightclick)
						rightpressed = true;
				}
				else
					MouseInput(MOUSEEVENTF_MOVE, state.TouchState.Locations[0].X, state.TouchState.Locations[0].Y);
			else
				if(state.TouchState.Count == 2 && laststate.TouchState.Count < 2 && !rightchanged && rightclickenabled)
					nextrightclick = true;
				else
					if(state.TouchState.Count == 3 && laststate.TouchState.Count < 3)
					{
						rightclickenabled = !rightclickenabled;

						if(rightclickenabled)
							rightchanged = true;
					}
#ifdef DEBUG
					else
						if(state.TouchState.Count == 4)
						{
							PROCESS_INFORMATION pinfo;
							CreateProcess(L"\\Windows\\compositor.exe", L"", NULL, NULL, NULL, NULL, NULL, NULL, NULL, &pinfo);
							Sleep(1000);
							CreateProcess(L"\\Windows\\gemstone.exe", L"", NULL, NULL, NULL, NULL, NULL, NULL, NULL, &pinfo);
						}
#endif

		Sleep(50);

		laststate = state;
	}
}
