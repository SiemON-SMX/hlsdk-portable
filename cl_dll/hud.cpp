/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
// hud.cpp
//
// implementation of CHud class
//

//LRC - define to help track what calls are made on changelevel, save/restore, etc
#define ENGINE_DEBUG

#include <fstream>
#include <string>
#include <vector>

#include "hud.h"
#include "cl_util.h"
#include <string.h>
#include <stdio.h>
#include "parsemsg.h"
#include "hud_servers.h"
#include "vgui_int.h"
#include "vgui_TeamFortressViewport.h"
#include "mp3.h"
#include "rain.h" // buz
#include "grass.h" // buz
#include "vgui_subtitles.h" // buz
#include "vgui_radio.h" // buz
#include "vgui_hud.h" // buz
#include "vgui_tabpanel.h" // buz

#include "demo.h"
#include "demo_api.h"
#include "vgui_ScorePanel.h"
#include "triapiobjects.h"

#include "glmanager.h" // buz
#include "gl_renderer.h" // buz
#include "cgwater.h"
#include "exception.h" // buz
#include "log.h"

//=========================================================
//=========================================================
typedef unsigned long long uint64;
typedef int HSteamUser;

class CSteamID
{
public:
	uint64 ConvertToUint64() const
	{
		return m_unAll64Bits;
	}

private:
	uint64 m_unAll64Bits;
};

class ISteamUser
{
public:
	virtual HSteamUser GetHSteamUser() = 0;
	virtual bool BLoggedOn() = 0;
	virtual CSteamID GetSteamID() = 0;
};

static inline ISteamUser * SteamUser() { return 0; }
//=========================================================

class CHLVoiceStatusHelper : public IVoiceStatusHelper
{
public:
	virtual void GetPlayerTextColor(int entindex, int color[3])
	{
		color[0] = color[1] = color[2] = 255;

		if( entindex >= 0 && entindex < sizeof(g_PlayerExtraInfo)/sizeof(g_PlayerExtraInfo[0]) )
		{
			int iTeam = g_PlayerExtraInfo[entindex].teamnumber;

			if ( iTeam < 0 )
			{
				iTeam = 0;
			}

			iTeam = iTeam % iNumberOfTeamColors;

			color[0] = iTeamColors[iTeam][0];
			color[1] = iTeamColors[iTeam][1];
			color[2] = iTeamColors[iTeam][2];
		}
	}

	virtual void UpdateCursorState()
	{
		gViewPort->UpdateCursorState();
	}

	virtual int	GetAckIconHeight()
	{
		return ScreenHeight - gHUD.m_iFontHeight*3 - 6;
	}

	virtual bool			CanShowSpeakerLabels()
	{
		if( gViewPort && gViewPort->m_pScoreBoard )
			return !gViewPort->m_pScoreBoard->isVisible();
		else
			return false;
	}
};
int iDefaultFOV = 90;
int iSkipSceneFramerate = 2;
static CHLVoiceStatusHelper g_VoiceStatusHelper;


extern client_sprite_t *GetSpriteList(client_sprite_t *pList, const char *psz, int iRes, int iCount);

extern cvar_t *sensitivity;

int iMenuMode;
bool bXboxFix;
int iHudActive;
cvar_t *cl_lw = NULL;
int iGoro;
float flWeaponIdleSpeed;
float flWeaponIdleDampener;

extern int iDonator;
extern cl_entity_t *pAttachCamEnt;
extern intptr_t iBoat;
extern intptr_t iWheelchair;
extern bool bWheelchairLock;
extern cl_entity_t *pTelescopeEnt;
extern cl_entity_t *pCamTargetEnt;
extern vec3_t vecIronsightOfs;
extern bool bCustomFingerTexture;
extern bool bCustomGloveTexture;
extern int iGreyFade;
extern float flNextSoundCacheTime;
extern bool bUnknown105431E9;
extern float flUnknown105595A0;
extern float flUnknown105595A4;
extern float flUnknown105595A8;
extern int iUnknown105595AC;
extern float flUnknown1055959C;

int IsInDonatorGroup1( void );
int IsInDonatorGroup2( void );

void IrrKlangCreate( void );
void IrrKlangSetFade( float flParm1, float flParm2, float flParm3, float flParm4 );
void IrrKlangStop( float flFadeTime );

void ShutdownInput (void);

//DECLARE_MESSAGE(m_Logo, Logo)
int __MsgFunc_Logo(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_Logo(pszName, iSize, pbuf );
}

//DECLARE_MESSAGE(m_Logo, Logo)
//LRC
int __MsgFunc_HUDColor(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_HUDColor(pszName, iSize, pbuf );
}

//LRC
int __MsgFunc_SetFog(const char *pszName, int iSize, void *pbuf)
{
	gHUD.MsgFunc_SetFog( pszName, iSize, pbuf );
	return 1;
}

//LRC
int __MsgFunc_KeyedDLight(const char *pszName, int iSize, void *pbuf)
{
	gHUD.MsgFunc_KeyedDLight( pszName, iSize, pbuf );
	return 1;
}

//LRC
int __MsgFunc_AddShine(const char *pszName, int iSize, void *pbuf)
{
	gHUD.MsgFunc_AddShine( pszName, iSize, pbuf );
	return 1;
}

//LRC
int __MsgFunc_SetSky(const char *pszName, int iSize, void *pbuf)
{
	gHUD.MsgFunc_SetSky( pszName, iSize, pbuf );
	return 1;
}

