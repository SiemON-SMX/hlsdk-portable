// ======================================
// ======================================

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "vgui_TeamFortressViewport.h"
#include "clothesmenu.h"
#include "hudcontrol.h"

extern bool g_bHoodieLoaded;
extern bool g_bSleeveLoaded;
extern bool g_bJeansLoaded;
extern bool g_bFaceLoaded;

void UseCustomHoodie();

CClothesMenu::CClothesMenu() : Panel( 0, 0, ScreenWidth, ScreenHeight )
{
	ColorButton *pButton;
	int y;
	int i;
	int tall = ScreenHeight / 12;
	int wide = ScreenWidth / 2;

	CSchemeManager *pSchemes = gViewPort->GetSchemeManager();

	for ( i = 0, y = 0; i <= 12; i++, y += tall )
	{
		m_pClothesButton[i] = new ColorButton( "", ScreenWidth / 2, y, wide, tall, false, false );
		m_pClothesButton[i]->setParent( this );
		m_pClothesButton[i]->addActionSignal( new CClothesMenuHandler_Command( this, i + 1 ) );
		m_pClothesButton[i]->setFont( pSchemes->getFont( pSchemes->getSchemeHandle( "credits" ) ) );
		m_pClothesButton[i]->setUnArmedBorderColor( 255, 255, 255, 200 );
		m_pClothesButton[i]->setUnArmedColor( 255, 255, 255, 0 );
		m_pClothesButton[i]->setArmedBorderColor( 255, 255, 255, 150 );
		m_pClothesButton[i]->setArmedColor( 255, 0, 0, 32 );
		m_pClothesButton[i]->setBgColor( 0, 0, 0, 128 );
		m_pClothesButton[i]->setFgColor( 255, 255, 255, 128 );
	}

	pButton = new ColorButton( "Close", 0, ScreenHeight - tall, wide / 2, tall, false, false );
	pButton->setParent( this );
	pButton->addActionSignal( new CClothesMenuHandler_Command( this, 50 ) );
	pButton->setFont( pSchemes->getFont( pSchemes->getSchemeHandle( "credits" ) ) );
	pButton->setUnArmedBorderColor( 255, 255, 255, 200 );
	pButton->setUnArmedColor( 255, 255, 255, 0 );
	pButton->setArmedBorderColor( 255, 255, 255, 150 );
	pButton->setArmedColor( 255, 0, 0, 32 );
	pButton->setBgColor( 0, 0, 0, 128 );
	pButton->setFgColor( 255, 255, 255, 128 );

	pButton = new ColorButton( "Reset", wide / 2, ScreenHeight - tall, wide / 2, tall, false, false );
	pButton->setParent( this );
	pButton->addActionSignal( new CClothesMenuHandler_Command( this, 66 ) );
	pButton->setFont( pSchemes->getFont( pSchemes->getSchemeHandle( "credits" ) ) );
	pButton->setUnArmedBorderColor( 255, 255, 255, 200 );
	pButton->setUnArmedColor( 255, 255, 255, 0 );
	pButton->setArmedBorderColor( 255, 255, 255, 150 );
	pButton->setArmedColor( 255, 0, 0, 32 );
	pButton->setBgColor( 0, 0, 0, 128 );
	pButton->setFgColor( 255, 255, 255, 128 );

	setBgColor( 0, 0, 0, 255 );

	Initialize();
}

