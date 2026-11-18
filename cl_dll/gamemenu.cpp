// ======================================
// ======================================

#include <string>

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "vgui_TeamFortressViewport.h"
#include "gamemenu.h"
#include "saveload.h"
#include "unlockables.h"
#include "vguiextramenu.h"
#include "language.h"
#include "difficulty.h"
#include "../game_shared/vgui_loadtga.h"
#include "client_port.h"
#include <sys/types.h>
#include <sys/stat.h>

void IrrKlangPlayMP3( const char *pszFile );
void IrrKlangSetLoop( bool bLoop );

const char *GetVersion();

CGameMenu::CGameMenu() : Panel( 0, 0, ScreenWidth, ScreenHeight )
{
	int wide, tall;
	int tsWide, tsTall;
	int logoWide, logoTall;
	int x, y;
	int logoX, logoY;
	int i;

	m_pLogoImage = vgui_LoadTGA( "gfx/vgui/menu_logo.tga" );
	if ( m_pLogoImage )
		m_pLogoImage->setColor( Color( 255, 255, 255, 0 ) );
	else
		gEngfuncs.Con_Printf( "Cannot load gfx/vgui/menu_logo.tga!\n" );

	m_pNewGameImage = vgui_LoadTGA( "gfx/vgui/menu/newgame.tga" );
	if ( m_pNewGameImage )
		m_pNewGameImage->setColor( Color( 255, 255, 255, 0 ) );

	m_pLoadGameImage = vgui_LoadTGA( "gfx/vgui/menu/loadgame.tga" );
	if ( m_pLoadGameImage )
		m_pNewGameImage->setColor( Color( 255, 255, 255, 0 ) );

	m_pCustomImage = vgui_LoadTGA( "gfx/vgui/menu/customc.tga" );
	if ( m_pCustomImage )
		m_pCustomImage->setColor( Color( 255, 255, 255, 0 ) );

	m_pJoinServerImage = vgui_LoadTGA( "gfx/vgui/menu/jserver.tga" );
	if ( m_pJoinServerImage )
		m_pJoinServerImage->setColor( Color( 255, 255, 255, 0 ) );

	m_pOptionsImage = vgui_LoadTGA( "gfx/vgui/menu/options.tga" );
	if ( m_pOptionsImage )
		m_pOptionsImage->setColor( Color( 255, 255, 255, 0 ) );

	m_pLanguageImage = vgui_LoadTGA( "gfx/vgui/menu/language.tga" );
	if ( m_pLanguageImage )
		m_pLanguageImage->setColor( Color( 255, 255, 255, 0 ) );

	m_pUnlockablesImage = vgui_LoadTGA( "gfx/vgui/menu/unlockables.tga" );
	if ( m_pUnlockablesImage )
		m_pUnlockablesImage->setColor( Color( 255, 255, 255, 0 ) );

	m_pExtrasImage = vgui_LoadTGA( "gfx/vgui/menu/extras.tga" );
	if ( m_pExtrasImage )
		m_pExtrasImage->setColor( Color( 255, 255, 255, 0 ) );

	m_pHostServerImage = vgui_LoadTGA( "gfx/vgui/menu/hserver.tga" );
	if ( m_pHostServerImage )
		m_pHostServerImage->setColor( Color( 255, 255, 255, 0 ) );

	m_pQuitImage = vgui_LoadTGA( "gfx/vgui/menu/quit.tga" );
	if ( m_pQuitImage )
		m_pQuitImage->setColor( Color( 255, 255, 255, 0 ) );

	m_pLogoTSImage = vgui_LoadTGA( "gfx/vgui/menu_logo_ts.tga" );
	if ( m_pLogoTSImage )
		m_pLogoTSImage->setColor( Color( 255, 255, 255, 0 ) );

	CSchemeManager *pSchemes = gViewPort->GetSchemeManager();
	SchemeHandle_t hCreditsScheme = pSchemes->getSchemeHandle( "credits" );
	m_pCreditsFont = pSchemes->getFont( hCreditsScheme );

	m_pVersionLabel = new Label( "", 5, 5, 256, 32 );
	m_pVersionLabel->setFont( m_pCreditsFont );
	m_pVersionLabel->setParent( this );
	m_pVersionLabel->setBgColor( 0, 0, 0, 255 );
	m_pVersionLabel->setFgColor( 255, 255, 255, 0 );
	m_pVersionLabel->setVisible( true );
	m_pVersionLabel->setText( GetVersion() );
	m_pVersionLabel->setContentAlignment( Label::a_west );

	m_pNewGameImage->getSize( wide, tall );
	m_pLogoTSImage->getSize( tsWide, tsTall );
	y = ScreenHeight - tall;
	x = ScreenWidth / 2 - wide / 2;

	m_pLogoImage->getSize( logoWide, logoTall );
	logoX = ScreenWidth / 2 - logoWide / 2;
	logoY = ScreenHeight / 2 - logoTall * 3;

	m_pLogo = new ImagePanel( m_pLogoImage );
	m_pLogo->setParent( this );
	m_pLogo->setPos( logoX, logoY );
	m_pLogo->setVisible( true );

	m_pLogoTS = new ImagePanel( m_pLogoTSImage );
	m_pLogoTS->setParent( this );
	m_pLogoTS->setPos( ScreenWidth - tsWide - 15, ScreenHeight - tsTall - 15 );
	m_pLogoTS->setVisible( true );

	m_pNewGame = new ImagePanel( m_pNewGameImage );
	m_pNewGame->setParent( this );
	m_pNewGame->setPos( x, y - tall * 10 );
	m_pNewGame->setVisible( true );

	m_pLoadGame = new ImagePanel( m_pLoadGameImage );
	m_pLoadGame->setParent( this );
	m_pLoadGame->setPos( x, y - tall * 9 );
	m_pLoadGame->setVisible( true );

	m_pCustom = new ImagePanel( m_pCustomImage );
	m_pCustom->setParent( this );
	m_pCustom->setPos( x, y - tall * 8 );
	m_pCustom->setVisible( true );

	m_pJoinServer = new ImagePanel( m_pJoinServerImage );
	m_pJoinServer->setParent( this );
	m_pJoinServer->setPos( x, y - tall * 7 );
	m_pJoinServer->setVisible( true );

	m_pOptions = new ImagePanel( m_pOptionsImage );
	m_pOptions->setParent( this );
	m_pOptions->setPos( x, y - tall * 2 );
	m_pOptions->setVisible( true );

	m_pLanguage = new ImagePanel( m_pLanguageImage );
	m_pLanguage->setParent( this );
	m_pLanguage->setPos( x, y - tall * 5 );
	m_pLanguage->setVisible( true );

	m_pUnlockables = new ImagePanel( m_pUnlockablesImage );
	m_pUnlockables->setParent( this );
	m_pUnlockables->setPos( x, y - tall * 4 );
	m_pUnlockables->setVisible( true );

	m_pExtras = new ImagePanel( m_pExtrasImage );
	m_pExtras->setParent( this );
	m_pExtras->setPos( x, y - tall * 3 );
	m_pExtras->setVisible( true );

	m_pHostServer = new ImagePanel( m_pHostServerImage );
	m_pHostServer->setParent( this );
	m_pHostServer->setPos( x, y - tall * 6 );
	m_pHostServer->setVisible( true );

	m_pQuit = new ImagePanel( m_pQuitImage );
	m_pQuit->setParent( this );
	m_pQuit->setPos( x, y - tall );
	m_pQuit->setVisible( true );

	m_pNewGameButton = new ColorButton( "", x, y - tall * 10, wide, tall, false, true );
	m_pNewGameButton->setParent( this );
	m_pNewGameButton->addActionSignal( new CGameMenuHandler_Command( this, 1 ) );

	m_pLoadGameButton = new ColorButton( "", x, y - tall * 9, wide, tall, false, true );
	m_pLoadGameButton->setParent( this );
	m_pLoadGameButton->addActionSignal( new CGameMenuHandler_Command( this, 2 ) );

	m_pCustomButton = new ColorButton( "", x, y - tall * 8, wide, tall, false, true );
	m_pCustomButton->setParent( this );
	m_pCustomButton->addActionSignal( new CGameMenuHandler_Command( this, 3 ) );

	m_pJoinServerButton = new ColorButton( "", x, y - tall * 7, wide, tall, false, true );
	m_pJoinServerButton->setParent( this );
	m_pJoinServerButton->addActionSignal( new CGameMenuHandler_Command( this, 4 ) );

	m_pOptionsButton = new ColorButton( "", x, y - tall * 2, wide, tall, false, true );
	m_pOptionsButton->setParent( this );
	m_pOptionsButton->addActionSignal( new CGameMenuHandler_Command( this, 5 ) );

	m_pLanguageButton = new ColorButton( "", x, y - tall * 5, wide, tall, false, true );
	m_pLanguageButton->setParent( this );
	m_pLanguageButton->addActionSignal( new CGameMenuHandler_Command( this, 6 ) );

	m_pUnlockablesButton = new ColorButton( "", x, y - tall * 4, wide, tall, false, true );
	m_pUnlockablesButton->setParent( this );
	m_pUnlockablesButton->addActionSignal( new CGameMenuHandler_Command( this, 7 ) );

	m_pExtrasButton = new ColorButton( "", x, y - tall * 3, wide, tall, false, true );
	m_pExtrasButton->setParent( this );
	m_pExtrasButton->addActionSignal( new CGameMenuHandler_Command( this, 8 ) );

	m_pHostServerButton = new ColorButton( "", x, y - tall * 6, wide, tall, false, true );
	m_pHostServerButton->setParent( this );
	m_pHostServerButton->addActionSignal( new CGameMenuHandler_Command( this, 9 ) );

	m_pQuitButton = new ColorButton( "", x, y - tall, wide, tall, false, true );
	m_pQuitButton->setParent( this );
	m_pQuitButton->addActionSignal( new CGameMenuHandler_Command( this, 10 ) );

	int iCancelWide = ScreenWidth;

	m_pCampaignList = new CTFScrollPanel( ScreenWidth / 2 - 350, ScreenHeight / 2 - 262, 700, 525 );
	m_pCampaignList->setParent( this );
	m_pCampaignList->setScrollBarVisible( false, false );

	ScrollBar *pScrollBar = m_pCampaignList->getVerticalScrollBar();
	pScrollBar->setSlider( new CCampaignSlider( 0, 700 - 1, 700, ( 525 - ( 700 * 2 ) ) + 2, true ) );

	for ( i = 0; i < MAX_CAMPAIGNS; i++ )
	{
		m_pCampaignButton[i] = new ColorButton( "N/A", 25, i * 32, 650, 32, false, true );
		m_pCampaignButton[i]->setParent( m_pCampaignList->getClient() );
		m_pCampaignButton[i]->addActionSignal( new CGameMenuHandler_Command( this, i + 50 ) );
		m_pCampaignButton[i]->setVisible( false );
		m_pCampaignButton[i]->setText( "N/A" );
		m_pCampaignButton[i]->setContentAlignment( Label::a_center );
	}

	m_pAuthorLabel = new Label( " ", 0, 0, ScreenWidth, 64 );
	m_pAuthorLabel->setParent( this );
	m_pAuthorLabel->setFgColor( 255, 255, 255, 0 );
	m_pAuthorLabel->setBgColor( 0, 0, 0, 255 );
	m_pAuthorLabel->setText( " " );
	m_pAuthorLabel->setFont( m_pCreditsFont );
	m_pAuthorLabel->setContentAlignment( Label::a_east );

	m_pCancelButton = new ColorButton( "Cancel", 0, ScreenHeight / 2 + 262, iCancelWide, 32, false, true );
	m_pCancelButton->setParent( this );
	m_pCancelButton->addActionSignal( new CGameMenuHandler_Command( this, 11 ) );
	m_pCancelButton->setVisible( false );
	m_pCancelButton->setContentAlignment( Label::a_center );
	m_pCancelButton->setUnArmedColor( 255, 255, 255, 0 );
	m_pCancelButton->setArmedColor( 255, 0, 0, 0 );

	setBgColor( 0, 0, 0, 255 );

	m_pCampaignList->setScrollValue( 0, 0 );
	m_pCampaignList->validate();

	Initialize();
}

