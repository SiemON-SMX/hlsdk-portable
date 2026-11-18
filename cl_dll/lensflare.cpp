// ======================================
// ======================================

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include <time.h>

DECLARE_MESSAGE( m_Lensflare, Lensflare )

int CHudLensflare::Init( void )
{
	HOOK_MESSAGE( Lensflare );

	m_iFlags |= HUD_ACTIVE;
	gHUD.AddHudElem( this );

	srand( time( NULL ) );

	return 1;
}

int CHudLensflare::VidInit( void )
{
	return 1;
}

int CHudLensflare::Draw( float flTime )
{
	return 0;
}

int CHudLensflare::MsgFunc_Lensflare( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	m_vecSunPos.x = READ_COORD();
	m_vecSunPos.y = READ_COORD();
	m_vecSunPos.z = READ_COORD();

	m_iFlags |= HUD_ACTIVE;

	return 1;
}
