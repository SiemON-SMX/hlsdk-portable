// ======================================
// ======================================

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "vgui_TeamFortressViewport.h"
#include "gameover.h"
#include "saveload.h"
#include "../game_shared/vgui_loadtga.h"

extern int iMenuMode;
extern int iDoctorMode;
extern int iVGUIMenuNoMusic;

void IrrKlangPlayMP3( const char *pszFile );

CGameOver::CGameOver() : Panel( 0, 0, ScreenWidth, ScreenHeight )
{
	int wide, tall;
	int x, y;
	int loadWide, loadTall;
	int exitWide, exitTall;
	int loadX, loadY;
	int exitX, exitY;
	int exitButtonX;

	CSchemeManager *pSchemes = gViewPort->GetSchemeManager();
	SchemeHandle_t hTitleScheme = pSchemes->getSchemeHandle( "Title Font" );

	m_pImage = vgui_LoadTGA( "gfx/vgui/game_over.tga" );
	if ( m_pImage )
		m_pImage->setColor( Color( 255, 255, 255, 1 ) );
	else
		gEngfuncs.Con_Printf( "Cannot load gfx/vgui/game_over.tga!\n" );

	m_pLoadImage = vgui_LoadTGA( "gfx/vgui/game_over_load.tga" );
	if ( m_pLoadImage )
		m_pLoadImage->setColor( Color( 255, 255, 255, 1 ) );
	else
		gEngfuncs.Con_Printf( "Cannot load gfx/vgui/game_over_load.tga!\n" );

	m_pExitImage = vgui_LoadTGA( "gfx/vgui/game_over_exit.tga" );
	if ( m_pExitImage )
		m_pExitImage->setColor( Color( 255, 255, 255, 1 ) );
	else
		gEngfuncs.Con_Printf( "Cannot load gfx/vgui/game_over_exit.tga!\n" );

	m_pImage->getSize( wide, tall );
	x = ScreenWidth / 2 - wide / 2;
	y = ScreenHeight / 2 - tall / 2;

	m_pLoadImage->getSize( loadWide, loadTall );
	m_pExitImage->getSize( exitWide, exitTall );

	loadX = wide / 2 - loadWide / 2;
	loadY = tall / 2 + loadTall;
	exitX = wide / 2 - exitWide / 2;
	exitY = loadY + exitTall;
	exitButtonX = wide / 2 - 51;

	m_pPanel = new ImagePanel( m_pImage );
	m_pPanel->setParent( this );
	m_pPanel->setPos( x, y );
	m_pPanel->setVisible( true );

	m_pLoad = new ImagePanel( m_pLoadImage );
	m_pLoad->setParent( m_pPanel );
	m_pLoad->setPos( loadX, loadY );
	m_pLoad->setVisible( true );

	m_pExit = new ImagePanel( m_pExitImage );
	m_pExit->setParent( m_pPanel );
	m_pExit->setPos( exitX, exitY );
	m_pExit->setVisible( true );

	m_pLoadButton = new ColorButton( "", loadX, loadY, 288, 36, false, true );
	m_pLoadButton->setParent( m_pPanel );
	m_pLoadButton->addActionSignal( new CGameOverHandler_Command( this, 1 ) );
	m_pLoadButton->setPos( loadX, loadY );

	m_pExitButton = new ColorButton( "", exitButtonX, exitY, 105, 37, false, true );
	m_pExitButton->setParent( m_pPanel );
	m_pExitButton->addActionSignal( new CGameOverHandler_Command( this, 2 ) );
	m_pExitButton->setPos( exitButtonX, exitY );

	m_flAlpha = 255;

	setBgColor( 0, 0, 0, 255 );

	Initialize();
}

CGameOver::~CGameOver()
{
	delete m_pImage;
	delete m_pLoadImage;
	delete m_pExitImage;
}

