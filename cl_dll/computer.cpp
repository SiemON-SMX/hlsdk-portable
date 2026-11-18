// ======================================
// ======================================

#include <VGUI_TextEntry.h>

#include "hud.h"
#include "client_port.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "event_api.h"
#include "vgui_TeamFortressViewport.h"
#include "computer.h"
#include "../game_shared/vgui_loadtga.h"
#include "../engine/mobility_int.h"

extern mobile_engfuncs_t *gMobileEngfuncs;

static void ComputerEnableTextInput( bool enable )
{
	if( gMobileEngfuncs && gMobileEngfuncs->pfnEnableTextInput )
		gMobileEngfuncs->pfnEnableTextInput( enable ? 1 : 0 );
}

class CComputerFieldSignal : public InputSignal
{
public:
	CComputerFieldSignal( CComputer *panel, TextEntry *field ) : m_pPanel( panel ), m_pField( field ) {}

	virtual void cursorMoved( int, int, Panel * ) {}
	virtual void cursorEntered( Panel * ) {}
	virtual void cursorExited( Panel * ) {}
	virtual void mousePressed( MouseCode code, Panel * )
	{
		if( code != MOUSE_LEFT || m_pPanel->m_iLoggedOn )
			return;

		m_pField->requestFocus();
		ComputerEnableTextInput( true );
	}
	virtual void mouseDoublePressed( MouseCode, Panel * ) {}
	virtual void mouseReleased( MouseCode code, Panel * )
	{
		if( code != MOUSE_LEFT || m_pPanel->m_iLoggedOn )
			return;

		ComputerEnableTextInput( true );
	}
	virtual void mouseWheeled( int, Panel * ) {}
	virtual void keyPressed( KeyCode, Panel * ) {}
	virtual void keyTyped( KeyCode code, Panel * )
	{
		if( code == KEY_ENTER )
			m_pPanel->ActionSignal( 1 );
	}
	virtual void keyReleased( KeyCode, Panel * ) {}
	virtual void keyFocusTicked( Panel * ) {}

private:
	CComputer *m_pPanel;
	TextEntry *m_pField;
};

CComputer::CComputer() : Panel( 0, 0, ScreenWidth, ScreenHeight )
{
	int wide, tall;
	int x, y;
	int r, g, b, a;
	Panel *pPanel;
	CommandButton *pButton;

	pPanel = new Panel( 0, 0, ScreenWidth, ScreenHeight );
	pPanel->setParent( this );
	pPanel->setSize( ScreenWidth, ScreenHeight );
	pPanel->setPos( 0, 0 );
	pPanel->setBgColor( 0, 0, 0, 0 );
	pPanel->setVisible( true );

	m_pBackgroundImage = vgui_LoadTGA( "gfx/vgui/backgroundtoall.tga" );
	if ( m_pBackgroundImage )
		m_pBackgroundImage->setColor( Color( 255, 255, 255, 1 ) );
	else
		gEngfuncs.Con_Printf( "Cannot load gfx/vgui/backgroundtoall.tga!\n" );

	m_pBackgroundImage->getSize( wide, tall );
	x = ScreenWidth / 2 - wide / 2;
	y = ScreenHeight / 2 - tall / 2;

	m_pBackground = new ImagePanel( m_pBackgroundImage );
	m_pBackground->setParent( this );
	m_pBackground->setPos( x, y );
	m_pBackground->setVisible( true );

	m_pScreenImage = vgui_LoadTGA( "gfx/vgui/puzzle/computer/computer_logon.tga" );
	if ( m_pScreenImage )
		m_pScreenImage->setColor( Color( 255, 255, 255, 1 ) );
	else
		gEngfuncs.Con_Printf( "Cannot load gfx/vgui/puzzle/computer/computer_logon.tga!\n" );

	x = ScreenWidth / 2 - 277;
	y = ScreenHeight / 2 - 240;

	m_pScreen = new ImagePanel( m_pScreenImage );
	m_pScreen->setParent( this );
	m_pScreen->setPos( x, y );
	m_pScreen->setVisible( true );

	pButton = new CommandButton( "", 157, 320, 75, 23, true, true );
	pButton->setParent( m_pScreen );
	pButton->addActionSignal( new CComputerHandler_Command( this, 1 ) );

	pButton = new CommandButton( "", 238, 320, 75, 23, true, true );
	pButton->setParent( m_pScreen );
	pButton->addActionSignal( new CComputerHandler_Command( this, 2 ) );

	pButton = new CommandButton( "", 478, 447, 29, 29, true, true );
	pButton->setParent( m_pScreen );
	pButton->addActionSignal( new CComputerHandler_Command( this, 2 ) );

	CSchemeManager *pSchemes = gViewPort->GetSchemeManager();
	SchemeHandle_t hTitleScheme = pSchemes->getSchemeHandle( "Title Font" );

	m_pUser = new TextEntry( "", 160, 204, 239, 16 );
	m_pUser->setVisible( true );
	m_pUser->setParent( m_pScreen );
	pSchemes->getFgColor( hTitleScheme, r, g, b, a );
	m_pUser->setFgColor( r, g, b, a );
	m_pUser->setFgColor( Scheme::sc_primary1 );
	pSchemes->getBgColor( hTitleScheme, r, g, b, a );
	m_pUser->setBgColor( r, g, b, a );
	m_pUser->addInputSignal( new CComputerFieldSignal( this, m_pUser ) );

	m_pPassword = new TextEntry( "", 160, 231, 239, 16 );
	m_pPassword->setVisible( true );
	m_pPassword->setParent( m_pScreen );
	pSchemes->getFgColor( hTitleScheme, r, g, b, a );
	m_pPassword->setFgColor( r, g, b, a );
	m_pPassword->setFgColor( Scheme::sc_primary1 );
	pSchemes->getBgColor( hTitleScheme, r, g, b, a );
	m_pPassword->setBgColor( r, g, b, a );
	m_pPassword->addInputSignal( new CComputerFieldSignal( this, m_pPassword ) );

	m_pText = new TextPanel( "", 194, 162, 211, 152 );
	m_pText->setParent( m_pScreen );
	m_pText->setVisible( false );

	setBgColor( 0, 0, 0, 255 );

	Initialize();
}

