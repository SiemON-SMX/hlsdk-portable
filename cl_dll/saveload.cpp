// ======================================
// ======================================

#include <fstream>
#include <string>
#include <vector>

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "com_weapons.h"
#include "vgui_TeamFortressViewport.h"
#include "saveload.h"
#include "gamemenu.h"
#include "gameover.h"
#include "../game_shared/vgui_loadtga.h"
#include "event_api.h"
#include "client_port.h"
#include <time.h>

extern int iMenuMode;

void IrrKlangStop( float flFadeTime );
void GetCurrentPath( char *path );

CSaveLoad::CSaveLoad() : Panel( 0, 0, ScreenWidth, ScreenHeight )
{
	int wide, tall;
	int x, y;
	Panel *pPanel;
	CommandButton *pButton;

	m_pBackgroundImage = vgui_LoadTGA( "gfx/vgui/save_load.tga" );
	if ( m_pBackgroundImage )
		m_pBackgroundImage->setColor( Color( 255, 255, 255, 1 ) );
	else
		gEngfuncs.Con_Printf( "Cannot load gfx/vgui/save_load.tga!\n" );

	m_pBackgroundImage->getSize( wide, tall );
	x = ScreenWidth / 2 - wide / 2;
	y = ScreenHeight / 2 - tall / 2;

	m_pBackground = new ImagePanel( m_pBackgroundImage );
	m_pBackground->setParent( this );
	m_pBackground->setPos( x, y );
	m_pBackground->setVisible( true );

	pButton = new CommandButton( "", 260, 398, 119, 43, true, true );
	pButton->setParent( m_pBackground );
	pButton->addActionSignal( new CSaveLoadHandler_Command( this, 1 ) );

	m_pSlot1Label = new Label( "", 93, 154, 455, 29 );
	m_pSlot1Label->setParent( m_pBackground );
	m_pSlot1Label->setBgColor( 0, 0, 0, 1 );
	m_pSlot1Label->setFgColor( 255, 255, 255, 1 );

	m_pSlot2Label = new Label( "", 93, 190, 455, 29 );
	m_pSlot2Label->setParent( m_pBackground );
	m_pSlot2Label->setBgColor( 0, 0, 0, 1 );
	m_pSlot2Label->setFgColor( 255, 255, 255, 1 );

	m_pSlot3Label = new Label( "", 93, 226, 455, 29 );
	m_pSlot3Label->setParent( m_pBackground );
	m_pSlot3Label->setBgColor( 0, 0, 0, 1 );
	m_pSlot3Label->setFgColor( 255, 255, 255, 1 );

	m_pSlot4Label = new Label( "", 93, 262, 455, 29 );
	m_pSlot4Label->setParent( m_pBackground );
	m_pSlot4Label->setBgColor( 0, 0, 0, 1 );
	m_pSlot4Label->setFgColor( 255, 255, 255, 1 );

	m_pSlot5Label = new Label( "", 93, 296, 455, 29 );
	m_pSlot5Label->setParent( m_pBackground );
	m_pSlot5Label->setBgColor( 0, 0, 0, 1 );
	m_pSlot5Label->setFgColor( 255, 255, 255, 1 );

	m_pSlot1Button = new ColorButton( "", 93, 154, 455, 29, false, false );
	m_pSlot1Button->setParent( m_pBackground );
	m_pSlot1Button->addActionSignal( new CSaveLoadHandler_Command( this, 2 ) );

	m_pSlot2Button = new ColorButton( "", 93, 190, 455, 29, true, true );
	m_pSlot2Button->setParent( m_pBackground );
	m_pSlot2Button->addActionSignal( new CSaveLoadHandler_Command( this, 3 ) );

	m_pSlot3Button = new ColorButton( "", 93, 226, 455, 29, true, true );
	m_pSlot3Button->setParent( m_pBackground );
	m_pSlot3Button->addActionSignal( new CSaveLoadHandler_Command( this, 4 ) );

	m_pSlot4Button = new ColorButton( "", 93, 262, 455, 29, true, true );
	m_pSlot4Button->setParent( m_pBackground );
	m_pSlot4Button->addActionSignal( new CSaveLoadHandler_Command( this, 5 ) );

	m_pSlot5Button = new ColorButton( "", 93, 296, 455, 29, true, true );
	m_pSlot5Button->setParent( m_pBackground );
	m_pSlot5Button->addActionSignal( new CSaveLoadHandler_Command( this, 6 ) );

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

	setBgColor( 0, 0, 0, 255 );

	Initialize();
}