void CGameMenu::ActionSignal( int cmd )
{
	int i;

	if ( !gEngfuncs.GetLocalPlayer() )
		return;

	int index = cmd - 50;

	if ( index >= 0 && index < 30 && m_Campaigns[index].bValid )
	{
		setVisible( false );
		gViewPort->UpdateCursorState();
		gEngfuncs.pfnPlaySoundByName( "ui/3d_click.wav", 0.5 );

		if ( gViewPort->m_pDifficulty )
		{
			m_pszCampaign = m_Campaigns[index].szFirstMap;
			gViewPort->m_pDifficulty->setVisible( true );
			gViewPort->UpdateCursorState();
		}
	}

	switch ( cmd )
	{
	case 1:
		setVisible( false );
		gViewPort->UpdateCursorState();
		gEngfuncs.pfnPlaySoundByName( "ui/3d_click.wav", 0.5 );

		if ( gViewPort->m_pDifficulty )
		{
			m_pszCampaign = NULL;
			gViewPort->m_pDifficulty->setVisible( true );
			gViewPort->UpdateCursorState();
		}
		break;
	case 2:
		setVisible( false );
		gViewPort->UpdateCursorState();
		gEngfuncs.pfnPlaySoundByName( "ui/3d_click.wav", 0.5 );

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

			gViewPort->m_pSaveLoad->m_iMode = 1;
			gViewPort->m_pSaveLoad->ReadAllSlots();
			gViewPort->m_pSaveLoad->setVisible( true );
			gViewPort->UpdateCursorState();
			gEngfuncs.pfnServerCmd( "freeze 1024" );
			gEngfuncs.pfnPlaySoundByName( "ui/3d_click.wav", 0.5 );
		}
		break;
	case 3:
		m_pNewGameButton->setVisible( false );
		m_pLoadGameButton->setVisible( false );
		m_pCustomButton->setVisible( false );
		m_pJoinServerButton->setVisible( false );
		m_pOptionsButton->setVisible( false );
		m_pLanguageButton->setVisible( false );
		m_pUnlockablesButton->setVisible( false );
		m_pExtrasButton->setVisible( false );
		m_pHostServerButton->setVisible( false );
		m_pQuitButton->setVisible( false );
		m_pLogo->setVisible( false );
		m_pNewGame->setVisible( false );
		m_pLoadGame->setVisible( false );
		m_pCustom->setVisible( false );
		m_pJoinServer->setVisible( false );
		m_pOptions->setVisible( false );
		m_pLanguage->setVisible( false );
		m_pUnlockables->setVisible( false );
		m_pExtras->setVisible( false );
		m_pHostServer->setVisible( false );
		m_pQuit->setVisible( false );
		m_pCancelButton->setVisible( true );
		m_pCampaignList->setVisible( true );
		gEngfuncs.pfnPlaySoundByName( "ui/3d_click.wav", 0.5 );

		for ( i = 0; i < MAX_CAMPAIGNS; i++ )
			m_pCampaignButton[i]->setVisible( true );

		ReadCampaigns();
		m_pCampaignList->validate();
		break;
	case 4:
		gEngfuncs.pfnServerCmd( "optionshack 0" );
		break;
	case 5:
		gEngfuncs.pfnServerCmd( "optionshack 2" );
		break;
	case 6:
		setVisible( false );
		gViewPort->UpdateCursorState();
		gEngfuncs.pfnPlaySoundByName( "ui/3d_click.wav", 0.5 );

		if ( gViewPort->m_pLanguage )
		{
			gViewPort->m_pLanguage->setVisible( true );
			gViewPort->UpdateCursorState();
		}
		break;
	case 7:
		if ( gViewPort && gViewPort->m_pUnlockables )
		{
			setVisible( false );
			gViewPort->UpdateCursorState();
			gViewPort->m_pUnlockables->setVisible( true );
			gViewPort->UpdateCursorState();
			gEngfuncs.pfnPlaySoundByName( "ui/3d_click.wav", 0.5 );
		}
		break;
	case 8:
		if ( gViewPort->m_pVGUIExtraMenu )
		{
			setVisible( false );
			gViewPort->UpdateCursorState();
			gViewPort->m_pVGUIExtraMenu->setVisible( true );
			gViewPort->UpdateCursorState();
		}
		break;
	case 9:
		gEngfuncs.pfnServerCmd( "optionshack 1" );
		break;
	case 10:
		setVisible( false );
		gEngfuncs.pfnClientCmd( "quit" );
		gViewPort->UpdateCursorState();
		break;
	case 11:
		m_pNewGameButton->setVisible( true );
		m_pLoadGameButton->setVisible( true );
		m_pCustomButton->setVisible( true );
		m_pJoinServerButton->setVisible( true );
		m_pOptionsButton->setVisible( true );
		m_pLanguageButton->setVisible( true );
		m_pUnlockablesButton->setVisible( true );
		m_pExtrasButton->setVisible( true );
		m_pHostServerButton->setVisible( true );
		m_pQuitButton->setVisible( true );
		m_pLogo->setVisible( true );
		m_pNewGame->setVisible( true );
		m_pLoadGame->setVisible( true );
		m_pCustom->setVisible( true );
		m_pJoinServer->setVisible( true );
		m_pOptions->setVisible( true );
		m_pLanguage->setVisible( true );
		m_pUnlockables->setVisible( true );
		m_pExtras->setVisible( true );
		m_pHostServer->setVisible( true );
		m_pQuit->setVisible( true );
		m_pCancelButton->setVisible( false );
		m_pCampaignList->setVisible( false );
		gEngfuncs.pfnPlaySoundByName( "ui/3d_click.wav", 0.5 );

		for ( i = 0; i < MAX_CAMPAIGNS; i++ )
			m_pCampaignButton[i]->setVisible( false );
		break;
	}
}

