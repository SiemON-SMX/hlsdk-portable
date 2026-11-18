// ======================================
// ======================================

#include "hud.h"
#include "cl_util.h"
#include "vgui_TeamFortressViewport.h"
#include "../game_shared/vgui_checkbutton2.h"
#include "difficulty.h"
#include "gamemenu.h"
#include "../game_shared/vgui_loadtga.h"

extern int iMenuMode;

void IrrKlangStop( float flFadeTime );

CDifficulty::CDifficulty() : Panel( 0, 0, ScreenWidth, ScreenHeight )
{
	int wide, tall;
	int x, y;
	ColorButton *pButton;

	CSchemeManager *pSchemes = gViewPort->GetSchemeManager();
	SchemeHandle_t hTitleScheme = pSchemes->getSchemeHandle( "Title Font" );

	x = ScreenWidth / 2 - 320;
	y = ScreenHeight / 2 - 240;

	m_pImage = vgui_LoadTGA( "gfx/vgui/640_difficulty.tga" );
	if ( m_pImage )
		m_pImage->setColor( Color( 255, 255, 255, 1 ) );
	else
		gEngfuncs.Con_Printf( "Cannot load gfx/vgui/640_difficulty.tga!\n" );

	m_pImagePanel = new ImagePanel( m_pImage );
	m_pImagePanel->setParent( this );
	m_pImagePanel->setPos( x, y );
	m_pImagePanel->setVisible( true );

	m_CommentaryButton.setParent( this );
	m_CommentaryButton.SetImages( "gfx/vgui/checked.tga", "gfx/vgui/unchecked.tga" );
	m_CommentaryButton.SetText( "Enable developer commentary" );
	m_CommentaryButton.SetCheckboxLeft( true );
	m_CommentaryButton.SetChecked( gEngfuncs.pfnGetCvarFloat( "dev_commentary" ) != 0 );
	m_CommentaryButton.SetHandler( this );
	m_CommentaryButton.getSize( wide, tall );
	m_CommentaryButton.setPos( ScreenWidth / 2 - wide / 2, y + 490 );

	pButton = new ColorButton( "", 120, 90, 400, 75, false, false );
	pButton->setParent( m_pImagePanel );
	pButton->addActionSignal( new CDifficultyHandler_Command( this, 1 ) );
	pButton->setUnArmedBorderColor( 0, 0, 0, 255 );
	pButton->setArmedBorderColor( 255, 255, 255, 192 );
	pButton->setUnArmedColor( 0, 0, 0, 255 );
	pButton->setArmedColor( 0, 0, 0, 255 );

	pButton = new ColorButton( "", 120, 165, 400, 75, false, false );
	pButton->setParent( m_pImagePanel );
	pButton->addActionSignal( new CDifficultyHandler_Command( this, 2 ) );
	pButton->setUnArmedBorderColor( 0, 0, 0, 255 );
	pButton->setArmedBorderColor( 255, 255, 255, 192 );
	pButton->setUnArmedColor( 0, 0, 0, 255 );
	pButton->setArmedColor( 0, 0, 0, 255 );

	pButton = new ColorButton( "", 120, 240, 400, 75, false, false );
	pButton->setParent( m_pImagePanel );
	pButton->addActionSignal( new CDifficultyHandler_Command( this, 3 ) );
	pButton->setUnArmedBorderColor( 0, 0, 0, 255 );
	pButton->setArmedBorderColor( 255, 255, 255, 192 );
	pButton->setUnArmedColor( 0, 0, 0, 255 );
	pButton->setArmedColor( 0, 0, 0, 255 );

	m_pSkill4Button = new ColorButton( "", 120, 315, 400, 75, false, false );
	m_pSkill4Button->setParent( m_pImagePanel );
	m_pSkill4Button->addActionSignal( new CDifficultyHandler_Command( this, 4 ) );
	m_pSkill4Button->setUnArmedBorderColor( 0, 0, 0, 255 );
	m_pSkill4Button->setArmedBorderColor( 255, 255, 255, 192 );
	m_pSkill4Button->setUnArmedColor( 0, 0, 0, 255 );
	m_pSkill4Button->setArmedColor( 0, 0, 0, 255 );

	pButton = new ColorButton( "", 95, 409, 450, 47, false, false );
	pButton->setParent( m_pImagePanel );
	pButton->addActionSignal( new CDifficultyHandler_Command( this, 5 ) );
	pButton->setUnArmedBorderColor( 0, 0, 0, 255 );
	pButton->setArmedBorderColor( 255, 255, 255, 192 );
	pButton->setUnArmedColor( 0, 0, 0, 255 );
	pButton->setArmedColor( 0, 0, 0, 255 );

	setBgColor( 0, 0, 0, 255 );

	Initialize();
}

