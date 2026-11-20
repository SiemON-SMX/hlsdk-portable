#include "extdll.h"
#include "util.h"
#include "keys.h"

static BOOL g_bCoFKeyHeld[256];
static float g_flCoFKeyTapEnd[256];

int KeyState( int vk )
{
	if( vk < 0 || vk > 255 )
		return 0;

	float flLeft = g_flCoFKeyTapEnd[vk] - gpGlobals->time;

	if( g_bCoFKeyHeld[vk] || ( flLeft > 0 && flLeft <= 0.1f ))
		return 0x8000;

	return 0;
}

void KeyCommand( int vk, int down )
{
	if( vk < 0 || vk > 255 )
		return;

	g_bCoFKeyHeld[vk] = down ? TRUE : FALSE;
}

void KeyTap( int vk )
{
	if( vk < 0 || vk > 255 )
		return;

	g_flCoFKeyTapEnd[vk] = gpGlobals->time + 0.1f;
}
