// ======================================
// ======================================

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "const.h"
#include "entity_types.h"
#include "cdll_int.h"
#include "vgui_TeamFortressViewport.h"
#include "hudcontrol.h"
#include "vgui_ScorePanel.h"
#include "../game_shared/vgui_loadtga.h"
#include "client_port.h"

#include <string.h>
#include <stdio.h>


extern bool		g_bHandsLoaded;
extern bool		bWheelchairLock;
extern int		g_iDickMode;
extern intptr_t		iWheelchair;

int		g_iLetterboxHeight;
float	g_flLookingAt;
int		g_iSimonMode;


int MsgStamina( const char *pszName, int iSize, void *pbuf )
{
	if ( gViewPort->m_pHUDControl )
	{
		BEGIN_READ( pbuf, iSize );
		gViewPort->m_pHUDControl->m_iStamina = READ_BYTE();
		float flTime = READ_COORD();
		if ( flTime > 0 )
			gViewPort->m_pHUDControl->m_flStaminaFlashTime = gEngfuncs.GetClientTime() + flTime;
	}

	return 1;
}

int MsgCoFLogo( const char *pszName, int iSize, void *pbuf )
{
	if ( !gViewPort->m_pHUDControl )
		return 1;

	BEGIN_READ( pbuf, iSize );
	gViewPort->m_pHUDControl->m_iLogo = READ_BYTE();
	gViewPort->m_pHUDControl->m_iLogoAlpha = 255;
	gViewPort->m_pHUDControl->m_bLogoF5D = false;
	gViewPort->m_pHUDControl->m_flLogoTime = 0;

	switch ( gViewPort->m_pHUDControl->m_iLogo )
	{
	case 0:
		gViewPort->m_pHUDControl->m_pLogoImage = vgui_LoadTGA( "gfx/vgui/640_logo_logo.tga" );
		break;
	case 1:
		gViewPort->m_pHUDControl->m_pLogoImage = vgui_LoadTGA( "gfx/vgui/640_logo_andreas.tga" );
		break;
	case 2:
		gViewPort->m_pHUDControl->m_pLogoImage = vgui_LoadTGA( "gfx/vgui/640_logo_james.tga" );
		break;
	case 3:
		gViewPort->m_pHUDControl->m_pLogoImage = vgui_LoadTGA( "gfx/vgui/640_logo_jordy.tga" );
		break;
	case 4:
		gViewPort->m_pHUDControl->m_pLogoImage = vgui_LoadTGA( "gfx/vgui/640_logo_stig.tga" );
		break;
	case 5:
		gViewPort->m_pHUDControl->m_pLogoImage = vgui_LoadTGA( "gfx/vgui/640_logo_lasse.tga" );
		break;
	case 6:
		gViewPort->m_pHUDControl->m_pLogoImage = vgui_LoadTGA( "gfx/vgui/640_logo_aina.tga" );
		break;
	}

	gViewPort->m_pHUDControl->m_pLogo = new ImagePanel( gViewPort->m_pHUDControl->m_pLogoImage );
	gViewPort->m_pHUDControl->m_pLogo->setParent( gViewPort->m_pHUDControl );
	int x = ScreenWidth / 2 - gViewPort->m_pHUDControl->m_pLogo->getWide() / 2;
	int y = ScreenHeight / 2 - gViewPort->m_pHUDControl->m_pLogo->getTall() / 2;
	gViewPort->m_pHUDControl->m_pLogo->setPos( x, y );
	gViewPort->m_pHUDControl->m_pLogoImage->setColor( Color( 255, 255, 255, 255 ) );

	if ( gViewPort->m_pHUDControl->m_iLogo == 0 )
		gViewPort->m_pHUDControl->m_flLogoTime = gEngfuncs.GetClientTime() + 5;
	else
		gViewPort->m_pHUDControl->m_flLogoTime = gEngfuncs.GetClientTime() + 3;

	gViewPort->m_pHUDControl->m_bShowLogo = true;

	return 1;
}

int MsgShowHint( const char *pszName, int iSize, void *pbuf )
{
	char szImage[256];

	BEGIN_READ( pbuf, iSize );

	if ( gViewPort->m_pHUDControl )
	{
		strcpy( szImage, READ_STRING() );

		if ( strlen( szImage ) > 1 )
		{
			gViewPort->m_pHUDControl->m_pInventoryHint->setImage( vgui_LoadTGA( szImage ) );
			gViewPort->m_pHUDControl->m_pInventoryHint->setPos( gViewPort->m_pHUDControl->m_pInventoryHint->getWide() * 1.2 - gViewPort->m_pHUDControl->m_pInventoryHint->getWide(), gViewPort->m_pHUDControl->m_iInventoryHintY );
			gViewPort->m_pHUDControl->m_bInventoryHint = true;
			gViewPort->m_pHUDControl->m_flHintSlideInTime = gEngfuncs.GetClientTime();
			gViewPort->m_pHUDControl->m_flHintSlideInLength = 1;
			gViewPort->m_pHUDControl->m_flHintSlideOutTime = gEngfuncs.GetClientTime() + 15;
			gViewPort->m_pHUDControl->m_flHintSlideOutLength = 1;
		}
	}

	return 1;
}

int MsgCameraZoom( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	gHUD.m_flCameraZoomFOV = READ_COORD();
	gHUD.m_iCameraZoom = READ_BYTE();
	gHUD.m_bCameraZoomOn = READ_BYTE() != 0;

	if ( gHUD.m_iCameraZoom )
		gHUD.m_flNextSwayTime = gHUD.m_flTime;

	return 1;
}

int MsgHealthCap( const char *pszName, int iSize, void *pbuf )
{
	if ( gViewPort->m_pHUDControl )
	{
		BEGIN_READ( pbuf, iSize );
		int iType = READ_BYTE();

		switch ( iType )
		{
		case 0:
			gViewPort->m_pHUDControl->m_pHealthBarFull->setImage( vgui_LoadTGA( "gfx/vgui/640_healthbar_bar.tga" ) );
			gViewPort->m_pHUDControl->m_pHealthBar->setImage( vgui_LoadTGA( "gfx/vgui/640_healthbar.tga" ) );
			gViewPort->m_pHUDControl->m_pHealthBarEmpty->setImage( vgui_LoadTGA( "gfx/vgui/640_healthbar_barempty.tga" ) );
			break;
		case 1:
			gViewPort->m_pHUDControl->m_pHealthBarFull->setImage( vgui_LoadTGA( "gfx/vgui/640_healthbar_bar2.tga" ) );
			gViewPort->m_pHUDControl->m_pHealthBar->setImage( vgui_LoadTGA( "gfx/vgui/640_healthbar2.tga" ) );
			gViewPort->m_pHUDControl->m_pHealthBarEmpty->setImage( vgui_LoadTGA( "gfx/vgui/640_healthbar_barempty2.tga" ) );
			break;
		case 2:
			gViewPort->m_pHUDControl->m_pHealthBarFull->setImage( vgui_LoadTGA( "gfx/vgui/640_healthbar_bar3.tga" ) );
			gViewPort->m_pHUDControl->m_pHealthBar->setImage( vgui_LoadTGA( "gfx/vgui/640_healthbar3.tga" ) );
			gViewPort->m_pHUDControl->m_pHealthBarEmpty->setImage( vgui_LoadTGA( "gfx/vgui/640_healthbar_barempty3.tga" ) );
			break;
		}
	}

	return 1;
}

int MsgChapter( const char *pszName, int iSize, void *pbuf )
{
	char szSound[256];

	BEGIN_READ( pbuf, iSize );

	if ( gViewPort->m_pHUDControl )
	{
		strcpy( gViewPort->m_pHUDControl->m_szChapter, READ_STRING() );
		gViewPort->m_pHUDControl->m_flChapterAlpha = 255;
		gViewPort->m_pHUDControl->m_bChapterFadeOut = false;
		gViewPort->m_pHUDControl->m_bChapterF1E = false;
		gViewPort->m_pHUDControl->m_flChapterHoldTime = 0;
		gViewPort->m_pHUDControl->m_flChapterNextChar = 0;
		gViewPort->m_pHUDControl->m_iChapterChars = 0;

		strncpy( szSound, READ_STRING(), 256 );
		if ( strlen( szSound ) > 1 )
			gEngfuncs.pfnPlaySoundByName( szSound, 1 );
	}

	return 1;
}

int MsgControlHint( const char *pszName, int iSize, void *pbuf )
{
	char szName[256];
	char szFile[256];
	char szText[256];

	BEGIN_READ( pbuf, iSize );

	if ( gViewPort->m_pHUDControl )
	{
		strncpy( szName, READ_STRING(), 256 );
		strncpy( szFile, READ_STRING(), 256 );
		ParseHintTokens( szName, szFile );

		gViewPort->m_pHUDControl->m_iHintLine = 1;

		if ( strlen( gViewPort->m_pHUDControl->m_szHintLine1 ) > 2 )
		{
			int iLen = 0;
			for ( unsigned int i = 0; i < strlen( gViewPort->m_pHUDControl->m_szHintLine1 ); i++ )
			{
				if ( gViewPort->m_pHUDControl->m_szHintLine1[i] == '@' )
				{
					iLen++;
					szText[i] = ' ';
				}
				else
				{
					iLen++;
					szText[i] = gViewPort->m_pHUDControl->m_szHintLine1[i];
				}
			}
			szText[iLen] = '\0';

			int w, h;
			gViewPort->m_pHUDControl->m_pHintLabel->setText( szText );
			gViewPort->m_pHUDControl->m_pCreditsFont->getTextSize( gViewPort->m_pHUDControl->m_szHintLine1, w, h );
			gViewPort->m_pHUDControl->m_pHintLabel->setPos( ScreenWidth / 2 - 400, -48 );
			gViewPort->m_pHUDControl->m_bHint = true;
			gViewPort->m_pHUDControl->m_iHintAlpha = 255;
			gViewPort->m_pHUDControl->m_flHintTime = gEngfuncs.GetClientTime() + 5;
		}
	}

	return 1;
}

int MsgDevDonator( const char *pszName, int iSize, void *pbuf )
{
	if ( gViewPort->m_pHUDControl )
	{
		BEGIN_READ( pbuf, iSize );
		int iPlayer = READ_BYTE();
		gViewPort->m_pHUDControl->m_iDevDonator[iPlayer] = READ_BYTE();
	}

	return 1;
}

int MsgHoldBreath( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	gHUD.m_iHoldBreath = READ_BYTE();

	return 1;
}