const char *GetVersion()
{
	return "1.6a";
}

void CGameMenu::Initialize()
{
	m_bUnknownC0 = false;
	memset( m_Campaigns, 0, sizeof( m_Campaigns ) );

	m_pCampaignList->setScrollValue( 0, 0 );
	m_pszCampaign = NULL;

	setVisible( false );
}

extern bool g_bCoFMainMenu;
void SetTouchMenuMode( bool bMenu );

int MsgGameMenu( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	g_bCoFMainMenu = true;
	SetTouchMenuMode( true );

	if ( gViewPort->m_pGameMenu )
	{
		gViewPort->m_pGameMenu->m_bNewGameArmed = false;
		gViewPort->m_pGameMenu->m_bLoadGameArmed = false;
		gViewPort->m_pGameMenu->m_bCustomArmed = false;
		gViewPort->m_pGameMenu->m_bJoinServerArmed = false;
		gViewPort->m_pGameMenu->m_bOptionsArmed = false;
		gViewPort->m_pGameMenu->m_bLanguageArmed = false;
		gViewPort->m_pGameMenu->m_bUnlockablesArmed = false;
		gViewPort->m_pGameMenu->m_bExtrasArmed = false;
		gViewPort->m_pGameMenu->m_bHostServerArmed = false;
		gViewPort->m_pGameMenu->m_bQuitArmed = false;

		gViewPort->m_pGameMenu->m_pNewGame->setImage( vgui_LoadTGA( "gfx/vgui/menu/newgame.tga" ) );
		gViewPort->m_pGameMenu->m_pLoadGame->setImage( vgui_LoadTGA( "gfx/vgui/menu/loadgame.tga" ) );
		gViewPort->m_pGameMenu->m_pCustom->setImage( vgui_LoadTGA( "gfx/vgui/menu/customc.tga" ) );
		gViewPort->m_pGameMenu->m_pJoinServer->setImage( vgui_LoadTGA( "gfx/vgui/menu/jserver.tga" ) );
		gViewPort->m_pGameMenu->m_pOptions->setImage( vgui_LoadTGA( "gfx/vgui/menu/options.tga" ) );
		gViewPort->m_pGameMenu->m_pLanguage->setImage( vgui_LoadTGA( "gfx/vgui/menu/language.tga" ) );
		gViewPort->m_pGameMenu->m_pUnlockables->setImage( vgui_LoadTGA( "gfx/vgui/menu/unlockables.tga" ) );
		gViewPort->m_pGameMenu->m_pExtras->setImage( vgui_LoadTGA( "gfx/vgui/menu/extras.tga" ) );
		gViewPort->m_pGameMenu->m_pHostServer->setImage( vgui_LoadTGA( "gfx/vgui/menu/hserver.tga" ) );
		gViewPort->m_pGameMenu->m_pQuit->setImage( vgui_LoadTGA( "gfx/vgui/menu/quit.tga" ) );

		gViewPort->m_pGameMenu->setVisible( true );
		gViewPort->UpdateCursorState();
		gEngfuncs.pfnServerCmd( "freeze 1024" );
	}

	return 1;
}

