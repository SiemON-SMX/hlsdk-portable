// ======================================
// ======================================

#include "hud.h"
#include "cl_util.h"
#include "vgui_TeamFortressViewport.h"
#include "vguiextramenu.h"
#include "gamemenu.h"
#include "../game_shared/vgui_loadtga.h"

//=========================================================
//=========================================================
typedef unsigned long long uint64;
typedef uint64 SteamAPICall_t;

class CSteamID
{
private:
	uint64 m_unAll64Bits;
};

struct FriendGameInfo_t;

class ISteamFriends
{
public:
	virtual const char *GetPersonaName() = 0;
	virtual SteamAPICall_t SetPersonaName( const char *pchPersonaName ) = 0;
	virtual int GetPersonaState() = 0;
	virtual int GetFriendCount( int iFriendFlags ) = 0;
	virtual CSteamID GetFriendByIndex( int iFriend, int iFriendFlags ) = 0;
	virtual int GetFriendRelationship( CSteamID steamIDFriend ) = 0;
	virtual int GetFriendPersonaState( CSteamID steamIDFriend ) = 0;
	virtual const char *GetFriendPersonaName( CSteamID steamIDFriend ) = 0;
	virtual bool GetFriendGamePlayed( CSteamID steamIDFriend, FriendGameInfo_t *pFriendGameInfo ) = 0;
	virtual const char *GetFriendPersonaNameHistory( CSteamID steamIDFriend, int iPersonaName ) = 0;
	virtual bool HasFriend( CSteamID steamIDFriend, int iFriendFlags ) = 0;
	virtual int GetClanCount() = 0;
	virtual CSteamID GetClanByIndex( int iClan ) = 0;
	virtual const char *GetClanName( CSteamID steamIDClan ) = 0;
	virtual const char *GetClanTag( CSteamID steamIDClan ) = 0;
	virtual bool GetClanActivityCounts( CSteamID steamIDClan, int *pnOnline, int *pnInGame, int *pnChatting ) = 0;
	virtual SteamAPICall_t DownloadClanActivityCounts( CSteamID *psteamIDClans, int cClansToRequest ) = 0;
	virtual int GetFriendCountFromSource( CSteamID steamIDSource ) = 0;
	virtual CSteamID GetFriendFromSourceByIndex( CSteamID steamIDSource, int iFriend ) = 0;
	virtual bool IsUserInSource( CSteamID steamIDUser, CSteamID steamIDSource ) = 0;
	virtual void SetInGameVoiceSpeaking( CSteamID steamIDUser, bool bSpeaking ) = 0;
	virtual void ActivateGameOverlay( const char *pchDialog ) = 0;
	virtual void ActivateGameOverlayToUser( const char *pchDialog, CSteamID steamID ) = 0;
	virtual void ActivateGameOverlayToWebPage( const char *pchURL ) = 0;
};

static inline ISteamFriends * SteamFriends() { return 0; }
//=========================================================

