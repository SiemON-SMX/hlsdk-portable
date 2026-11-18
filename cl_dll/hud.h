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
//  hud.h
//
// class CHud declaration
//
// CHud handles the message, calculation, and drawing the HUD
//

#define FOG_LIMIT 30000
#define RGB_YELLOWISH 0x00FFA000 //255,160,0
#define RGB_REDISH 0x00FF1010 //255,160,0
#define RGB_GREENISH 0x0000A000 //0,160,0

#include "wrect.h"
#include "cl_dll.h"
#include "ammo.h"

#define DHN_DRAWZERO 1
#define DHN_2DIGITS  2
#define DHN_3DIGITS  4
#define MIN_ALPHA	 170	

#define		HUDELEM_ACTIVE	1

typedef struct {
	int x, y;
} POSITION;

enum 
{ 
	MAX_PLAYERS = 64,
	MAX_TEAMS = 64,
	MAX_TEAM_NAME = 16,
};

typedef struct {
	unsigned char r,g,b,a;
} RGBA;

typedef struct cvar_s cvar_t;


#define HUD_ACTIVE	1
#define HUD_INTERMISSION 2

#define MAX_PLAYER_NAME_LENGTH		32

#define	MAX_MOTD_LENGTH				1536

//
//-----------------------------------------------------
//
class CHudBase
{
public:
	POSITION  m_pos;
	int   m_type;
	int	  m_iFlags; // active, moving, 
	virtual		~CHudBase() {}
	virtual int Init( void ) {return 0;}
	virtual int VidInit( void ) {return 0;}
	virtual int Draw(float flTime) {return 0;}
	virtual void Think(void) {return;}
	virtual void Reset(void) {return;}
	virtual void InitHUDData( void ) {}		// called every time a server is connected to

};

struct HUDLIST {
	CHudBase	*p;
	HUDLIST		*pNext;
};



//
//-----------------------------------------------------
//
#include "../game_shared/voice_status.h"
#include "hud_spectator.h"


