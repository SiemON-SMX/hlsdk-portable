#include "hud.h"
#include "cl_util.h"
#include "keydefs.h"
#include "client_port.h"
#include "msgbox.h"

static int g_bCoFClientKeyDown[256];
static float g_flCoFClientKeyTapEnd[256];

static int KeynumToVK( int keynum )
{
	switch( keynum )
	{
	case K_TAB:			return VKEY_TAB;
	case K_ENTER:		return VKEY_RETURN;
	case K_KP_ENTER:	return VKEY_RETURN;
	case K_ESCAPE:		return VKEY_ESCAPE;
	case K_KP_INS:		return VKEY_NUMPAD0 + 0;
	case K_KP_END:		return VKEY_NUMPAD0 + 1;
	case K_KP_DOWNARROW:	return VKEY_NUMPAD0 + 2;
	case K_KP_PGDN:		return VKEY_NUMPAD0 + 3;
	case K_KP_LEFTARROW:	return VKEY_NUMPAD0 + 4;
	case K_KP_5:		return VKEY_NUMPAD0 + 5;
	case K_KP_RIGHTARROW:	return VKEY_NUMPAD0 + 6;
	case K_KP_HOME:		return VKEY_NUMPAD0 + 7;
	case K_KP_UPARROW:	return VKEY_NUMPAD0 + 8;
	case K_KP_PGUP:		return VKEY_NUMPAD0 + 9;
	}

	if( keynum >= '0' && keynum <= '9' )
		return keynum;

	if( keynum >= 'a' && keynum <= 'z' )
		return keynum - 'a' + 'A';

	return -1;
}

static int IsServerKey( int vk )
{
	if( vk == VKEY_RETURN || vk == VKEY_ESCAPE || vk == VKEY_TAB )
		return 1;

	if( vk >= '0' && vk <= '9' )
		return 1;

	if( vk >= VKEY_NUMPAD0 && vk <= VKEY_NUMPAD9 )
		return 1;

	return 0;
}

int ClientKeyState( int vk )
{
	if( vk < 0 || vk > 255 )
		return 0;

	if( g_bCoFClientKeyDown[vk] || g_flCoFClientKeyTapEnd[vk] > gEngfuncs.GetClientTime() )
		return 0x8000;

	return 0;
}

void ClientKeyEvent( int down, int keynum )
{
	char szCmd[64];
	int vk = KeynumToVK( keynum );

	if( vk < 0 || vk > 255 )
		return;

	down = down ? 1 : 0;

	if( g_bCoFClientKeyDown[vk] == down )
		return;

	g_bCoFClientKeyDown[vk] = down;

	if( IsServerKey( vk ) )
	{
		sprintf( szCmd, "cof_key %d %d", vk, down );
		gEngfuncs.pfnServerCmd( szCmd );
	}
}

static void __CmdFunc_CoFTap( void )
{
	char szCmd[64];
	int vk;

	if( gEngfuncs.Cmd_Argc() < 2 )
	{
		gEngfuncs.Con_Printf( "usage: cof_tap <virtual key code>\n" );
		return;
	}

	vk = atoi( gEngfuncs.Cmd_Argv( 1 ) );

	if( vk < 0 || vk > 255 )
		return;

	g_flCoFClientKeyTapEnd[vk] = gEngfuncs.GetClientTime() + 0.1f;

	if( IsServerKey( vk ) )
	{
		sprintf( szCmd, "cof_keytap %d", vk );
		gEngfuncs.pfnServerCmd( szCmd );
	}
}

void InitClientKeys( void )
{
	gEngfuncs.pfnAddCommand( "cof_tap", __CmdFunc_CoFTap );

	if( gEngfuncs.pfnGetCvarPointer( "sv_savepoints_only" ))
		gEngfuncs.Cvar_SetValue( "sv_savepoints_only", 1 );
}

#if !_WIN32
int ShowMessageBox( void *hWnd, const char *pszText, const char *pszCaption, unsigned int uType )
{
	gEngfuncs.Con_Printf( "%s: %s\n", pszCaption ? pszCaption : "Cry of Fear", pszText ? pszText : "" );

	if( uType & MB_YESNO )
		return IDNO;

	return IDOK;
}
#endif