CVGUIExtraMenu::CVGUIExtraMenu() : Panel( ScreenWidth / 2 - 400, ScreenHeight / 2 - 300, 800, 600 )
{
	int wide, tall;
	int cancelWide, cancelTall;

	m_pDonateImage = vgui_LoadTGA( "gfx/vgui/menu/donate.tga" );
	m_pDonateImage->setColor( Color( 255, 255, 255, 0 ) );

	m_pSoundtrack1Image = vgui_LoadTGA( "gfx/vgui/menu/soundtrack1.tga" );
	m_pSoundtrack1Image->setColor( Color( 255, 255, 255, 0 ) );

	m_pSoundtrack2Image = vgui_LoadTGA( "gfx/vgui/menu/soundtrack2.tga" );
	m_pSoundtrack2Image->setColor( Color( 255, 255, 255, 0 ) );

	m_pClothesImage = vgui_LoadTGA( "gfx/vgui/menu/clothes.tga" );
	m_pClothesImage->setColor( Color( 255, 255, 255, 0 ) );

	m_pAoMDCImage = vgui_LoadTGA( "gfx/vgui/menu/aomdc.tga" );
	m_pAoMDCImage->setColor( Color( 255, 255, 255, 0 ) );

	m_pFacebookImage = vgui_LoadTGA( "gfx/vgui/menu/facebook.tga" );
	m_pFacebookImage->setColor( Color( 255, 255, 255, 0 ) );

	m_pCancelImage = vgui_LoadTGA( "gfx/vgui/menu/cancel.tga" );
	m_pCancelImage->setColor( Color( 255, 255, 255, 0 ) );

	m_pDonateImage->getSize( wide, tall );
	m_pCancelImage->getSize( cancelWide, cancelTall );

	m_pDonate = new ImagePanel( m_pDonateImage );
	m_pDonate->setParent( this );
	m_pDonate->setPos( 400 - wide / 2, 0 );
	m_pDonate->setVisible( true );

	m_pSoundtrack1 = new ImagePanel( m_pSoundtrack1Image );
	m_pSoundtrack1->setParent( this );
	m_pSoundtrack1->setPos( 400 - wide / 2, tall );
	m_pSoundtrack1->setVisible( true );

	m_pSoundtrack2 = new ImagePanel( m_pSoundtrack2Image );
	m_pSoundtrack2->setParent( this );
	m_pSoundtrack2->setPos( 400 - wide / 2, tall * 2 );
	m_pSoundtrack2->setVisible( true );

	m_pClothes = new ImagePanel( m_pClothesImage );
	m_pClothes->setParent( this );
	m_pClothes->setPos( 400 - wide / 2, tall * 3 );
	m_pClothes->setVisible( true );

	m_pAoMDC = new ImagePanel( m_pAoMDCImage );
	m_pAoMDC->setParent( this );
	m_pAoMDC->setPos( 400 - wide / 2, tall * 4 );
	m_pAoMDC->setVisible( true );

	m_pFacebook = new ImagePanel( m_pFacebookImage );
	m_pFacebook->setParent( this );
	m_pFacebook->setPos( 400 - wide / 2, tall * 5 );
	m_pFacebook->setVisible( true );

	m_pCancel = new ImagePanel( m_pCancelImage );
	m_pCancel->setParent( this );
	m_pCancel->setPos( 400 - cancelWide / 2, 600 - cancelTall );
	m_pCancel->setVisible( true );

	m_pDonateButton = new ColorButton( "", 400 - wide / 2, 0, wide, tall, false, true );
	m_pDonateButton->setParent( this );
	m_pDonateButton->addActionSignal( new CVGUIExtraMenuHandler_Command( this, 2 ) );

	m_pSoundtrack1Button = new ColorButton( "", 400 - wide / 2, tall, wide, tall, false, true );
	m_pSoundtrack1Button->setParent( this );
	m_pSoundtrack1Button->addActionSignal( new CVGUIExtraMenuHandler_Command( this, 3 ) );

	m_pSoundtrack2Button = new ColorButton( "", 400 - wide / 2, tall * 2, wide, tall, false, true );
	m_pSoundtrack2Button->setParent( this );
	m_pSoundtrack2Button->addActionSignal( new CVGUIExtraMenuHandler_Command( this, 4 ) );

	m_pClothesButton = new ColorButton( "", 400 - wide / 2, tall * 3, wide, tall, false, true );
	m_pClothesButton->setParent( this );
	m_pClothesButton->addActionSignal( new CVGUIExtraMenuHandler_Command( this, 5 ) );

	m_pAoMDCButton = new ColorButton( "", 400 - wide / 2, tall * 4, wide, tall, false, true );
	m_pAoMDCButton->setParent( this );
	m_pAoMDCButton->addActionSignal( new CVGUIExtraMenuHandler_Command( this, 6 ) );

	m_pFacebookButton = new ColorButton( "", 400 - wide / 2, tall * 5, wide, tall, false, true );
	m_pFacebookButton->setParent( this );
	m_pFacebookButton->addActionSignal( new CVGUIExtraMenuHandler_Command( this, 7 ) );

	m_pCancelButton = new ColorButton( "", 400 - cancelWide / 2, 600 - cancelTall, cancelWide, cancelTall, false, true );
	m_pCancelButton->setParent( this );
	m_pCancelButton->addActionSignal( new CVGUIExtraMenuHandler_Command( this, 1 ) );

	setBgColor( 0, 0, 0, 255 );

	Initialize();
}