void CComputer::ActionSignal( int cmd )
{
	char szUser[256];
	char szPassword[256];
	char szCommand[128];

	switch ( cmd )
	{
	case 0:
		break;
	case 1:
		if ( m_iLoggedOn )
		{
			setVisible( false );
			gViewPort->UpdateCursorState();
			return;
		}
		m_pUser->getText( 0, szUser, 256 );
		m_pPassword->getText( 0, szPassword, 256 );
		sprintf( szCommand, "computercheck %s %s", szUser, szPassword );
		gEngfuncs.pfnServerCmd( szCommand );
		break;
	case 2:
		setVisible( false );
		gViewPort->UpdateCursorState();
		break;
	default:
		gEngfuncs.Con_DPrintf( "Unknown Computer ActionSignal %i.\n", cmd );
		break;
	}
}

void CComputer::Initialize()
{
	setVisible( false );
	m_iLoggedOn = 0;
	m_flNextKeyTime = gEngfuncs.GetClientTime();
}

int CComputer::KeyInput( int down, int keynum, const char *pszCurrentBinding )
{
	if ( pszCurrentBinding && !strcmp( pszCurrentBinding, "toggleconsole" ) )
		return 1;

	return 0;
}

int MsgComputer( const char *pszName, int iSize, void *pbuf )
{
	char szText[512];

	BEGIN_READ( pbuf, iSize );

	if ( gViewPort && gViewPort->m_pComputer )
	{
		if ( !gViewPort->m_pComputer->isVisible() )
		{
			gViewPort->m_pComputer->setVisible( true );
			gViewPort->UpdateCursorState();
		}

		gViewPort->UpdateCursorState();

		gViewPort->m_pComputer->m_iLoggedOn = READ_BYTE();
		int iWrongUser = READ_BYTE();
		int iWrongPassword = READ_BYTE();
		int iSilent = READ_BYTE();

		sprintf( szText, "Jill - %s! - David\n", READ_STRING() );
		gViewPort->m_pComputer->m_pText->setText( szText );
		gViewPort->m_pComputer->m_pText->setFgColor( 0, 0, 0, 0 );

		if ( gViewPort->m_pComputer->m_iLoggedOn )
		{
			gViewPort->m_pComputer->m_pScreen->setImage( vgui_LoadTGA( "gfx/vgui/puzzle/computer/computer_loggedon.tga" ) );
			gViewPort->m_pComputer->m_pUser->setVisible( false );
			gViewPort->m_pComputer->m_pPassword->setVisible( false );

			cl_entity_t *pPlayer = gEngfuncs.GetLocalPlayer();
			gViewPort->m_pComputer->m_pText->setVisible( true );
			ComputerEnableTextInput( false );

			if ( pPlayer && !iSilent )
				gEngfuncs.pEventAPI->EV_PlaySound( -1, pPlayer->curstate.origin, CHAN_AUTO, "buttons/computer_logon.wav", 0.7, 0.8, 0, PITCH_NORM );
		}
		else
		{
			gViewPort->m_pComputer->m_pScreen->setImage( vgui_LoadTGA( "gfx/vgui/puzzle/computer/computer_logon.tga" ) );
			gViewPort->m_pComputer->m_pUser->setVisible( true );
			gViewPort->m_pComputer->m_pPassword->setVisible( true );

			cl_entity_t *pPlayer = gEngfuncs.GetLocalPlayer();
			if ( pPlayer && ( iWrongUser || iWrongPassword ) )
				gEngfuncs.pEventAPI->EV_PlaySound( -1, pPlayer->curstate.origin, CHAN_AUTO, "buttons/computer_deny.wav", 0.7, 0.8, 0, PITCH_NORM );

			gViewPort->m_pComputer->m_pText->setVisible( false );

			if ( iWrongUser || iWrongPassword )
			{
				gViewPort->m_pComputer->m_pUser->setText( "Invalid username or password!", 29 );
				gViewPort->m_pComputer->m_pPassword->setText( "Invalid password or password!", 29 );
			}
		}
	}

	return 1;
}