void CGameMenu::ParseCampaign( char *pszFile, int index )
{
	char token[1024];
	char szPath[256];
	char *pFile;

	gEngfuncs.Con_Printf( "PARSING CUSTOM CAMPAIGN: %s\n", pszFile );

	strcpy( szPath, "maps/" );
	strcat( szPath, pszFile );

	pFile = (char *)gEngfuncs.COM_LoadFile( szPath, 5, NULL );

	if ( !pFile )
	{
		gEngfuncs.Con_Printf( "PARSING CUSTOM CAMPAIGN: Couldn't open campaign file %s\n", pszFile );
		return;
	}

	for ( pFile = gEngfuncs.COM_ParseFile( pFile, token ); pFile; pFile = gEngfuncs.COM_ParseFile( pFile, token ) )
	{
		if ( !stricmp( token, "firstmap" ) )
		{
			pFile = gEngfuncs.COM_ParseFile( pFile, token );
			strncpy( m_Campaigns[index].szFirstMap, token, 64 );
			m_Campaigns[index].bValid = true;
		}
		else if ( !stricmp( token, "author" ) )
		{
			pFile = gEngfuncs.COM_ParseFile( pFile, token );
			strncpy( m_Campaigns[index].szAuthor, token, 64 );
			m_Campaigns[index].bValid = true;
		}
		else if ( !stricmp( token, "description" ) )
		{
			pFile = gEngfuncs.COM_ParseFile( pFile, token );
			strncpy( m_Campaigns[index].szDescription, token, 64 );
			m_Campaigns[index].bValid = true;

			if ( m_pCampaignButton[index] )
			{
				int w, h;
				int x, y;
				int wide, tall;

				m_pCreditsFont->getTextSize( token, w, h );
				m_pCampaignButton[index]->getPos( x, y );
				m_pCampaignButton[index]->setText( token );
				m_pCampaignButton[index]->setArmedColor( 255, 0, 0, 0 );
				m_pCampaignButton[index]->setUnArmedColor( 255, 255, 255, 0 );
				m_pCampaignButton[index]->getSize( wide, tall );
				w = w * 1.2;
				m_pCampaignButton[index]->setContentAlignment( Label::a_center );
			}
		}
	}

	if ( !m_Campaigns[index].bValid )
		gEngfuncs.Con_Printf( "PARSING CUSTOM CAMPAIGN: Couldn't parse data from campaign file %s\n", pszFile );
}