int MsgIntrodn( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	int iType = READ_BYTE();
	READ_BYTE();
	int iCoop = READ_BYTE();

	if ( !gViewPort || !gViewPort->m_pHUDControl )
		return 1;

	int iLanguage;
	if ( iCoop )
		iLanguage = CVAR_GET_FLOAT( "cl_coop_language" );
	else
		iLanguage = CVAR_GET_FLOAT( "cof_subtitlelanguage" );

	if ( !gViewPort->m_pHUDControl->m_pIntroduction->isVisible() )
	{
		gViewPort->m_pHUDControl->m_iIntroductionAlpha = 255;
		gViewPort->m_pHUDControl->m_pIntroductionImage->setColor( Color( 255, 255, 255, 255 ) );
		gViewPort->m_pHUDControl->m_bIntroductionFadeIn = true;

		switch ( iLanguage )
		{
		case 0:
			if ( iType == 0 )
				gViewPort->m_pHUDControl->m_pIntroductionImage = vgui_LoadTGA( "gfx/vgui/introductions/english/english_sp.TGA" );
			else if ( iType == 1 )
				gViewPort->m_pHUDControl->m_pIntroductionImage = vgui_LoadTGA( "gfx/vgui/introductions/english/english_doc.TGA" );
			else if ( iType == 2 )
				gViewPort->m_pHUDControl->m_pIntroductionImage = vgui_LoadTGA( "gfx/vgui/introductions/english/english_coop.TGA" );
			break;
		case 2:
			if ( iType == 0 )
				gViewPort->m_pHUDControl->m_pIntroductionImage = vgui_LoadTGA( "gfx/vgui/introductions/dutch/dutch_sp.TGA" );
			else if ( iType == 1 )
				gViewPort->m_pHUDControl->m_pIntroductionImage = vgui_LoadTGA( "gfx/vgui/introductions/dutch/dutch_doc.TGA" );
			else if ( iType == 2 )
				gViewPort->m_pHUDControl->m_pIntroductionImage = vgui_LoadTGA( "gfx/vgui/introductions/dutch/dutch_coop.TGA" );
			break;
		case 3:
			if ( iType == 0 )
				gViewPort->m_pHUDControl->m_pIntroductionImage = vgui_LoadTGA( "gfx/vgui/introductions/french/french_sp.TGA" );
			else if ( iType == 1 )
				gViewPort->m_pHUDControl->m_pIntroductionImage = vgui_LoadTGA( "gfx/vgui/introductions/french/french_doc.TGA" );
			else if ( iType == 2 )
				gViewPort->m_pHUDControl->m_pIntroductionImage = vgui_LoadTGA( "gfx/vgui/introductions/french/french_coop.TGA" );
			break;
		case 4:
			if ( iType == 0 )
				gViewPort->m_pHUDControl->m_pIntroductionImage = vgui_LoadTGA( "gfx/vgui/introductions/german/german_sp.TGA" );
			else if ( iType == 1 )
				gViewPort->m_pHUDControl->m_pIntroductionImage = vgui_LoadTGA( "gfx/vgui/introductions/german/german_doc.TGA" );
			else if ( iType == 2 )
				gViewPort->m_pHUDControl->m_pIntroductionImage = vgui_LoadTGA( "gfx/vgui/introductions/german/german_coop.TGA" );
			break;
		case 5:
			if ( iType == 0 )
				gViewPort->m_pHUDControl->m_pIntroductionImage = vgui_LoadTGA( "gfx/vgui/introductions/norwegian/norwegian_sp.TGA" );
			else if ( iType == 1 )
				gViewPort->m_pHUDControl->m_pIntroductionImage = vgui_LoadTGA( "gfx/vgui/introductions/norwegian/norwegian_doc.TGA" );
			else if ( iType == 2 )
				gViewPort->m_pHUDControl->m_pIntroductionImage = vgui_LoadTGA( "gfx/vgui/introductions/norwegian/norwegian_coop.TGA" );
			break;
		case 6:
			if ( iType == 0 )
				gViewPort->m_pHUDControl->m_pIntroductionImage = vgui_LoadTGA( "gfx/vgui/introductions/spanish/spanish_sp.TGA" );
			else if ( iType == 1 )
				gViewPort->m_pHUDControl->m_pIntroductionImage = vgui_LoadTGA( "gfx/vgui/introductions/spanish/spanish_doc.TGA" );
			else if ( iType == 2 )
				gViewPort->m_pHUDControl->m_pIntroductionImage = vgui_LoadTGA( "gfx/vgui/introductions/spanish/spanish_coop.TGA" );
			break;
		case 7:
			if ( iType == 0 )
				gViewPort->m_pHUDControl->m_pIntroductionImage = vgui_LoadTGA( "gfx/vgui/introductions/swedish/swedish_sp.TGA" );
			else if ( iType == 1 )
				gViewPort->m_pHUDControl->m_pIntroductionImage = vgui_LoadTGA( "gfx/vgui/introductions/swedish/swedish_doc.TGA" );
			else if ( iType == 2 )
				gViewPort->m_pHUDControl->m_pIntroductionImage = vgui_LoadTGA( "gfx/vgui/introductions/swedish/swedish_coop.TGA" );
			break;
		default:
			if ( iType == 0 )
				gViewPort->m_pHUDControl->m_pIntroductionImage = vgui_LoadTGA( "gfx/vgui/introductions/english/english_sp.TGA" );
			else if ( iType == 1 )
				gViewPort->m_pHUDControl->m_pIntroductionImage = vgui_LoadTGA( "gfx/vgui/introductions/english/english_doc.TGA" );
			else if ( iType == 2 )
				gViewPort->m_pHUDControl->m_pIntroductionImage = vgui_LoadTGA( "gfx/vgui/introductions/english/english_coop.TGA" );
			break;
		}

		gViewPort->m_pHUDControl->m_pIntroduction->setImage( gViewPort->m_pHUDControl->m_pIntroductionImage );
		gViewPort->m_pHUDControl->m_pIntroduction->setVisible( true );
	}
	else
		gViewPort->m_pHUDControl->m_bIntroductionFadeIn = false;

	return 1;
}

int MsgLobby( const char *pszName, int iSize, void *pbuf )
{
	if ( gViewPort->m_pHUDControl )
	{
		BEGIN_READ( pbuf, iSize );
		int iPlayer = READ_BYTE();
		int iReady = READ_BYTE();
		int iLobby = READ_BYTE();
		int i;

		if ( iLobby )
		{
			gViewPort->m_pHUDControl->m_bLobby = false;
			gViewPort->m_pHUDControl->m_pVoteLabel->setVisible( false );
			gViewPort->m_pHUDControl->m_pVoteKickLabel->setVisible( false );
			for ( i = 0; i < 4; i++ )
				gViewPort->m_pHUDControl->m_pLobbyLabel[i]->setVisible( false );
			gViewPort->m_pHUDControl->m_pLobbyBackground->setVisible( false );
		}
		else
		{
			gViewPort->m_pHUDControl->m_bLobby = true;
			gViewPort->m_pHUDControl->m_pVoteLabel->setVisible( true );
			gViewPort->m_pHUDControl->m_pVoteKickLabel->setVisible( true );
			for ( i = 0; i < 4; i++ )
				gViewPort->m_pHUDControl->m_pLobbyLabel[i]->setVisible( true );
			gViewPort->m_pHUDControl->m_pLobbyBackground->setImage( vgui_LoadTGA( "gfx/vgui/lobby_background.tga" ) );
			gViewPort->m_pHUDControl->m_pLobbyBackground->setSize( 800, 600 );
			gViewPort->m_pHUDControl->m_pLobbyBackground->setPos( ScreenWidth / 2 - 400, ScreenHeight / 2 - 300 );
			gViewPort->m_pHUDControl->m_pLobbyBackground->setVisible( true );
		}

		int iIndex = iPlayer - 1;
		if ( iIndex >= 0 && iIndex < 4 )
		{
			if ( iReady )
				gViewPort->m_pHUDControl->m_bPlayerReady[iIndex] = true;
			else
				gViewPort->m_pHUDControl->m_bPlayerReady[iIndex] = false;
		}
	}

	return 1;
}

int MsgLookingAt( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	g_flLookingAt = READ_BYTE();

	return 1;
}

int MsgMPModel( const char *pszName, int iSize, void *pbuf )
{
	g_bHandsLoaded = false;

	return 1;
}

int MsgSimonMode( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	g_iSimonMode = READ_BYTE();

	return 1;
}

int MsgTimerMode( const char *pszName, int iSize, void *pbuf )
{
	if ( gViewPort->m_pHUDControl )
	{
		BEGIN_READ( pbuf, iSize );
		int iMode = READ_BYTE();
		int iSeconds = READ_BYTE();
		int iMinutes = READ_BYTE();
		int iHours = READ_BYTE();
		int iLimit = READ_SHORT();

		if ( iMode == 0 )
		{
			gViewPort->m_pHUDControl->m_bTimer = false;
			gViewPort->m_pHUDControl->m_flTimerNextSecond = gEngfuncs.GetClientTime();
			gViewPort->m_pHUDControl->m_iTimerSeconds = 0;
			gViewPort->m_pHUDControl->m_iTimerMinutes = 0;
			gViewPort->m_pHUDControl->m_iTimerHours = 0;
		}
		else if ( iMode == 1 )
		{
			gViewPort->m_pHUDControl->m_bTimer = true;
			gViewPort->m_pHUDControl->m_flTimerNextSecond = gEngfuncs.GetClientTime();
			gViewPort->m_pHUDControl->m_iTimerSeconds = iSeconds;
			gViewPort->m_pHUDControl->m_iTimerMinutes = iMinutes;
			gViewPort->m_pHUDControl->m_iTimerHours = iHours;
			gViewPort->m_pHUDControl->m_iTimerLimit = iLimit;
		}
		else if ( iMode == 2 )
		{
			gViewPort->m_pHUDControl->m_iTimerSeconds = iSeconds;
			gViewPort->m_pHUDControl->m_iTimerMinutes = iMinutes;
			gViewPort->m_pHUDControl->m_iTimerHours = iHours;
			gViewPort->m_pHUDControl->m_iTimerLimit = iLimit;
		}
	}

	return 1;
}

int MsgTimerPlus( const char *pszName, int iSize, void *pbuf )
{
	char szSeconds[32];
	char szMinutes[32];
	char szHours[32];
	char szTime[128];

	BEGIN_READ( pbuf, iSize );
	int iSeconds = READ_BYTE();
	int iShow = READ_BYTE();
	int iMode = READ_BYTE();

	if ( gViewPort && gViewPort->m_pHUDControl )
	{
		switch ( iMode )
		{
		case 1:
			gViewPort->m_pHUDControl->m_bTimerPaused = true;
			return 1;
		case 2:
			gViewPort->m_pHUDControl->m_bTimerPaused = false;
			break;
		case 3:
			gViewPort->m_pHUDControl->m_bTimerPaused = true;
			break;
		}

		gViewPort->m_pHUDControl->m_iTimerSeconds += iSeconds;

		if ( gViewPort->m_pHUDControl->m_iTimerSeconds > 59 )
		{
			gViewPort->m_pHUDControl->m_iTimerSeconds = ( gViewPort->m_pHUDControl->m_iTimerSeconds > 60 ) ? gViewPort->m_pHUDControl->m_iTimerSeconds - 60 : 0;
			gViewPort->m_pHUDControl->m_iTimerMinutes++;
		}

		if ( gViewPort->m_pHUDControl->m_iTimerMinutes > 59 )
		{
			gViewPort->m_pHUDControl->m_iTimerHours++;
			gViewPort->m_pHUDControl->m_iTimerSeconds = 0;
			gViewPort->m_pHUDControl->m_iTimerMinutes = 0;
		}

		int iSecondsLeft = 60 - gViewPort->m_pHUDControl->m_iTimerSeconds;
		int iMinutesLeft = gViewPort->m_pHUDControl->m_iTimerLimit / 60 - gViewPort->m_pHUDControl->m_iTimerMinutes - 1;

		if ( iMinutesLeft < 0 )
			iMinutesLeft = 0;
		if ( iSecondsLeft < 0 )
			iSecondsLeft = 0;
		if ( gViewPort->m_pHUDControl->m_iTimerHours < 0 )
			gViewPort->m_pHUDControl->m_iTimerHours = 0;

		sprintf( szSeconds, ( iSecondsLeft >= 10 ) ? "%i" : "0%i", iSecondsLeft );
		sprintf( szMinutes, ( iMinutesLeft >= 10 ) ? "%i" : "0%i", iMinutesLeft );
		sprintf( szHours, ( gViewPort->m_pHUDControl->m_iTimerHours >= 10 ) ? "%i" : "0%i", gViewPort->m_pHUDControl->m_iTimerHours );
		sprintf( szTime, "%s:%s:%s", szHours, szMinutes, szSeconds );
		gViewPort->m_pHUDControl->m_pTimerLabel->setText( szTime );
		gViewPort->m_pHUDControl->m_flTimerNextSecond = gEngfuncs.GetClientTime() + 1;
		gEngfuncs.pfnPlaySoundByName( "common/survival_timeplus.wav", 2 );

		if ( iShow )
		{
			for ( int i = 0; i < 32; i++ )
			{
				if ( gViewPort && !gViewPort->m_pHUDControl->m_pTimerPlusLabel[i]->isVisible() )
				{
					int w, h;
					gViewPort->m_pHUDControl->m_pCreditsFont->getTextSize( szTime, w, h );
					gViewPort->m_pHUDControl->m_pTimerPlusLabel[i]->setFgColor( 0, 128, 0, 0 );
					gViewPort->m_pHUDControl->m_pTimerPlusLabel[i]->setPos( ScreenWidth / 2 + w / 2, 0 );
					gViewPort->m_pHUDControl->m_pTimerLabel->setFgColor( 128, 255, 128, 0 );
					sprintf( szTime, "+%i", iSeconds );
					gViewPort->m_pHUDControl->m_pTimerPlusLabel[i]->setText( szTime );
					gViewPort->m_pHUDControl->m_pTimerPlusLabel[i]->setVisible( true );
					break;
				}
			}
		}
	}

	return 1;
}

