// ======================================
// ======================================

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "vgui_TeamFortressViewport.h"
#include "scope.h"
#include "../game_shared/vgui_loadtga.h"

CScope::CScope() : Panel( 0, 0, ScreenWidth, ScreenHeight )
{
	int wide, tall;
	int x, y;
	Panel *pPanel;

	m_pScopeImage = vgui_LoadTGA( "gfx/vgui/telescope_scope.tga" );
	if ( m_pScopeImage )
		m_pScopeImage->setColor( Color( 255, 255, 255, 1 ) );
	else
		gEngfuncs.Con_Printf( "Cannot load gfx/vgui/telescope_scope.tga!\n" );

	m_pScopeImage->getSize( wide, tall );
	x = ScreenWidth / 2 - wide / 2;
	y = ScreenHeight / 2 - tall / 2;

	pPanel = new Panel( 0, 0, 0, 0 );
	pPanel->setParent( this );
	pPanel->setSize( ScreenWidth / 2 - wide / 2, ScreenHeight );
	pPanel->setBgColor( 0, 0, 0, 0 );
	pPanel->setVisible( true );

	pPanel = new Panel( 0, 0, 0, 0 );
	pPanel->setParent( this );
	pPanel->setSize( ScreenWidth / 2 - wide / 2, ScreenHeight );
	pPanel->setPos( ScreenWidth / 2 + wide / 2, 0 );
	pPanel->setBgColor( 0, 0, 0, 0 );
	pPanel->setVisible( true );

	pPanel = new Panel( 0, 0, 0, 0 );
	pPanel->setParent( this );
	pPanel->setSize( wide, ScreenHeight / 2 - tall / 2 );
	pPanel->setPos( ScreenWidth / 2 - wide / 2, 0 );
	pPanel->setBgColor( 0, 0, 0, 0 );
	pPanel->setVisible( true );

	pPanel = new Panel( 0, 0, 0, 0 );
	pPanel->setParent( this );
	pPanel->setSize( wide, ScreenHeight / 2 - tall / 2 );
	pPanel->setPos( ScreenWidth / 2 - wide / 2, ScreenHeight / 2 + tall / 2 );
	pPanel->setBgColor( 0, 0, 0, 0 );
	pPanel->setVisible( true );

	m_pScopePanel = new ImagePanel( m_pScopeImage );
	m_pScopePanel->setParent( this );
	m_pScopePanel->setPos( x, y );
	m_pScopePanel->setVisible( true );

	setBgColor( 0, 0, 0, 255 );

	Initialize();
}

void CScope::Initialize()
{
	m_iScope = 0;
	setVisible( false );
}

int MsgScope( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	if ( gViewPort->m_pScope )
	{
		int iScope = READ_BYTE();
		gViewPort->m_pScope->m_iScope = iScope;

		if ( iScope )
		{
			switch ( iScope )
			{
			case 1:
				gViewPort->m_pScope->m_pScopePanel->setImage( vgui_LoadTGA( "gfx/vgui/telescope_scope.tga" ) );
				break;
			case 2:
				gViewPort->m_pScope->m_pScopePanel->setImage( vgui_LoadTGA( "gfx/vgui/telescope_scope.tga" ) );
				break;
			default:
				gViewPort->m_pScope->m_pScopePanel->setImage( vgui_LoadTGA( "gfx/vgui/telescope_scope.tga" ) );
				break;
			}

			if ( !gViewPort->m_pScope->isVisible() )
				gViewPort->m_pScope->setVisible( true );
		}
		else
		{
			if ( gViewPort->m_pScope->isVisible() )
				gViewPort->m_pScope->setVisible( false );
		}
	}

	return 1;
}

void ScopeInit()
{
	gEngfuncs.pfnHookUserMsg( "Scope", MsgScope );
}

CScope::~CScope( void )
{
	delete m_pScopeImage;
}
