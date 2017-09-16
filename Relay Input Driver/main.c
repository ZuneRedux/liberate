#include <windows.h>
#include <zdkinput.h>
 
DWORD INP_Close(DWORD dwData)
{
	return 1;
}

DWORD INP_Deinit(DWORD dwData)
{
	return 1;
}

DWORD INP_Init(DWORD dwData)
{
	return 1;
}

DWORD INP_Open(DWORD dwData, DWORD dwAccess, DWORD dwShareMode)
{
	return 1;
}

BOOL INP_IOControl(DWORD hOpenContext, DWORD dwCode, PBYTE pBufIn, DWORD dwLenIn, PBYTE pBufOut, DWORD dwLenOut, PDWORD pdwActualOut)
{
	BOOL RetVal = TRUE;

	switch(dwCode)
	{
		case 0:
			{
				HRESULT hr = 0;

				if(dwLenOut != sizeof(ZDKINPUT_STATE))
				{
					SetLastError(ERROR_INSUFFICIENT_BUFFER);
					RetVal = FALSE;
				}

				hr = ZDKInput_GetState((ZDKINPUT_STATE*)pBufOut);

				if(hr)
				{
					SetLastError(ERROR_GEN_FAILURE);
					RetVal = FALSE;
				}

				if(pdwActualOut)
					*pdwActualOut = sizeof(ZDKINPUT_STATE);
			}
			break;

		default:
			RETAILMSG(1,(TEXT("INP_IoControl default\n")));
			SetLastError(ERROR_INVALID_PARAMETER);
			RetVal = FALSE;
			break;
	}

    return RetVal;
}