void ParseHintTokens( char *pszName, char *pszFile )
{
	char szBefore1[256];
	char szBefore2[256];
	char szBefore3[256];
	char token[1024];
	char szAfter[256];
	bool bFound;
	int i, j;

	if ( !gViewPort->m_pHUDControl )
		return;

	gViewPort->m_pHUDControl->m_szHintLine1[0] = '\0';
	gViewPort->m_pHUDControl->m_szHintLine2[0] = '\0';
	gViewPort->m_pHUDControl->m_szHintLine3[0] = '\0';
	gViewPort->m_pHUDControl->m_szHintNoButton[0] = '\0';
	gViewPort->m_pHUDControl->m_szHintCfgButton[0] = '\0';

	char *pfile = (char *)gEngfuncs.COM_LoadFile( pszFile, 5, NULL );
	if ( !pfile )
	{
		gEngfuncs.Con_Printf( "ParseHintTokens Couldn't open file %s\n", pszFile );
		return;
	}

	while ( pfile = gEngfuncs.COM_ParseFile( pfile, token ) )
	{
		if ( _stricmp( token, pszName ) )
			continue;

		pfile = gEngfuncs.COM_ParseFile( pfile, token );
		if ( _stricmp( token, "{" ) )
		{
			gEngfuncs.Con_Printf( "ParseHintTokens Error parsing file %s. (expected { )\n", pszFile );
			return;
		}

		pfile = gEngfuncs.COM_ParseFile( pfile, token );
		while ( _stricmp( token, "}" ) )
		{
			if ( !_stricmp( token, "line1" ) )
			{
				pfile = gEngfuncs.COM_ParseFile( pfile, token );
				strncpy( gViewPort->m_pHUDControl->m_szHintLine1, token, 256 );
			}
			else if ( !_stricmp( token, "line2" ) )
			{
				pfile = gEngfuncs.COM_ParseFile( pfile, token );
				strncpy( gViewPort->m_pHUDControl->m_szHintLine2, token, 256 );
			}
			else if ( !_stricmp( token, "line3" ) )
			{
				pfile = gEngfuncs.COM_ParseFile( pfile, token );
				strncpy( gViewPort->m_pHUDControl->m_szHintLine3, token, 256 );
			}
			else if ( !_stricmp( token, "nobutton" ) )
			{
				pfile = gEngfuncs.COM_ParseFile( pfile, token );
				strncpy( gViewPort->m_pHUDControl->m_szHintNoButton, token, 256 );
			}
			else if ( !_stricmp( token, "cfgbutton" ) )
			{
				pfile = gEngfuncs.COM_ParseFile( pfile, token );
				strncpy( gViewPort->m_pHUDControl->m_szHintCfgButton, token, 64 );
			}

			pfile = gEngfuncs.COM_ParseFile( pfile, token );
		}
	}

	gEngfuncs.COM_FreeFile( pfile );

	if ( strlen( gViewPort->m_pHUDControl->m_szHintLine1 ) > 2 )
	{
		bFound = false;
		j = 0;
		for ( i = 0; i < 256; i++ )
		{
			if ( !bFound )
			{
				if ( gViewPort->m_pHUDControl->m_szHintLine1[i] == '*' )
				{
					szBefore1[i] = '\0';
					bFound = true;
				}
				else
					szBefore1[i] = gViewPort->m_pHUDControl->m_szHintLine1[i];
			}
			else
			{
				if ( gViewPort->m_pHUDControl->m_szHintLine1[i] != '\0' )
					szAfter[j++] = gViewPort->m_pHUDControl->m_szHintLine1[i];
				else
					szAfter[j++] = '\0';
			}
		}

		strcpy( gViewPort->m_pHUDControl->m_szHintLine1, szBefore1 );

		if ( bFound )
		{
			if ( gHUD.GetKeyForCommand( gViewPort->m_pHUDControl->m_szHintCfgButton ) )
				strcat( gViewPort->m_pHUDControl->m_szHintLine1, gHUD.GetKeyForCommand( gViewPort->m_pHUDControl->m_szHintCfgButton ) );
			else
				strcat( gViewPort->m_pHUDControl->m_szHintLine1, gViewPort->m_pHUDControl->m_szHintNoButton );
			strcat( gViewPort->m_pHUDControl->m_szHintLine1, szAfter );
		}
	}

	if ( strlen( gViewPort->m_pHUDControl->m_szHintLine2 ) > 2 )
	{
		bFound = false;
		j = 0;
		for ( i = 0; i < 256; i++ )
		{
			if ( !bFound )
			{
				if ( gViewPort->m_pHUDControl->m_szHintLine2[i] == '*' )
				{
					szBefore2[i] = '\0';
					bFound = true;
				}
				else
					szBefore2[i] = gViewPort->m_pHUDControl->m_szHintLine2[i];
			}
			else
			{
				if ( gViewPort->m_pHUDControl->m_szHintLine2[i] != '\0' )
					szAfter[j++] = gViewPort->m_pHUDControl->m_szHintLine2[i];
				else
					szAfter[j++] = '\0';
			}
		}

		strcpy( gViewPort->m_pHUDControl->m_szHintLine2, szBefore2 );

		if ( bFound )
		{
			if ( gHUD.GetKeyForCommand( gViewPort->m_pHUDControl->m_szHintCfgButton ) )
				strcat( gViewPort->m_pHUDControl->m_szHintLine2, gHUD.GetKeyForCommand( gViewPort->m_pHUDControl->m_szHintCfgButton ) );
			else
				strcat( gViewPort->m_pHUDControl->m_szHintLine2, gViewPort->m_pHUDControl->m_szHintNoButton );
			strcat( gViewPort->m_pHUDControl->m_szHintLine2, szAfter );
		}
	}

	if ( strlen( gViewPort->m_pHUDControl->m_szHintLine3 ) > 2 )
	{
		bFound = false;
		j = 0;
		for ( i = 0; i < 256; i++ )
		{
			if ( !bFound )
			{
				if ( gViewPort->m_pHUDControl->m_szHintLine3[i] == '*' )
				{
					szBefore3[i] = '\0';
					bFound = true;
				}
				else
					szBefore3[i] = gViewPort->m_pHUDControl->m_szHintLine3[i];
			}
			else
			{
				if ( gViewPort->m_pHUDControl->m_szHintLine3[i] != '\0' )
					szAfter[j++] = gViewPort->m_pHUDControl->m_szHintLine3[i];
				else
					szAfter[j++] = '\0';
			}
		}

		strcpy( gViewPort->m_pHUDControl->m_szHintLine3, szBefore3 );

		if ( bFound )
		{
			if ( gHUD.GetKeyForCommand( gViewPort->m_pHUDControl->m_szHintCfgButton ) )
				strcat( gViewPort->m_pHUDControl->m_szHintLine3, gHUD.GetKeyForCommand( gViewPort->m_pHUDControl->m_szHintCfgButton ) );
			else
				strcat( gViewPort->m_pHUDControl->m_szHintLine3, gViewPort->m_pHUDControl->m_szHintNoButton );
			strcat( gViewPort->m_pHUDControl->m_szHintLine3, szAfter );
		}
	}
}

void HUDControlInit()
{
	gEngfuncs.pfnHookUserMsg( "Stamina", MsgStamina );
	gEngfuncs.pfnHookUserMsg( "CoFLogo", MsgCoFLogo );
	gEngfuncs.pfnHookUserMsg( "ShowHint", MsgShowHint );
	gEngfuncs.pfnHookUserMsg( "CameraZoom", MsgCameraZoom );
	gEngfuncs.pfnHookUserMsg( "HealthCap", MsgHealthCap );
	gEngfuncs.pfnHookUserMsg( "Chapter", MsgChapter );
	gEngfuncs.pfnHookUserMsg( "ControlHint", MsgControlHint );
	gEngfuncs.pfnHookUserMsg( "Timer", MsgTimerMode );
	gEngfuncs.pfnHookUserMsg( "Lobby", MsgLobby );
	gEngfuncs.pfnHookUserMsg( "MPModel", MsgMPModel );
	gEngfuncs.pfnHookUserMsg( "HoldBreath", MsgHoldBreath );
	gEngfuncs.pfnHookUserMsg( "DevDonator", MsgDevDonator );
	gEngfuncs.pfnHookUserMsg( "Introdn", MsgIntrodn );
	gEngfuncs.pfnHookUserMsg( "LookingAt", MsgLookingAt );
	gEngfuncs.pfnHookUserMsg( "TimerPlus", MsgTimerPlus );
	gEngfuncs.pfnHookUserMsg( "SimonMode", MsgSimonMode );
}

