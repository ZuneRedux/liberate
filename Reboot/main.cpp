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
#include <pm.h>

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
		if (SUCCEEDED(hr))
			hr = HRESULT_FROM_WIN32(RegFlushKey(key));
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

int main(int argc, WCHAR *argv[])
{
	SetReboot(true);

	if(MessageBox(NULL, L"Are you sure you want to reboot?", L"Reboot?", MB_YESNO) == IDYES)
		SetSystemPowerState(NULL, POWER_STATE_RESET, POWER_FORCE);
	else
		SetReboot(false);

	return 0;
}