void VGUI_ComputerInit()
{
	gEngfuncs.pfnHookUserMsg( "Computer", MsgComputer );
}

void CComputer::setVisible( bool state )
{
	if ( state )
	{
		m_flNextKeyTime = gEngfuncs.GetClientTime();
		m_pUser->setText( "Enter a username...", 19 );
		m_pPassword->setText( "Enter a password...", 19 );

		if ( m_iLoggedOn )
		{
			m_pUser->setVisible( false );
			m_pPassword->setVisible( false );
		}
		else
		{
			m_pUser->setVisible( true );
			m_pPassword->setVisible( true );
		}
	}
	else
	{
		m_pUser->setText( "Enter a username...", 19 );
		m_pPassword->setText( "Enter a password...", 19 );

		if ( m_iLoggedOn )
		{
			m_pUser->setVisible( false );
			m_pPassword->setVisible( false );
		}
		else
		{
			m_pUser->setVisible( true );
			m_pPassword->setVisible( true );
		}

		if( isVisible() )
			ComputerEnableTextInput( false );

		gEngfuncs.pfnServerCmd( "stopcomputer" );
	}

	Panel::setVisible( state );
}

void CComputer::solve()
{
	char szUser[256];
	char szPassword[256];

	if ( isVisible() && m_pUser->isVisible() && m_pPassword->isVisible() && !m_iLoggedOn )
	{
		m_pUser->getText( 0, szUser, 256 );
		m_pPassword->getText( 0, szPassword, 256 );

		if ( ( GetAsyncKeyState( VK_RETURN ) & 0x8000 ) && gEngfuncs.GetClientTime() > m_flNextKeyTime )
		{
			char szUser2[256];
			char szPassword2[256];
			char szCommand[128];

			m_flNextKeyTime = gEngfuncs.GetClientTime() + 1;
			m_pUser->getText( 0, szUser2, 256 );
			m_pPassword->getText( 0, szPassword2, 256 );
			sprintf( szCommand, "computercheck %s %s", szUser2, szPassword2 );
			gEngfuncs.pfnServerCmd( szCommand );
		}

		if ( m_pUser->hasFocus() )
		{
			if ( !strcmp( szUser, "Enter a username..." ) )
				m_pUser->setText( "", 0 );

			if ( ( GetAsyncKeyState( VK_TAB ) & 0x8000 ) && gEngfuncs.GetClientTime() > m_flNextKeyTime )
			{
				m_flNextKeyTime = gEngfuncs.GetClientTime() + 1;
				m_pPassword->requestFocus();
			}
		}

		if ( m_pPassword->hasFocus() )
		{
			if ( !strcmp( szPassword, "Enter a password..." ) )
				m_pPassword->setText( "", 0 );

			if ( ( GetAsyncKeyState( VK_TAB ) & 0x8000 ) && gEngfuncs.GetClientTime() > m_flNextKeyTime )
			{
				m_flNextKeyTime = gEngfuncs.GetClientTime() + 1;
				m_pUser->requestFocus();
			}
		}

		cl_entity_t *pPlayer = gEngfuncs.GetLocalPlayer();
		int iPitch = PITCH_NORM - gEngfuncs.pfnRandomLong( -2, 2 );

		if ( m_iUserLength != strlen( szUser ) )
		{
			if ( pPlayer )
				gEngfuncs.pEventAPI->EV_PlaySound( -1, pPlayer->curstate.origin, CHAN_BODY, "buttons/keyboard_type.wav", 0.7, 0.8, 0, iPitch );

			m_iUserLength = strlen( szUser );
		}

		if ( m_iPasswordLength != strlen( szPassword ) )
		{
			if ( pPlayer )
				gEngfuncs.pEventAPI->EV_PlaySound( -1, pPlayer->curstate.origin, CHAN_BODY, "buttons/keyboard_type.wav", 0.7, 0.8, 0, iPitch );

			m_iPasswordLength = strlen( szPassword );
		}
	}

	Panel::solve();
}

CComputer::~CComputer( void )
{
	delete m_pScreenImage;
}