void CGameMenu::ReadCampaigns()
{
	int i;

	for ( i = 0; i < MAX_CAMPAIGNS; i++ )
	{
		m_pCampaignButton[i]->setText( "N/A" );
		m_pCampaignButton[i]->setUnArmedColor( 255, 255, 255, 0 );
		m_pCampaignButton[i]->setArmedColor( 255, 255, 255, 0 );
	}

	m_pszCampaign = NULL;
	m_bUnknownC0 = false;
	memset( m_Campaigns, 0, sizeof( m_Campaigns ) );

	char szDir[1024];
	strcpy( szDir, gEngfuncs.pfnGetGameDirectory() );
	strcat( szDir, "/maps/" );

	std::string dir = szDir;
	std::string ext = "*.custom";
	std::string search = dir + ext;

	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile( search.c_str(), &fd );

	if ( hFind != INVALID_HANDLE_VALUE )
	{
		do
		{
			std::string file = dir + fd.cFileName;
			struct stat st;

			if ( !stat( file.c_str(), &st ) )
			{
				for ( i = 0; i < MAX_CAMPAIGNS; i++ )
				{
					if ( !m_Campaigns[i].bValid )
					{
						ParseCampaign( fd.cFileName, i );
						break;
					}
				}
			}
		} while ( FindNextFile( hFind, &fd ) > 0 );
	}
}