CHUDControl::CHUDControl() : Panel( 0, 0, ScreenWidth, ScreenHeight )
{
	int i;

	m_pLobbyBackground = new ImagePanel( vgui_LoadTGA( "gfx/vgui/vignette/vignette_tl.tga" ) );
	m_pLobbyBackground->setParent( this );
	m_pLobbyBackground->setPos( ScreenWidth / 2 - 400, ScreenHeight / 2 - 300 );
	m_pLobbyBackground->setVisible( true );

	m_pVignetteTL = new ImagePanel( vgui_LoadTGA( "gfx/vgui/vignette/vignette_tl.tga" ) );
	m_pVignetteTL->setParent( this );
	m_pVignetteTL->setPos( 0, 0 );
	m_pVignetteTL->setVisible( false );

	m_pVignetteTR = new ImagePanel( vgui_LoadTGA( "gfx/vgui/vignette/vignette_tr.tga" ) );
	m_pVignetteTR->setParent( this );
	m_pVignetteTR->setPos( ScreenWidth - 256, 0 );
	m_pVignetteTR->setVisible( false );

	m_pVignetteBL = new ImagePanel( vgui_LoadTGA( "gfx/vgui/vignette/vignette_bl.tga" ) );
	m_pVignetteBL->setParent( this );
	m_pVignetteBL->setPos( 0, ScreenHeight - 256 );
	m_pVignetteBL->setVisible( false );

	m_pVignetteBR = new ImagePanel( vgui_LoadTGA( "gfx/vgui/vignette/vignette_br.tga" ) );
	m_pVignetteBR->setParent( this );
	m_pVignetteBR->setPos( ScreenWidth - 256, ScreenHeight - 256 );
	m_pVignetteBR->setVisible( false );

	if ( m_pSMSIconImage = vgui_LoadTGA( "gfx/vgui/640_hudelem_smsicon.tga" ) )
		m_pSMSIconImage->setColor( Color( 255, 255, 255, 1 ) );
	else
		gEngfuncs.Con_Printf( "Cannot load gfx/vgui/640_hudelem_smsicon.tga!\n" );

	if ( m_pInventoryHintImage = vgui_LoadTGA( "gfx/vgui/hints/hint_inventory.tga" ) )
		m_pInventoryHintImage->setColor( Color( 255, 255, 255, 1 ) );
	else
		gEngfuncs.Con_Printf( "Cannot load gfx/vgui/hints/hint_inventory.tga!\n" );

	if ( m_pCrosshairImage = vgui_LoadTGA( "gfx/vgui/crosshair.tga" ) )
		m_pCrosshairImage->setColor( Color( 255, 255, 255, 1 ) );
	else
		gEngfuncs.Con_Printf( "Cannot load gfx/vgui/crosshair.tga!\n" );

	if ( m_pLogoImage = vgui_LoadTGA( "gfx/vgui/640_logo_logo.tga" ) )
		m_pLogoImage->setColor( Color( 255, 255, 255, 1 ) );
	else
		gEngfuncs.Con_Printf( "Cannot load gfx/vgui/640_logo_logo.tga!\n" );

	if ( m_pHealthBarImage = vgui_LoadTGA( "gfx/vgui/640_healthbar.tga" ) )
		m_pHealthBarImage->setColor( Color( 255, 255, 255, 1 ) );
	else
		gEngfuncs.Con_Printf( "Cannot load gfx/vgui/640_healthbar.tga!\n" );

	CSchemeManager *pSchemes = gViewPort->GetSchemeManager();
	SchemeHandle_t hCreditsScheme = pSchemes->getSchemeHandle( "credits" );
	SchemeHandle_t hChapterTitleScheme = pSchemes->getSchemeHandle( "chaptertitle" );
	SchemeHandle_t hPagerScheme = pSchemes->getSchemeHandle( "pager" );
	Font *pFont = pSchemes->getFont( hCreditsScheme );
	m_pPagerFont = pSchemes->getFont( hPagerScheme );
	m_pCreditsFont = pSchemes->getFont( hCreditsScheme );
	m_pChapterTitleFont = pSchemes->getFont( hChapterTitleScheme );

	if ( m_pHealthBarFullImage = vgui_LoadTGA( "gfx/vgui/640_healthbar_bar.tga" ) )
		m_pHealthBarFullImage->setColor( Color( 255, 255, 255, 1 ) );
	else
		gEngfuncs.Con_Printf( "Cannot load gfx/vgui/640_healthbar_bar.tga!\n" );

	if ( m_pHealthBarEmptyImage = vgui_LoadTGA( "gfx/vgui/640_healthbar_barempty.tga" ) )
		m_pHealthBarEmptyImage->setColor( Color( 255, 255, 255, 1 ) );
	else
		gEngfuncs.Con_Printf( "Cannot load gfx/vgui/640_healthbar_barempty.tga!\n" );

	if ( m_pStaminaBarImage = vgui_LoadTGA( "gfx/vgui/640_staminabar.tga" ) )
		m_pStaminaBarImage->setColor( Color( 255, 255, 255, 1 ) );
	else
		gEngfuncs.Con_Printf( "Cannot load gfx/vgui/640_staminabar.tga!\n" );

	if ( m_pStaminaBarFullImage = vgui_LoadTGA( "gfx/vgui/640_staminabar_bar.tga" ) )
		m_pStaminaBarFullImage->setColor( Color( 255, 255, 255, 1 ) );
	else
		gEngfuncs.Con_Printf( "Cannot load gfx/vgui/640_staminabar_bar.tga!\n" );

	if ( m_pStaminaBarEmptyImage = vgui_LoadTGA( "gfx/vgui/640_staminabar_barempty.tga" ) )
		m_pStaminaBarEmptyImage->setColor( Color( 255, 255, 255, 1 ) );
	else
		gEngfuncs.Con_Printf( "Cannot load gfx/vgui/640_staminabar_barempty.tga!\n" );

	if ( m_pHintBarImage = vgui_LoadTGA( "gfx/vgui/640_hint_bar.tga" ) )
		m_pHintBarImage->setColor( Color( 255, 255, 255, 1 ) );
	else
		gEngfuncs.Con_Printf( "Cannot load gfx/vgui/640_hint_bar.tga!\n" );

	if ( m_pIntroductionImage = vgui_LoadTGA( "gfx/vgui/introductions/english/english_sp.tga" ) )
		m_pIntroductionImage->setColor( Color( 255, 255, 255, 1 ) );
	else
		gEngfuncs.Con_Printf( "Cannot load gfx/vgui/introductions/english/english_sp.tga!\n" );

	for ( i = 0; i < 4; i++ )
	{
		if ( m_pFollowIcon[i] = vgui_LoadTGA( "gfx/vgui/coop_follow_icon.tga" ) )
			m_pFollowIcon[i]->setColor( Color( 255, 255, 255, 1 ) );
		else
			gEngfuncs.Con_Printf( "Cannot load gfx/vgui/coop_follow_icon.tga %i!\n", i );

		if ( m_pSyringeIcon[i] = vgui_LoadTGA( "gfx/vgui/coop_syringe_icon.tga" ) )
			m_pSyringeIcon[i]->setColor( Color( 255, 255, 255, 1 ) );
		else
			gEngfuncs.Con_Printf( "Cannot load gfx/vgui/coop_syringe_icon.tga %i!\n", i );
	}

	setBgColor( 0, 0, 0, 255 );

	m_pIntroduction = new ImagePanel( m_pIntroductionImage );
	m_pIntroduction->setParent( this );
	m_pIntroduction->setPos( ScreenWidth / 2 - 256, ScreenHeight / 2 - 256 );
	m_pIntroduction->setVisible( false );
	m_pIntroductionImage->setColor( Color( 255, 255, 255, m_iIntroductionAlpha ) );

	m_pSMSIcon = new ImagePanel( m_pSMSIconImage );
	m_pSMSIcon->setParent( this );
	m_pSMSIcon->setPos( m_pSMSIcon->getWide(), m_pSMSIcon->getTall() );
	m_pSMSIcon->setVisible( false );

	m_pInventoryHint = new ImagePanel( m_pInventoryHintImage );
	m_pInventoryHint->setParent( this );
	m_pInventoryHint->setPos( m_pInventoryHint->getWide() * 1.2 - m_pInventoryHint->getWide(), m_iInventoryHintY );
	m_pInventoryHint->setVisible( false );

	m_pLogo = new ImagePanel( m_pLogoImage );
	m_pLogo->setParent( this );
	int x = ScreenWidth / 2 - m_pLogo->getWide() / 2;
	int y = ScreenHeight / 2 - m_pLogo->getTall() / 2;
	m_pLogo->setPos( x, y );
	m_pLogoImage->setColor( Color( 255, 255, 255, 255 ) );
	m_pLogo->setVisible( false );

	m_pOverlay = new Panel( 0, 0, ScreenWidth, ScreenHeight );
	m_pOverlay->setParent( this );
	m_pOverlay->setSize( ScreenWidth, ScreenHeight );
	m_pOverlay->setPos( 0, 0 );
	m_pOverlay->setBgColor( 33, 177, 120, 128 );
	m_pOverlay->setVisible( false );

	m_pHealthBar = new ImagePanel( m_pHealthBarImage );
	m_pHealthBar->setParent( this );
	m_pHealthBar->setPos( m_pHealthBar->getWide(), ScreenHeight - m_pHealthBar->getTall() );
	m_pHealthBar->setVisible( true );

	m_pHealthBarFull = new ImagePanel( m_pHealthBarFullImage );
	m_pHealthBarFull->setParent( m_pHealthBar );
	m_pHealthBarFull->setPos( 11, 5 );
	m_pHealthBarFull->setVisible( true );

	m_pHealthBarEmpty = new ImagePanel( m_pHealthBarEmptyImage );
	m_pHealthBarEmpty->setParent( m_pHealthBar );
	m_pHealthBarEmpty->setPos( 11, 5 );
	m_pHealthBarEmpty->setVisible( true );

	m_pStaminaBar = new ImagePanel( m_pStaminaBarImage );
	m_pStaminaBar->setParent( this );
	m_pStaminaBar->setPos( m_pStaminaBar->getWide() * 2, ScreenHeight - m_pStaminaBar->getTall() );
	m_pStaminaBar->setVisible( true );

	m_pStaminaBarFull = new ImagePanel( m_pStaminaBarFullImage );
	m_pStaminaBarFull->setParent( m_pStaminaBar );
	m_pStaminaBarFull->setPos( 11, 5 );
	m_pStaminaBarFull->setVisible( true );

	m_pStaminaBarEmpty = new ImagePanel( m_pStaminaBarEmptyImage );
	m_pStaminaBarEmpty->setParent( m_pStaminaBar );
	m_pStaminaBarEmpty->setPos( 11, 5 );
	m_pStaminaBarEmpty->setVisible( true );

	int wide, tall;
	m_pStaminaBarEmptyImage->getSize( wide, tall );

	m_pStaminaFlash = new Panel( 255, 0, 0, 255 );
	m_pStaminaFlash->setParent( m_pStaminaBarFull );
	m_pStaminaFlash->setSize( wide, tall );
	m_pStaminaFlash->setPos( 0, 0 );
	m_pStaminaFlash->setBgColor( 255, 0, 0, 255 );
	m_pStaminaFlash->setVisible( true );

	m_pBlackScreen = new Panel( 0, 0, 0, 0 );
	m_pBlackScreen->setParent( this );
	m_pBlackScreen->setSize( ScreenWidth, ScreenHeight );
	m_pBlackScreen->setBgColor( 0, 0, 0, 255 );
	m_pBlackScreen->setVisible( false );

	m_pVoteLabel = new Label( " ", ScreenWidth / 2 - 128, ScreenHeight / 2, 256, 32 );
	m_pVoteLabel->setParent( this );
	m_pVoteLabel->setFont( m_pPagerFont );
	m_pVoteLabel->setFgColor( 255, 255, 255, 0 );
	m_pVoteLabel->setBgColor( 0, 0, 0, 255 );
	m_pVoteLabel->setText( "Press Vote button when ready" );
	m_pVoteLabel->setContentAlignment( Label::a_center );
	m_pVoteLabel->setVisible( false );

	for ( i = 0; i < 4; i++ )
	{
		m_pLobbyLabel[i] = new Label( " ", ScreenWidth / 2 - 512 + i * 256, ScreenHeight / 2 + 32, 256, 32 );
		m_pLobbyLabel[i]->setParent( m_pLobbyBackground );
		m_pLobbyLabel[i]->setFont( m_pPagerFont );
		m_pLobbyLabel[i]->setFgColor( 255, 255, 255, 0 );
		m_pLobbyLabel[i]->setBgColor( 0, 0, 0, 255 );
		m_pLobbyLabel[i]->setText( "Gay" );
		m_pLobbyLabel[i]->setContentAlignment( Label::a_center );
		m_pLobbyLabel[i]->setVisible( false );
	}

	m_pLobbyLabel[0]->setPos( 116, 257 );
	m_pLobbyLabel[1]->setPos( 116, 440 );
	m_pLobbyLabel[2]->setPos( 429, 257 );
	m_pLobbyLabel[3]->setPos( 429, 440 );

	m_pVoteKickLabel = new Label( " ", ScreenWidth / 2 - 128, ScreenHeight / 2 + 64, 256, 32 );
	m_pVoteKickLabel->setParent( this );
	m_pVoteKickLabel->setFont( m_pPagerFont );
	m_pVoteKickLabel->setFgColor( 255, 255, 255, 0 );
	m_pVoteKickLabel->setBgColor( 0, 0, 0, 255 );
	m_pVoteKickLabel->setText( "Type votekick name in chat to votekick a player" );
	m_pVoteKickLabel->setContentAlignment( Label::a_center );
	m_pVoteKickLabel->setVisible( false );

	m_pVoteLabel->setParent( m_pLobbyBackground );
	m_pVoteLabel->setPos( 272, 492 );
	m_pVoteKickLabel->setParent( m_pLobbyBackground );
	m_pVoteKickLabel->setPos( 272, 524 );

	m_pTimerLabel = new Label( " ", 0, 0, ScreenWidth, 64 );
	m_pTimerLabel->setParent( this );
	m_pTimerLabel->setFont( m_pCreditsFont );
	m_pTimerLabel->setFgColor( 255, 255, 255, 0 );
	m_pTimerLabel->setBgColor( 0, 0, 0, 255 );
	m_pTimerLabel->setText( "00:00:00" );
	m_pTimerLabel->setContentAlignment( Label::a_center );
	m_pTimerLabel->setVisible( false );

	for ( i = 0; i < 32; i++ )
	{
		m_pTimerPlusLabel[i] = new Label( " ", 0, 0, 64, 128 );
		m_pTimerPlusLabel[i]->setParent( this );
		m_pTimerPlusLabel[i]->setFont( m_pChapterTitleFont );
		m_pTimerPlusLabel[i]->setFgColor( 0, 128, 0, 0 );
		m_pTimerPlusLabel[i]->setBgColor( 0, 0, 0, 255 );
		m_pTimerPlusLabel[i]->setText( "+0" );
		m_pTimerPlusLabel[i]->setContentAlignment( Label::a_northwest );
		m_pTimerPlusLabel[i]->setVisible( false );
	}

	m_pMessageLabel2 = new Label( " ", 0, ScreenHeight / 2, ScreenWidth, 64 );
	m_pMessageLabel2->setParent( this );
	m_pMessageLabel2->setFont( m_pChapterTitleFont );
	m_pMessageLabel2->setFgColor( 0, 0, 0, 0 );
	m_pMessageLabel2->setBgColor( 0, 0, 0, 255 );
	m_pMessageLabel2->setText( " " );
	m_pMessageLabel2->setContentAlignment( Label::a_center );

	m_pLookingAtLabel = new Label( " ", ScreenWidth / 2 - 128, ScreenHeight / 2 - 40, 256, 32 );
	m_pLookingAtLabel->setParent( this );
	m_pLookingAtLabel->setFont( m_pPagerFont );
	m_pLookingAtLabel->setFgColor( 255, 255, 255, 0 );
	m_pLookingAtLabel->setBgColor( 255, 255, 255, 255 );
	m_pLookingAtLabel->setText( " " );
	m_pLookingAtLabel->setContentAlignment( Label::a_center );

	int iBarX, iBarY;
	m_pStaminaBar->getPos( iBarX, iBarY );

	m_pDeathLabel = new Label( "", ScreenWidth - 133, 5, 128, 40 );
	m_pDeathLabel->setParent( this );
	m_pDeathLabel->setFont( m_pPagerFont );
	m_pDeathLabel->setFgColor( 255, 0, 0, 255 );
	m_pDeathLabel->setBgColor( 255, 255, 255, 255 );
	m_pDeathLabel->setText( "" );
	m_pDeathLabel->setContentAlignment( Label::a_east );

	for ( i = 0; i < 4; i++ )
	{
		m_pPlayerLabel[i] = new Label( "", ScreenWidth - 133, ScreenHeight / 1.5 + i * 48, 128, 48 );
		m_pPlayerLabel[i]->setParent( this );
		m_pPlayerLabel[i]->setFont( m_pPagerFont );
		m_pPlayerLabel[i]->setFgColor( 255, 255, 255, 0 );
		m_pPlayerLabel[i]->setBgColor( 255, 255, 255, 255 );
		m_pPlayerLabel[i]->setText( "" );
		m_pPlayerLabel[i]->setContentAlignment( Label::a_east );
		m_pPlayerLabel[i]->setVisible( false );

		m_pFollowIconPanel[i] = new ImagePanel( m_pFollowIcon[i] );
		m_pFollowIconPanel[i]->setParent( this );
		m_pFollowIconPanel[i]->setPos( ScreenWidth - 261, ScreenHeight / 1.5 + i * 48 );
		m_pFollowIconPanel[i]->setVisible( false );

		m_pSyringeIconPanel[i] = new ImagePanel( m_pSyringeIcon[i] );
		m_pSyringeIconPanel[i]->setParent( this );
		m_pSyringeIconPanel[i]->setPos( ScreenWidth - 133, ScreenHeight / 1.5 + i * 48 );
		m_pSyringeIconPanel[i]->setVisible( false );
	}

	for ( i = 0; i < 5; i++ )
	{
		if ( !m_iSayTextY )
			m_iSayTextY = ScreenHeight / 2;
		if ( !m_iSayTextX )
			m_iSayTextX = 5;

		int w, h;
		m_pPagerFont->getTextSize( "ysizetestg", w, h );

		m_pSayTextLabel[i] = new Label( "", m_iSayTextX, m_iSayTextY + i * 32, ScreenWidth, 32 );
		m_pSayTextLabel[i]->setParent( this );
		m_pSayTextLabel[i]->setFont( m_pPagerFont );
		m_pSayTextLabel[i]->setFgColor( 255, 255, 255, 0 );
		m_pSayTextLabel[i]->setBgColor( 0, 0, 0, 255 );
		m_pSayTextLabel[i]->setText( "" );
		m_pSayTextLabel[i]->setContentAlignment( Label::a_west );
		m_pSayTextLabel[i]->setVisible( true );
	}

	g_iLetterboxHeight = ScreenHeight / 480.0f * 25.0f + 0.5f;

	m_pLetterboxTop = new Panel( 0, 0, ScreenWidth, g_iLetterboxHeight );
	m_pLetterboxTop->setParent( this );
	m_pLetterboxTop->setSize( ScreenWidth, g_iLetterboxHeight );
	m_pLetterboxTop->setBgColor( 0, 0, 0, 0 );
	m_pLetterboxTop->setVisible( true );

	m_pLetterboxBottom = new Panel( 0, ScreenHeight - g_iLetterboxHeight, ScreenWidth, g_iLetterboxHeight );
	m_pLetterboxBottom->setParent( this );
	m_pLetterboxBottom->setSize( ScreenWidth, g_iLetterboxHeight );
	m_pLetterboxBottom->setBgColor( 0, 0, 0, 0 );
	m_pLetterboxBottom->setVisible( true );

	m_pMessageLabel = new Label( " ", 0, 0, ScreenWidth, 64 );
	m_pMessageLabel->setParent( this );
	m_pMessageLabel->setFont( m_pCreditsFont );
	m_pMessageLabel->setFgColor( 0, 0, 0, 0 );
	m_pMessageLabel->setBgColor( 0, 0, 0, 255 );
	m_pMessageLabel->setText( " " );
	m_pMessageLabel->setContentAlignment( Label::a_center );

	m_pHintBar = new ImagePanel( m_pHintBarImage );
	m_pHintBar->setParent( this );
	m_pHintBar->setPos( ScreenHeight / 2 - 400, -48 );
	m_pHintBar->setVisible( false );

	m_pHintLabel = new Label( " ", ScreenHeight / 2 - 400, -48, 800, 48 );
	m_pHintLabel->setParent( this );
	m_pHintLabel->setFont( m_pCreditsFont );
	m_pHintLabel->setFgColor( 0, 0, 0, 0 );
	m_pHintLabel->setBgColor( 0, 0, 0, 255 );
	m_pHintLabel->setText( " " );
	m_pHintLabel->setContentAlignment( Label::a_center );

	Initialize();
}

