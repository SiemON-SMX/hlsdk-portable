// ======================================
// ======================================

#include "hud.h"
#include "cl_util.h"
#include "com_model.h"
#include "vgui_TeamFortressViewport.h"
#include "bookmenu.h"
#include "unlockables.h"
#include "../game_shared/vgui_loadtga.h"

#define SHIELD_ON	0
#define SHIELD_OFF	1

extern cl_entity_t g_HeadShieldEnt;
extern int g_HeadShieldState;
extern int g_iHeadShieldType;

CBookMenu::CBookMenu() : Panel( 0, 0, ScreenWidth, ScreenHeight )
{
	int wide, tall;
	int x, y;
	CommandButton *pButton;

	m_pCancelImage = vgui_LoadTGA( "gfx/vgui/640_notecancel.tga" );
	if ( m_pCancelImage )
		m_pCancelImage->setColor( Color( 255, 255, 255, 1 ) );
	else
		gEngfuncs.Con_Printf( "Cannot load gfx/vgui/640_notecancel.tga!\n" );

	m_pArrowsImage = vgui_LoadTGA( "gfx/vgui/left_right.tga" );
	if ( m_pArrowsImage )
		m_pArrowsImage->setColor( Color( 255, 255, 255, 1 ) );
	else
		gEngfuncs.Con_Printf( "Cannot load gfx/vgui/left_right.tga!\n" );

	m_pCancelImage->getSize( wide, tall );

	m_pCancel = new ImagePanel( m_pCancelImage );
	m_pCancel->setParent( this );
	m_pCancel->setPos( wide, tall );
	m_pCancel->setVisible( true );

	pButton = new CommandButton( "", wide, tall, wide, tall, true, true );
	pButton->setParent( this );
	pButton->addActionSignal( new CBookMenuHandler_Command( this, 3 ) );

	m_pArrowsImage->getSize( wide, tall );
	x = ScreenWidth / 2 - wide / 2;
	y = ScreenHeight - tall * 2;

	m_pArrows = new ImagePanel( m_pArrowsImage );
	m_pArrows->setParent( this );
	m_pArrows->setPos( x, y );
	m_pArrows->setVisible( true );

	pButton = new CommandButton( "", x, y, 100, 50, true, true );
	pButton->setParent( this );
	pButton->addActionSignal( new CBookMenuHandler_Command( this, 1 ) );

	pButton = new CommandButton( "", x + 100, y, 100, 50, true, true );
	pButton->setParent( this );
	pButton->addActionSignal( new CBookMenuHandler_Command( this, 2 ) );

	setBgColor( 0, 0, 0, 255 );

	Initialize();
}

CBookMenu::~CBookMenu()
{
	delete m_pArrowsImage;
	delete m_pCancelImage;
}

void CBookMenu::ActionSignal( int cmd )
{
	switch ( cmd )
	{
	case 1:
		if ( gHUD.m_bMenuClick && g_iHeadShieldType == 7 && gEngfuncs.GetClientTime() > gHUD.m_flMenuClickTime )
		{
			switch ( g_HeadShieldEnt.curstate.sequence )
			{
			case 2:
				g_HeadShieldEnt.curstate.animtime = gEngfuncs.GetClientTime();
				g_HeadShieldEnt.curstate.sequence = 7;
				gHUD.m_flMenuClickTime = gEngfuncs.GetClientTime() + 1;
				gEngfuncs.pfnPlaySoundByName( "common/page_turn.wav", 1 );
				break;
			case 4:
				g_HeadShieldEnt.curstate.animtime = gEngfuncs.GetClientTime();
				g_HeadShieldEnt.curstate.sequence = 6;
				gHUD.m_flMenuClickTime = gEngfuncs.GetClientTime() + 1;
				gEngfuncs.pfnPlaySoundByName( "common/page_turn.wav", 1 );
				break;
			case 6:
				g_HeadShieldEnt.curstate.animtime = gEngfuncs.GetClientTime();
				g_HeadShieldEnt.curstate.sequence = 7;
				gHUD.m_flMenuClickTime = gEngfuncs.GetClientTime() + 1;
				gEngfuncs.pfnPlaySoundByName( "common/page_turn.wav", 1 );
				break;
			}
		}
		break;
	case 2:
		if ( gHUD.m_bMenuClick && g_iHeadShieldType == 7 && gEngfuncs.GetClientTime() > gHUD.m_flMenuClickTime )
		{
			switch ( g_HeadShieldEnt.curstate.sequence )
			{
			case 0:
				g_HeadShieldEnt.curstate.animtime = gEngfuncs.GetClientTime();
				g_HeadShieldEnt.curstate.sequence = 2;
				gHUD.m_flMenuClickTime = gEngfuncs.GetClientTime() + 1;
				gEngfuncs.pfnPlaySoundByName( "common/page_turn.wav", 1 );
				break;
			case 1:
				g_HeadShieldEnt.curstate.animtime = gEngfuncs.GetClientTime();
				g_HeadShieldEnt.curstate.sequence = 2;
				gHUD.m_flMenuClickTime = gEngfuncs.GetClientTime() + 1;
				gEngfuncs.pfnPlaySoundByName( "common/page_turn.wav", 1 );
				break;
			case 2:
				g_HeadShieldEnt.curstate.animtime = gEngfuncs.GetClientTime();
				g_HeadShieldEnt.curstate.sequence = 4;
				gHUD.m_flMenuClickTime = gEngfuncs.GetClientTime() + 1;
				gEngfuncs.pfnPlaySoundByName( "common/page_turn.wav", 1 );
				break;
			case 6:
				g_HeadShieldEnt.curstate.animtime = gEngfuncs.GetClientTime();
				g_HeadShieldEnt.curstate.sequence = 4;
				gHUD.m_flMenuClickTime = gEngfuncs.GetClientTime() + 1;
				gEngfuncs.pfnPlaySoundByName( "common/page_turn.wav", 1 );
				break;
			case 7:
				g_HeadShieldEnt.curstate.animtime = gEngfuncs.GetClientTime();
				g_HeadShieldEnt.curstate.sequence = 2;
				gHUD.m_flMenuClickTime = gEngfuncs.GetClientTime() + 1;
				gEngfuncs.pfnPlaySoundByName( "common/page_turn.wav", 1 );
				break;
			}
		}
		break;
	case 3:
		if ( gViewPort && gViewPort->m_pUnlockables )
		{
			setVisible( false );
			gViewPort->UpdateCursorState();

			gHUD.m_bMenuClick = false;
			g_iHeadShieldType = 0;
			g_HeadShieldState = SHIELD_OFF;

			gViewPort->m_pUnlockables->setVisible( true );
			gViewPort->UpdateCursorState();
			gEngfuncs.pfnPlaySoundByName( "ui/3d_click.wav", 1 );
		}
		break;
	}
}

void CBookMenu::Initialize()
{
	setVisible( false );
}

void CBookMenu::setVisible( bool state )
{
	if ( state )
	{
		if ( g_iHeadShieldType != 7 )
			g_iHeadShieldType = 7;

		g_HeadShieldState = SHIELD_ON;
		gHUD.m_bMenuClick = true;
		gEngfuncs.pfnPlaySoundByName( "common/page_turn.wav", 1 );
		g_HeadShieldEnt.curstate.animtime = gEngfuncs.GetClientTime();
		g_HeadShieldEnt.curstate.sequence = 0;
	}

	Panel::setVisible( state );
}