void CSaveLoad::ActionSignal( int cmd )
{
	cl_entity_t *pPlayer = gEngfuncs.GetLocalPlayer();

	switch ( cmd )
	{
	case 1:
		if ( !strcmp( gEngfuncs.pfnGetLevelName(), "maps/c_loadgame.bsp" ) )
		{
			setVisible( false );
			gViewPort->UpdateCursorState();
			gEngfuncs.pfnServerCmd( "closegame" );

			if ( pPlayer )
				gEngfuncs.pEventAPI->EV_PlaySound( -1, pPlayer->curstate.origin, CHAN_AUTO, "save/save_no.wav", 1.0, 0.8, 0, PITCH_NORM );
		}
		else if ( strstr( gEngfuncs.pfnGetLevelName(), "game_menu" ) )
		{
			setVisible( false );
			gViewPort->UpdateCursorState();

			if ( gViewPort && gViewPort->m_pGameMenu )
			{
				gViewPort->m_pGameMenu->setVisible( true );
				gViewPort->UpdateCursorState();
			}
		}
		else if ( strstr( gEngfuncs.pfnGetLevelName(), "difficulty_" ) || strstr( gEngfuncs.pfnGetLevelName(), "server_" ) || strstr( gEngfuncs.pfnGetLevelName(), "unlockables" ) )
		{
			if ( iMenuMode )
			{
				setVisible( false );
				gViewPort->UpdateCursorState();
				gEngfuncs.pfnClientCmd( "map c_game_menu1" );
			}
			else
			{
				setVisible( false );
				gViewPort->UpdateCursorState();
				gEngfuncs.pfnClientCmd( "disconnect" );
			}
		}
		else if ( CL_IsDead() )
		{
			setVisible( false );
			gViewPort->UpdateCursorState();

			if ( gViewPort && gViewPort->m_pGameOver )
			{
				gViewPort->m_pGameOver->setVisible( true );
				gViewPort->UpdateCursorState();
			}
		}
		else
		{
			setVisible( false );
			gViewPort->UpdateCursorState();
			gEngfuncs.pfnServerCmd( "unfreeze 666" );

			if ( pPlayer )
				gEngfuncs.pEventAPI->EV_PlaySound( -1, pPlayer->curstate.origin, CHAN_AUTO, "save/save_no.wav", 1.0, 0.8, 0, PITCH_NORM );
		}
		break;
	case 2:
		if ( gEngfuncs.GetClientTime() < m_flSlot1ClickTime )
		{
			if ( m_iMode )
			{
				byte *pFile = gEngfuncs.COM_LoadFile( "SAVE/saveinfo1.cof", 5, NULL );

				if ( pFile )
				{
					setVisible( false );
					gViewPort->UpdateCursorState();
					gEngfuncs.pfnServerCmd( "unfreeze 1024" );

					if ( strstr( gEngfuncs.pfnGetLevelName(), "game_menu" ) )
						IrrKlangStop( 0 );

					gEngfuncs.COM_FreeFile( pFile );
					gEngfuncs.pfnServerCmd( "cofload 1" );
				}
			}
			else
			{
				setVisible( false );
				gViewPort->UpdateCursorState();
				gEngfuncs.pfnServerCmd( "unfreezesave 1024 1" );

				gHUD.m_flSaveHackTime = gEngfuncs.GetClientTime() + 3.25;
				gHUD.m_iSaveHackSlot = 1;
				gHUD.m_bSaveHack = true;

				WriteSaveInfo( "saveinfo1.cof" );
			}
		}

		m_flSlot1ClickTime = gEngfuncs.GetClientTime() + 0.5;
		break;
	case 3:
		if ( gEngfuncs.GetClientTime() < m_flSlot2ClickTime )
		{
			if ( m_iMode )
			{
				byte *pFile = gEngfuncs.COM_LoadFile( "SAVE/saveinfo2.cof", 5, NULL );

				if ( pFile )
				{
					setVisible( false );
					gViewPort->UpdateCursorState();
					gEngfuncs.pfnServerCmd( "unfreeze 1024" );

					if ( strstr( gEngfuncs.pfnGetLevelName(), "game_menu" ) )
						IrrKlangStop( 0 );

					gEngfuncs.COM_FreeFile( pFile );
					gEngfuncs.pfnServerCmd( "cofload 2" );
				}
			}
			else
			{
				setVisible( false );
				gViewPort->UpdateCursorState();
				gEngfuncs.pfnServerCmd( "unfreezesave 1024 2" );

				gHUD.m_flSaveHackTime = gEngfuncs.GetClientTime() + 3.25;
				gHUD.m_iSaveHackSlot = 2;
				gHUD.m_bSaveHack = true;

				WriteSaveInfo( "saveinfo2.cof" );
			}
		}

		m_flSlot2ClickTime = gEngfuncs.GetClientTime() + 0.5;
		break;
	case 4:
		if ( gEngfuncs.GetClientTime() < m_flSlot3ClickTime )
		{
			if ( m_iMode )
			{
				byte *pFile = gEngfuncs.COM_LoadFile( "SAVE/saveinfo3.cof", 5, NULL );

				if ( pFile )
				{
					setVisible( false );
					gViewPort->UpdateCursorState();
					gEngfuncs.pfnServerCmd( "unfreeze 1024" );

					if ( strstr( gEngfuncs.pfnGetLevelName(), "game_menu" ) )
						IrrKlangStop( 0 );

					gEngfuncs.COM_FreeFile( pFile );
					gEngfuncs.pfnServerCmd( "cofload 3" );
				}
			}
			else
			{
				setVisible( false );
				gViewPort->UpdateCursorState();
				gEngfuncs.pfnServerCmd( "unfreezesave 1024 3" );

				gHUD.m_flSaveHackTime = gEngfuncs.GetClientTime() + 3.25;
				gHUD.m_iSaveHackSlot = 3;
				gHUD.m_bSaveHack = true;

				WriteSaveInfo( "saveinfo3.cof" );
			}
		}

		m_flSlot3ClickTime = gEngfuncs.GetClientTime() + 0.5;
		break;
	case 5:
		if ( gEngfuncs.GetClientTime() < m_flSlot4ClickTime )
		{
			if ( m_iMode )
			{
				byte *pFile = gEngfuncs.COM_LoadFile( "SAVE/saveinfo4.cof", 5, NULL );

				if ( pFile )
				{
					setVisible( false );
					gViewPort->UpdateCursorState();
					gEngfuncs.pfnServerCmd( "unfreeze 1024" );

					if ( strstr( gEngfuncs.pfnGetLevelName(), "game_menu" ) )
						IrrKlangStop( 0 );

					gEngfuncs.COM_FreeFile( pFile );
					gEngfuncs.pfnServerCmd( "cofload 4" );
				}
			}
			else
			{
				setVisible( false );
				gViewPort->UpdateCursorState();
				gEngfuncs.pfnServerCmd( "unfreezesave 1024 4" );

				gHUD.m_flSaveHackTime = gEngfuncs.GetClientTime() + 3.25;
				gHUD.m_iSaveHackSlot = 4;
				gHUD.m_bSaveHack = true;

				WriteSaveInfo( "saveinfo4.cof" );
			}
		}

		m_flSlot4ClickTime = gEngfuncs.GetClientTime() + 0.5;
		break;
	case 6:
		if ( gEngfuncs.GetClientTime() < m_flSlot5ClickTime )
		{
			if ( m_iMode )
			{
				byte *pFile = gEngfuncs.COM_LoadFile( "SAVE/saveinfo5.cof", 5, NULL );

				if ( pFile )
				{
					setVisible( false );
					gViewPort->UpdateCursorState();
					gEngfuncs.pfnServerCmd( "unfreeze 1024" );

					if ( strstr( gEngfuncs.pfnGetLevelName(), "game_menu" ) )
						IrrKlangStop( 0 );

					gEngfuncs.COM_FreeFile( pFile );
					gEngfuncs.pfnServerCmd( "cofload 5" );
				}
			}
			else
			{
				setVisible( false );
				gViewPort->UpdateCursorState();
				gEngfuncs.pfnServerCmd( "unfreezesave 1024 5" );

				gHUD.m_flSaveHackTime = gEngfuncs.GetClientTime() + 3.25;
				gHUD.m_iSaveHackSlot = 5;
				gHUD.m_bSaveHack = true;

				WriteSaveInfo( "saveinfo5.cof" );
			}
		}

		m_flSlot5ClickTime = gEngfuncs.GetClientTime() + 0.5;
		break;
	}
}