CHUDControl::~CHUDControl()
{
	delete m_pSMSIconImage;
	delete m_pInventoryHintImage;
	delete m_pCrosshairImage;
	delete m_pHealthBarImage;
	delete m_pHealthBarFullImage;
	delete m_pHealthBarEmptyImage;
	delete m_pStaminaBarImage;
	delete m_pStaminaBarFullImage;
	delete m_pStaminaBarEmptyImage;
	delete m_pLogoImage;
	delete m_pHintBarImage;
	delete m_pIntroductionImage;
}

void CHUDControl::HandleHUDVisibility()
{
	int i;

	if ( !strcmp( gEngfuncs.pfnGetLevelName(), "maps/c_nightmare.bsp" ) )
	{
		if ( m_pSMSIcon->isVisible() )
			m_pSMSIcon->setVisible( false );
		if ( m_pInventoryHint->isVisible() )
			m_pInventoryHint->setVisible( false );
		if ( m_pHealthBar->isVisible() )
			m_pHealthBar->setVisible( false );
		if ( m_pHealthBarFull->isVisible() )
			m_pHealthBarFull->setVisible( false );
		if ( m_pHealthBarEmpty->isVisible() )
			m_pHealthBarEmpty->setVisible( false );
		if ( m_pStaminaBar->isVisible() )
			m_pStaminaBar->setVisible( false );
		if ( m_pStaminaBarFull->isVisible() )
			m_pStaminaBarFull->setVisible( false );
		if ( m_pStaminaBarEmpty->isVisible() )
			m_pStaminaBarEmpty->setVisible( false );
		return;
	}

	if ( !gHUD.m_pCvarDraw->value || ( gHUD.m_iHideHUDDisplay & ( HIDEHUD_ALL | HIDEHUD_HEALTH ) ) )
	{
		if ( m_pSMSIcon->isVisible() )
			m_pSMSIcon->setVisible( false );
		if ( m_pInventoryHint->isVisible() )
			m_pInventoryHint->setVisible( false );
		if ( m_pTimerLabel->isVisible() )
			m_pTimerLabel->setVisible( false );
		for ( i = 0; i < 32; i++ )
		{
			if ( m_pTimerPlusLabel[i]->isVisible() )
				m_pTimerPlusLabel[i]->setVisible( false );
		}
		if ( m_pOverlay->isVisible() )
			m_pOverlay->setVisible( false );
		if ( m_pHealthBar->isVisible() )
			m_pHealthBar->setVisible( false );
		if ( m_pHealthBarFull->isVisible() )
			m_pHealthBarFull->setVisible( false );
		if ( m_pHealthBarEmpty->isVisible() )
			m_pHealthBarEmpty->setVisible( false );
		if ( m_pStaminaBar->isVisible() )
			m_pStaminaBar->setVisible( false );
		if ( m_pStaminaBarFull->isVisible() )
			m_pStaminaBarFull->setVisible( false );
		if ( m_pStaminaBarEmpty->isVisible() )
			m_pStaminaBarEmpty->setVisible( false );
	}

	if ( !gHUD.m_iHideHUDDisplay && gHUD.m_pCvarDraw->value )
	{
		if ( !m_pSMSIcon->isVisible() && m_bSMSIcon )
			m_pSMSIcon->setVisible( true );
		if ( !m_pInventoryHint->isVisible() && m_bInventoryHint )
			m_pInventoryHint->setVisible( true );
		if ( !m_pHealthBar->isVisible() )
			m_pHealthBar->setVisible( true );
		if ( !m_pHealthBarFull->isVisible() )
			m_pHealthBarFull->setVisible( true );
		if ( !m_pHealthBarEmpty->isVisible() )
			m_pHealthBarEmpty->setVisible( true );
		if ( !m_pStaminaBar->isVisible() )
			m_pStaminaBar->setVisible( true );
		if ( !m_pStaminaBarFull->isVisible() )
			m_pStaminaBarFull->setVisible( true );
		if ( !m_pStaminaBarEmpty->isVisible() )
			m_pStaminaBarEmpty->setVisible( true );
	}
}

void CHUDControl::Initialize()
{
	int i;

	m_bTimerPaused = false;
	g_iSimonMode = 0;
	gHUD.m_szSleeveTexture[0] = '\0';
	gHUD.m_szFingerTexture[0] = '\0';
	gHUD.m_szGloveTexture[0] = '\0';

	m_pTimerLabel->setFgColor( 128, 255, 128, 0 );

	if ( m_bTimer || ( m_pTimerLabel && m_pTimerLabel->isVisible() ) )
	{
		if ( m_pTimerLabel )
		{
			m_bTimer = false;
			m_pTimerLabel->setVisible( false );
			for ( i = 0; i < 32; i++ )
			{
				if ( m_pTimerPlusLabel[i]->isVisible() )
					m_pTimerPlusLabel[i]->setVisible( false );
			}
		}
	}

	m_iTimerLimit = 0;
	m_iTimerSeconds = 0;
	m_iTimerMinutes = 0;
	m_iTimerHours = 0;
	m_iIntroductionAlpha = 255;
	m_bIntroductionFadeIn = false;
	m_pIntroduction->setVisible( false );
	m_pLobbyBackground->setVisible( false );
	m_bLobby = false;

	for ( i = 0; i <= MAX_PLAYERS; i++ )
	{
		gHUD.m_Health.m_iAdLibState[i] = 0;
		gHUD.m_Health.m_bSyringe[i] = false;
		gHUD.m_Health.m_flAdLibTime[i] = gEngfuncs.GetClientTime();
	}

	for ( i = 0; i < 4; i++ )
	{
		m_bPlayerReady[i] = false;
		m_pFollowIconPanel[i]->setVisible( false );
		m_pSyringeIconPanel[i]->setVisible( false );
	}

	m_pLookingAtLabel->setText( "" );
	m_pLookingAtLabel->setVisible( false );
	m_flLookingAtTime = gEngfuncs.GetClientTime();
	m_flTimerPlusTime = gEngfuncs.GetClientTime();
	m_flTimerNextSecond = gEngfuncs.GetClientTime();
	m_pDeathLabel->setText( "" );
	m_iDeathAlpha = 255;
	m_bLogoF5C = false;
	m_iLogo = 0;
	setVisible( true );

	m_flSMSIconTime = gEngfuncs.GetClientTime();
	m_iSMSIconAlpha = 255;
	m_pSMSIconImage->setColor( Color( 255, 255, 255, 255 ) );
	ShowSMSIcon( false );
	m_pLogo->setVisible( false );
	m_bSMSIcon = false;

	m_iMessageType = 5;
	m_iChapterType = 5;
	m_flStaminaFlashTime = gEngfuncs.GetClientTime();
	m_szHintLine1[0] = '\0';
	m_szHintLine2[0] = '\0';
	m_szHintLine3[0] = '\0';
	m_szHintNoButton[0] = '\0';
	m_iLogoAlpha = 255;
	m_bShowLogo = false;
	m_szMessage2[0] = '\0';
	m_flHintSlideInTime = -1;
	m_flHintSlideInLength = -1;
	m_flHintSlideOutTime = -1;
	m_flHintSlideOutLength = -1;
	m_flLogoTime = 0;
	m_iMessageType = 0;
	m_iMessageTime = 0;
	m_iMessageC4 = 0;
	m_iMessageC8 = 0;
	m_iMessageCC = 0;
	m_iMessageD0 = 0;
	m_szMessage[0] = '\0';
	m_flZoomLength = 0;
	m_flZoomTime = 0;
	m_flZoomFOV = 0;

	for ( i = 0; i < 5; i++ )
		m_pSayTextLabel[i]->setText( "" );

	m_pMessageLabel2->setText( "" );
	m_pMessageLabel->setText( "" );
	m_pHintLabel->setText( "" );

	m_bHint = false;
	m_iHintAlpha = 255;
	m_szChapter[0] = '\0';
	m_flMessageAlpha = 0;
	m_bMessageFadeOut = false;
	m_bMessageHold = false;
	m_flMessageHoldTime = 0;
	m_flMessageF24 = 0;
	m_iMessageChars = 0;
	m_iChapterType = 0;
	m_flChapterAlpha = 0;
	m_bChapterFadeOut = false;
	m_bChapterF1E = false;
	m_flChapterHoldTime = 0;
	m_flChapterNextChar = 0;
	m_iChapterChars = 0;

	m_pInventoryHint->setVisible( false );
	m_bInventoryHint = false;
	m_flInventoryHintTime = gEngfuncs.GetClientTime();
	m_iInventoryHintY = -160;
}

