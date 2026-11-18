// ======================================
// ======================================

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "vgui_TeamFortressViewport.h"
#include "padlock.h"
#include "../game_shared/vgui_loadtga.h"

CPadlock::CPadlock() : Panel( 0, 0, ScreenWidth, ScreenHeight )
{
	int wide, tall;
	int x, y;
	CommandButton *pButton;

	m_pButtonsImage = vgui_LoadTGA( "gfx/vgui/puzzle/padlock_buttons.tga" );
	if ( m_pButtonsImage )
		m_pButtonsImage->setColor( Color( 255, 255, 255, 1 ) );
	else
		gEngfuncs.Con_Printf( "Cannot load gfx/vgui/puzzle/padlock_buttons.tga!\n" );

	m_pButtonsImage->getSize( wide, tall );
	x = ScreenWidth / 2 - wide / 2;
	y = ScreenHeight - tall;

	m_pButtons = new ImagePanel( m_pButtonsImage );
	m_pButtons->setParent( this );
	m_pButtons->setPos( x, y );
	m_pButtons->setVisible( true );

	pButton = new CommandButton( "", 18, 20, 66, 66, true, true );
	pButton->setParent( m_pButtons );
	pButton->addActionSignal( new CPadlockHandler_Command( this, 1 ) );

	pButton = new CommandButton( "", 84, 20, 66, 66, true, true );
	pButton->setParent( m_pButtons );
	pButton->addActionSignal( new CPadlockHandler_Command( this, 2 ) );

	pButton = new CommandButton( "", 150, 20, 66, 66, true, true );
	pButton->setParent( m_pButtons );
	pButton->addActionSignal( new CPadlockHandler_Command( this, 3 ) );

	pButton = new CommandButton( "", 216, 20, 66, 66, true, true );
	pButton->setParent( m_pButtons );
	pButton->addActionSignal( new CPadlockHandler_Command( this, 4 ) );

	pButton = new CommandButton( "", 117, 86, 66, 14, true, true );
	pButton->setParent( m_pButtons );
	pButton->addActionSignal( new CPadlockHandler_Command( this, 5 ) );

	setBgColor( 0, 0, 0, 255 );

	Initialize();
}

void CPadlock::ActionSignal( int cmd )
{
	switch ( cmd )
	{
	case 1:
		gEngfuncs.pfnServerCmd( "padlockrotate 1" );
		break;
	case 2:
		gEngfuncs.pfnServerCmd( "padlockrotate 2" );
		break;
	case 3:
		gEngfuncs.pfnServerCmd( "padlockrotate 3" );
		break;
	case 4:
		gEngfuncs.pfnServerCmd( "padlockrotate 4" );
		break;
	case 5:
		gEngfuncs.pfnServerCmd( "padlockcancel" );
		setVisible( false );
		gViewPort->UpdateCursorState();
		break;
	}
}

void CPadlock::Initialize()
{
	setVisible( false );
}

int MsgPadlock( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	int iOn = READ_BYTE();

	if ( gViewPort->m_pPadlock )
	{
		if ( iOn )
			gViewPort->m_pPadlock->setVisible( true );
		else
			gViewPort->m_pPadlock->setVisible( false );

		gViewPort->UpdateCursorState();
	}

	return 1;
}

void PadlockInit()
{
	gEngfuncs.pfnHookUserMsg( "Padlock", MsgPadlock );
}

CPadlock::~CPadlock( void )
{
	delete m_pButtonsImage;
}
