// ======================================
// ======================================

#include "hud.h"
#include "client_port.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "vgui_TeamFortressViewport.h"
#include "keypad.h"
#include "../game_shared/vgui_loadtga.h"

CKeypad::CKeypad() : Panel( 0, 0, ScreenWidth, ScreenHeight )
{
	int wide, tall;
	int x, y;
	CommandButton *pButton;

	m_pBackgroundPanel = new Panel( 0, 0, ScreenWidth, ScreenHeight );
	m_pBackgroundPanel->setParent( this );
	m_pBackgroundPanel->setBgColor( 0, 0, 0, 0 );

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

	m_pPanelBackImage = vgui_LoadTGA( "gfx/vgui/640_panelback.tga" );
	if ( m_pPanelBackImage )
		m_pPanelBackImage->setColor( Color( 255, 255, 255, 1 ) );
	else
		gEngfuncs.Con_Printf( "Cannot load gfx/vgui/640_panelback.tga!\n" );

	m_pPanel2BackImage = vgui_LoadTGA( "gfx/vgui/640_panel2back.tga" );
	if ( m_pPanel2BackImage )
		m_pPanel2BackImage->setColor( Color( 255, 255, 255, 1 ) );
	else
		gEngfuncs.Con_Printf( "Cannot load gfx/vgui/640_panel2back.tga!\n" );

	m_pPanelOffImage = vgui_LoadTGA( "gfx/vgui/640_paneloff.tga" );
	if ( m_pPanelOffImage )
		m_pPanelOffImage->setColor( Color( 255, 255, 255, 1 ) );
	else
		gEngfuncs.Con_Printf( "Cannot load gfx/vgui/640_paneloff.tga!\n" );

	m_pPanelOnImage = vgui_LoadTGA( "gfx/vgui/640_panelon.tga" );
	if ( m_pPanelOnImage )
		m_pPanelOnImage->setColor( Color( 255, 255, 255, 1 ) );
	else
		gEngfuncs.Con_Printf( "Cannot load gfx/vgui/640_panelon.tga!\n" );

	CSchemeManager *pSchemes = gViewPort->GetSchemeManager();
	SchemeHandle_t hCreditsScheme = pSchemes->getSchemeHandle( "credits" );
	m_pFont = pSchemes->getFont( hCreditsScheme );

	x = ScreenWidth / 2 - 320;
	y = ScreenHeight / 2 - 240;

	m_pPanelBack = new ImagePanel( m_pPanelBackImage );
	m_pPanelBack->setParent( this );
	m_pPanelBack->setPos( x, y );
	m_pPanelBack->setVisible( true );

	m_pPanel2Back = new ImagePanel( m_pPanel2BackImage );
	m_pPanel2Back->setParent( m_pPanelBack );
	m_pPanel2Back->setPos( 0, 0 );
	m_pPanel2Back->setVisible( false );

	pButton = new CommandButton( "", 3, 3, 20, 20, true, true );
	pButton->setParent( m_pPanelBack );
	pButton->addActionSignal( new CKeypadHandler_Command( this, 1 ) );

	m_pPanelOff = new ImagePanel( m_pPanelOffImage );
	m_pPanelOff->setParent( m_pPanelBack );
	m_pPanelOff->setPos( 520, 0 );
	m_pPanelOff->setVisible( true );

	m_pPanelOn = new ImagePanel( m_pPanelOnImage );
	m_pPanelOn->setParent( m_pPanelBack );
	m_pPanelOn->setPos( 520, 0 );
	m_pPanelOn->setVisible( false );

	m_pKey1 = new CommandButton( "", 170, 90, 100, 100, true, true );
	m_pKey1->setParent( m_pPanelBack );
	m_pKey1->addActionSignal( new CKeypadHandler_Command( this, 2 ) );

	m_pKey2 = new CommandButton( "", 270, 90, 100, 100, true, true );
	m_pKey2->setParent( m_pPanelBack );
	m_pKey2->addActionSignal( new CKeypadHandler_Command( this, 3 ) );

	m_pKey3 = new CommandButton( "", 370, 90, 100, 100, true, true );
	m_pKey3->setParent( m_pPanelBack );
	m_pKey3->addActionSignal( new CKeypadHandler_Command( this, 4 ) );

	m_pKey4 = new CommandButton( "", 170, 190, 100, 100, true, true );
	m_pKey4->setParent( m_pPanelBack );
	m_pKey4->addActionSignal( new CKeypadHandler_Command( this, 5 ) );

	m_pKey5 = new CommandButton( "", 270, 190, 100, 100, true, true );
	m_pKey5->setParent( m_pPanelBack );
	m_pKey5->addActionSignal( new CKeypadHandler_Command( this, 6 ) );

	m_pKey6 = new CommandButton( "", 370, 190, 100, 100, true, true );
	m_pKey6->setParent( m_pPanelBack );
	m_pKey6->addActionSignal( new CKeypadHandler_Command( this, 7 ) );

	m_pKey7 = new CommandButton( "", 170, 290, 100, 100, true, true );
	m_pKey7->setParent( m_pPanelBack );
	m_pKey7->addActionSignal( new CKeypadHandler_Command( this, 8 ) );

	m_pKey8 = new CommandButton( "", 270, 290, 100, 100, true, true );
	m_pKey8->setParent( m_pPanelBack );
	m_pKey8->addActionSignal( new CKeypadHandler_Command( this, 9 ) );

	m_pKey9 = new CommandButton( "", 370, 290, 100, 100, true, true );
	m_pKey9->setParent( m_pPanelBack );
	m_pKey9->addActionSignal( new CKeypadHandler_Command( this, 10 ) );

	m_pCodeLabel = new Label( "", 247, 39, 149, 32 );
	m_pCodeLabel->setParent( m_pPanelBack );
	m_pCodeLabel->setFont( m_pFont );
	m_pCodeLabel->setFgColor( 0, 0, 0, 1 );
	m_pCodeLabel->setBgColor( 0, 0, 0, 255 );
	m_pCodeLabel->setText( "    " );
	m_pCodeLabel->setContentAlignment( Label::a_center );

	setBgColor( 0, 0, 0, 255 );

	Initialize();
}