void CClothesMenu::ActionSignal( int cmd )
{
	if ( cmd != 50 && gEngfuncs.GetClientTime() < m_flNextClick )
		return;

	switch ( cmd )
	{
	case 0:
		break;
	case 1:
		if ( m_iUnlocked & 1 )
		{
			gHUD.m_iClothesState = 1;
			g_bHoodieLoaded = false;
			g_bJeansLoaded = false;
			g_bSleeveLoaded = false;
			g_bFaceLoaded = false;
			gEngfuncs.pfnServerCmd( "setclothes 1" );
		}
		break;
	case 2:
		if ( m_iUnlocked & 2 )
		{
			gHUD.m_iClothesState = 2;
			g_bHoodieLoaded = false;
			g_bJeansLoaded = false;
			g_bSleeveLoaded = false;
			g_bFaceLoaded = false;
			gEngfuncs.pfnServerCmd( "setclothes 2" );
		}
		break;
	case 3:
		if ( m_iUnlocked & 4 )
		{
			gHUD.m_iClothesState = 3;
			g_bHoodieLoaded = false;
			g_bSleeveLoaded = false;
			g_bJeansLoaded = false;
			g_bFaceLoaded = false;
			gEngfuncs.pfnServerCmd( "setclothes 3" );
		}
		break;
	case 4:
		if ( m_iUnlocked & 8 )
		{
			gHUD.m_iClothesState = 4;
			g_bHoodieLoaded = false;
			g_bSleeveLoaded = false;
			g_bJeansLoaded = false;
			g_bFaceLoaded = false;
			gEngfuncs.pfnServerCmd( "setclothes 4" );
		}
		break;
	case 5:
		if ( m_iUnlocked & 0x10 )
		{
			gHUD.m_iClothesState = 5;
			g_bHoodieLoaded = false;
			g_bSleeveLoaded = false;
			g_bJeansLoaded = false;
			g_bFaceLoaded = false;
			gEngfuncs.pfnServerCmd( "setclothes 5" );
		}
		break;
	case 6:
		if ( m_iUnlocked & 0x20 )
		{
			gHUD.m_iClothesState = 6;
			g_bHoodieLoaded = false;
			g_bSleeveLoaded = false;
			g_bJeansLoaded = false;
			g_bFaceLoaded = false;
			gEngfuncs.pfnServerCmd( "setclothes 6" );
		}
		break;
	case 7:
		if ( m_iUnlocked & 0x40 )
		{
			gHUD.m_iClothesState = 7;
			g_bHoodieLoaded = false;
			g_bSleeveLoaded = false;
			g_bJeansLoaded = false;
			g_bFaceLoaded = false;
			gEngfuncs.pfnServerCmd( "setclothes 7" );
		}
		break;
	case 8:
		if ( m_iUnlocked & 0x80 )
		{
			gHUD.m_iClothesState = 8;
			g_bHoodieLoaded = false;
			g_bSleeveLoaded = false;
			g_bJeansLoaded = false;
			g_bFaceLoaded = false;
			gEngfuncs.pfnServerCmd( "setclothes 8" );
		}
		break;
	case 9:
		if ( m_iUnlocked & 0x100 )
		{
			gHUD.m_iClothesState = 9;
			g_bHoodieLoaded = false;
			g_bSleeveLoaded = false;
			g_bJeansLoaded = false;
			g_bFaceLoaded = false;
			gEngfuncs.pfnServerCmd( "setclothes 9" );
		}
		break;
	case 10:
		if ( m_iUnlocked & 0x200 )
		{
			gHUD.m_iClothesState = 10;
			g_bHoodieLoaded = false;
			g_bSleeveLoaded = false;
			g_bJeansLoaded = false;
			g_bFaceLoaded = false;
			gEngfuncs.pfnServerCmd( "setclothes 10" );
		}
		break;
	case 11:
		if ( m_iUnlocked & 0x400 )
		{
			gHUD.m_iClothesState = 11;
			g_bHoodieLoaded = false;
			g_bSleeveLoaded = false;
			g_bFaceLoaded = false;
			g_bJeansLoaded = false;
			gEngfuncs.pfnServerCmd( "setclothes 11" );
		}
		break;
	case 12:
		if ( m_iUnlocked & 0x800 )
		{
			UseCustomHoodie();
			m_flNextClick = gEngfuncs.GetClientTime() + 0.5;
		}
		break;
	case 50:
		setVisible( false );
		gViewPort->UpdateCursorState();
		gEngfuncs.pfnServerCmd( "closeclothes" );
		break;
	case 66:
		gHUD.m_iClothesState = 0;
		g_bHoodieLoaded = false;
		g_bSleeveLoaded = false;
		g_bJeansLoaded = false;
		g_bFaceLoaded = false;
		gEngfuncs.pfnServerCmd( "setclothes 0" );
		break;
	default:
		gEngfuncs.Con_DPrintf( "Unknown clothesmenu ActionSignal %i.\n", cmd );
		break;
	}
}

void CClothesMenu::Initialize()
{
	m_iUnlocked = 0;
	setVisible( false );
}

int CClothesMenu::KeyInput( int down, int keynum, const char *pszCurrentBinding )
{
	if ( pszCurrentBinding && !strcmp( pszCurrentBinding, "toggleconsole" ) )
		return 1;

	return 0;
}

int MsgClothes( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	if ( gViewPort->m_pClothesMenu )
	{
		gViewPort->m_pClothesMenu->m_iUnlocked = READ_LONG();
		gHUD.m_iClothes = READ_BYTE();
		gHUD.m_iClothesState = READ_BYTE();
		int iShow = READ_BYTE();

		gViewPort->m_pClothesMenu->UpdateButtons();

		if ( iShow )
		{
			gViewPort->m_pClothesMenu->setVisible( true );
			gViewPort->UpdateCursorState();
			gViewPort->m_pClothesMenu->m_flNextClick = gEngfuncs.GetClientTime() + 0.5;
		}
	}

	return 1;
}