CDifficulty::~CDifficulty()
{
	delete m_pImage;
}

void CDifficulty::ActionSignal( int cmd )
{
	char szCommand[256];

	switch ( cmd )
	{
	case 1:
		if ( gViewPort && gViewPort->m_pGameMenu && gViewPort->m_pGameMenu->m_pszCampaign )
		{
			sprintf( szCommand, "campaign %s", gViewPort->m_pGameMenu->m_pszCampaign );
			gEngfuncs.pfnServerCmd( szCommand );
		}
		gEngfuncs.pfnServerCmd( "skillset 1" );
		setVisible( false );
		if ( strstr( gEngfuncs.pfnGetLevelName(), "game_menu" ) )
			IrrKlangStop( 1 );
		break;
	case 2:
		if ( gViewPort && gViewPort->m_pGameMenu && gViewPort->m_pGameMenu->m_pszCampaign )
		{
			sprintf( szCommand, "campaign %s", gViewPort->m_pGameMenu->m_pszCampaign );
			gEngfuncs.pfnServerCmd( szCommand );
		}
		gEngfuncs.pfnServerCmd( "skillset 2" );
		setVisible( false );
		if ( strstr( gEngfuncs.pfnGetLevelName(), "game_menu" ) )
			IrrKlangStop( 1 );
		break;
	case 3:
		if ( gViewPort && gViewPort->m_pGameMenu && gViewPort->m_pGameMenu->m_pszCampaign )
		{
			sprintf( szCommand, "campaign %s", gViewPort->m_pGameMenu->m_pszCampaign );
			gEngfuncs.pfnServerCmd( szCommand );
		}
		gEngfuncs.pfnServerCmd( "skillset 3" );
		setVisible( false );
		if ( strstr( gEngfuncs.pfnGetLevelName(), "game_menu" ) )
			IrrKlangStop( 1 );
		break;
	case 4:
		if ( gHUD.m_iUnlocks & 32 )
		{
			if ( gViewPort && gViewPort->m_pGameMenu && gViewPort->m_pGameMenu->m_pszCampaign )
			{
				sprintf( szCommand, "campaign %s", gViewPort->m_pGameMenu->m_pszCampaign );
				gEngfuncs.pfnServerCmd( szCommand );
			}
			gEngfuncs.pfnServerCmd( "skillset 4" );
			setVisible( false );
			if ( strstr( gEngfuncs.pfnGetLevelName(), "game_menu" ) )
				IrrKlangStop( 1 );
		}
		break;
	case 5:
		if ( iMenuMode )
		{
			setVisible( false );
			gViewPort->UpdateCursorState();

			if ( gViewPort && gViewPort->m_pGameMenu )
			{
				gViewPort->m_pGameMenu->setVisible( true );
				gViewPort->UpdateCursorState();
			}
		}
		else
		{
			gEngfuncs.pfnServerCmd( "closegame" );
			setVisible( false );
			gViewPort->UpdateCursorState();
		}
		break;
	}
}

void CDifficulty::Initialize()
{
	setVisible( false );
}

int CDifficulty::KeyInput( int down, int keynum, const char *pszCurrentBinding )
{
	if ( pszCurrentBinding && ( !strcmp( pszCurrentBinding, "toggleconsole" ) || !strcmp( pszCurrentBinding, "skillset" ) ) )
		return 1;

	return 0;
}

void CDifficulty::StateChanged( CCheckButton2 *pButton )
{
	if ( pButton == &m_CommentaryButton )
	{
		if ( pButton->IsChecked() )
			gEngfuncs.pfnClientCmd( "dev_commentary 1" );
		else
			gEngfuncs.pfnClientCmd( "dev_commentary 0" );
	}
}

void CDifficulty::setVisible( bool state )
{
	Panel::setVisible( state );
	gViewPort->UpdateCursorState();

	if ( state )
	{
		if ( gHUD.m_iUnlocks & 0x400000 )
			m_CommentaryButton.setVisible( true );
		else
			m_CommentaryButton.setVisible( false );

		if ( gHUD.m_iUnlocks & 32 )
		{
			m_pSkill4Button->setVisible( true );
			m_pImagePanel->setImage( vgui_LoadTGA( "gfx/vgui/640_difficulty.tga" ) );
		}
		else
		{
			m_pSkill4Button->setVisible( false );
			m_pImagePanel->setImage( vgui_LoadTGA( "gfx/vgui/640_difficultylocked.tga" ) );
		}
	}
}