int __MsgFunc_ResetHUD(const char *pszName, int iSize, void *pbuf)
{
#ifdef ENGINE_DEBUG
//	CONPRINT("## ResetHUD\n");
#endif
	return gHUD.MsgFunc_ResetHUD(pszName, iSize, pbuf );
}

int __MsgFunc_InitHUD(const char *pszName, int iSize, void *pbuf)
{
#ifdef ENGINE_DEBUG
//	CONPRINT("## InitHUD\n");
#endif
	gHUD.MsgFunc_InitHUD( pszName, iSize, pbuf );
	return 1;
}

int __MsgFunc_ViewMode(const char *pszName, int iSize, void *pbuf)
{
	gHUD.MsgFunc_ViewMode( pszName, iSize, pbuf );
	return 1;
}

int __MsgFunc_SetFOV(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_SetFOV( pszName, iSize, pbuf );
}

int __MsgFunc_Concuss(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_Concuss( pszName, iSize, pbuf );
}

int __MsgFunc_GameMode(const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_GameMode( pszName, iSize, pbuf );
}

// buz
int __MsgFunc_GasMask(const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_GasMask( pszName, iSize, pbuf );
}

// buz
int __MsgFunc_SpecTank(const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_SpecTank( pszName, iSize, pbuf );
}

int __MsgFunc_ProFont(const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_ProFont( pszName, iSize, pbuf );
}

int __MsgFunc_Maglite(const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_Maglite( pszName, iSize, pbuf );
}

int __MsgFunc_Blur(const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_Blur( pszName, iSize, pbuf );
}

int __MsgFunc_UnreadSMS(const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_UnreadSMS( pszName, iSize, pbuf );
}

int __MsgFunc_ViewKnock(const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_ViewKnock( pszName, iSize, pbuf );
}

int __MsgFunc_CoFWeapon(const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_CoFWeapon( pszName, iSize, pbuf );
}

int __MsgFunc_ViewEntity(const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_ViewEntity( pszName, iSize, pbuf );
}

int __MsgFunc_ViewLock(const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_ViewLock( pszName, iSize, pbuf );
}

int __MsgFunc_Ladder(const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_Ladder( pszName, iSize, pbuf );
}

int __MsgFunc_FallOver(const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_FallOver( pszName, iSize, pbuf );
}

int __MsgFunc_BlackWhite(const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_BlackWhite( pszName, iSize, pbuf );
}

int __MsgFunc_WeaponBody(const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_WeaponBody( pszName, iSize, pbuf );
}

int __MsgFunc_PhoneOff(const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_PhoneOff( pszName, iSize, pbuf );
}

int __MsgFunc_Muzzleflash(const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_Muzzleflash( pszName, iSize, pbuf );
}

int __MsgFunc_FlashFlags(const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_FlashFlags( pszName, iSize, pbuf );
}

int __MsgFunc_Boat(const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_Boat( pszName, iSize, pbuf );
}

int __MsgFunc_Ripple(const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_Ripple( pszName, iSize, pbuf );
}

int __MsgFunc_AddNote(const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_AddNote( pszName, iSize, pbuf );
}

int __MsgFunc_AttachCam(const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_AttachCam( pszName, iSize, pbuf );
}

int __MsgFunc_BigPrick(const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_BigPrick( pszName, iSize, pbuf );
}

int __MsgFunc_NewFade(const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_NewFade( pszName, iSize, pbuf );
}

int __MsgFunc_SkipScene(const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_SkipScene( pszName, iSize, pbuf );
}

int __MsgFunc_IronSights(const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_IronSights( pszName, iSize, pbuf );
}

int __MsgFunc_Wheelchair(const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_Wheelchair( pszName, iSize, pbuf );
}

int __MsgFunc_GreyFade(const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_GreyFade( pszName, iSize, pbuf );
}

int __MsgFunc_TakePhoto(const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_TakePhoto( pszName, iSize, pbuf );
}

int __MsgFunc_Objectives(const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_Objectives( pszName, iSize, pbuf );
}

int __MsgFunc_Unlocks(const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_Unlocks( pszName, iSize, pbuf );
}

int __MsgFunc_CustomSlv(const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_CustomSlv( pszName, iSize, pbuf );
}

int __MsgFunc_CustomFng(const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_CustomFng( pszName, iSize, pbuf );
}

int __MsgFunc_CustomGlv(const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_CustomGlv( pszName, iSize, pbuf );
}

int __MsgFunc_PlaySFX(const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_PlaySFX( pszName, iSize, pbuf );
}

int __MsgFunc_Inventory(const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_Inventory( pszName, iSize, pbuf );
}

int __MsgFunc_Materials(const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_Materials( pszName, iSize, pbuf );
}

// TFFree Command Menu
void __CmdFunc_OpenCommandMenu(void)
{
	if ( gViewPort )
	{
		gViewPort->ShowCommandMenu( gViewPort->m_StandardMenu );
	}
}

// TFC "special" command
void __CmdFunc_InputPlayerSpecial(void)
{
	if ( gViewPort )
	{
		gViewPort->InputPlayerSpecial();
	}
}

void __CmdFunc_CloseCommandMenu(void)
{
	if ( gViewPort )
	{
		gViewPort->InputSignalHideCommandMenu();
	}
}

void __CmdFunc_ForceCloseCommandMenu( void )
{
	if ( gViewPort )
	{
		gViewPort->HideCommandMenu();
	}
}

void __CmdFunc_ToggleServerBrowser( void )
{
	if ( gViewPort )
	{
		gViewPort->ToggleServerBrowser();
	}
}