void CClothesMenu::UpdateButtons()
{
	if ( m_iUnlocked & 1 )
		m_pClothesButton[0]->setText( "David Leatherhoff suit" );
	else
		m_pClothesButton[0]->setText( "Locked" );
	m_pClothesButton[0]->setContentAlignment( Label::a_center );

	if ( m_iUnlocked & 2 )
		m_pClothesButton[1]->setText( "ModDB hoodie" );
	else
		m_pClothesButton[1]->setText( "Locked" );
	m_pClothesButton[1]->setContentAlignment( Label::a_center );

	if ( m_iUnlocked & 4 )
		m_pClothesButton[2]->setText( "Hello Kitty suit" );
	else
		m_pClothesButton[2]->setText( "Locked" );
	m_pClothesButton[2]->setContentAlignment( Label::a_center );

	if ( m_iUnlocked & 8 )
		m_pClothesButton[3]->setText( "Afraid of Monsters suit" );
	else
		m_pClothesButton[3]->setText( "Locked" );
	m_pClothesButton[3]->setContentAlignment( Label::a_center );

	if ( m_iUnlocked & 0x10 )
		m_pClothesButton[4]->setText( "Camouflage hoodie" );
	else
		m_pClothesButton[4]->setText( "Locked" );
	m_pClothesButton[4]->setContentAlignment( Label::a_center );

	if ( m_iUnlocked & 0x20 )
		m_pClothesButton[5]->setText( "Half-Life Creations hoodie" );
	else
		m_pClothesButton[5]->setText( "Locked" );
	m_pClothesButton[5]->setContentAlignment( Label::a_center );

	if ( m_iUnlocked & 0x40 )
		m_pClothesButton[6]->setText( "Black Metal suit" );
	else
		m_pClothesButton[6]->setText( "Locked" );
	m_pClothesButton[6]->setContentAlignment( Label::a_center );

	if ( m_iUnlocked & 0x80 )
		m_pClothesButton[7]->setText( "Team Psykskallar hoodie" );
	else
		m_pClothesButton[7]->setText( "Locked" );
	m_pClothesButton[7]->setContentAlignment( Label::a_center );

	if ( m_iUnlocked & 0x100 )
		m_pClothesButton[8]->setText( "Awesome suit" );
	else
		m_pClothesButton[8]->setText( "Locked" );
	m_pClothesButton[8]->setContentAlignment( Label::a_center );

	if ( m_iUnlocked & 0x200 )
		m_pClothesButton[9]->setText( "Sick Simon suit" );
	else
		m_pClothesButton[9]->setText( "Locked" );
	m_pClothesButton[9]->setContentAlignment( Label::a_center );

	if ( m_iUnlocked & 0x400 )
		m_pClothesButton[10]->setText( "AoM Twitcher suit" );
	else
		m_pClothesButton[10]->setText( "Locked" );
	m_pClothesButton[10]->setContentAlignment( Label::a_center );

	if ( m_iUnlocked & 0x800 )
		m_pClothesButton[11]->setText( "Custom hoodie" );
	else
		m_pClothesButton[11]->setText( "Locked" );
	m_pClothesButton[11]->setContentAlignment( Label::a_center );

}

void UseCustomHoodie()
{
	char szCommand[256];

	if ( gEngfuncs.COM_LoadFile( "models/costumes/custom_hoodie.tga", 5, NULL ) )
	{
		gHUD.m_iClothesState = 12;
		g_bHoodieLoaded = false;
		g_bJeansLoaded = false;
		g_bSleeveLoaded = false;
		g_bFaceLoaded = false;
		sprintf( szCommand, "setclothes 12 %s", "custom_hoodie.tga" );
		gEngfuncs.pfnServerCmd( szCommand );
	}
	else
	{
		strcpy( gViewPort->m_pHUDControl->m_szMessage, "To use custom hoodies place a .tga called custom_hoodie.tga in the models\\costumes folder" );
		gViewPort->m_pHUDControl->m_flMessageAlpha = 255;
		gViewPort->m_pHUDControl->m_bMessageFadeOut = false;
		gViewPort->m_pHUDControl->m_bMessageHold = false;
		gViewPort->m_pHUDControl->m_flMessageHoldTime = 0;
		gViewPort->m_pHUDControl->m_iMessageType = 5;
	}
}

void VGUI_ClothesInit()
{
	gHUD.m_iClothesState = 0;
	gEngfuncs.pfnHookUserMsg( "Clothes", MsgClothes );
}

CClothesMenu::~CClothesMenu( void )
{
}
