// ======================================
// ======================================

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "vgui_TeamFortressViewport.h"
#include "hudcontrol.h"
#include "credits.h"

extern int g_iLetterboxHeight;

DECLARE_MESSAGE( m_BlackBar, BlackBars )

int CBlackBar::Init( void )
{
	HOOK_MESSAGE( BlackBars );

	m_iFlags |= HUD_ACTIVE;
	gHUD.AddHudElem( this );

	return 1;
}

int CBlackBar::VidInit( void )
{
	m_HUD_blackbar = gHUD.GetSpriteIndex( "cof_blackbar" );
	int iHeight = gHUD.GetSpriteRect( m_HUD_blackbar ).bottom - gHUD.GetSpriteRect( m_HUD_blackbar ).top;
	m_iBottom = ScreenHeight;
	m_iTop = -iHeight - 1;

	return 1;
}

int CBlackBar::Draw( float flTime )
{
	return 1;
}

int CBlackBar::MsgFunc_BlackBars( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	m_iOn = READ_BYTE();

	if ( m_iOn )
	{
		int iHeight = gHUD.GetSpriteRect( m_HUD_blackbar ).bottom - gHUD.GetSpriteRect( m_HUD_blackbar ).top;
		m_iBottom = ScreenHeight;
		m_iTop = -iHeight - 1;

		if ( gViewPort )
		{
			if ( gViewPort->m_pHUDControl )
			{
				int x, y, x2, y2;

				gViewPort->m_pHUDControl->m_pLetterboxTop->getPos( x, y );
				gViewPort->m_pHUDControl->m_pLetterboxBottom->getPos( x2, y2 );
				gViewPort->m_pHUDControl->m_pLetterboxTop->setPos( x, -g_iLetterboxHeight );
				gViewPort->m_pHUDControl->m_pLetterboxBottom->setPos( x2, ScreenHeight );
			}

			if ( gViewPort && gViewPort->m_pCredits && gViewPort->m_pCredits->isVisible() )
			{
				int x, y, x2, y2;

				gViewPort->m_pCredits->m_pLetterboxTop->getPos( x, y );
				gViewPort->m_pCredits->m_pLetterboxBottom->getPos( x2, y2 );
				gViewPort->m_pCredits->m_pLetterboxTop->setPos( x, -g_iLetterboxHeight );
				gViewPort->m_pCredits->m_pLetterboxBottom->setPos( x2, ScreenHeight );
			}
		}
	}

	return 1;
}