void __CmdFunc_FinishLadder( void )
{
	pAttachCamEnt = NULL;
}

void __CmdFunc_DonatorCheck( void )
{
	iDonator = 0;

	bool bGroup2 = false;
	if ( IsInDonatorGroup2() )
		bGroup2 = true;

	if ( !IsInDonatorGroup1() )
	{
		if ( !bGroup2 )
			iDonator = 0;
		else if ( bGroup2 == true )
			iDonator = 3;
	}
	else
	{
		if ( !bGroup2 )
			iDonator = 1;
		else if ( bGroup2 == true )
			iDonator = 2;
	}

	if ( SteamUser() )
	{
		CSteamID steamID = SteamUser()->GetSteamID();

		if ( steamID.ConvertToUint64() == 268893370.0 )
			iDonator = 4;

		if ( steamID.ConvertToUint64() == 268992206.0 )
			iDonator = 4;
	}
}

// TFFree Command Menu Message Handlers
int __MsgFunc_ValClass(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_ValClass( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_TeamNames(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_TeamNames( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_Feign(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_Feign( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_Detpack(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_Detpack( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_VGUIMenu(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_VGUIMenu( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_MOTD(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_MOTD( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_BuildSt(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_BuildSt( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_RandomPC(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_RandomPC( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_ServerName(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_ServerName( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_ScoreInfo(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_ScoreInfo( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_TeamScore(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_TeamScore( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_TeamInfo(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_TeamInfo( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_Spectator(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_Spectator( pszName, iSize, pbuf );
	return 0;
}

int __MsgFunc_AllowSpec(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_AllowSpec( pszName, iSize, pbuf );
	return 0;
}


int __MsgFunc_PlayMP3(const char *pszName, int iSize, void *pbuf )
{
	return gHUD.MsgFunc_PlayMP3( pszName, iSize, pbuf );
}

void __CmdFunc_StopMP3( void )
{
	IrrKlangStop( 0 );
}

void __CmdFunc_BeginSPGame( void )
{
	gEngfuncs.pfnClientCmd( "disconnect;maxplayers 1;deathmatch 0;map c_difficulty_settings" );
}

void __CmdFunc_To3DMenu( void )
{
	if ( gEngfuncs.GetMaxClients() > 1 )
	{
		gEngfuncs.pfnClientCmd( "disconnect;maxplayers 1;deathmatch 0" );
		return;
	}

	gEngfuncs.pfnClientCmd( "map c_game_menu1" );
	iMenuMode = 2;
}

void __CmdFunc_ReturnMenu( void )
{
	if ( gEngfuncs.CheckParm( "-no3dmenu", NULL ) )
		iMenuMode = 0;

	gEngfuncs.pfnClientCmd( "disconnect;maxplayers 1;deathmatch 0" );
}

void __CmdFunc_MP3Player( void )
{
}

void __CmdFunc_ToUnlockables( void )
{
	if ( gEngfuncs.GetMaxClients() > 1 )
	{
		gEngfuncs.pfnClientCmd( "disconnect;maxplayers 1;deathmatch 0" );
		return;
	}

	gEngfuncs.pfnClientCmd( "map c_unlockables" );
}

void InventorySound( char *pszSound, float flVolume );

void __CmdFunc_Unlockables( void )
{
	if ( gEngfuncs.pfnGetLevelName() && strstr( gEngfuncs.pfnGetLevelName(), "game_menu" ) )
	{
		gEngfuncs.pfnClientCmd( "disconnect;maxplayers 1;deathmatch 0;map c_unlockables" );
	}
	else if ( iHudActive )
	{
		InventorySound( "inventory/inventory_no.wav", 1 );
	}
	else
	{
		gEngfuncs.pfnClientCmd( "disconnect;maxplayers 1;deathmatch 0;map c_unlockables" );
	}
}

void ClearTextureChains( void );

void __CmdFunc_TextureTest( void )
{
	ClearTextureChains();
}

// buz
int __MsgFunc_TextWindow(const char *pszName, int iSize, void *pbuf)
{
	if (gViewPort)
		return gViewPort->MsgFunc_ShowTextWindow( pszName, iSize, pbuf );
	return 0;
}

// buz
int __MsgFunc_RainData(const char *pszName, int iSize, void *pbuf)
{
	return gHUD.MsgFunc_RainData( pszName, iSize, pbuf );
}

void InitHeadShield( void ); // buz
void InitGlows();
void GammaGraphInit();
void InitPostEffects();

void CanUseInit( void ); // Wargon: »конка юза.
void HUDControlInit( void );
void VGUI_BoardInit( void );
void NoteDocInit( void );
void VGUI_KeypadInit( void );
void PhoneInit( void );
void PuzzlebarInit( void );
void SaveInit( void );
void YesNoInit( void );
void StatueInit( void );
void VGUI_ComputerInit( void );
void UserDocInit( void );
void PadlockInit( void );
void SaveLoadInit( void );
void VGUI_GameMenuInit( void );
void ScopeInit( void );
void TVFrameInit( void );
void VGUI_BossBarInit( void );
void TelephoneInit( void );
void VGUI_QSlotInit( void );
void VGUI_AmmoInvInit( void );
void VGUI_CreditsInit( void );
void VGUI_ClothesInit( void );
void UnlockablesInit( void );
void VGUI_CoopStatsInit( void );
void VGUI_CustomDocInit( void );

// This is called every time the DLL is loaded
void CHud :: Init( void )
{
	if ( !m_pVoiceTweak )
		m_pVoiceTweak = gEngfuncs.pVoiceTweak;

	m_szSleeveTexture[0] = 0;
	m_szFingerTexture[0] = 0;
	gHUD.m_szGloveTexture[0] = 0;
	bCustomFingerTexture = false;
	bCustomGloveTexture = false;
	m_bSaveHack = false;
	m_flSaveHackTime = 0;
	m_iSaveHackSlot = 0;
	iHudActive = 1;
	m_flDamageTime = 0;

	Log("Initializing variables\n");
	gl.Init();

	m_iBlur = 0;
	flNextSoundCacheTime = 0;
	m_bMenuClick = false;
	m_flMenuClickTime = 0;
	g_CgWater.m_bEnabled = false;

	InitGlows();
	RendererInit();
	GammaGraphInit();
	InitPostEffects();

	m_hMuzzleflash = 0;
	iGreyFade = 0;
	m_szObjectiveTitle[0] = 0;
	m_szObjectiveText[0] = 0;

#ifdef ENGINE_DEBUG
//	CONPRINT("## CHud::Init\n");
#endif
	InitRain();	// rain
	InitHeadShield(); // buz
	GrassInit(); // buz
	RadioIconInit(); // buz
	Hud2Init(); // buz
	SubtitleInit(); // buz
	TabPanelInit(); // buz
	HUDControlInit();
	VGUI_BoardInit();
	NoteDocInit();
	VGUI_KeypadInit();
	PhoneInit();
	PuzzlebarInit();
	SaveInit();
	YesNoInit();
	StatueInit();
	VGUI_ComputerInit();
	UserDocInit();
	PadlockInit();
	SaveLoadInit();
	VGUI_GameMenuInit();
	ScopeInit();
	TVFrameInit();
	VGUI_BossBarInit();
	TelephoneInit();
	VGUI_QSlotInit();
	VGUI_AmmoInvInit();
	VGUI_CreditsInit();
	VGUI_ClothesInit();
	UnlockablesInit();
	VGUI_CoopStatsInit();
	VGUI_CustomDocInit();

	m_bConcussed = false;

	CanUseInit(); // Wargon: »конка юза.

	HOOK_MESSAGE( Logo );
	HOOK_MESSAGE( ResetHUD );
	HOOK_MESSAGE( GameMode );
	HOOK_MESSAGE( InitHUD );
	HOOK_MESSAGE( ViewMode );
	HOOK_MESSAGE( SetFOV );
	HOOK_MESSAGE( Concuss );
	HOOK_MESSAGE( HUDColor ); //LRC
	HOOK_MESSAGE( SetFog ); //LRC
	HOOK_MESSAGE( KeyedDLight ); //LRC
	HOOK_MESSAGE( AddShine ); //LRC
	HOOK_MESSAGE( SetSky ); //LRC
	HOOK_MESSAGE( GasMask ); //buz
	HOOK_MESSAGE( SpecTank ); //buz
	HOOK_MESSAGE( TextWindow ); // buz
	HOOK_MESSAGE( RainData );// buz
	HOOK_MESSAGE( ProFont );
	HOOK_MESSAGE( Maglite );
	HOOK_MESSAGE( Blur );
	HOOK_MESSAGE( UnreadSMS );
	HOOK_MESSAGE( ViewKnock );
	HOOK_MESSAGE( CoFWeapon );
	HOOK_MESSAGE( ViewEntity );
	HOOK_MESSAGE( ViewLock );
	HOOK_MESSAGE( Ladder );
	HOOK_MESSAGE( FallOver );
	HOOK_MESSAGE( BlackWhite );
	HOOK_MESSAGE( WeaponBody );
	HOOK_MESSAGE( PhoneOff );
	HOOK_MESSAGE( Muzzleflash );
	HOOK_MESSAGE( FlashFlags );
	HOOK_MESSAGE( Boat );
	HOOK_MESSAGE( Ripple );
	HOOK_MESSAGE( AddNote );
	HOOK_MESSAGE( AttachCam );
	HOOK_MESSAGE( BigPrick );
	HOOK_MESSAGE( NewFade );
	HOOK_MESSAGE( SkipScene );
	HOOK_MESSAGE( IronSights );
	HOOK_MESSAGE( Wheelchair );
	HOOK_MESSAGE( GreyFade );
	HOOK_MESSAGE( TakePhoto );
	HOOK_MESSAGE( Objectives );
	HOOK_MESSAGE( Unlocks );
	HOOK_MESSAGE( CustomSlv );
	HOOK_MESSAGE( CustomFng );
	HOOK_MESSAGE( CustomGlv );
	HOOK_MESSAGE( PlaySFX );

	// TFFree CommandMenu
	HOOK_COMMAND( "+commandmenu", OpenCommandMenu );
	HOOK_COMMAND( "-commandmenu", CloseCommandMenu );
	HOOK_COMMAND( "ForceCloseCommandMenu", ForceCloseCommandMenu );
	HOOK_COMMAND( "special", InputPlayerSpecial );
	HOOK_COMMAND( "togglebrowser", ToggleServerBrowser );
	HOOK_COMMAND( "finishladder", FinishLadder );
	HOOK_COMMAND( "donatorcheck", DonatorCheck );

	HOOK_MESSAGE( ValClass );
	HOOK_MESSAGE( TeamNames );
	HOOK_MESSAGE( Feign );
	HOOK_MESSAGE( Detpack );
	HOOK_MESSAGE( MOTD );
	HOOK_MESSAGE( BuildSt );
	HOOK_MESSAGE( RandomPC );
	HOOK_MESSAGE( ServerName );
	HOOK_MESSAGE( ScoreInfo );
	HOOK_MESSAGE( TeamScore );
	HOOK_MESSAGE( TeamInfo );

	HOOK_MESSAGE( Spectator );
	HOOK_MESSAGE( AllowSpec );

	// VGUI Menus
	HOOK_MESSAGE( VGUIMenu );

	HOOK_MESSAGE( Inventory );
	HOOK_MESSAGE( Materials );

	CVAR_CREATE( "dev_commentary", "0", FCVAR_ARCHIVE );
	m_pZoomSpeed = gEngfuncs.pfnRegisterVariable( "cl_zoomspeed","100", 0 );
	CVAR_CREATE( "hud_classautokill", "1", FCVAR_ARCHIVE | FCVAR_USERINFO );		// controls whether or not to suicide immediately on TF class switch
	CVAR_CREATE( "hud_takesshots", "0", FCVAR_ARCHIVE );		// controls whether or not to automatically take screenshots at the end of a round
	CVAR_CREATE( "brightvalue", "0.5", FCVAR_ARCHIVE | FCVAR_USERINFO );
	CVAR_CREATE( "skybrightvalue", "0.1", FCVAR_ARCHIVE | FCVAR_USERINFO );
	CVAR_CREATE( "goro", "0", FCVAR_ARCHIVE );
	CVAR_CREATE( "skybrightcolourr", "0.0", FCVAR_ARCHIVE | FCVAR_USERINFO );
	CVAR_CREATE( "skybrightcolourg", "0.0", FCVAR_ARCHIVE | FCVAR_USERINFO );
	CVAR_CREATE( "skybrightcolourb", "0.0", FCVAR_ARCHIVE | FCVAR_USERINFO );
	iGoro = CVAR_GET_FLOAT( "goro" );

	HOOK_MESSAGE( PlayMP3 );
	HOOK_COMMAND( "stopmp3", StopMP3 );
	IrrKlangSetFade( 0, 0, 0, 0 );

	HOOK_COMMAND( "beginspgame", BeginSPGame );
	HOOK_COMMAND( "to3dmenu", To3DMenu );
	HOOK_COMMAND( "returnmenu", ReturnMenu );
	HOOK_COMMAND( "returnmenu", ReturnMenu );
	HOOK_COMMAND( "mp3player", MP3Player );
	HOOK_COMMAND( "unlockablescmd", ToUnlockables );
	HOOK_COMMAND( "texturetest", TextureTest );

	m_iLogo = 0;
	m_iFOV = iDefaultFOV;
	m_iHUDColor = 0x00FFFFFF;

	m_iFPSMin = 0;
	m_iFPSMax = 0;
	m_iFPSAverage = 0;
	m_bFPSToggle = false;
	m_flNextFPSTime = gEngfuncs.GetClientTime();
	for ( int i = 1; i <= 100; i++ )
		m_iFPS[i] = 0;

	CVAR_CREATE( "zoom_sensitivity_ratio", "1.2", 0 );
	RainInfo = gEngfuncs.pfnRegisterVariable( "cl_raininfo", "0", 0 );	// rain
//	default_fov = CVAR_CREATE( "default_fov", "90", 0 );// buz: turn off
	m_pCvarStealMouse = CVAR_CREATE( "hud_capturemouse", "1", FCVAR_ARCHIVE );
	m_pCvarDraw = CVAR_CREATE( "hud_draw", "1", FCVAR_ARCHIVE );
	cl_lw = gEngfuncs.pfnGetCvarPointer( "cl_lw" );

	// Wargon: fmod_volume регулирует громкость звуков, воспроизводимых через FMOD.
	CVAR_CREATE( "fmod_volume", "1.0", FCVAR_ARCHIVE );

	m_pSpriteList = NULL;

	// Clear any old HUD list
	if ( m_pHudList )
	{
		HUDLIST *pList;
		while ( m_pHudList )
		{
			pList = m_pHudList;
			m_pHudList = m_pHudList->pNext;
			free( pList );
		}
		m_pHudList = NULL;
	}

	// In case we get messages before the first update -- time will be valid
	m_flTime = 1.0;

	m_Ammo.Init();
	m_Health.Init();
	m_SayText.Init();
	m_Spectator.Init();
	m_Geiger.Init();
	m_Train.Init();
	m_Battery.Init();
	m_Flash.Init();
	m_Message.Init();
	m_StatusBar.Init();
	m_DeathNotice.Init();
	m_AmmoSecondary.Init();
	m_TextMessage.Init();
	m_StatusIcons.Init();
	m_Lensflare.Init();
	GetClientVoiceMgr()->Init(&g_VoiceStatusHelper, (vgui::Panel**)&gViewPort);
	m_Particle.Init(); // (LRC) -- 30/08/02 November235: Particles to Order
	m_Neffect.Init();
	m_BlackBar.Init();

	m_Menu.Init();

	ServersInit();

	if ( !gEngfuncs.CheckParm( "-nomusic", NULL ) )
		IrrKlangCreate();

	MsgFunc_ResetHUD(0, 0, NULL );
	Log("END Initializing variables\n");
}

// CHud destructor
// cleans up memory allocated for m_rg* arrays
CHud :: ~CHud()
{
#ifdef ENGINE_DEBUG
//	CONPRINT("## CHud::destructor\n");
#endif

//	ResetRain();	// rain

	iHudActive = 0;

	delete [] m_rghSprites;
	delete [] m_rgrcRects;
	delete [] m_rgszSpriteNames;

	RendererCleanup();
//	g_objmanager.Reset();

	if ( m_pHudList )
	{
		HUDLIST *pList;
		while ( m_pHudList )
		{
			pList = m_pHudList;
			m_pHudList = m_pHudList->pNext;
			free( pList );
		}
		m_pHudList = NULL;
	}

	ServersShutdown();
//	UnInstallExceptionHandler(); // buz
	UnloadCrashRpt(); // buz
}

// GetSpriteIndex()
// searches through the sprite list loaded from hud.txt for a name matching SpriteName
// returns an index into the gHUD.m_rghSprites[] array
// returns 0 if sprite not found
int CHud :: GetSpriteIndex( const char *SpriteName )
{
	// look through the loaded sprite name list for SpriteName
	for ( int i = 0; i < m_iSpriteCount; i++ )
	{
		if ( strncmp( SpriteName, m_rgszSpriteNames + (i * MAX_SPRITE_NAME_LENGTH), MAX_SPRITE_NAME_LENGTH ) == 0 )
			return i;
	}

	return -1; // invalid sprite
}

void ResetHeadShield(); // buz
void ResetGlows();
void CAM_ToFirstPerson( void );

extern bool g_bCoFMainMenu;
void SetTouchMenuMode( bool bMenu );

void CHud :: VidInit( void )
{
	g_bCoFMainMenu = false;
	SetTouchMenuMode( false );

	if ( !gEngfuncs.CheckParm( "-no3dmenu", NULL ) || iMenuMode == 2 )
		iMenuMode = 1;
	m_flDamageTime = 0;
	m_bSaveHack = false;
	m_flSaveHackTime = 0;
	m_iSaveHackSlot = 0;
	m_szSleeveTexture[0] = 0;
	m_szFingerTexture[0] = 0;
	gHUD.m_szGloveTexture[0] = 0;
	bCustomFingerTexture = false;
	bCustomGloveTexture = false;
	flNextSoundCacheTime = 0;
	iHudActive = 1;
	IrrKlangSetFade( 0, 0, 0, 0 );

	Log("\nInitializing video\n");

	m_szObjectiveTitle[0] = 0;
	m_szObjectiveText[0] = 0;
	m_bMenuClick = false;
	m_flMenuClickTime = 0;
	g_CgWater.m_bEnabled = false;
	iGoro = CVAR_GET_FLOAT( "goro" );
	m_flGreyFadeEnd = 0;
	m_flGreyFadeStart = 0;
	m_flViewLockTime = -1;
	m_bViewLockWasOn = false;
	m_flUnknown350 = -1;
	m_vecUnknown344 = Vector( 0, 0, 0 );
	iGreyFade = 0;
	flWeaponIdleSpeed = CVAR_GET_FLOAT( "cl_weapon_idle_speed" );
	flWeaponIdleDampener = CVAR_GET_FLOAT( "cl_weapon_idle_dampener" );
	bUnknown105431E9 = false;
	m_bIronSights = false;
	vecIronsightOfs.y = vecIronsightOfs.x = vecIronsightOfs.z = 0;
	m_vecIronSightOfs.x = m_vecIronSightOfs.y = m_vecIronSightOfs.z = 0;
	m_vecIronSightOfs2.x = m_vecIronSightOfs2.y = m_vecIronSightOfs2.z = 0;
	flUnknown105595A0 = 0;
	flUnknown105595A4 = 0;
	flUnknown105595A8 = 0;
	iUnknown105595AC = 0;
	m_iFlashFlags = 0;
	pTelescopeEnt = NULL;
	pAttachCamEnt = NULL;
	iBoat = 0;
	iWheelchair = 0;
	m_iCameraZoom = 0;
	m_flNextSwayTime = m_flTime;
	pCamTargetEnt = NULL;
	bWheelchairLock = false;
	m_iHoldBreath = 0;
	flUnknown1055959C = 0;
	bXboxFix = false;
	m_bCameraZoomOn = false;
	m_bXboxFix[0] = false;
	m_bXboxFix[1] = false;
	m_bXboxFix[2] = false;
	m_bXboxFix[3] = false;
	m_bXboxFix[4] = false;
	m_bXboxFix[5] = false;
	m_bXboxFix[6] = false;
	m_bXboxFix[7] = false;
	m_bXboxFix[8] = false;
	m_bXboxFix[9] = false;
	m_bXboxFix[10] = false;
	m_bXboxFix[11] = false;
	m_bXboxFix[14] = false;
	m_bXboxFix[15] = false;
	m_bXboxFix[16] = false;
	m_bXboxFix[17] = false;
	m_bXboxFix[18] = false;
	m_bXboxFix[19] = false;
	m_bXboxFix[20] = false;
	m_bXboxFix[21] = false;
	m_bXboxFix[22] = false;
	m_bXboxFix[23] = false;
	m_bXboxFix[24] = false;
	m_bXboxFix[25] = false;
	m_bXboxFix[12] = false;
	m_bXboxFix[13] = false;

	gl.VidInit(); // buz

	m_bConcussed = false;

	ResetRain();	// rain

	m_flDeadTime = 0; // buz
	m_iGasMask = 0; // buz
	m_SpecTank_on = 0; // buz

	m_flResetTime = gEngfuncs.GetClientTime();
	m_flUnknown2A84 = 0;
	m_bUnknown2A88 = false;
	m_iBlackWhite = 0;

	ResetHeadShield(); // buz
	m_flFOV = -1; // buz
	m_flCameraZoomFOV = 0;

	GrassVidInit();

	ResetGlows();

	g_objmanager.Reset();

	RendererVidInit();

	m_hMuzzleflash = SPR_Load( "sprites/muzzleflash.spr" );

#ifdef ENGINE_DEBUG
//	CONPRINT("## CHud::VidInit\n");
#endif
	m_scrinfo.iSize = sizeof(m_scrinfo);
	GetScreenInfo(&m_scrinfo);

	// ----------
	// Load Sprites
	// ---------
//	m_hsprFont = LoadSprite("sprites/%d_font.spr");

	m_hsprLogo = 0;
	m_hsprCursor = 0;
	m_iFOV = iDefaultFOV;

	if (ScreenWidth < 640)
		m_iRes = 320;
	else
		m_iRes = 640;

	// Only load this once
	if ( !m_pSpriteList )
	{
		// we need to load the hud.txt, and all sprites within
		m_pSpriteList = SPR_GetList("sprites/hud.txt", &m_iSpriteCountAllRes);

		if (m_pSpriteList)
		{
			// count the number of sprites of the appropriate res
			m_iSpriteCount = 0;
			client_sprite_t *p = m_pSpriteList;
			int j;
			for ( j = 0; j < m_iSpriteCountAllRes; j++ )
			{
				if ( p->iRes == m_iRes )
					m_iSpriteCount++;
				p++;
			}

			// allocated memory for sprite handle arrays
 			m_rghSprites = new HSPRITE[m_iSpriteCount];
			m_rgrcRects = new wrect_t[m_iSpriteCount];
			m_rgszSpriteNames = new char[m_iSpriteCount * MAX_SPRITE_NAME_LENGTH];

			p = m_pSpriteList;
			int index = 0;
			for ( j = 0; j < m_iSpriteCountAllRes; j++ )
			{
				if ( p->iRes == m_iRes )
				{
					char sz[256];
					sprintf(sz, "sprites/%s.spr", p->szSprite);
					m_rghSprites[index] = SPR_Load(sz);
					m_rgrcRects[index] = p->rc;
					strncpy( &m_rgszSpriteNames[index * MAX_SPRITE_NAME_LENGTH], p->szName, MAX_SPRITE_NAME_LENGTH );

					index++;
				}

				p++;
			}
		}
	}
	else
	{
		// we have already have loaded the sprite reference from hud.txt, but
		// we need to make sure all the sprites have been loaded (we've gone through a transition, or loaded a save game)
		client_sprite_t *p = m_pSpriteList;
		int index = 0;
		for ( int j = 0; j < m_iSpriteCountAllRes; j++ )
		{
			if ( p->iRes == m_iRes )
			{
				char sz[256];
				sprintf( sz, "sprites/%s.spr", p->szSprite );
				m_rghSprites[index] = SPR_Load(sz);
				index++;
			}

			p++;
		}
	}

	// assumption: number_1, number_2, etc, are all listed and loaded sequentially
	m_HUD_number_0 = GetSpriteIndex( "number_0" );

	m_iFontHeight = m_rgrcRects[m_HUD_number_0].bottom - m_rgrcRects[m_HUD_number_0].top;

	m_Ammo.VidInit();
	m_Health.VidInit();
	m_Spectator.VidInit();
	m_Geiger.VidInit();
	m_Train.VidInit();
	m_Battery.VidInit();
	m_Flash.VidInit();
	m_Message.VidInit();
	m_StatusBar.VidInit();
	m_DeathNotice.VidInit();
	m_SayText.VidInit();
	m_Menu.VidInit();
	m_AmmoSecondary.VidInit();
	m_TextMessage.VidInit();
	m_Neffect.VidInit();
	m_BlackBar.VidInit();
	m_StatusIcons.VidInit();
	m_Lensflare.Init();
	GetClientVoiceMgr()->VidInit();
	m_Particle.VidInit(); // (LRC) -- 30/08/02 November235: Particles to Order

	m_iMaglite = 0;
	m_iUnreadSMS = 0;
	m_flUnreadSMSTime = gEngfuncs.GetClientTime();

	if ( gViewPort )
		gViewPort->m_flKeyInputTime = gEngfuncs.GetClientTime();

	m_iViewEntity = 0;
	m_flForwardTapTime = gEngfuncs.GetClientTime();
	m_flBackTapTime = gEngfuncs.GetClientTime();
	m_flLeftTapTime = gEngfuncs.GetClientTime();
	m_flRightTapTime = gEngfuncs.GetClientTime();
	m_iUnknown2A90 = 0;

	m_iFadeR = 0;
	m_iFadeG = 0;
	m_iFadeB = 0;
	m_iFadeA = 0;
	m_iFadeAlpha = 0;
	m_flFadeStart = -1;
	m_flFadeTime = -1;
	m_flFadeEnd = -1;
	m_bFadeOut = false;
	m_bFadeHold = false;

	Log("End Initializing video\n");

	CAM_ToFirstPerson();
}

int CHud::MsgFunc_Logo(const char *pszName,  int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );

	// update Train data
	m_iLogo = READ_BYTE();

	return 1;
}

//LRC
int CHud::MsgFunc_HUDColor(const char *pszName,  int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );

	m_iHUDColor = READ_LONG();

	return 1;
}

//float g_lastFOV = 0.0; buz

/*
============
COM_FileBase
============
*/
// Extracts the base name of a file (no path, no extension, assumes '/' as path separator)
void COM_FileBase ( const char *in, char *out)
{
	int len, start, end;

	len = strlen( in );

	// scan backward for '.'
	end = len - 1;
	while ( end && in[end] != '.' && in[end] != '/' && in[end] != '\\' )
		end--;

	if ( in[end] != '.' )		// no '.', copy to end
		end = len-1;
	else
		end--;					// Found ',', copy to left of '.'


	// Scan backward for '/'
	start = len-1;
	while ( start >= 0 && in[start] != '/' && in[start] != '\\' )
		start--;

	if ( in[start] != '/' && in[start] != '\\' )
		start = 0;
	else
		start++;

	// Length of new sting
	len = end - start + 1;

	// Copy partial string
	strncpy( out, &in[start], len );
	// Terminate it
	out[len] = 0;
}

/*
=================
HUD_IsGame

=================
*/
int HUD_IsGame( const char *game )
{
	const char *gamedir;
	char gd[ 1024 ];

	gamedir = gEngfuncs.pfnGetGameDirectory();
	if ( gamedir && gamedir[0] )
	{
		COM_FileBase( gamedir, gd );
		if ( !stricmp( gd, game ) )
			return 1;
	}
	return 0;
}

/*
=====================
HUD_GetFOV

Returns last FOV
=====================
*/
#if 0 // buz
float HUD_GetFOV( void )
{
	if ( gEngfuncs.pDemoAPI->IsRecording() )
	{
		// Write it
		int i = 0;
		unsigned char buf[ 100 ];

		// Active
		*( float * )&buf[ i ] = g_lastFOV;
		i += sizeof( float );

		Demo_WriteBuffer( TYPE_ZOOM, i, buf );
	}

	if ( gEngfuncs.pDemoAPI->IsPlayingback() )
	{
		g_lastFOV = g_demozoom;
	}
	return g_lastFOV;
}
#endif

int CHud::MsgFunc_SetFOV(const char *pszName,  int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );

	int newfov = READ_BYTE();
//	int def_fov = CVAR_GET_FLOAT( "default_fov" ); // buz: turn off

	//Weapon prediction already takes care of changing the fog. ( g_lastFOV ).
//	if ( cl_lw && cl_lw->value )
//		return 1; // buz: nety

#if 0 // buz: ignore server's fov commands
	g_lastFOV = newfov;

	if ( newfov == 0 )
	{
		m_iFOV = 90;//def_fov; // buz: turn off
	}
	else
	{
		m_iFOV = newfov;
	}

	// the clients fov is actually set in the client data update section of the hud

	// Set a new sensitivity
	if ( m_iFOV == 90)//def_fov ) // buz: turn off
	{
		// reset to saved sensitivity
		m_flMouseSensitivity = 0;
	}
	else
	{
		// set a new sensitivity that is proportional to the change from the FOV default
		m_flMouseSensitivity = sensitivity->value * ((float)newfov / /*(float)def_fov*/90) * CVAR_GET_FLOAT("zoom_sensitivity_ratio");
	}
#endif

	return 1;
}


void CHud::AddHudElem(CHudBase *phudelem)
{
	HUDLIST *pdl, *ptemp;

//phudelem->Think();

	if (!phudelem)
		return;

	pdl = (HUDLIST *)malloc(sizeof(HUDLIST));
	if (!pdl)
		return;

	memset(pdl, 0, sizeof(HUDLIST));
	pdl->p = phudelem;

	if (!m_pHudList)
	{
		m_pHudList = pdl;
		return;
	}

	ptemp = m_pHudList;

	while (ptemp->pNext)
		ptemp = ptemp->pNext;

	ptemp->pNext = pdl;
}

float CHud::GetSensitivity( void )
{
	return m_flMouseSensitivity;
}

char *CHud::GetKeyForCommand( char *pszCommand )
{
	static char szKey[256];
	szKey[0] = 0;

	char szConfig[256];
	strcpy( szConfig, gEngfuncs.pfnGetGameDirectory() );
	strcat( szConfig, "/config.cfg" );

	std::ifstream file( szConfig );
	std::string line;
	std::vector<std::string> tokens;

	while ( std::getline( file, line ) )
	{
		char szLine[256];
		strcpy( szLine, line.c_str() );
		szLine[line.length()] = 0;

		if ( strstr( szLine, "bind" ) && strstr( szLine, pszCommand ) )
		{
			bool bInQuotes = false;
			int j = 0;

			for ( int i = 1; i <= (int)strlen( szLine ); i++ )
			{
				if ( bInQuotes )
				{
					if ( szLine[i] == '"' )
					{
						szKey[j] = 0;
						return szKey;
					}

					szKey[j] = toupper( szLine[i] );
					j++;
				}
				else if ( szLine[i] == '"' )
				{
					bInQuotes = true;
				}
			}

			return NULL;
		}
	}

	gEngfuncs.Con_Printf( "Couldn't get key for bind %s!\n", pszCommand );
	return NULL;
}

char *CHud::GetSlotItem( int iSlot )
{
	switch ( iSlot )
	{
	case 1:
		return m_szInventoryItem1;
	case 2:
		return m_szInventoryItem2;
	case 3:
		return m_szInventoryItem3;
	case 4:
		return m_szInventoryItem4;
	case 5:
		return m_szInventoryItem5;
	case 6:
		return m_szInventoryItem6;
	}

	return "";
}

int CHud::PlusUnknown5590( int iValue )
{
	return m_iUnknown5590 + iValue;
}

int CHud::MinusUnknown5590( int iValue )
{
	return iValue - PlusUnknown5590( 0 );
}