CVGUIExtraMenu::~CVGUIExtraMenu()
{
	delete m_pDonateImage;
	delete m_pSoundtrack1Image;
	delete m_pSoundtrack2Image;
	delete m_pClothesImage;
	delete m_pAoMDCImage;
	delete m_pFacebookImage;
	delete m_pCancelImage;
}

void CVGUIExtraMenu::ActionSignal( int cmd )
{
	switch ( cmd )
	{
	case 1:
		setVisible( false );
		gViewPort->UpdateCursorState();

		if ( gViewPort && gViewPort->m_pGameMenu )
		{
			gViewPort->m_pGameMenu->setVisible( true );
			gViewPort->UpdateCursorState();
		}
		break;
	case 2:
		if ( SteamFriends() )
			SteamFriends()->ActivateGameOverlayToWebPage( "http://www.cry-of-fear.com/donate.php" );

		gEngfuncs.pfnPlaySoundByName( "ui/3d_click.wav", 1 );
		break;
	case 3:
		if ( SteamFriends() )
			SteamFriends()->ActivateGameOverlayToWebPage( "http://www.cdbaby.com/cd/andreasronnberg" );

		gEngfuncs.pfnPlaySoundByName( "ui/3d_click.wav", 1 );
		break;
	case 4:
		if ( SteamFriends() )
			SteamFriends()->ActivateGameOverlayToWebPage( "http://www.cdbaby.com/cd/andreasronnberg2" );

		gEngfuncs.pfnPlaySoundByName( "ui/3d_click.wav", 1 );
		break;
	case 5:
		if ( SteamFriends() )
			SteamFriends()->ActivateGameOverlayToWebPage( "http://teampsykskallar.spreadshirt.se/" );

		gEngfuncs.pfnPlaySoundByName( "ui/3d_click.wav", 1 );
		break;
	case 6:
		if ( SteamFriends() )
			SteamFriends()->ActivateGameOverlayToWebPage( "http://www.moddb.com/mods/afraid-of-monsters-dc" );

		gEngfuncs.pfnPlaySoundByName( "ui/3d_click.wav", 1 );
		break;
	case 7:
		if ( SteamFriends() )
			SteamFriends()->ActivateGameOverlayToWebPage( "http://www.facebook.com/CryOfFearOfficial" );

		gEngfuncs.pfnPlaySoundByName( "ui/3d_click.wav", 1 );
		break;
	}
}

void CVGUIExtraMenu::Initialize()
{
	setVisible( false );
}

void CVGUIExtraMenu::setVisible( bool state )
{
	if ( state )
	{
		m_bDonateArmed = false;
		m_bSoundtrack1Armed = false;
		m_bSoundtrack2Armed = false;
		m_bClothesArmed = false;
		m_bAoMDCArmed = false;
		m_bFacebookArmed = false;
		m_bCancelArmed = false;
	}

	Panel::setVisible( state );
}