void CHUDControl::LogoUpdate()
{
	if ( m_bShowLogo )
	{
		if ( CVAR_GET_FLOAT( "host_framerate" ) != 0 )
		{
			if ( m_pLogo->isVisible() )
				m_pLogo->setVisible( false );
			m_bShowLogo = false;
			m_pLogoImage->setColor( Color( 255, 255, 255, 255 ) );
			return;
		}

		if ( gEngfuncs.GetClientTime() < m_flLogoTime )
		{
			if ( !m_pLogo->isVisible() )
				m_pLogo->setVisible( true );

			m_iLogoAlpha -= 15;
			if ( m_iLogoAlpha < 0 )
				m_iLogoAlpha = 0;
			m_pLogoImage->setColor( Color( 255, 255, 255, m_iLogoAlpha ) );
		}
		else
		{
			m_iLogoAlpha += 15;
			if ( m_iLogoAlpha > 255 )
				m_iLogoAlpha = 255;
			m_pLogoImage->setColor( Color( 255, 255, 255, m_iLogoAlpha ) );

			if ( m_pLogo->isVisible() && m_iLogoAlpha >= 255 )
			{
				m_pLogo->setVisible( false );
				m_bShowLogo = false;
				m_iLogoAlpha = 255;
			}
		}
	}
	else
	{
		if ( m_pLogo->isVisible() )
			m_pLogo->setVisible( false );
	}
}

void CHUDControl::RenderMessages()
{
	float flTime = gEngfuncs.GetClientTime();

	if ( m_bHint )
	{
		int x, y;

		if ( m_flHintTime > flTime )
		{
			if ( !m_pHintLabel->isVisible() )
				m_pHintLabel->setVisible( true );
			if ( !m_pHintBar->isVisible() )
				m_pHintBar->setVisible( true );

			if ( m_iHintAlpha > 0 )
				m_iHintAlpha -= 10;
			if ( m_iHintAlpha < 0 )
				m_iHintAlpha = 0;

			m_pHintLabel->getPos( x, y );
			if ( y < 0 )
				y += 2;
			if ( y > 0 )
				y = 0;

			m_pHintBar->setPos( ScreenWidth / 2 - 400, y );
			m_pHintBarImage->setColor( Color( 255, 255, 255, m_iHintAlpha ) );
			m_pHintLabel->setPos( ScreenWidth / 2 - 400, y );
			m_pHintLabel->setFgColor( 255, 255, 255, m_iHintAlpha );
		}
		else
		{
			if ( m_iHintAlpha < 255 )
				m_iHintAlpha += 10;
			if ( m_iHintAlpha > 255 )
				m_iHintAlpha = 255;

			m_pHintLabel->setFgColor( 255, 255, 255, m_iHintAlpha );

			m_pHintLabel->getPos( x, y );
			if ( y > -48 )
				y -= 2;
			if ( y < -48 )
				y = -48;

			m_pHintLabel->setPos( ScreenWidth / 2 - 400, y );
			m_pHintBar->setPos( ScreenWidth / 2 - 400, y );
			m_pHintBarImage->setColor( Color( 255, 255, 255, m_iHintAlpha ) );

			if ( m_iHintAlpha >= 255 )
			{
				int w, h;

				switch ( m_iHintLine )
				{
				case 1:
					m_iHintLine = 2;
					if ( strlen( m_szHintLine2 ) > 2 )
					{
						m_pHintLabel->setText( m_szHintLine2 );
						m_pCreditsFont->getTextSize( m_szHintLine2, w, h );
						m_pHintLabel->setPos( ScreenWidth / 2 - 400, -48 );
						m_bHint = true;
						m_iHintAlpha = 255;
						m_flHintTime = gEngfuncs.GetClientTime() + 5;
					}
					else
					{
						if ( !m_pHintLabel->isVisible() )
						{
							m_bHint = false;
							m_pHintLabel->setVisible( true );
						}
						if ( m_pHintBar->isVisible() == true )
							m_pHintBar->setVisible( false );
					}
					break;
				case 2:
					m_iHintLine = 3;
					if ( strlen( m_szHintLine3 ) > 2 )
					{
						m_pHintLabel->setText( m_szHintLine3 );
						m_pCreditsFont->getTextSize( m_szHintLine3, w, h );
						m_pHintLabel->setPos( ScreenWidth / 2 - 400, -48 );
						m_bHint = true;
						m_iHintAlpha = 255;
						m_flHintTime = gEngfuncs.GetClientTime() + 5;
					}
					else
					{
						if ( !m_pHintLabel->isVisible() )
						{
							m_bHint = false;
							m_pHintLabel->setVisible( true );
						}
						if ( m_pHintBar->isVisible() == true )
							m_pHintBar->setVisible( false );
					}
					break;
				case 3:
					if ( !m_pHintLabel->isVisible() )
					{
						m_bHint = false;
						m_pHintLabel->setVisible( true );
					}
					if ( m_pHintBar->isVisible() == true )
						m_pHintBar->setVisible( false );
					break;
				default:
					if ( !m_pHintLabel->isVisible() )
					{
						m_bHint = false;
						m_pHintLabel->setVisible( true );
					}
					if ( m_pHintBar->isVisible() == true )
						m_pHintBar->setVisible( false );
					break;
				}
			}
		}
	}
	else
	{
		if ( m_pHintLabel->isVisible() )
			m_pHintLabel->setVisible( false );
		if ( m_pHintBar->isVisible() == true )
			m_pHintBar->setVisible( false );
	}

	if ( m_szChapter[0] )
	{
		int w, h, x, y;

		if ( m_bChapterHold && flTime > m_flChapterHoldTime )
		{
			m_bChapterFadeOut = true;
			m_bChapterHold = false;
			m_flChapterAlpha = 0;
			m_iChapterChars = strlen( m_szChapter );
		}

		if ( flTime > m_flChapterNextChar )
		{
			m_flChapterNextChar = flTime + 0.025;
			if ( !m_bMessageFadeOut )
			{
				if ( m_iChapterChars < (int)strlen( m_szChapter ) )
					m_iChapterChars++;
			}
		}

		if ( !m_bChapterFadeOut )
		{
			if ( !m_bChapterHold )
			{
				if ( m_flChapterAlpha > 0 )
					m_flChapterAlpha = m_flChapterAlpha - gHUD.m_flTimeDelta * 250;
				if ( m_flChapterAlpha < 0 )
				{
					m_bChapterHold = true;
					m_flChapterAlpha = 0;
					m_flChapterHoldTime = flTime + 3.0;
				}
			}
		}
		else if ( m_bChapterFadeOut == true && !m_bChapterHold )
		{
			if ( m_flChapterAlpha < 255.0 )
				m_flChapterAlpha = gHUD.m_flTimeDelta * 125 + m_flChapterAlpha;
			if ( m_flChapterAlpha > 255.0 )
			{
				m_bChapterFadeOut = false;
				m_flChapterAlpha = 255;
				m_szChapter[0] = '\0';
			}
		}

		drawSetTextFont( m_pChapterTitleFont );
		if ( m_iChapterType <= 5 )
			drawSetTextColor( 255, 255, 255, (int)m_flChapterAlpha );
		else
			drawSetTextColor( 255, 0, 0, (int)m_flChapterAlpha );
		m_pChapterTitleFont->getTextSize( m_szChapter, w, h );
		x = ScreenWidth / 2 - w / 2;
		y = ScreenHeight / 2 - h / 2;
		drawSetTextPos( x, y );

		if ( m_szChapter[0] && m_iChapterType <= 5 )
			gEngfuncs.pfnFillRGBA( x, y + h, (int)( m_iChapterChars / (float)strlen( m_szChapter ) * w ), 2, 255, 255, 255, (int)( 255.0f - m_flChapterAlpha ) );

		m_pMessageLabel2->setSize( ScreenWidth, 64 );
		m_pMessageLabel2->setText( m_szChapter );
		m_pMessageLabel2->setPos( 0, ScreenHeight / 2 - 32 );
		m_pMessageLabel2->setContentAlignment( Label::a_center );
		if ( m_iChapterType <= 5 )
			m_pMessageLabel2->setFgColor( 255, 255, 255, (int)m_flChapterAlpha );
		else
			m_pMessageLabel2->setFgColor( 255, 0, 0, (int)m_flChapterAlpha );
	}

	if ( !(int)CVAR_GET_FLOAT( "cl_subtitles" ) )
		return;

	if ( m_szMessage[0] )
	{
		unsigned int iLen2 = strlen( m_szMessage2 );
		if ( strlen( m_szMessage ) > 1 )
		{
			if ( m_bMessageHold && flTime > m_flMessageHoldTime )
			{
				m_bMessageFadeOut = true;
				m_bMessageHold = false;
				m_flMessageAlpha = 0;
				m_iMessageTime = 0;
				m_iMessageChars = strlen( m_szChapter );
			}

			if ( !m_bMessageFadeOut )
			{
				if ( !m_bMessageHold )
				{
					if ( m_flMessageAlpha > 0 )
						m_flMessageAlpha = m_flMessageAlpha - gHUD.m_flTimeDelta * 500;
					if ( m_flMessageAlpha < 0 )
					{
						m_bMessageHold = true;
						m_flMessageAlpha = 0;
						m_flMessageHoldTime = flTime + 4.0;
					}
				}
			}
			else if ( m_bMessageFadeOut == true && !m_bMessageHold )
			{
				if ( m_flMessageAlpha < 255.0 )
					m_flMessageAlpha = gHUD.m_flTimeDelta * 500 + m_flMessageAlpha;
				if ( m_flMessageAlpha > 255.0 )
				{
					m_bMessageFadeOut = false;
					m_flMessageAlpha = 255;
					m_szMessage[0] = '\0';
				}
			}

			int a = m_flMessageAlpha;
			drawSetTextFont( m_pCreditsFont );

			switch ( m_iMessageType )
			{
			case 0:
			case 3:
				drawSetTextColor( 255, 255, 0, a );
				break;
			case 1:
				drawSetTextColor( 0, 128, 255, a );
				break;
			case 4:
				drawSetTextColor( 0, 255, 0, a );
				break;
			case 5:
				drawSetTextColor( 200, 200, 200, a );
				break;
			case 2:
			case 6:
				drawSetTextColor( 255, 0, 0, a );
				break;
			}

			int w, h, w2, x, y;
			m_pCreditsFont->getTextSize( m_szMessage, w, h );
			if ( iLen2 > 1 )
				m_pCreditsFont->getTextSize( m_szMessage2, w2, h );

			if ( m_iMessageType <= 5 )
			{
				x = ScreenWidth / 2 - w / 2;
				y = ScreenHeight - h * 2;
			}
			else
			{
				x = 10;
				y = ScreenHeight / 2 - h / 2;
			}

			drawSetTextPos( x, iLen2 > 1 ? y - h : y );

			m_pMessageLabel->setSize( ScreenWidth, 64 );
			m_pMessageLabel->setText( m_szMessage );
			m_pCreditsFont->getTextSize( m_szMessage, w, h );
			m_pMessageLabel->setPos( 0, ScreenHeight - g_iLetterboxHeight * 3 );
			m_pMessageLabel->setContentAlignment( Label::a_center );

			switch ( m_iMessageType )
			{
			case 0:
			case 3:
				m_pMessageLabel->setFgColor( 255, 255, 0, a );
				break;
			case 1:
				m_pMessageLabel->setFgColor( 0, 128, 255, a );
				break;
			case 4:
				m_pMessageLabel->setFgColor( 0, 255, 0, a );
				break;
			case 5:
				m_pMessageLabel->setFgColor( 200, 200, 200, a );
				break;
			case 2:
			case 6:
				m_pMessageLabel->setFgColor( 255, 0, 0, a );
				break;
			}

			if ( iLen2 > 1 )
			{
				m_pCreditsFont->getTextSize( m_szMessage2, w, h );
				drawSetTextPos( ScreenWidth / 2 - w / 2, ScreenHeight - h * 2 );
			}
		}
	}
	else
	{
		m_iMessageCC = 0;
		m_iMessageD0 = 0;
		m_iMessageC4 = 0;
		m_iMessageC8 = 0;
		m_iMessageType = 5;
		if ( strlen( m_szMessage ) > 1 )
			m_szMessage[0] = '\0';
		if ( strlen( m_szMessage2 ) > 1 )
			m_szMessage2[0] = '\0';
	}
}

