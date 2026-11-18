// ======================================
// ======================================

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "vgui_TeamFortressViewport.h"
#include "puzzlebar.h"
#include "../game_shared/vgui_loadtga.h"

CPuzzleBar::CPuzzleBar() : Panel( 0, 0, ScreenWidth, ScreenHeight )
{
	int wide, tall;
	int x, y;
	CommandButton *pButton;

	m_pImage = vgui_LoadTGA( "gfx/vgui/640_boilerpuzzle.tga" );
	if ( m_pImage )
		m_pImage->setColor( Color( 255, 255, 255, 1 ) );
	else
		gEngfuncs.Con_Printf( "Cannot load gfx/vgui/640_boilerpuzzle.tga!\n" );

	m_pImage->getSize( wide, tall );
	x = ScreenWidth / 2 - wide / 2;
	y = ScreenHeight - tall;

	m_pImagePanel = new ImagePanel( m_pImage );
	m_pImagePanel->setParent( this );
	m_pImagePanel->setPos( x, y );
	m_pImagePanel->setVisible( true );

	pButton = new CommandButton( "", 0, 0, 20, 20 );
	pButton->setParent( m_pImagePanel );
	pButton->addActionSignal( new CPuzzleBarHandler_Command( this, 5 ) );

	pButton = new CommandButton( "", 16, 74, 40, 40, true, true );
	pButton->setParent( m_pImagePanel );
	pButton->addActionSignal( new CPuzzleBarHandler_Command( this, 1 ) );

	pButton = new CommandButton( "", 73, 74, 40, 40, true, true );
	pButton->setParent( m_pImagePanel );
	pButton->addActionSignal( new CPuzzleBarHandler_Command( this, 2 ) );

	pButton = new CommandButton( "", 128, 74, 40, 40, true, true );
	pButton->setParent( m_pImagePanel );
	pButton->addActionSignal( new CPuzzleBarHandler_Command( this, 3 ) );

	pButton = new CommandButton( "", 184, 74, 40, 40, true, true );
	pButton->setParent( m_pImagePanel );
	pButton->addActionSignal( new CPuzzleBarHandler_Command( this, 4 ) );

	setBgColor( 0, 0, 0, 255 );

	Initialize();
}

void CPuzzleBar::ActionSignal( int cmd )
{
	switch ( cmd )
	{
	case 1:
		gEngfuncs.pfnServerCmd( "boilercheck 1" );
		break;
	case 2:
		gEngfuncs.pfnServerCmd( "boilercheck 2" );
		break;
	case 3:
		gEngfuncs.pfnServerCmd( "boilercheck 3" );
		break;
	case 4:
		gEngfuncs.pfnServerCmd( "boilercheck 4" );
		break;
	case 5:
		setVisible( false );
		gViewPort->UpdateCursorState();
		gEngfuncs.pfnServerCmd( "boilercheck 5" );
		break;
	}
}

void CPuzzleBar::Initialize()
{
	setVisible( false );
}

int CPuzzleBar::KeyInput( int down, int keynum, const char *pszCurrentBinding )
{
	if ( pszCurrentBinding && ( !strcmp( pszCurrentBinding, "toggleconsole" ) || !strcmp( pszCurrentBinding, "boilercheck" ) ) )
		return 1;

	return 0;
}

int MsgPuzzlebar( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	int iOn = READ_BYTE();

	if ( gViewPort->m_pPuzzleBar )
	{
		if ( iOn )
			gViewPort->m_pPuzzleBar->setVisible( true );
		else
			gViewPort->m_pPuzzleBar->setVisible( false );

		gViewPort->UpdateCursorState();
	}

	return 1;
}

void PuzzlebarInit()
{
	gEngfuncs.pfnHookUserMsg( "Puzzlebar", MsgPuzzlebar );
}

void CPuzzleBar::setVisible( bool state )
{
	Panel::setVisible( state );
}