void CSaveLoad::Initialize()
{
	setVisible( false );
}

int MsgSaveLoad( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	if ( gViewPort->m_pSaveLoad )
	{
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

		gViewPort->m_pSaveLoad->m_iMode = READ_BYTE();

		if ( !gViewPort->m_pSaveLoad->m_iMode )
			strcpy( gViewPort->m_pSaveLoad->m_szSaveName, READ_STRING() );

		gViewPort->m_pSaveLoad->ReadAllSlots();
		gViewPort->m_pSaveLoad->setVisible( true );
		gViewPort->UpdateCursorState();
		gEngfuncs.pfnServerCmd( "freeze 1024" );

		cl_entity_t *pPlayer = gEngfuncs.GetLocalPlayer();

		if ( pPlayer )
			gEngfuncs.pEventAPI->EV_PlaySound( -1, pPlayer->curstate.origin, CHAN_AUTO, "save/save_yes.wav", 1.0, 0.8, 0, PITCH_NORM );
	}

	return 1;
}

bool PathExists( const char *pszPath )
{
	return GetFileAttributes( pszPath ) != INVALID_FILE_ATTRIBUTES;
}

void CSaveLoad::ReadAllSlots()
{
	m_pSlot1Label->setText( "" );
	m_pSlot2Label->setText( "" );
	m_pSlot3Label->setText( "" );
	m_pSlot4Label->setText( "" );
	m_pSlot5Label->setText( "" );

	ReadSlot( 1 );
	ReadSlot( 2 );
	ReadSlot( 3 );
	ReadSlot( 4 );
	ReadSlot( 5 );
}

