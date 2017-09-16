/*--------------------------------------------------------------------------*/
#include "dd.h"
#include <ddraw.h>
#include <stdio.h>
/*--------------------------------------------------------------------------*/
LPDIRECTDRAW        g_pDD          = NULL;  // The DirectDraw object
LPDIRECTDRAWCLIPPER g_pClipper     = NULL;  // Clipper for primary surface
LPDIRECTDRAWSURFACE g_pDDS         = NULL;  // Primary surface
LPDIRECTDRAWSURFACE g_pDDSBack     = NULL;  // Back surface
HWND                g_hWnd         = NULL;  // To store the main windows handle
bool                g_bFullScreen  = false; // Full-screen mode?

int                 g_iBpp         = 0;     // Remember the main surface bit depth
/*--------------------------------------------------------------------------*/
bool DDFailedCheck(HRESULT hr, wchar_t *szMessage)
{
	if (FAILED(hr))
	{
		wchar_t buf[1024];
		wsprintf( buf, L"%s (%s)\n", szMessage, DDErrorString(hr) );
		OutputDebugString( buf );
		return true;
	}
	return false;
}
/*--------------------------------------------------------------------------*/
// Initialize DirectDraw stuff
bool DDInit( HWND hWnd )
{
	HRESULT hr;
	
	g_hWnd = hWnd;
	
	// TODO: Enumerate devices here, get latest interfaces etc.
	
	// Initialize DirectDraw
	hr = DirectDrawCreate( NULL, &g_pDD, NULL );
	if (DDFailedCheck(hr, L"DirectDrawCreate failed" ))
		return false;
	
	return true;
}
/*--------------------------------------------------------------------------*/
// Create surfaces
bool DDCreateSurfaces( bool bFullScreen)
{
	HRESULT hr; // Holds return values for DirectX function calls
	
	g_bFullScreen = bFullScreen;
	
	// If we want to be in full-screen mode
	if (g_bFullScreen)
	{
		// Set the "cooperative level" so we can use full-screen mode
		hr = g_pDD->SetCooperativeLevel(g_hWnd, DDSCL_FULLSCREEN);
		if (DDFailedCheck(hr, L"SetCooperativeLevel"))
			return false;
		
		// Set 640x480x256 full-screen mode
		hr = g_pDD->SetDisplayMode(640, 480, 8, 0, 0);
		if (DDFailedCheck(hr, L"SetDisplayMode" ))
			return false;
	}
	else
	{
		// Set DDSCL_NORMAL to use windowed mode
		hr = g_pDD->SetCooperativeLevel(g_hWnd, DDSCL_NORMAL);
		if (DDFailedCheck(hr, L"SetCooperativeLevel windowed" ))
			return false;
	}
	
	DDSURFACEDESC ddsd; // A structure to describe the surfaces we want
	// Clear all members of the structure to 0
	memset(&ddsd, 0, sizeof(ddsd));
	// The first parameter of the structure must contain the size of the structure
	ddsd.dwSize = sizeof(ddsd);
	
	if (g_bFullScreen)
	{
		// Screw the full-screen mode (for now) (FIXME)
	}
	else
	{
		
		//-- Create the primary surface
		
		// The dwFlags paramater tell DirectDraw which DDSURFACEDESC
		// fields will contain valid values
		ddsd.dwFlags = DDSD_CAPS;
		ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
		
		hr = g_pDD->CreateSurface(&ddsd, &g_pDDS, NULL);
		if (DDFailedCheck(hr, L"Create primary surface"))
			return false;
		
		//-- Create the back buffer
		
		ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
		// Make our off-screen surface 320x240
		ddsd.dwWidth = 320;
		ddsd.dwHeight = 240;
		ddsd.dwBackBufferCount = 1;
		// Create an offscreen surface

		ddsd.ddsCaps.dwCaps = 0x114;

		hr = g_pDD->CreateSurface(&ddsd, &g_pDDSBack, NULL);

		if (DDFailedCheck(hr, L"Create back surface"))
			return false;	
	}
	
	
	//-- Create a clipper for the primary surface in windowed mode
	if (!g_bFullScreen)
	{
		
		// Create the clipper using the DirectDraw object
		hr = g_pDD->CreateClipper(0, &g_pClipper, NULL);
		if (DDFailedCheck(hr, L"Create clipper"))
			return false;
		
		// Assign your window's HWND to the clipper
		hr = g_pClipper->SetHWnd(0, g_hWnd);
		if (DDFailedCheck(hr, L"Assign hWnd to clipper"))
			return false;
		
		// Attach the clipper to the primary surface
		hr = g_pDDS->SetClipper(g_pClipper);
		if (DDFailedCheck(hr, L"Set clipper"))
			return false;
	}
	
	
	//-- Lock back buffer to retrieve surface information
	if (g_pDDSBack)
	{
		hr= g_pDDSBack->Lock( NULL, &ddsd, DDLOCK_WAITNOTBUSY, NULL );
		if (DDFailedCheck(hr, L"Lock back buffer failed" ))
			return false;
		
		// Store bit depth of surface
		g_iBpp = ddsd.ddpfPixelFormat.dwRGBBitCount;
		
		// Unlock surface
		hr = g_pDDSBack->Unlock( NULL );
		if (DDFailedCheck(hr, L"Unlock back buffer failed" ))
			return false;
	}
	
	return true;
}
/*--------------------------------------------------------------------------*/
// Destroy surfaces
void DDDestroySurfaces()
{
}
/*--------------------------------------------------------------------------*/
// Clean up DirectDraw stuff
void DDDone()
{
	if (g_pDD != NULL)
	{
		g_pDD->Release();
		g_pDD = NULL;
	}
}
/*--------------------------------------------------------------------------*/
// PutPixel routine for a DirectDraw surface
void DDPutPixel( LPDIRECTDRAWSURFACE pDDS, int x, int y, int r, int g, int b )
{
	HRESULT hr;
	DDBLTFX ddbfx;
	RECT    rcDest;
	
	// Safety net
	if (pDDS == NULL)
		return;
	
	// Initialize the DDBLTFX structure with the pixel color
	ddbfx.dwSize = sizeof( ddbfx );
	ddbfx.dwFillColor = (DWORD)CreateRGB( r, g, b );
	
	// Prepare the destination rectangle as a 1x1 (1 pixel) rectangle
	SetRect( &rcDest, x, y, x+1, y+1 );
	
	// Blit 1x1 rectangle using solid color op
	hr = pDDS->Blt( &rcDest, NULL, NULL, DDBLT_WAITNOTBUSY | DDBLT_COLORFILL, &ddbfx );
	DDFailedCheck(hr, L"Blt failure");
}
/*--------------------------------------------------------------------------*/
// Create color from RGB triple
unsigned int CreateRGB( int r, int g, int b )
{
	switch (g_iBpp)
	{
	case 8:
		// Here you should do a palette lookup to find the closes match.
		// I'm not going to bother with that. Many modern games no
		// longer support 256-color modes, and neither should you :)
		return 0;
	case 16:
		// Break down r,g,b into 5-6-5 format.
		return ((r/8)<<11) | ((g/4)<<5) | (b/8);
	case 24:
	case 32:
		return (r<<16) | (g<<8) | (b);
	}
	return 0;
}
/*--------------------------------------------------------------------------*/
wchar_t *DDErrorString(HRESULT hr)
{
	switch (hr)
	{
	case DDERR_GENERIC:                      return L"DDERR_GENERIC";
	case DDERR_HEIGHTALIGN:                  return L"DDERR_HEIGHTALIGN";
	case DDERR_INCOMPATIBLEPRIMARY:          return L"DDERR_INCOMPATIBLEPRIMARY";
	case DDERR_INVALIDCAPS:                  return L"DDERR_INVALIDCAPS";
	case DDERR_INVALIDCLIPLIST:              return L"DDERR_INVALIDCLIPLIST";
	case DDERR_INVALIDMODE:                  return L"DDERR_INVALIDMODE";
	case DDERR_INVALIDOBJECT:                return L"DDERR_INVALIDOBJECT";
	case DDERR_INVALIDPARAMS:                return L"DDERR_INVALIDPARAMS";
	case DDERR_INVALIDPIXELFORMAT:           return L"DDERR_INVALIDPIXELFORMAT";
	case DDERR_INVALIDRECT:                  return L"DDERR_INVALIDRECT";
	case DDERR_LOCKEDSURFACES:               return L"DDERR_LOCKEDSURFACES";
	case DDERR_NOALPHAHW:                    return L"DDERR_NOALPHAHW";
	case DDERR_NOCLIPLIST:                   return L"DDERR_NOCLIPLIST";
	case DDERR_NOCOLORCONVHW:                return L"DDERR_NOCOLORCONVHW";
	case DDERR_NOCOOPERATIVELEVELSET:        return L"DDERR_NOCOOPERATIVELEVELSET";
	case DDERR_NOCOLORKEYHW:                 return L"DDERR_NOCOLORKEYHW";
	case DDERR_NOFLIPHW:                     return L"DDERR_NOFLIPHW";
	case DDERR_NOTFOUND:                     return L"DDERR_NOTFOUND";
	case DDERR_NOOVERLAYHW:                  return L"DDERR_NOOVERLAYHW";
	case DDERR_NORASTEROPHW:                 return L"DDERR_NORASTEROPHW";
	case DDERR_NOSTRETCHHW:                  return L"DDERR_NOSTRETCHHW";
	case DDERR_NOVSYNCHW:                    return L"DDERR_NOVSYNCHW";
	case DDERR_NOZOVERLAYHW:                 return L"DDERR_NOZOVERLAYHW";
	case DDERR_OUTOFCAPS:                    return L"DDERR_OUTOFCAPS";
	case DDERR_OUTOFMEMORY:                  return L"DDERR_OUTOFMEMORY";
	case DDERR_OUTOFVIDEOMEMORY:             return L"DDERR_OUTOFVIDEOMEMORY";
	case DDERR_PALETTEBUSY:                  return L"DDERR_PALETTEBUSY";
	case DDERR_COLORKEYNOTSET:               return L"DDERR_COLORKEYNOTSET";
	case DDERR_SURFACEBUSY:                  return L"DDERR_SURFACEBUSY";
	case DDERR_CANTLOCKSURFACE:              return L"DDERR_CANTLOCKSURFACE";
	case DDERR_SURFACELOST:                  return L"DDERR_SURFACELOST";
	case DDERR_SURFACENOTATTACHED:           return L"DDERR_SURFACENOTATTACHED";
	case DDERR_TOOBIGHEIGHT:                 return L"DDERR_TOOBIGHEIGHT";
	case DDERR_TOOBIGSIZE:                   return L"DDERR_TOOBIGSIZE";
	case DDERR_TOOBIGWIDTH:                  return L"DDERR_TOOBIGWIDTH";
	case DDERR_UNSUPPORTED:                  return L"DDERR_UNSUPPORTED";
	case DDERR_UNSUPPORTEDFORMAT:            return L"DDERR_UNSUPPORTEDFORMAT";
	case DDERR_VERTICALBLANKINPROGRESS:      return L"DDERR_VERTICALBLANKINPROGRESS";
	case DDERR_WASSTILLDRAWING:              return L"DDERR_WASSTILLDRAWING";
	case DDERR_INVALIDDIRECTDRAWGUID:        return L"DDERR_INVALIDDIRECTDRAWGUID";
	case DDERR_DIRECTDRAWALREADYCREATED:     return L"DDERR_DIRECTDRAWALREADYCREATED";
	case DDERR_PRIMARYSURFACEALREADYEXISTS:  return L"DDERR_PRIMARYSURFACEALREADYEXISTS";
	case DDERR_REGIONTOOSMALL:               return L"DDERR_REGIONTOOSMALL";
	case DDERR_CLIPPERISUSINGHWND:           return L"DDERR_CLIPPERISUSINGHWND";
	case DDERR_NOCLIPPERATTACHED:            return L"DDERR_NOCLIPPERATTACHED";
	case DDERR_NOPALETTEATTACHED:            return L"DDERR_NOPALETTEATTACHED";
	case DDERR_NOPALETTEHW:                  return L"DDERR_NOPALETTEHW";
	case DDERR_NOBLTHW:                      return L"DDERR_NOBLTHW";
	case DDERR_OVERLAYNOTVISIBLE:            return L"DDERR_OVERLAYNOTVISIBLE";
	case DDERR_NOOVERLAYDEST:                return L"DDERR_NOOVERLAYDEST";
	case DDERR_INVALIDPOSITION:              return L"DDERR_INVALIDPOSITION";
	case DDERR_NOTAOVERLAYSURFACE:           return L"DDERR_NOTAOVERLAYSURFACE";
	case DDERR_EXCLUSIVEMODEALREADYSET:      return L"DDERR_EXCLUSIVEMODEALREADYSET";
	case DDERR_NOTFLIPPABLE:                 return L"DDERR_NOTFLIPPABLE";
	case DDERR_NOTLOCKED:                    return L"DDERR_NOTLOCKED";
	case DDERR_CANTCREATEDC:                 return L"DDERR_CANTCREATEDC";
	case DDERR_NODC:                         return L"DDERR_NODC";
	case DDERR_WRONGMODE:                    return L"DDERR_WRONGMODE";
	case DDERR_IMPLICITLYCREATED:            return L"DDERR_IMPLICITLYCREATED";
	case DDERR_NOTPALETTIZED:                return L"DDERR_NOTPALETTIZED";
	case DDERR_DCALREADYCREATED:             return L"DDERR_DCALREADYCREATED";
	}
	return L"Unknown Error";
}
/*--------------------------------------------------------------------------*/
// Checks if the memory associated with surfaces is lost and restores if necessary.
void CheckSurfaces()
{
	// Check the primary surface
	if (g_pDDS)
	{
		if (g_pDDS->IsLost() == DDERR_SURFACELOST)
			g_pDDS->Restore();
	}
	// Check the back buffer
	if (g_pDDSBack)
	{
		if (g_pDDSBack->IsLost() == DDERR_SURFACELOST)
			g_pDDSBack->Restore();
	}
}
/*--------------------------------------------------------------------------*/
// Double buffering flip
void DDFlip()
{
	HRESULT hr;
	
	// if we're windowed do the blit, else just Flip
	if (!g_bFullScreen)
	{
		RECT    rcSrc;  // source blit rectangle
		RECT    rcDest; // destination blit rectangle
		POINT   p;
		
		// find out where on the primary surface our window lives
		p.x = 0; p.y = 0;
		::ClientToScreen(g_hWnd, &p);
		::GetClientRect(g_hWnd, &rcDest);
		OffsetRect(&rcDest, p.x, p.y);
		SetRect(&rcSrc, 0, 0, 320, 240);
		hr = g_pDDS->Blt(&rcDest, g_pDDSBack, &rcSrc, DDBLT_WAITVSYNC, NULL);
	}
	else
	{
		hr = g_pDDS->Flip(NULL, DDFLIP_WAITVSYNC);
	}
}
/*--------------------------------------------------------------------------*/
// Clear a surface area with black
void DDClear( LPDIRECTDRAWSURFACE pDDS, int x1, int y1, int x2, int y2 )
{
	HRESULT hr;
	DDBLTFX ddbfx;
	RECT    rcDest;
	
	// Safety net
	if (pDDS == NULL)
		return;
	
	// Initialize the DDBLTFX structure with the pixel color
	ddbfx.dwSize = sizeof( ddbfx );
	ddbfx.dwFillColor = (DWORD)CreateRGB( 0, 0, 0 );
	
	SetRect( &rcDest, x1, y1, x2, y2 );
	
	// Blit 1x1 rectangle using solid color op
	hr = pDDS->Blt( &rcDest, NULL, NULL, DDBLT_WAITNOTBUSY | DDBLT_COLORFILL, &ddbfx );
	DDFailedCheck(hr, L"Blt failure");
}
/*--------------------------------------------------------------------------*/