CKeypad::~CKeypad()
{
	delete m_pPanelBackImage;
	delete m_pPanelOffImage;
	delete m_pPanelOnImage;
}

void CKeypad::ActionSignal( int cmd )
{
	switch ( cmd )
	{
	case 0:
		break;
	case 1:
		setVisible( false );
		gViewPort->UpdateCursorState();
		break;
	case 2:
		EnterKey( 1 );
		break;
	case 3:
		EnterKey( 2 );
		break;
	case 4:
		EnterKey( 3 );
		break;
	case 5:
		EnterKey( 4 );
		break;
	case 6:
		EnterKey( 5 );
		break;
	case 7:
		EnterKey( 6 );
		break;
	case 8:
		EnterKey( 7 );
		break;
	case 9:
		EnterKey( 8 );
		break;
	case 10:
		EnterKey( 9 );
		break;
	default:
		gEngfuncs.Con_DPrintf( "Unknown Keypad ActionSignal %i.\n", cmd );
		break;
	}
}

void CKeypad::Activate()
{
	gEngfuncs.pfnPlaySoundByName( "keypad/keypad_activate.wav", 1 );

	if ( m_iType == 0 )
	{
		m_pPanelOn->setVisible( true );
		m_pPanelOff->setVisible( false );
	}
}

void CKeypad::EnterKey( int key )
{
	char szDigit[4];
	char szStars[8];

	if ( m_iActivated )
	{
		gEngfuncs.pfnPlaySoundByName( "keypad/keypad_press.wav", 1 );
		return;
	}

	if ( strlen( m_szCode ) == 4 )
	{
		if ( m_iType == 2 )
		{
			for ( int i = 1; i <= strlen( m_szCode ); i++ )
			{
				if ( i == 1 )
					strcpy( szStars, "*" );
				else
					strcat( szStars, "*" );
			}

			m_pCodeLabel->setText( szStars );
			m_pCodeLabel->setContentAlignment( Label::a_center );
		}

		Submit();
		return;
	}

	itoa( key, szDigit, 10 );
	strcat( m_szCode, szDigit );
	gEngfuncs.pfnPlaySoundByName( "keypad/keypad_press.wav", 1 );

	if ( m_iType == 2 )
	{
		for ( int i = 1; i <= strlen( m_szCode ); i++ )
		{
			if ( i == 1 )
				strcpy( szStars, "*" );
			else
				strcat( szStars, "*" );
		}

		m_pCodeLabel->setText( szStars );
		m_pCodeLabel->setContentAlignment( Label::a_center );
	}

	if ( strlen( m_szCode ) == 4 )
		Submit();
}