void CHUDControl::ShowSMSIcon( bool bShow )
{
	m_iSMSIconAlpha = 255;
	m_bSMSIcon = bShow;
	m_pSMSIcon->setVisible( bShow );
}

void CHUDControl::UpdateOnPlayerInfo()
{
}

void CHUDControl::paint()
{
	int h, w;
	int x, y;

	if ( g_iDickMode )
	{
		switch ( gEngfuncs.pfnRandomLong( 1, 4 ) )
		{
		case 1:
			drawSetTextFont( m_pCreditsFont );
			drawSetTextColor( 255, 255, 0, 1 );
			m_pCreditsFont->getTextSize( "Dicks", w, h );
			x = gEngfuncs.pfnRandomLong( 0, 50 ) + ( ScreenWidth / 2 - w / 2 );
			y = gEngfuncs.pfnRandomLong( 0, 50 ) + ( ScreenHeight / 2 - h / 2 );
			drawSetTextPos( x, y );
			drawPrintText( "Dicks", 5 );
			break;
		case 2:
			drawSetTextFont( m_pCreditsFont );
			drawSetTextColor( 255, 255, 0, 1 );
			m_pCreditsFont->getTextSize( "Balls", w, h );
			x = gEngfuncs.pfnRandomLong( 0, 50 ) + ( ScreenWidth / 2 - w / 2 );
			y = gEngfuncs.pfnRandomLong( 0, 50 ) + ( ScreenHeight / 2 - h / 2 );
			drawSetTextPos( x, y );
			drawPrintText( "Balls", 5 );
			break;
		case 3:
			drawSetTextFont( m_pCreditsFont );
			drawSetTextColor( 255, 255, 0, 1 );
			m_pCreditsFont->getTextSize( "Dongs", w, h );
			x = gEngfuncs.pfnRandomLong( 0, 50 ) + ( ScreenWidth / 2 - w / 2 );
			y = gEngfuncs.pfnRandomLong( 0, 50 ) + ( ScreenHeight / 2 - h / 2 );
			drawSetTextPos( x, y );
			drawPrintText( "Dongs", 5 );
			break;
		case 4:
			drawSetTextFont( m_pCreditsFont );
			drawSetTextColor( 255, 255, 0, 1 );
			m_pCreditsFont->getTextSize( "Butts", w, h );
			x = gEngfuncs.pfnRandomLong( 0, 50 ) + ( ScreenWidth / 2 - w / 2 );
			y = gEngfuncs.pfnRandomLong( 0, 50 ) + ( ScreenHeight / 2 - h / 2 );
			drawSetTextPos( x, y );
			drawPrintText( "Butts", 5 );
			break;
		}
	}

	// port credit, once per game, not on landmark changes
	static char szCreditMap[64];
	static float flCreditEnd;
	static bool bCreditShown;
	const char *pszLevel = gEngfuncs.pfnGetLevelName();
	float flNow = gEngfuncs.GetClientTime();

	if ( !pszLevel || !pszLevel[0] || strstr( pszLevel, "c_game_menu" ) )
		bCreditShown = false;

	if ( pszLevel && strcmp( pszLevel, szCreditMap ) )
	{
		strncpy( szCreditMap, pszLevel, sizeof( szCreditMap ) - 1 );
		szCreditMap[sizeof( szCreditMap ) - 1] = '\0';

		if ( !bCreditShown && ( strstr( pszLevel, "c_start.bsp" ) || strstr( pszLevel, "c_apartment1.bsp" ) ) )
		{
			flCreditEnd = flNow + 4.0f;
			bCreditShown = true;
		}
		else
			flCreditEnd = 0;
	}

	if ( flCreditEnd > flNow )
	{
		int a = 0;
		if ( flCreditEnd - flNow < 0.5f )
			a = (int)( 255 - ( flCreditEnd - flNow ) * 510 );

		drawSetTextFont( m_pCreditsFont );
		drawSetTextColor( 255, 255, 255, a );
		m_pCreditsFont->getTextSize( "Ported By SiemON", w, h );
		x = ScreenWidth / 2 - w / 2;
		y = ScreenHeight / 2 - h / 2;
		if ( m_szChapter[0] )
			y += 64;
		drawSetTextPos( x, y );
		drawPrintText( "Ported By SiemON", 16 );
	}

	RenderMessages();
	Panel::paint();
}

void CHUDControl::paintBackground()
{
	Panel::paintBackground();
}