//
//-----------------------------------------------------
//
class CHudAmmo: public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw(float flTime);
	void Think(void);
	void Reset(void);
	void UpdateCrosshair(); // buz
	int DrawWList(float flTime);
	int MsgFunc_CurWeapon(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_WeaponList(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_AmmoX(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_AmmoPickup( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_WeapPickup( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_ItemPickup( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_HideWeapon( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_WpnInfo( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_Accuracy( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_DualAmmo( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_WpnDeploy( const char *pszName, int iSize, void *pbuf );

	void SlotInput( int iSlot );
	void _cdecl UserCmd_Slot1( void );
	void _cdecl UserCmd_Slot2( void );
	void _cdecl UserCmd_Slot3( void );
	void _cdecl UserCmd_Slot4( void );
	void _cdecl UserCmd_Slot5( void );
	void _cdecl UserCmd_Slot6( void );
	void _cdecl UserCmd_Slot7( void );
	void _cdecl UserCmd_Slot8( void );
	void _cdecl UserCmd_Slot9( void );
	void _cdecl UserCmd_Slot10( void );
	void _cdecl UserCmd_Close( void );
	void _cdecl UserCmd_NextWeapon( void );
	void _cdecl UserCmd_PrevWeapon( void );

	void LoadMenuSettings( void );

	int GetMags( int iId );
	void SetMags( int iId, int iDivisor, int iAmount );

	// buz: these variables loaded from file menu_settings.txt
	int		m_menuSizeX;
	int		m_menuSizeY;
	float	m_menuScale;
	float	m_menuMinAlpha;
	float	m_menuAddAlpha;
	float	m_menuSpeed;
	int		m_menuRenderMode;
	int		m_menuOfsX;
	int		m_menuOfsY;
	int		m_menuSpaceX;
	int		m_menuSpaceY;
	int		m_menuNumberSizeX;
	int		m_menuNumberSizeY;

	int		m_iDualAmmoLeft;
	int		m_iDualAmmoRight;

	int		m_iGlockMags;
	int		m_iVP70Mags;
	int		m_iShotgunMags;
	int		m_iP345Mags;
	int		m_iG43Mags;
	int		m_iMP5Mags;
	int		m_iSyringes;
	int		m_iM16Mags;
	int		m_iTMPMags;
	int		m_iRifleMags;
	int		m_iFlares;
	int		m_iBrowningMags;
	int		m_iRevolverAmmo;

//private:
	float m_fFade;
	RGBA  m_rgba;
	WEAPON *m_pWeapon;
	int	m_HUD_bucket0;
	int m_HUD_selection;
	int m_mach; // buz

	int m_HUD_glock_mag;
	int m_HUD_revolver_cylinder;
	int m_HUD_browning_mag;
	int m_HUD_p345_mag;
	int m_HUD_g43_mag;
	int m_HUD_mp5_mag;
	int m_HUD_m76_mag;
	int m_HUD_shotgun_box;
	int m_HUD_glock_mag_icon;
	int m_HUD_p345_mag_icon;
	int m_HUD_g43_mag_icon;
	int m_HUD_mp5_mag_icon;
	int m_HUD_shotgun_box_icon;
	int m_HUD_syringe_icon;
	int m_HUD_m76_mag_icon;
	int m_HUD_mag_cross;
	int m_HUD_crosshair;
	int m_HUD_m16_mag;
	int m_HUD_m16_mag_icon;
	int m_HUD_vp70_mag;
	int m_HUD_vp70_mag_icon;
	int m_HUD_tmp_mag;
	int m_HUD_tmp_mag_icon;
	int m_HUD_flare_icon;
	int m_HUD_famas_mag;
	int m_HUD_famas_mag_icon;
	int m_HUD_gm_inf;
	int m_HUD_rifle_mag;
	int m_HUD_rifle_mag_icon;

	int m_iDrawAmmo;
	float m_flAccuracy;

	wrect_t *m_prcGlockMag;
	wrect_t *m_prcShotgunBox;
	wrect_t *m_prcM76Mag;
	wrect_t *m_prcM16Mag;
	wrect_t *m_prcP345Mag;
	wrect_t *m_prcTMPMag;
	wrect_t *m_prcBrowningMag;
	wrect_t *m_prcRevolverCylinder;
	wrect_t *m_prcGmInf;
	wrect_t *m_prcRifleMag;
	wrect_t *m_prcG43Mag;
	wrect_t *m_prcMP5Mag;
};

//
//-----------------------------------------------------
//

class CHudAmmoSecondary: public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	void Reset( void );
	int Draw(float flTime);

	int MsgFunc_SecAmmoVal( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_SecAmmoIcon( const char *pszName, int iSize, void *pbuf );

private:
	enum {
		MAX_SEC_AMMO_VALUES = 4
	};

	int m_HUD_ammoicon; // sprite indices
	int m_iAmmoAmounts[MAX_SEC_AMMO_VALUES];
	float m_fFade;
};


#include "health.h"


#define FADE_TIME 100


//
//-----------------------------------------------------
//
class CHudGeiger: public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw(float flTime);
	int MsgFunc_Geiger(const char *pszName, int iSize, void *pbuf);
	
private:
	int m_iGeigerRange;

};

//
//-----------------------------------------------------
//
class CHudTrain: public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw(float flTime);
	int MsgFunc_Train(const char *pszName, int iSize, void *pbuf);

private:
	HSPRITE m_hSprite;
	int m_iPos;

};

//
//-----------------------------------------------------
//
// REMOVED: Vgui has replaced this.
//
/*
class CHudMOTD : public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw( float flTime );
	void Reset( void );

	int MsgFunc_MOTD( const char *pszName, int iSize, void *pbuf );

protected:
	static int MOTD_DISPLAY_TIME;
	char m_szMOTD[ MAX_MOTD_LENGTH ];
	float m_flActiveRemaining;
	int m_iLines;
};
*/

//
//-----------------------------------------------------
//
class CHudStatusBar : public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw( float flTime );
	void Reset( void );
	void ParseStatusString( int line_num );

	int MsgFunc_StatusText( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_StatusValue( const char *pszName, int iSize, void *pbuf );

protected:
	enum { 
		MAX_STATUSTEXT_LENGTH = 128,
		MAX_STATUSBAR_VALUES = 8,
		MAX_STATUSBAR_LINES = 2,
	};

	char m_szStatusText[MAX_STATUSBAR_LINES][MAX_STATUSTEXT_LENGTH];  // a text string describing how the status bar is to be drawn
	char m_szStatusBar[MAX_STATUSBAR_LINES][MAX_STATUSTEXT_LENGTH];	// the constructed bar that is drawn
	int m_iStatusValues[MAX_STATUSBAR_VALUES];  // an array of values for use in the status bar

	int m_bReparseString; // set to TRUE whenever the m_szStatusBar needs to be recalculated

	// an array of colors...one color for each line
	float *m_pflNameColors[MAX_STATUSBAR_LINES];
};

//
//-----------------------------------------------------
//
// REMOVED: Vgui has replaced this.
//
/*
class CHudScoreboard: public CHudBase
{
public:
	int Init( void );
	void InitHUDData( void );
	int VidInit( void );
	int Draw( float flTime );
	int DrawPlayers( int xoffset, float listslot, int nameoffset = 0, char *team = NULL ); // returns the ypos where it finishes drawing
	void UserCmd_ShowScores( void );
	void UserCmd_HideScores( void );
	int MsgFunc_ScoreInfo( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_TeamInfo( const char *pszName, int iSize, void *pbuf );
	int MsgFunc_TeamScore( const char *pszName, int iSize, void *pbuf );
	void DeathMsg( int killer, int victim );

	int m_iNumTeams;

	int m_iLastKilledBy;
	int m_fLastKillTime;
	int m_iPlayerNum;
	int m_iShowscoresHeld;

	void GetAllPlayersInfo( void );
private:
	struct cvar_s *cl_showpacketloss;

};
*/

struct extra_player_info_t 
{
	short frags;
	short deaths;
	short playerclass;
	short teamnumber;
	// two extra bytes in 1.6a, the player entries are 0x1A apart
	short unknown;
	char teamname[MAX_TEAM_NAME];
};

struct team_info_t 
{
	char name[MAX_TEAM_NAME];
	short frags;
	short deaths;
	short ping;
	short packetloss;
	short ownteam;
	short players;
	int already_drawn;
	int scores_overriden;
	int teamnumber;
};

extern hud_player_info_t	g_PlayerInfoList[MAX_PLAYERS+1];	   // player info from the engine
extern extra_player_info_t  g_PlayerExtraInfo[MAX_PLAYERS+1];   // additional player info sent directly to the client dll
extern team_info_t			g_TeamInfo[MAX_TEAMS+1];
extern int					g_IsSpectator[MAX_PLAYERS+1];


//
//-----------------------------------------------------
//
class CHudDeathNotice : public CHudBase
{
public:
	int Init( void );
	void InitHUDData( void );
	int VidInit( void );
	int Draw( float flTime );
	int MsgFunc_DeathMsg( const char *pszName, int iSize, void *pbuf );

private:
	int m_HUD_d_skull;  // sprite index of skull icon
};

//
//-----------------------------------------------------
//
class CHudMenu : public CHudBase
{
public:
	int Init( void );
	void InitHUDData( void );
	int VidInit( void );
	void Reset( void );
	int Draw( float flTime );
	int MsgFunc_ShowMenu( const char *pszName, int iSize, void *pbuf );

	void SelectMenuItem( int menu_item );

	int m_fMenuDisplayed;
	int m_bitsValidSlots;
	float m_flShutoffTime;
	int m_fWaitingForMore;
};

//
//-----------------------------------------------------
//
class CHudSayText : public CHudBase
{
public:
	int Init( void );
	void InitHUDData( void );
	int VidInit( void );
	int Draw( float flTime );
	int MsgFunc_SayText( const char *pszName, int iSize, void *pbuf );
	void SayTextPrint( const char *pszBuf, int iBufSize, int clientIndex = -1 );
	void EnsureTextFitsInOneLineAndWrapIfHaveTo( int line );
friend class CHudSpectator;

private:

	struct cvar_s *	m_HUD_saytext;
	struct cvar_s *	m_HUD_saytext_time;
};

//
//-----------------------------------------------------
//
class CHudBattery: public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw(float flTime);
	int MsgFunc_Battery(const char *pszName,  int iSize, void *pbuf );
	
private:
	HSPRITE m_hSprite1;
	HSPRITE m_hSprite2;
	wrect_t *m_prc1;
	wrect_t *m_prc2;
	int	  m_iBat;	
	float m_fFade;
	int	  m_iHeight;		// width of the battery innards
};


//
//-----------------------------------------------------
//
class CHudFlashlight: public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw(float flTime);
	void Reset( void );
	int MsgFunc_Flashlight(const char *pszName,  int iSize, void *pbuf );
	int MsgFunc_FlashBat(const char *pszName,  int iSize, void *pbuf );
	
private:
	HSPRITE m_hSprite1;
	HSPRITE m_hSprite2;
	HSPRITE m_hBeam;
	wrect_t *m_prc1;
	wrect_t *m_prc2;
	wrect_t *m_prcBeam;
	float m_flBat;	
	int	  m_iBat;	
	int	  m_fOn;
	float m_fFade;
	int	  m_iWidth;		// width of the battery innards
};

//
//-----------------------------------------------------
//
// (LRC) -- 30/08/02 November235: Particles to Order
class CHudParticle: public CHudBase
{
public:
	int Init(void);
	int VidInit(void);
	int Draw(float flTime);
	int MsgFunc_Particle(const char *pszName, int iSize, void *pbuf );
	int MsgFunc_DelParticle(const char *pszName, int iSize, void *pbuf ); // buz
	int MsgFunc_KillWpnPart(const char *pszName, int iSize, void *pbuf );
	int MsgFunc_Ignite(const char *pszName, int iSize, void *pbuf );
};

//
//-----------------------------------------------------
//
class CBlackBar: public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw( float flTime );
	int MsgFunc_BlackBars( const char *pszName, int iSize, void *pbuf );

	int m_iOn;
	int m_HUD_blackbar;
	int m_iTop;
	int m_iBottom;
};

//
//-----------------------------------------------------
//
class CNeffect: public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw( float flTime );

	int m_HUD_snoise;
	int m_HUD_arghnoise;
	int m_HUD_tv_scanlines;
};

//
//-----------------------------------------------------
//
class CHudLensflare: public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw( float flTime );
	int MsgFunc_Lensflare( const char *pszName, int iSize, void *pbuf );

	int m_iUnused[3];
	Vector m_vecSunPos;
	int m_iUnused2[90];
};

//
//-----------------------------------------------------
//
const int maxHUDMessages = 16;
struct message_parms_t
{
	client_textmessage_t	*pMessage;
	float	time;
	int x, y;
	int	totalWidth, totalHeight;
	int width;
	int lines;
	int lineLength;
	int length;
	int r, g, b;
	int text;
	int fadeBlend;
	float charTime;
	float fadeTime;
};

//
//-----------------------------------------------------
//

class CHudTextMessage: public CHudBase
{
public:
	int Init( void );
	static char *LocaliseTextString( const char *msg, char *dst_buffer, int buffer_size );
	static char *BufferedLocaliseTextString( const char *msg );
	char *LookupString( const char *msg_name, int *msg_dest = NULL );
	int MsgFunc_TextMsg(const char *pszName, int iSize, void *pbuf);
};

//
//-----------------------------------------------------
//

class CHudMessage: public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	int Draw(float flTime);
	int MsgFunc_HudText(const char *pszName, int iSize, void *pbuf);
	int MsgFunc_GameTitle(const char *pszName, int iSize, void *pbuf);

	float FadeBlend( float fadein, float fadeout, float hold, float localTime );
	int	XPosition( float x, int width, int lineWidth );
	int YPosition( float y, int height );

	void MessageAdd( const char *pName, float time );
	void MessageAdd(client_textmessage_t * newMessage );
	void MessageDrawScan( client_textmessage_t *pMessage, float time );
	void MessageScanStart( void );
	void MessageScanNextChar( void );
	void Reset( void );

private:
	client_textmessage_t		*m_pMessages[maxHUDMessages];
	float						m_startTime[maxHUDMessages];
	message_parms_t				m_parms;
	float						m_gameTitleTime;
	client_textmessage_t		*m_pGameTitle;

	int m_HUD_title_life;
	int m_HUD_title_half;
};

//
//-----------------------------------------------------
//
#define MAX_SPRITE_NAME_LENGTH	24

class CHudStatusIcons: public CHudBase
{
public:
	int Init( void );
	int VidInit( void );
	void Reset( void );
	int Draw(float flTime);
	int MsgFunc_StatusIcon(const char *pszName, int iSize, void *pbuf);

	enum { 
		MAX_ICONSPRITENAME_LENGTH = MAX_SPRITE_NAME_LENGTH,
		MAX_ICONSPRITES = 4,
	};

	
	//had to make these public so CHud could access them (to enable concussion icon)
	//could use a friend declaration instead...
	void EnableIcon( char *pszIconName, unsigned char red, unsigned char green, unsigned char blue );
	void DisableIcon( char *pszIconName );

private:

	typedef struct
	{
		char szSpriteName[MAX_ICONSPRITENAME_LENGTH];
		HSPRITE spr;
		wrect_t rc;
		unsigned char r, g, b;
	} icon_sprite_t;

	icon_sprite_t m_IconList[MAX_ICONSPRITES];

};

//
//-----------------------------------------------------
//

//LRC
//methods actually defined in tri.cpp

class CShinySurface
{
	float m_fMinX, m_fMinY, m_fMaxX, m_fMaxY, m_fZ;
	char m_fScale;
	float m_fAlpha; // texture scale and brighness
	HSPRITE m_hsprSprite;
	char m_szSprite[128];

public:
	CShinySurface *m_pNext;

	CShinySurface( float fScale, float fAlpha, float fMinX, float fMaxX, float fMinY, float fMaxY, float fZ, char *szSprite);
	~CShinySurface();

	// draw the surface as seen from the given position
	void Draw(const vec3_t &org);

	void DrawAll(const vec3_t &org);
};

//
//-----------------------------------------------------
//


//LRC - for the moment, skymode has only two settings
#define SKY_OFF 0
#define SKY_ON  1



class CHud
{
private:
	HUDLIST						*m_pHudList;
	int							m_iLogo;
	client_sprite_t				*m_pSpriteList;
	int							m_iSpriteCount;
	int							m_iSpriteCountAllRes;
	float						m_flMouseSensitivity;
	int							m_iConcussionEffect;

public:

	char	m_szSleeveTexture[256];
	char	m_szFingerTexture[256];
	char	m_szGloveTexture[256];

	bool	m_bSaveHack;
	float	m_flSaveHackTime;
	int		m_iSaveHackSlot;

	float	m_flDamageTime;
	struct IVoiceTweak_s *m_pVoiceTweak;
	char	*m_pszHeadShieldSound;
	int		m_iHeadShieldSoundFlags;
	int		m_iUnlocks;
	bool	m_bMenuClick;
	float	m_flMenuClickTime;
	Vector	m_vecUnknown344;
	float	m_flUnknown350;
	float	m_flViewLockTime;
	bool	m_bViewLockWasOn;
	int		m_iFlashFlags;
	HSPRITE	m_hMuzzleflash;

	char	m_szUnused364[3000];

	struct
	{
		char	szLine[8][32];
		char	szText1[48];
		char	szText2[64];
		char	szText3[48];
	} m_Notes[16];

	Vector	m_vecIronSightOfs;
	Vector	m_vecIronSightOfs2;
	bool	m_bIronSights;

	int		m_iFadeR;
	int		m_iFadeG;
	int		m_iFadeB;
	int		m_iFadeA;
	int		m_iFadeAlpha;
	float	m_flFadeStart;
	float	m_flFadeTime;
	float	m_flFadeEnd;
	bool	m_bFadeOut;
	bool	m_bFadeHold;

	bool	m_bConcussed;
	int		m_iClothes;
	int		m_iClothesState;
	bool	m_bXboxFix[26];

	int		m_iUnused2980[64];

	float	m_flResetTime;
	float	m_flUnknown2A84;
	bool	m_bUnknown2A88;
	bool	m_bPhoneOff;
	int		m_iBlackWhite;
	int		m_iUnknown2A90;
	int		m_iLadder;
	int		m_iFallOver;
	float	m_flUnknown2A9C;
	Vector	m_vecViewOfs;
	float	m_flLadderTime;
	Vector	m_vecLadder;

	int		m_iFPSMin;
	int		m_iFPSMax;
	int		m_iFPSAverage;
	bool	m_bFPSToggle;
	float	m_flNextFPSTime;
	int		m_iFPS[100];
	int		m_iFPSCount;

	char	m_szInventoryItem1[128];
	char	m_szInventoryItem2[128];
	char	m_szInventoryItem3[128];
	char	m_szInventoryItem4[128];
	char	m_szInventoryItem5[128];
	char	m_szInventoryItem6[128];
	char	m_szInventoryWeapon1[128];
	char	m_szInventoryWeapon2[128];
	char	m_szInventoryWeapon3[128];
	int		m_iUnused30E4;		// not referenced

	HSPRITE						m_hsprCursor;
	HSPRITE						m_hsprLogo; // buz: make public
	float m_flTime;	   // the current client time
	float m_fOldTime;  // the time at which the HUD was last redrawn
	double m_flTimeDelta; // the difference between flTime and fOldTime
	Vector	m_vecOrigin;
	Vector	m_vecAngles;

	int		m_iKeyBits;
	int		m_iHideHUDDisplay;
	int		m_iFOV;
	float	m_flFOV; // buz - that's my FOV!
	int		m_Teamplay;
	int		m_iRes;
	cvar_t  *m_pCvarStealMouse;
	cvar_t	*m_pCvarDraw;
	cvar_t	*RainInfo; // rain
	cvar_t	*m_pZoomSpeed; // buz

	bool	m_bCameraZoomOn;
	float	m_flCameraZoomFOV;
	int		m_iCameraZoom;
	int		m_iHoldBreath;
	float	m_flNextSwayTime;

	Vector	m_vecSkyPos; //LRC
	int		m_iSkyMode;  //LRC
	int		m_iGasMask; // buz
//	float	m_flNextBreathTime; // buz

	// buz: spec tank variables;
	int		m_SpecTank_on;
	Vector	m_SpecTank_point;
	float	m_SpecTank_defYaw;
	float	m_SpecTank_coneHor;
	float	m_SpecTank_coneVer;
	float	m_SpecTank_distFwd;
	float	m_SpecTank_distUp;
	int		m_SpecTank_Ammo;

	float	m_flPitchOfs;
	float	m_flYawOfs;
	Vector	m_vecSpecTankAngles;
	float	m_flSpecTankTime;
	float	m_flSpecTankFrametime;
	int		m_iMaglite;
	int		m_iBlur;
	int		m_iUnreadSMS;
	float	m_flUnreadSMSTime;
	bool	m_bViewLock;
	int		m_iViewEntity;
	float	m_flForwardTapTime;
	float	m_flBackTapTime;
	float	m_flLeftTapTime;
	float	m_flRightTapTime;

	int		m_iUnused31D4[14];

	// buz: die time
	float	m_flDeadTime;

	float	m_flGreyFadeStart;
	float	m_flGreyFadeEnd;

	int m_iFontHeight;
	int DrawHudNumber(int x, int y, int iFlags, int iNumber, int r, int g, int b );
	int DrawHudString(int x, int y, int iMaxX, char *szString, int r, int g, int b );
	int DrawHudStringReverse( int xpos, int ypos, int iMinX, char *szString, int r, int g, int b );
	int DrawHudNumberString( int xpos, int ypos, int iMinX, int iNumber, int r, int g, int b );
	int GetNumWidth(int iNumber, int iFlags);

	int m_iHUDColor; //LRC

private:
	// the memory for these arrays are allocated in the first call to CHud::VidInit(), when the hud.txt and associated sprites are loaded.
	// freed in ~CHud()
	HSPRITE *m_rghSprites;	/*[HUD_SPRITE_COUNT]*/			// the sprites loaded from hud.txt
	wrect_t *m_rgrcRects;	/*[HUD_SPRITE_COUNT]*/
	char *m_rgszSpriteNames; /*[HUD_SPRITE_COUNT][MAX_SPRITE_NAME_LENGTH]*/

//	struct cvar_s *default_fov; // buz: turn off
public:
	char	m_szObjectiveTitle[256];
	char	m_szObjectiveText[256];

	HSPRITE GetSprite( int index )
	{
		return (index < 0) ? 0 : m_rghSprites[index];
	}

	wrect_t& GetSpriteRect( int index )
	{
		return m_rgrcRects[index];
	}


	int GetSpriteIndex( const char *SpriteName );	// gets a sprite index, for use in the m_rghSprites[] array

	CHudAmmo		m_Ammo;
	CHudHealth		m_Health;
	CHudSpectator		m_Spectator;
	CHudGeiger		m_Geiger;
	CHudBattery		m_Battery;
	CBlackBar		m_BlackBar;
	CNeffect		m_Neffect;
	CHudTrain		m_Train;
	CHudFlashlight	m_Flash;
	CHudMessage		m_Message;
	CHudStatusBar   m_StatusBar;
	CHudDeathNotice m_DeathNotice;
	CHudSayText		m_SayText;
	CHudMenu		m_Menu;
	CHudAmmoSecondary	m_AmmoSecondary;
	CHudTextMessage m_TextMessage;
	CHudStatusIcons m_StatusIcons;
	CHudLensflare	m_Lensflare;
	CHudParticle	m_Particle; // (LRC) -- 30/08/02 November235: Particles to Order

	void Init( void );
	void VidInit( void );
	void Think(void);
	int Redraw( float flTime, int intermission );
	int UpdateClientData( client_data_t *cdata, float time );

	CHud() : m_iSpriteCount(0), m_pHudList(NULL) {}
	~CHud();			// destructor, frees allocated memory

	// user messages
	int _cdecl MsgFunc_Damage(const char *pszName, int iSize, void *pbuf );
	int _cdecl MsgFunc_GameMode(const char *pszName, int iSize, void *pbuf );
	int _cdecl MsgFunc_Logo(const char *pszName,  int iSize, void *pbuf);
	int _cdecl MsgFunc_ResetHUD(const char *pszName,  int iSize, void *pbuf);
	void _cdecl MsgFunc_InitHUD( const char *pszName, int iSize, void *pbuf );
	void _cdecl MsgFunc_ViewMode( const char *pszName, int iSize, void *pbuf );
	int _cdecl MsgFunc_SetFOV(const char *pszName,  int iSize, void *pbuf);
	int  _cdecl MsgFunc_Concuss( const char *pszName, int iSize, void *pbuf );

	int _cdecl MsgFunc_HUDColor(const char *pszName,  int iSize, void *pbuf);		//LRC
	void _cdecl MsgFunc_SetFog( const char *pszName, int iSize, void *pbuf );		//LRC
	void _cdecl MsgFunc_KeyedDLight( const char *pszName, int iSize, void *pbuf );	//LRC
	void _cdecl MsgFunc_SetSky( const char *pszName, int iSize, void *pbuf );		//LRC
    void _cdecl MsgFunc_AddShine( const char *pszName, int iSize, void *pbuf );     //LRC

	int  _cdecl MsgFunc_GasMask( const char *pszName, int iSize, void *pbuf ); // buz
	int  _cdecl MsgFunc_SpecTank( const char *pszName, int iSize, void *pbuf ); // buz
	int  _cdecl MsgFunc_PlayMP3( const char *pszName, int iSize, void *pbuf ); // buz
	int  _cdecl MsgFunc_RainData( const char *pszName, int iSize, void *pbuf ); // buz

	int  _cdecl MsgFunc_ProFont( const char *pszName, int iSize, void *pbuf );
	int  _cdecl MsgFunc_Maglite( const char *pszName, int iSize, void *pbuf );
	int  _cdecl MsgFunc_Blur( const char *pszName, int iSize, void *pbuf );
	int  _cdecl MsgFunc_UnreadSMS( const char *pszName, int iSize, void *pbuf );
	int  _cdecl MsgFunc_ViewKnock( const char *pszName, int iSize, void *pbuf );
	int  _cdecl MsgFunc_CoFWeapon( const char *pszName, int iSize, void *pbuf );
	int  _cdecl MsgFunc_ViewEntity( const char *pszName, int iSize, void *pbuf );
	int  _cdecl MsgFunc_ViewLock( const char *pszName, int iSize, void *pbuf );
	int  _cdecl MsgFunc_Ladder( const char *pszName, int iSize, void *pbuf );
	int  _cdecl MsgFunc_FallOver( const char *pszName, int iSize, void *pbuf );
	int  _cdecl MsgFunc_BlackWhite( const char *pszName, int iSize, void *pbuf );
	int  _cdecl MsgFunc_WeaponBody( const char *pszName, int iSize, void *pbuf );
	int  _cdecl MsgFunc_PhoneOff( const char *pszName, int iSize, void *pbuf );
	int  _cdecl MsgFunc_Muzzleflash( const char *pszName, int iSize, void *pbuf );
	int  _cdecl MsgFunc_FlashFlags( const char *pszName, int iSize, void *pbuf );
	int  _cdecl MsgFunc_Boat( const char *pszName, int iSize, void *pbuf );
	int  _cdecl MsgFunc_Ripple( const char *pszName, int iSize, void *pbuf );
	int  _cdecl MsgFunc_AddNote( const char *pszName, int iSize, void *pbuf );
	int  _cdecl MsgFunc_AttachCam( const char *pszName, int iSize, void *pbuf );
	int  _cdecl MsgFunc_BigPrick( const char *pszName, int iSize, void *pbuf );
	int  _cdecl MsgFunc_NewFade( const char *pszName, int iSize, void *pbuf );
	int  _cdecl MsgFunc_SkipScene( const char *pszName, int iSize, void *pbuf );
	int  _cdecl MsgFunc_IronSights( const char *pszName, int iSize, void *pbuf );
	int  _cdecl MsgFunc_Wheelchair( const char *pszName, int iSize, void *pbuf );
	int  _cdecl MsgFunc_GreyFade( const char *pszName, int iSize, void *pbuf );
	int  _cdecl MsgFunc_TakePhoto( const char *pszName, int iSize, void *pbuf );
	int  _cdecl MsgFunc_Objectives( const char *pszName, int iSize, void *pbuf );
	int  _cdecl MsgFunc_Unlocks( const char *pszName, int iSize, void *pbuf );
	int  _cdecl MsgFunc_CustomSlv( const char *pszName, int iSize, void *pbuf );
	int  _cdecl MsgFunc_CustomFng( const char *pszName, int iSize, void *pbuf );
	int  _cdecl MsgFunc_CustomGlv( const char *pszName, int iSize, void *pbuf );
	int  _cdecl MsgFunc_PlaySFX( const char *pszName, int iSize, void *pbuf );
	int  _cdecl MsgFunc_Inventory( const char *pszName, int iSize, void *pbuf );
	int  _cdecl MsgFunc_Materials( const char *pszName, int iSize, void *pbuf );

	// Screen information
	SCREENINFO	m_scrinfo;

	int	m_iWeaponBits;
	int	m_fPlayerDead;
	int m_iIntermission;

	// sprite indexes
	int m_HUD_number_0;


	void AddHudElem(CHudBase *p);

	float GetSensitivity();

	char *GetKeyForCommand( char *pszCommand );
	char *GetSlotItem( int iSlot );
	char *CheckItem( char *pszName, char *pszFile );
	int AverageFPS( int iFPS );
	void DrawBlurTest( float frametime );
	void DrawNewFade( float flTime );

	int PlusUnknown5590( int iValue );
	int MinusUnknown5590( int iValue );

	int		m_iUnknown5590;
};

class TeamFortressViewport;

extern CHud gHUD;
extern TeamFortressViewport *gViewPort;

bool IsViewEntityActive( void );

extern int g_iPlayerClass;
extern int g_iTeamNumber;
extern int g_iUser1;
extern int g_iUser2;
extern int g_iUser3;