void CVGUIExtraMenu::solve()
{
	if ( m_pDonateButton->isArmed() )
	{
		if ( !m_bDonateArmed )
		{
			m_pDonate->setImage( vgui_LoadTGA( "gfx/vgui/menu/donate_mo.tga" ) );
			gEngfuncs.pfnPlaySoundByName( "ui/3d_rollover.wav", 1 );
			m_bDonateArmed = true;
		}
	}
	else if ( m_bDonateArmed )
	{
		m_pDonate->setImage( vgui_LoadTGA( "gfx/vgui/menu/donate.tga" ) );
		m_bDonateArmed = false;
	}

	if ( m_pSoundtrack1Button->isArmed() )
	{
		if ( !m_bSoundtrack1Armed )
		{
			m_pSoundtrack1->setImage( vgui_LoadTGA( "gfx/vgui/menu/soundtrack1_mo.tga" ) );
			gEngfuncs.pfnPlaySoundByName( "ui/3d_rollover.wav", 1 );
			m_bSoundtrack1Armed = true;
		}
	}
	else if ( m_bSoundtrack1Armed )
	{
		m_pSoundtrack1->setImage( vgui_LoadTGA( "gfx/vgui/menu/soundtrack1.tga" ) );
		m_bSoundtrack1Armed = false;
	}

	if ( m_pSoundtrack2Button->isArmed() )
	{
		if ( !m_bSoundtrack2Armed )
		{
			m_pSoundtrack2->setImage( vgui_LoadTGA( "gfx/vgui/menu/soundtrack2_mo.tga" ) );
			gEngfuncs.pfnPlaySoundByName( "ui/3d_rollover.wav", 1 );
			m_bSoundtrack2Armed = true;
		}
	}
	else if ( m_bSoundtrack2Armed )
	{
		m_pSoundtrack2->setImage( vgui_LoadTGA( "gfx/vgui/menu/soundtrack2.tga" ) );
		m_bSoundtrack2Armed = false;
	}

	if ( m_pClothesButton->isArmed() )
	{
		if ( !m_bClothesArmed )
		{
			m_pClothes->setImage( vgui_LoadTGA( "gfx/vgui/menu/clothes_mo.tga" ) );
			gEngfuncs.pfnPlaySoundByName( "ui/3d_rollover.wav", 1 );
			m_bClothesArmed = true;
		}
	}
	else if ( m_bClothesArmed )
	{
		m_pClothes->setImage( vgui_LoadTGA( "gfx/vgui/menu/clothes.tga" ) );
		m_bClothesArmed = false;
	}

	if ( m_pAoMDCButton->isArmed() )
	{
		if ( !m_bAoMDCArmed )
		{
			m_pAoMDC->setImage( vgui_LoadTGA( "gfx/vgui/menu/aomdc_mo.tga" ) );
			gEngfuncs.pfnPlaySoundByName( "ui/3d_rollover.wav", 1 );
			m_bAoMDCArmed = true;
		}
	}
	else if ( m_bAoMDCArmed )
	{
		m_pAoMDC->setImage( vgui_LoadTGA( "gfx/vgui/menu/aomdc.tga" ) );
		m_bAoMDCArmed = false;
	}

	if ( m_pFacebookButton->isArmed() )
	{
		if ( !m_bFacebookArmed )
		{
			m_pFacebook->setImage( vgui_LoadTGA( "gfx/vgui/menu/facebook_mo.tga" ) );
			gEngfuncs.pfnPlaySoundByName( "ui/3d_rollover.wav", 1 );
			m_bFacebookArmed = true;
		}
	}
	else if ( m_bFacebookArmed )
	{
		m_pFacebook->setImage( vgui_LoadTGA( "gfx/vgui/menu/facebook.tga" ) );
		m_bFacebookArmed = false;
	}

	if ( m_pCancelButton->isArmed() )
	{
		if ( !m_bCancelArmed )
		{
			m_pCancel->setImage( vgui_LoadTGA( "gfx/vgui/menu/cancel_mo.tga" ) );
			gEngfuncs.pfnPlaySoundByName( "ui/3d_rollover.wav", 1 );
			m_bCancelArmed = true;
		}
	}
	else if ( m_bCancelArmed )
	{
		m_pCancel->setImage( vgui_LoadTGA( "gfx/vgui/menu/cancel.tga" ) );
		m_bCancelArmed = false;
	}

	Panel::solve();
}