void CGameOver::ActionSignal( int cmd )
{
	char szCommand[256];

	switch ( cmd )
	{
	case 1:
		if ( iDoctorMode )
		{
			if ( strlen( m_szCustomDoc ) > 2 )
			{
				strcpy( szCommand, "map " );
				strcat( szCommand, m_szCustomDoc );
				gEngfuncs.pfnClientCmd( szCommand );
			}
			else
			{
				gEngfuncs.pfnClientCmd( "map c_doc_city" );
			}
		}
		else if ( gViewPort->m_pSaveLoad )
		{
			setVisible( false );

			gViewPort->m_pSaveLoad->m_pSlot1Button->cursorExited();
			gViewPort->m_pSaveLoad->m_pSlot2Button->cursorExited();
			gViewPort->m_pSaveLoad->m_pSlot3Button->cursorExited();
			gViewPort->m_pSaveLoad->m_pSlot4Button->cursorExited();
			gViewPort->m_pSaveLoad->m_pSlot5Button->cursorExited();

			gViewPort->m_pSaveLoad->m_flSlot1ClickTime = 0;
			gViewPort->m_pSaveLoad->m_flSlot2ClickTime = 0;
			gViewPort->m_pSaveLoad->m_flSlot3ClickTime = 0;
			gViewPort->m_pSaveLoad->m_flSlot4ClickTime = 0;
			gViewPort->m_pSaveLoad->m_flSlot5ClickTime = 0;
			gViewPort->m_pSaveLoad->m_iMode = 1;
			gViewPort->m_pSaveLoad->ReadAllSlots();
			gViewPort->m_pSaveLoad->setVisible( true );
			gViewPort->UpdateCursorState();

			gEngfuncs.pfnServerCmd( "freeze 1024" );
			gEngfuncs.pfnPlaySoundByName( "ui/3d_click.wav", 1 );
		}
		break;
	case 2:
		setVisible( false );

		if ( iMenuMode )
		{
			gEngfuncs.pfnClientCmd( "map c_game_menu1" );
			gViewPort->UpdateCursorState();
		}
		else
		{
			gEngfuncs.pfnServerCmd( "closegame" );
			gViewPort->UpdateCursorState();
		}
		break;
	}
}

void CGameOver::Initialize()
{
	m_szCustomDoc[0] = 0;
	setVisible( false );
}

int MsgCustomDoc( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	if ( gViewPort->m_pGameOver )
		strcpy( gViewPort->m_pGameOver->m_szCustomDoc, READ_STRING() );

	return 1;
}

void VGUI_CustomDocInit()
{
	gEngfuncs.pfnHookUserMsg( "CustomDoc", MsgCustomDoc );
}

void CGameOver::setVisible( bool state )
{
	if ( state )
	{
		if ( !iVGUIMenuNoMusic )
			IrrKlangPlayMP3( "game_over.mp3" );

		m_bLoadArmed = false;
		m_bExitArmed = false;
		m_flAlpha = 255;

		if ( iDoctorMode )
			m_pLoad->setImage( vgui_LoadTGA( "gfx/vgui/game_over_restart.tga" ) );
		else
			m_pLoad->setImage( vgui_LoadTGA( "gfx/vgui/game_over_load.tga" ) );

		m_pExit->setImage( vgui_LoadTGA( "gfx/vgui/game_over_exit.tga" ) );
	}

	Panel::setVisible( state );
	gViewPort->UpdateCursorState();
}

void CGameOver::solve()
{
	if ( m_flAlpha > 0 )
		m_flAlpha -= gHUD.m_flTimeDelta * 500;

	if ( m_flAlpha < 0 )
		m_flAlpha = 0;

	m_pImage->setColor( Color( 255, 255, 255, m_flAlpha ) );
	m_pLoadImage->setColor( Color( 255, 255, 255, m_flAlpha ) );
	m_pExitImage->setColor( Color( 255, 255, 255, m_flAlpha ) );

	if ( !gEngfuncs.GetLocalPlayer() )
		return;

	if ( m_pLoadButton->isArmed() )
	{
		if ( !m_bLoadArmed )
		{
			if ( iDoctorMode )
				m_pLoad->setImage( vgui_LoadTGA( "gfx/vgui/game_over_restart2.tga" ) );
			else
				m_pLoad->setImage( vgui_LoadTGA( "gfx/vgui/game_over_load2.tga" ) );

			gEngfuncs.pfnPlaySoundByName( "ui/3d_rollover.wav", 1 );
			m_bLoadArmed = true;
		}
	}
	else if ( m_bLoadArmed )
	{
		if ( iDoctorMode )
			m_pLoad->setImage( vgui_LoadTGA( "gfx/vgui/game_over_restart.tga" ) );
		else
			m_pLoad->setImage( vgui_LoadTGA( "gfx/vgui/game_over_load.tga" ) );

		m_bLoadArmed = false;
	}

	if ( m_pExitButton->isArmed() )
	{
		if ( !m_bExitArmed )
		{
			m_pExit->setImage( vgui_LoadTGA( "gfx/vgui/game_over_exit2.tga" ) );
			gEngfuncs.pfnPlaySoundByName( "ui/3d_rollover.wav", 1 );
			m_bExitArmed = true;
		}
	}
	else if ( m_bExitArmed )
	{
		m_pExit->setImage( vgui_LoadTGA( "gfx/vgui/game_over_exit.tga" ) );
		m_bExitArmed = false;
	}

	Panel::solve();
}