void CKeypad::Initialize()
{
	strncpy( m_szCode, "", 8 );
	setVisible( false );

	m_iType = 0;
	m_iActivated = 0;

	m_pPanelBack->setVisible( true );
	m_pPanel2Back->setVisible( false );

	if ( m_iType == 0 )
	{
		m_pPanelOff->setVisible( true );
		m_pPanelOn->setVisible( false );
	}
	else
	{
		m_pPanelOff->setVisible( false );
		m_pPanelOn->setVisible( false );
	}
}

int CKeypad::KeyInput( int down, int keynum, const char *pszCurrentBinding )
{
	if ( pszCurrentBinding && ( !strcmp( pszCurrentBinding, "toggleconsole" ) || !strcmp( pszCurrentBinding, "keypadnumber" ) ) )
		return 1;

	return 0;
}

int MsgKeypad( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	if ( gViewPort->m_pKeypad )
	{
		gViewPort->m_pKeypad->m_iType = READ_BYTE();
		gViewPort->m_pKeypad->m_iActivated = READ_BYTE();
		gViewPort->m_pKeypad->UpdateLayout();

		int iOn = READ_BYTE();

		switch ( gViewPort->m_pKeypad->m_iType )
		{
		case 0:
			gViewPort->m_pKeypad->m_pPanelBack->setImage( vgui_LoadTGA( "gfx/vgui/640_panelback.tga" ) );
			gViewPort->m_pKeypad->m_pPanelOff->setVisible( true );
			gViewPort->m_pKeypad->m_pPanelOn->setVisible( false );
			break;
		case 1:
			gViewPort->m_pKeypad->m_pPanelOff->setVisible( false );
			gViewPort->m_pKeypad->m_pPanelOn->setVisible( false );
			gViewPort->m_pKeypad->m_pPanelBack->setImage( vgui_LoadTGA( "gfx/vgui/640_panel2back.tga" ) );
			break;
		case 2:
			gViewPort->m_pKeypad->m_pPanelOff->setVisible( false );
			gViewPort->m_pKeypad->m_pPanelOn->setVisible( false );
			gViewPort->m_pKeypad->m_pPanelBack->setImage( vgui_LoadTGA( "gfx/vgui/keypad3.tga" ) );
			break;
		}

		if ( gViewPort->m_pKeypad->m_iActivated )
			gViewPort->m_pKeypad->Activate();

		if ( !gViewPort->m_pKeypad->isVisible() && iOn )
		{
			gViewPort->m_pKeypad->setVisible( true );
			gViewPort->UpdateCursorState();
		}
	}

	return 1;
}

void CKeypad::Submit()
{
	char szCommand[128];

	sprintf( szCommand, "keypadnumber %s", m_szCode );
	gEngfuncs.pfnServerCmd( szCommand );

	strncpy( m_szCode, "", 8 );
}

void CKeypad::UpdateLayout()
{
	if ( m_iType == 0 || m_iType == 1 )
	{
		m_pCodeLabel->setText( "" );
		m_pCodeLabel->setVisible( false );
	}
	else
	{
		m_pCodeLabel->setText( "" );
		m_pCodeLabel->setVisible( true );
	}

	m_pKey1->setSize( 100, 100 );
	m_pKey1->setPos( 170, 90 );
	m_pKey2->setSize( 100, 100 );
	m_pKey2->setPos( 270, 90 );
	m_pKey3->setSize( 100, 100 );
	m_pKey3->setPos( 370, 90 );
	m_pKey4->setSize( 100, 100 );
	m_pKey4->setPos( 170, 190 );
	m_pKey5->setSize( 100, 100 );
	m_pKey5->setPos( 270, 190 );
	m_pKey6->setSize( 100, 100 );
	m_pKey6->setPos( 370, 190 );
	m_pKey7->setSize( 100, 100 );
	m_pKey7->setPos( 170, 290 );
	m_pKey8->setSize( 100, 100 );
	m_pKey8->setPos( 270, 290 );
	m_pKey9->setSize( 100, 100 );
	m_pKey9->setPos( 370, 290 );
}

void VGUI_KeypadInit()
{
	gEngfuncs.pfnHookUserMsg( "Keypad", MsgKeypad );
}