void CSaveLoad::ReadSlot( int slot )
{
	char szPath[256];

	strcpy( szPath, gEngfuncs.pfnGetGameDirectory() );
	strcat( szPath, "/SAVE/" );
	sprintf( szPath, "%ssaveinfo%i.cof", szPath, slot );

	std::ifstream file( szPath );
	std::string line;
	std::vector<std::string> lines;

	if ( file )
	{
		while ( std::getline( file, line ) )
			lines.push_back( line );

		switch ( slot )
		{
		case 1:
			m_pSlot1Label->setText( line.c_str() );
			m_pSlot1Label->setContentAlignment( Label::a_center );
			break;
		case 2:
			m_pSlot2Label->setText( line.c_str() );
			m_pSlot2Label->setContentAlignment( Label::a_center );
			break;
		case 3:
			m_pSlot3Label->setText( line.c_str() );
			m_pSlot3Label->setContentAlignment( Label::a_center );
			break;
		case 4:
			m_pSlot4Label->setText( line.c_str() );
			m_pSlot4Label->setContentAlignment( Label::a_center );
			break;
		case 5:
			m_pSlot5Label->setText( line.c_str() );
			m_pSlot5Label->setContentAlignment( Label::a_center );
			break;
		}
	}
}

void SaveLoadInit()
{
	gEngfuncs.pfnHookUserMsg( "SaveLoad", MsgSaveLoad );
}

void CSaveLoad::WriteSaveInfo( const char *pszFile )
{
	char szDir[MAX_PATH];
	char szPath[256];
	char szTime[256];

	GetCurrentPath( szDir );
	strcat( szDir, "/" );
	strcat( szDir, gEngfuncs.pfnGetGameDirectory() );
	strcat( szDir, "/SAVE/" );

	if ( !PathExists( szDir ) )
	{
		gEngfuncs.Con_Printf( "Creating save directory %s\n", szDir );

		if ( CreateDirectory( szDir, NULL ) )
			gEngfuncs.Con_Printf( "Made save directory\n" );
	}

	strcpy( szPath, gEngfuncs.pfnGetGameDirectory() );
	strcat( szPath, "/SAVE/" );
	strcat( szPath, pszFile );

	std::ofstream file;
	file.open( szPath );

	if ( file )
	{
		time_t t;
		time( &t );
		strcpy( szTime, ctime( &t ) );

		if ( szTime[strlen( szTime ) - 1] == '\n' )
			szTime[strlen( szTime ) - 1] = 0;

		file << m_szSaveName << " - " << szTime;
		file.close();
	}
}

CSaveLoad::~CSaveLoad( void )
{
	delete m_pBackgroundImage;
}