void CGameMenu::ResetButtonPositions()
{
	int wide, tall;
	int tsWide, tsTall;
	int logoWide, logoTall;
	int x, y;

	m_pNewGameImage->getSize( wide, tall );
	m_pLogoTSImage->getSize( tsWide, tsTall );
	y = ScreenHeight - tall;
	x = ScreenWidth / 2 - wide / 2;

	m_pLogoImage->getSize( logoWide, logoTall );

	m_pNewGame->setPos( x, y - tall * 10 );
	m_pLoadGame->setPos( x, y - tall * 9 );
	m_pCustom->setPos( x, y - tall * 8 );
	m_pJoinServer->setPos( x, y - tall * 7 );
	m_pOptions->setPos( x, y - tall * 2 );
	m_pLanguage->setPos( x, y - tall * 5 );
	m_pUnlockables->setPos( x, y - tall * 4 );
	m_pExtras->setPos( x, y - tall * 3 );
	m_pHostServer->setPos( x, y - tall * 6 );
	m_pQuit->setPos( x, y - tall );
}

void VGUI_GameMenuInit()
{
	gEngfuncs.pfnHookUserMsg( "GameMenu", MsgGameMenu );
}

void CCampaignSlider::paintBackground()
{
	int wide, tall, nobx, noby;

	getPaintSize( wide, tall );
	getNobPos( nobx, noby );

	drawSetColor( 128, 128, 128, 150 );
	drawFilledRect( 0, 0, wide, tall );

	drawSetColor( 200, 200, 200, 0 );
	drawFilledRect( 0, nobx, wide, noby );

	drawSetColor( 0, 0, 0, 150 );
	drawOutlinedRect( 0, nobx, wide - 1, noby - 1 );

	drawSetColor( 255, 255, 255, 150 );
	drawOutlinedRect( 1, nobx + 1, wide, noby );

	drawSetColor( 0, 0, 0, 0 );
	drawOutlinedRect( 0, nobx, wide, noby );
}

void CGameMenu::setVisible( bool state )
{
	if ( state )
	{
		ResetButtonPositions();
		IrrKlangSetLoop( true );
		IrrKlangPlayMP3( "gamestartup.mp3" );

		m_iArmedCampaign = -1;
		m_bCampaignArmed = true;

		m_pNewGameButton->setVisible( true );
		m_pLoadGameButton->setVisible( true );
		m_pCustomButton->setVisible( true );
		m_pJoinServerButton->setVisible( true );
		m_pOptionsButton->setVisible( true );
		m_pLanguageButton->setVisible( true );
		m_pUnlockablesButton->setVisible( true );
		m_pExtrasButton->setVisible( true );
		m_pHostServerButton->setVisible( true );
		m_pQuitButton->setVisible( true );
		m_pLogo->setVisible( true );
		m_pNewGame->setVisible( true );
		m_pLoadGame->setVisible( true );
		m_pCustom->setVisible( true );
		m_pJoinServer->setVisible( true );
		m_pOptions->setVisible( true );
		m_pLanguage->setVisible( true );
		m_pUnlockables->setVisible( true );
		m_pExtras->setVisible( true );
		m_pHostServer->setVisible( true );
		m_pQuit->setVisible( true );
		m_pCancelButton->setVisible( false );
		m_pCampaignList->setVisible( false );
		gEngfuncs.pfnPlaySoundByName( "ui/3d_click.wav", 0.5 );

		for ( int i = 0; i < MAX_CAMPAIGNS; i++ )
			m_pCampaignButton[i]->setVisible( false );
	}

	Panel::setVisible( state );
}