void CHUDControl::solve()
{
	int i;
	int r, g, b, a;
	int x, y, x2, y2;
	char szText[128];
	char szTime[128];
	char szSeconds[32];
	char szMinutes[32];
	char szHours[32];

	if ( m_pIntroduction->isVisible() )
	{
		if ( m_bIntroductionFadeIn )
		{
			if ( m_iIntroductionAlpha > 0 )
				m_iIntroductionAlpha -= 2;
			if ( m_iIntroductionAlpha < 0 )
				m_iIntroductionAlpha = 0;
			m_pIntroductionImage->setColor( Color( 255, 255, 255, m_iIntroductionAlpha ) );
		}
		else
		{
			if ( m_iIntroductionAlpha < 255 )
				m_iIntroductionAlpha += 2;
			m_pIntroductionImage->setColor( Color( 255, 255, 255, m_iIntroductionAlpha ) );

			if ( m_iIntroductionAlpha >= 255 )
			{
				m_iIntroductionAlpha = 255;
				m_pIntroductionImage->setColor( Color( 255, 255, 255, 255 ) );
				m_pIntroduction->setVisible( false );
			}
		}
	}

	if ( m_bTimer )
	{
		if ( gEngfuncs.GetClientTime() > m_flTimerPlusTime )
		{
			m_flTimerPlusTime = gEngfuncs.GetClientTime() + 0.05;

			m_pTimerLabel->getFgColor( r, g, b, a );
			if ( r < 255 )
			{
				b += 5;
				r += 5;
			}
			if ( r > 255 )
			{
				r = 255;
				b = 255;
				m_pTimerLabel->setFgColor( 255, g, 255, a );
			}
			if ( g_iSimonMode )
			{
				r = 128;
				g = 0;
				b = 0;
			}
			m_pTimerLabel->setFgColor( r, g, b, a );

			for ( i = 0; i < 32; i++ )
			{
				if ( m_pTimerPlusLabel[i]->isVisible() )
				{
					m_pTimerPlusLabel[i]->getFgColor( r, g, b, a );
					a += 15;
					if ( a > 255 )
					{
						a = 255;
						m_pTimerPlusLabel[i]->setFgColor( r, g, b, 255 );
						m_pTimerPlusLabel[i]->setVisible( false );
					}
					else
					{
						m_pTimerPlusLabel[i]->setFgColor( r, g, b, a );
						m_pTimerPlusLabel[i]->getPos( x, y );
						x++;
						m_pTimerPlusLabel[i]->setPos( x, y );
					}
				}
			}
		}
	}

	if ( m_pLobbyLabel[0] && m_bLobby )
	{
		if ( gHUD.GetKeyForCommand( "+attack" ) )
			sprintf( szText, "Press %s to become ready/unready", gHUD.GetKeyForCommand( "+attack" ) );
		else
			strcpy( szText, "Press Primary Attack to become ready/unready" );
		m_pVoteLabel->setText( szText );

		for ( i = 0; i < 4; i++ )
		{
			if ( g_PlayerInfoList[i + 1].name && strlen( g_PlayerInfoList[i + 1].name ) >= 1 )
			{
				char szReady[256];
				if ( m_bPlayerReady[i] )
				{
					m_pLobbyLabel[i]->setFgColor( 0, 128, 0, 0 );
					sprintf( szReady, "%s - Ready", g_PlayerInfoList[i + 1].name );
				}
				else
				{
					m_pLobbyLabel[i]->setFgColor( 128, 0, 0, 0 );
					sprintf( szReady, "%s - Not ready", g_PlayerInfoList[i + 1].name );
				}
				m_pLobbyLabel[i]->setText( szReady );
			}
			else
			{
				m_pLobbyLabel[i]->setText( "-" );
				m_pLobbyLabel[i]->setFgColor( 255, 255, 255, 0 );
			}
		}
	}

	if ( m_bTimer )
	{
		if ( !m_pTimerLabel->isVisible() )
			m_pTimerLabel->setVisible( true );

		if ( gEngfuncs.GetClientTime() > m_flTimerNextSecond && !m_bTimerPaused )
		{
			m_iTimerSeconds++;
			if ( m_iTimerSeconds > 59 )
			{
				m_iTimerSeconds = 0;
				m_iTimerMinutes++;
			}

			int iSecondsLeft = 60 - m_iTimerSeconds;
			int iMinutesLeft = m_iTimerLimit / 60 - m_iTimerMinutes - 1;

			if ( iMinutesLeft < 0 )
				iMinutesLeft = 0;
			if ( iSecondsLeft < 0 )
				iSecondsLeft = 0;
			if ( m_iTimerHours < 0 )
				m_iTimerHours = 0;

			sprintf( szSeconds, ( iSecondsLeft >= 10 ) ? "%i" : "0%i", iSecondsLeft );
			sprintf( szMinutes, ( iMinutesLeft >= 10 ) ? "%i" : "0%i", iMinutesLeft );
			sprintf( szHours, ( m_iTimerHours >= 10 ) ? "%i" : "0%i", m_iTimerHours );
			sprintf( szTime, "%s:%s:%s", szHours, szMinutes, szSeconds );
			m_pTimerLabel->setText( szTime );
			m_flTimerNextSecond = gEngfuncs.GetClientTime() + 1;
		}
	}

	if ( gHUD.m_iFlashFlags & 0x40 )
	{
		if ( !m_pOverlay->isVisible() )
			m_pOverlay->setVisible( true );
		else
			m_pOverlay->setBgColor( 33, 177, 120, gEngfuncs.pfnRandomLong( 0, 5 ) + 192 );
	}
	else
	{
		if ( m_pOverlay->isVisible() )
			m_pOverlay->setVisible( false );
	}

	if ( gHUD.m_fPlayerDead )
	{
		m_pMessageLabel2->setText( "" );
		m_pMessageLabel->setText( "" );
		m_pHintLabel->setText( "" );

		m_bHint = false;
		m_iHintAlpha = 255;
		m_szChapter[0] = '\0';
		m_flMessageAlpha = 0;
		m_bMessageFadeOut = false;
		m_bMessageHold = false;
		m_flMessageHoldTime = 0;
		m_flMessageF24 = 0;
		m_iMessageChars = 0;
		m_iChapterType = 0;
		m_flChapterAlpha = 0;
		m_bChapterFadeOut = false;
		m_bChapterF1E = false;
		m_flChapterHoldTime = 0;
		m_flChapterNextChar = 0;
		m_iChapterChars = 0;
	}

	if ( gEngfuncs.pfnGetLevelName() && strstr( gEngfuncs.pfnGetLevelName(), "cof_" ) )
	{
		if ( m_iDeathAlpha < 255 )
			m_iDeathAlpha += 5;
		if ( m_iDeathAlpha < 0 )
			m_iDeathAlpha = 0;
		else if ( m_iDeathAlpha > 255 )
			m_iDeathAlpha = 255;

		if ( !m_pDeathLabel->isVisible() )
			m_pDeathLabel->setVisible( true );
		m_pDeathLabel->setFgColor( 255, 0, 0, m_iDeathAlpha );

		for ( i = 0; i < 5; i++ )
		{
			if ( !m_pSayTextLabel[i]->isVisible() )
				m_pSayTextLabel[i]->setVisible( true );
		}

		for ( i = 0; i < 4; i++ )
		{
			if ( g_PlayerInfoList[i + 1].name && strlen( g_PlayerInfoList[i + 1].name ) >= 1 )
			{
				if ( !m_pPlayerLabel[i]->isVisible() )
					m_pPlayerLabel[i]->setVisible( true );

				if ( gHUD.m_Health.m_flAdLibTime[i] > gEngfuncs.GetClientTime() )
				{
					if ( !m_pFollowIconPanel[i]->isVisible() )
						m_pFollowIconPanel[i]->setVisible( true );
				}
				else
				{
					if ( m_pFollowIconPanel[i]->isVisible() )
						m_pFollowIconPanel[i]->setVisible( false );
				}

				if ( gHUD.m_Health.m_bSyringe[i] )
				{
					if ( !m_pSyringeIconPanel[i]->isVisible() )
						m_pSyringeIconPanel[i]->setVisible( true );
				}
				else
				{
					if ( m_pSyringeIconPanel[i]->isVisible() )
						m_pSyringeIconPanel[i]->setVisible( false );
				}

				int w, h, w2, h2;
				sprintf( szTime, "%s - Health %i - Ping %d", g_PlayerInfoList[i + 1].name, gHUD.m_Health.m_iCoopHealth[i + 1], g_PlayerInfoList[i + 1].ping );
				m_pPagerFont->getTextSize( szTime, w, h );
				m_pPlayerLabel[i]->getSize( w2, h2 );
				m_pPlayerLabel[i]->getPos( x, y );
				m_pPlayerLabel[i]->setSize( w, h );
				m_pPlayerLabel[i]->setPos( ScreenWidth - w - 5, h * i + ScreenHeight / 1.5 );
				m_pPlayerLabel[i]->setText( szTime );
				m_pPlayerLabel[i]->getPos( x, y );
				m_pFollowIconPanel[i]->setPos( x - 28, y + 4 );
				m_pSyringeIconPanel[i]->setPos( x - 14, y + 4 );

				if ( m_iDevDonator[i + 1] == 1 )
					m_pPlayerLabel[i]->setFgColor( 117, 0, 0, 0 );
				else if ( m_iDevDonator[i + 1] == 2 )
					m_pPlayerLabel[i]->setFgColor( 255, 211, 81, 0 );
				else
					m_pPlayerLabel[i]->setFgColor( 255, 255, 255, 0 );
			}
			else
			{
				if ( m_pPlayerLabel[i]->isVisible() )
					m_pPlayerLabel[i]->setVisible( false );
				if ( m_pFollowIconPanel[i]->isVisible() )
					m_pFollowIconPanel[i]->setVisible( false );
				if ( m_pSyringeIconPanel[i]->isVisible() )
					m_pSyringeIconPanel[i]->setVisible( false );
			}
		}

		if ( m_flLookingAtTime > gEngfuncs.GetClientTime() )
		{
			if ( !m_pLookingAtLabel->isVisible() )
				m_pLookingAtLabel->setVisible( true );
		}
		else
		{
			if ( m_pLookingAtLabel->isVisible() )
				m_pLookingAtLabel->setVisible( false );
		}

		if ( g_flLookingAt != 0 && g_flLookingAt <= 4 )
		{
			char *pszName = g_PlayerInfoList[(int)g_flLookingAt].name;
			if ( pszName )
			{
				m_flLookingAtTime = gEngfuncs.GetClientTime() + 0.1;
				m_pLookingAtLabel->setText( pszName );
			}
		}
	}
	else
	{
		for ( i = 0; i < 4; i++ )
		{
			if ( m_pPlayerLabel[i]->isVisible() )
				m_pPlayerLabel[i]->setVisible( false );
			if ( m_pFollowIconPanel[i]->isVisible() )
				m_pFollowIconPanel[i]->setVisible( false );
			if ( m_pSyringeIconPanel[i]->isVisible() )
				m_pSyringeIconPanel[i]->setVisible( false );
		}

		for ( i = 0; i < 5; i++ )
		{
			if ( m_pSayTextLabel[i]->isVisible() )
				m_pSayTextLabel[i]->setVisible( false );
		}

		if ( m_pLookingAtLabel->isVisible() )
			m_pLookingAtLabel->setVisible( false );
		if ( m_pDeathLabel->isVisible() )
			m_pDeathLabel->setVisible( false );
		if ( m_iDeathAlpha < 255 )
			m_iDeathAlpha = 255;
	}

	m_pStaminaFlash->getBgColor( r, g, b, a );
	if ( m_flStaminaFlashTime > gEngfuncs.GetClientTime() )
	{
		m_pStaminaFlash->setBgColor( 255, 0, 0, 128 );
	}
	else
	{
		if ( a < 255 )
		{
			a += 15;
			m_pStaminaFlash->setBgColor( r, g, b, a );
		}
		if ( a > 255 )
		{
			a = 255;
			m_pStaminaFlash->setBgColor( r, g, b, 255 );
		}
	}

	static bool s_bLetterboxFrame = false;
	int iSpeed = g_iLetterboxHeight / 15;
	if ( iSpeed <= 0 )
		iSpeed = 1;

	if ( gHUD.m_BlackBar.m_iOn )
	{
		s_bLetterboxFrame = !s_bLetterboxFrame;
		if ( s_bLetterboxFrame )
		{
			m_pLetterboxTop->getPos( x, y );
			m_pLetterboxBottom->getPos( x2, y2 );

			if ( y < 0 )
				y += iSpeed;
			if ( y > 0 )
				y = 0;
			if ( y2 > ScreenHeight - g_iLetterboxHeight )
				y2 -= iSpeed;
			if ( y2 < ScreenHeight - g_iLetterboxHeight )
				y2 = ScreenHeight - g_iLetterboxHeight;

			m_pLetterboxTop->setPos( x, y );
			m_pLetterboxBottom->setPos( x2, y2 );
		}
	}
	else
	{
		s_bLetterboxFrame = !s_bLetterboxFrame;
		if ( s_bLetterboxFrame )
		{
			m_pLetterboxTop->getPos( x, y );
			m_pLetterboxBottom->getPos( x2, y2 );

			if ( y > -g_iLetterboxHeight )
				y -= iSpeed;
			if ( y < -g_iLetterboxHeight )
				y = -g_iLetterboxHeight;
			if ( y2 < ScreenHeight )
				y2 += iSpeed;
			if ( y2 > ScreenHeight )
				y2 = ScreenHeight;

			m_pLetterboxTop->setPos( x, y );
			m_pLetterboxBottom->setPos( x2, y2 );
		}
	}

	HandleHUDVisibility();
	LogoUpdate();

	if ( iWheelchair && m_pStaminaBar->isVisible() )
		m_pStaminaBar->setVisible( false );
	if ( iWheelchair && m_pStaminaBarFull->isVisible() )
		m_pStaminaBarFull->setVisible( false );
	if ( iWheelchair && m_pStaminaBarEmpty->isVisible() )
		m_pStaminaBarEmpty->setVisible( false );

	if ( gEngfuncs.GetClientTime() >= m_flZoomTime )
	{
		if ( m_flZoomTime + m_flZoomLength > gEngfuncs.GetClientTime() )
		{
			float flFrac = ( gEngfuncs.GetClientTime() - m_flZoomTime ) / m_flZoomLength;
			if ( flFrac > 1.0f )
				gHUD.m_iFOV = m_flZoomFOV;

			if ( gEngfuncs.GetClientTime() > m_flZoomTime + m_flZoomLength )
				gHUD.m_iFOV = m_flZoomFOV;
			else
				gHUD.m_iFOV = 90 - (int)( flFrac * m_flZoomFOV );

			if ( gHUD.m_iFOV > m_flZoomFOV )
				gHUD.m_iFOV = m_flZoomFOV;
		}
	}

	if ( m_pSMSIcon->isVisible() && m_bSMSIcon )
	{
		if ( gEngfuncs.GetClientTime() > m_flSMSIconTime )
		{
			if ( m_iSMSIconAlpha > 0 )
			{
				m_iSMSIconAlpha -= 15;
				m_pSMSIconImage->setColor( Color( 255, 255, 255, m_iSMSIconAlpha ) );
			}
			m_flSMSIconTime = gEngfuncs.GetClientTime() + 0.05;
		}
	}

	if ( m_pInventoryHint->isVisible() && m_bInventoryHint )
	{
		m_pInventoryHint->getPos( x, y );

		if ( gEngfuncs.GetClientTime() >= m_flHintSlideInTime )
		{
			if ( m_flHintSlideInTime + m_flHintSlideInLength > gEngfuncs.GetClientTime() )
			{
				m_pInventoryHint->setPos( x, (int)( ( gEngfuncs.GetClientTime() - m_flHintSlideInTime ) / m_flHintSlideInLength * 160 ) - 154 );
				if ( y < -161 )
				{
					m_bInventoryHint = false;
					y = -161;
				}
			}
		}

		if ( gEngfuncs.GetClientTime() >= m_flHintSlideOutTime )
		{
			if ( m_flHintSlideOutTime + m_flHintSlideOutLength > gEngfuncs.GetClientTime() )
			{
				float flFrac = ( gEngfuncs.GetClientTime() - m_flHintSlideOutTime ) / m_flHintSlideOutLength;
				if ( flFrac > 1.0f )
					m_bInventoryHint = false;
				m_pInventoryHint->setPos( x, (int)( flFrac * -160 ) );
				if ( y < -161 )
				{
					m_bInventoryHint = false;
					y = -161;
				}
				else if ( y > 0 )
					y = 0;
			}
		}

		if ( gEngfuncs.GetClientTime() < m_flHintSlideOutTime && ( GetAsyncKeyState( VK_RETURN ) & 0x8000 ) )
		{
			m_flHintSlideInTime = 0;
			m_flHintSlideInLength = 0;
			m_flHintSlideOutTime = gEngfuncs.GetClientTime();
			m_flHintSlideOutLength = 1;
		}
	}

	int iWide = m_pHealthBarEmpty->getWide();
	int iHealth = gHUD.m_Health.m_iHealth;
	if ( bWheelchairLock )
		iHealth = 0;
	if ( iHealth < 0 )
		iHealth = 0;
	m_pHealthBarEmpty->setSize( iWide, ( 100 - iHealth ) * 1.18 );
	m_pStaminaBarEmpty->setSize( m_pStaminaBarEmpty->getWide(), ( 100 - m_iStamina ) * 1.18 );

	Panel::solve();
}
