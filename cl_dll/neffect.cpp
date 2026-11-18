// ======================================
// ======================================

#include "hud.h"
#include "cl_util.h"

extern cvar_t *cl_noiseeffect;
extern cvar_t *cl_noiseamount;

int CNeffect::Init( void )
{
	m_iFlags |= HUD_ACTIVE;
	gHUD.AddHudElem( this );

	return 1;
}

int CNeffect::VidInit( void )
{
	m_HUD_snoise = gHUD.GetSpriteIndex( "snoise" );
	m_HUD_arghnoise = gHUD.GetSpriteIndex( "arghnoise" );
	m_HUD_tv_scanlines = gHUD.GetSpriteIndex( "tv_scanlines" );

	return 1;
}

int CNeffect::Draw( float flTime )
{
	int x, y;
	int iWidth, iHeight;
	int a;

	if ( !cl_noiseeffect->value )
		return 1;

	iWidth = gHUD.GetSpriteRect( m_HUD_snoise ).right - gHUD.GetSpriteRect( m_HUD_snoise ).left;
	iHeight = gHUD.GetSpriteRect( m_HUD_snoise ).bottom - gHUD.GetSpriteRect( m_HUD_snoise ).top;

	for ( y = 0; y < ScreenHeight; y += iWidth )
	{
		for ( x = 0; x < ScreenWidth; x += iHeight )
		{
			a = cl_noiseamount->value * 255;
			SPR_Set( gHUD.GetSprite( m_HUD_snoise ), a, a, a );
			SPR_DrawAdditive( gEngfuncs.pfnRandomLong( 1, 19 ), x, y, &gHUD.GetSpriteRect( m_HUD_snoise ) );
		}
	}

	return 1;
}