void CGameMenu::solve()
{
	char szText[256];

	if ( !gEngfuncs.GetLocalPlayer() )
		return;

	m_pLogoImage->setColor( Color( 255, 255, 255, gEngfuncs.pfnRandomLong( 0, 50 ) ) );

	if ( m_pNewGameButton->isArmed() )
	{
		if ( !m_bNewGameArmed )
		{
			m_pNewGame->setImage( vgui_LoadTGA( "gfx/vgui/menu/newgame_mo.tga" ) );
			gEngfuncs.pfnPlaySoundByName( "ui/3d_rollover.wav", 0.5 );
			m_bNewGameArmed = true;
			m_pNewGame->getPos( m_iArmedX, m_iArmedY );
		}
		else
		{
			m_pNewGame->setPos( m_iArmedX + gEngfuncs.pfnRandomLong( -3, 3 ), m_iArmedY );
		}
	}
	else if ( m_bNewGameArmed )
	{
		m_pNewGame->setImage( vgui_LoadTGA( "gfx/vgui/menu/newgame.tga" ) );
		m_bNewGameArmed = false;
		ResetButtonPositions();
	}

	if ( m_pLoadGameButton->isArmed() )
	{
		if ( !m_bLoadGameArmed )
		{
			m_pLoadGame->setImage( vgui_LoadTGA( "gfx/vgui/menu/loadgame_mo.tga" ) );
			gEngfuncs.pfnPlaySoundByName( "ui/3d_rollover.wav", 0.5 );
			m_bLoadGameArmed = true;
			m_pLoadGame->getPos( m_iArmedX, m_iArmedY );
		}
		else
		{
			m_pLoadGame->setPos( m_iArmedX + gEngfuncs.pfnRandomLong( -3, 3 ), m_iArmedY );
		}
	}
	else if ( m_bLoadGameArmed )
	{
		m_pLoadGame->setImage( vgui_LoadTGA( "gfx/vgui/menu/loadgame.tga" ) );
		m_bLoadGameArmed = false;
		ResetButtonPositions();
	}

	if ( m_pCustomButton->isArmed() )
	{
		if ( !m_bCustomArmed )
		{
			m_pCustom->setImage( vgui_LoadTGA( "gfx/vgui/menu/customc_mo.tga" ) );
			gEngfuncs.pfnPlaySoundByName( "ui/3d_rollover.wav", 0.5 );
			m_bCustomArmed = true;
			m_pCustom->getPos( m_iArmedX, m_iArmedY );
		}
		else
		{
			m_pCustom->setPos( m_iArmedX + gEngfuncs.pfnRandomLong( -3, 3 ), m_iArmedY );
		}
	}
	else if ( m_bCustomArmed )
	{
		m_pCustom->setImage( vgui_LoadTGA( "gfx/vgui/menu/customc.tga" ) );
		m_bCustomArmed = false;
		ResetButtonPositions();
	}

	if ( m_pJoinServerButton->isArmed() )
	{
		if ( !m_bJoinServerArmed )
		{
			m_pJoinServer->setImage( vgui_LoadTGA( "gfx/vgui/menu/jserver_mo.tga" ) );
			gEngfuncs.pfnPlaySoundByName( "ui/3d_rollover.wav", 0.5 );
			m_bJoinServerArmed = true;
			m_pJoinServer->getPos( m_iArmedX, m_iArmedY );
		}
		else
		{
			m_pJoinServer->setPos( m_iArmedX + gEngfuncs.pfnRandomLong( -3, 3 ), m_iArmedY );
		}
	}
	else if ( m_bJoinServerArmed )
	{
		m_pJoinServer->setImage( vgui_LoadTGA( "gfx/vgui/menu/jserver.tga" ) );
		m_bJoinServerArmed = false;
		ResetButtonPositions();
	}

	if ( m_pOptionsButton->isArmed() )
	{
		if ( !m_bOptionsArmed )
		{
			m_pOptions->setImage( vgui_LoadTGA( "gfx/vgui/menu/options_mo.tga" ) );
			gEngfuncs.pfnPlaySoundByName( "ui/3d_rollover.wav", 0.5 );
			m_bOptionsArmed = true;
			m_pOptions->getPos( m_iArmedX, m_iArmedY );
		}
		else
		{
			m_pOptions->setPos( m_iArmedX + gEngfuncs.pfnRandomLong( -3, 3 ), m_iArmedY );
		}
	}
	else if ( m_bOptionsArmed )
	{
		m_pOptions->setImage( vgui_LoadTGA( "gfx/vgui/menu/options.tga" ) );
		m_bOptionsArmed = false;
		ResetButtonPositions();
	}

	if ( m_pLanguageButton->isArmed() )
	{
		if ( !m_bLanguageArmed )
		{
			m_pLanguage->setImage( vgui_LoadTGA( "gfx/vgui/menu/language_mo.tga" ) );
			gEngfuncs.pfnPlaySoundByName( "ui/3d_rollover.wav", 0.5 );
			m_bLanguageArmed = true;
			m_pLanguage->getPos( m_iArmedX, m_iArmedY );
		}
		else
		{
			m_pLanguage->setPos( m_iArmedX + gEngfuncs.pfnRandomLong( -3, 3 ), m_iArmedY );
		}
	}
	else if ( m_bLanguageArmed )
	{
		m_pLanguage->setImage( vgui_LoadTGA( "gfx/vgui/menu/language.tga" ) );
		m_bLanguageArmed = false;
		ResetButtonPositions();
	}

	if ( m_pUnlockablesButton->isArmed() )
	{
		if ( !m_bUnlockablesArmed )
		{
			m_pUnlockables->setImage( vgui_LoadTGA( "gfx/vgui/menu/unlockables_mo.tga" ) );
			gEngfuncs.pfnPlaySoundByName( "ui/3d_rollover.wav", 0.5 );
			m_bUnlockablesArmed = true;
			m_pUnlockables->getPos( m_iArmedX, m_iArmedY );
		}
		else
		{
			m_pUnlockables->setPos( m_iArmedX + gEngfuncs.pfnRandomLong( -3, 3 ), m_iArmedY );
		}
	}
	else if ( m_bUnlockablesArmed )
	{
		m_pUnlockables->setImage( vgui_LoadTGA( "gfx/vgui/menu/unlockables.tga" ) );
		m_bUnlockablesArmed = false;
		ResetButtonPositions();
	}

	if ( m_pExtrasButton->isArmed() )
	{
		if ( !m_bExtrasArmed )
		{
			m_pExtras->setImage( vgui_LoadTGA( "gfx/vgui/menu/extras_mo.tga" ) );
			gEngfuncs.pfnPlaySoundByName( "ui/3d_rollover.wav", 0.5 );
			m_bExtrasArmed = true;
			m_pExtras->getPos( m_iArmedX, m_iArmedY );
		}
		else
		{
			m_pExtras->setPos( m_iArmedX + gEngfuncs.pfnRandomLong( -3, 3 ), m_iArmedY );
		}
	}
	else if ( m_bExtrasArmed )
	{
		m_pExtras->setImage( vgui_LoadTGA( "gfx/vgui/menu/extras.tga" ) );
		m_bExtrasArmed = false;
		ResetButtonPositions();
	}

	if ( m_pHostServerButton->isArmed() )
	{
		if ( !m_bHostServerArmed )
		{
			m_pHostServer->setImage( vgui_LoadTGA( "gfx/vgui/menu/hserver_mo.tga" ) );
			gEngfuncs.pfnPlaySoundByName( "ui/3d_rollover.wav", 0.5 );
			m_bHostServerArmed = true;
			m_pHostServer->getPos( m_iArmedX, m_iArmedY );
		}
		else
		{
			m_pHostServer->setPos( m_iArmedX + gEngfuncs.pfnRandomLong( -3, 3 ), m_iArmedY );
		}
	}
	else if ( m_bHostServerArmed )
	{
		m_pHostServer->setImage( vgui_LoadTGA( "gfx/vgui/menu/hserver.tga" ) );
		m_bHostServerArmed = false;
		ResetButtonPositions();
	}

	if ( m_pQuitButton->isArmed() )
	{
		if ( !m_bQuitArmed )
		{
			m_pQuit->setImage( vgui_LoadTGA( "gfx/vgui/menu/quit_mo.tga" ) );
			gEngfuncs.pfnPlaySoundByName( "ui/3d_rollover.wav", 0.5 );
			m_bQuitArmed = true;
			m_pQuit->getPos( m_iArmedX, m_iArmedY );
		}
		else
		{
			m_pQuit->setPos( m_iArmedX + gEngfuncs.pfnRandomLong( -3, 3 ), m_iArmedY );
		}
	}
	else if ( m_bQuitArmed )
	{
		m_pQuit->setImage( vgui_LoadTGA( "gfx/vgui/menu/quit.tga" ) );
		m_bQuitArmed = false;
		ResetButtonPositions();
	}

	bool bArmed = false;
	m_iArmedCampaign = -1;

	for ( int i = 0; i < MAX_CAMPAIGNS; i++ )
	{
		if ( m_pCampaignButton[i]->isArmed() && m_iArmedCampaign != i )
		{
			m_iArmedCampaign = i;

			if ( m_Campaigns[i].szAuthor && m_Campaigns[i].bValid )
			{
				sprintf( szText, "Author: %s", m_Campaigns[i].szAuthor );
				m_pAuthorLabel->setText( szText );
				m_pAuthorLabel->setVisible( true );
			}
			else
			{
				m_pAuthorLabel->setText( "" );
				m_pAuthorLabel->setVisible( false );
			}

			m_bCampaignArmed = true;
		}

		if ( m_pCampaignButton[i]->isArmed() )
		{
			m_iArmedCampaign = i;
			bArmed = true;
		}
	}

	if ( !m_bCampaignArmed && m_iArmedCampaign >= 0 )
		m_iArmedCampaign = -1;

	if ( !bArmed && m_pAuthorLabel->isVisible() )
	{
		m_pAuthorLabel->setText( "" );
		m_pAuthorLabel->setVisible( false );
	}

	Panel::solve();
}

CGameMenu::~CGameMenu( void )
{
	delete m_pLogoImage;
}
