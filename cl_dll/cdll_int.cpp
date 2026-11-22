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
//  cdll_int.c
//
// this implementation handles the linking of the engine to the DLL
//

#include "MP3FileInfo.h"
#include "hud.h"
#include "cl_util.h"
#include "netadr.h"
#include "vgui_SchemeManager.h"
#include "log.h" // buz
#include "exception.h" // buz



extern "C"
{
#include "pm_shared.h"
}

#include <string.h>
#include "hud_servers.h"
#include "vgui_int.h"
#include "vcs_info.h"
#include "glmanager.h"
#include "../engine/mobility_int.h"
#include "mp3.h"
#include "irrklang.h"

#include "exportdef.h"


cl_enginefunc_t gEngfuncs;
CHud gHUD;
MP3FileInfo gMP3Info;
TeamFortressViewport *gViewPort = NULL;
mobile_engfuncs_t *gMobileEngfuncs = NULL;

#if defined( INTERNAL_VGUI_SUPPORT )
extern "C" void InitVGUISupportAPI( void *api );
void *g_pKeepVGUISupport = (void *)InitVGUISupportAPI;
#endif

void InitInput (void);
void EV_HookEvents( void );
void IN_Commands( void );
void IrrKlangFrame( void );

/*
========================== 
    Initialize

Called when the DLL is first loaded.
==========================
*/
extern "C" 
{
int		DLLEXPORT Initialize( cl_enginefunc_t *pEnginefuncs, int iVersion );
int		DLLEXPORT HUD_VidInit( void );
void	DLLEXPORT HUD_Init( void );
int		DLLEXPORT HUD_Redraw( float flTime, int intermission );
int		DLLEXPORT HUD_UpdateClientData( client_data_t *cdata, float flTime );
void	DLLEXPORT HUD_Reset ( void );
void	DLLEXPORT HUD_PlayerMove( struct playermove_s *ppmove, int server );
void	DLLEXPORT HUD_PlayerMoveInit( struct playermove_s *ppmove );
char	DLLEXPORT HUD_PlayerMoveTexture( char *name );
int		DLLEXPORT HUD_ConnectionlessPacket( const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size );
int		DLLEXPORT HUD_GetHullBounds( int hullnumber, float *mins, float *maxs );
void	DLLEXPORT HUD_Frame( double time );
void	DLLEXPORT HUD_VoiceStatus(int entindex, qboolean bTalking);
void	DLLEXPORT HUD_DirectorMessage( int iSize, void *pbuf );
}

/*
================================
HUD_GetHullBounds

  Engine calls this to enumerate player collision hulls, for prediction.  Return 0 if the hullnumber doesn't exist.
================================
*/
int DLLEXPORT HUD_GetHullBounds( int hullnumber, float *mins, float *maxs )
{
	int iret = 0;

	switch ( hullnumber )
	{
	case 0:				// Normal player
		mins = Vector(-16, -16, -36);
		maxs = Vector(16, 16, 36);
		iret = 1;
		break;
	case 1:				// Crouched player
		mins = Vector(-16, -16, -18 );
		maxs = Vector(16, 16, 18 );
		iret = 1;
		break;
	case 2:				// Point based hull
		mins = Vector( 0, 0, 0 );
		maxs = Vector( 0, 0, 0 );
		iret = 1;
		break;
	}

	return iret;
}

/*
================================
HUD_ConnectionlessPacket

 Return 1 if the packet is valid.  Set response_buffer_size if you want to send a response packet.  Incoming, it holds the max
  size of the response_buffer, so you must zero it out if you choose not to respond.
================================
*/
int	DLLEXPORT HUD_ConnectionlessPacket( const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size )
{
	// Parse stuff from args
	int max_buffer_size = *response_buffer_size;

	// Zero it out since we aren't going to respond.
	// If we wanted to response, we'd write data into response_buffer
	*response_buffer_size = 0;

	// Since we don't listen for anything here, just respond that it's a bogus message
	// If we didn't reject the message, we'd return 1 for success instead.
	return 0;
}

void DLLEXPORT HUD_PlayerMoveInit( struct playermove_s *ppmove )
{
	PM_Init( ppmove );
}

char DLLEXPORT HUD_PlayerMoveTexture( char *name )
{
	return PM_FindTextureType( name );
}

void DLLEXPORT HUD_PlayerMove( struct playermove_s *ppmove, int server )
{
	PM_Move( ppmove, server );
}

static int ( *g_pfnEngineServerCmd )( const char *szCmdString );
static int ( *g_pfnEngineServerCmdUnreliable )( const char *szCmdString );

static int EngineHackCmd( const char *szCmdString )
{
	char szCmd[256];

	if( strstr( szCmdString, "optionshack" ))
	{
		const char *pszArg = strstr( szCmdString, "optionshack" ) + strlen( "optionshack" );

		while( *pszArg == ' ' )
			pszArg++;

		switch( atoi( pszArg ))
		{
		case 0:
			gEngfuncs.pfnClientCmd( "togglemenu\nmenu_internetgames\n" );
			break;
		case 1:
			gEngfuncs.pfnClientCmd( "togglemenu\nmenu_creategame\n" );
			break;
		case 2:
			gEngfuncs.pfnClientCmd( "togglemenu\nmenu_options\n" );
			break;
		}

		return 1;
	}

	if( strstr( szCmdString, "savehack" ))
	{
		const char *pszArg = strstr( szCmdString, "savehack" ) + strlen( "savehack" );

		while( *pszArg == ' ' )
			pszArg++;

		snprintf( szCmd, sizeof( szCmd ), "savehack %s", pszArg );

		if( !strchr( szCmd, '\n' ))
			strncat( szCmd, "\n", sizeof( szCmd ) - strlen( szCmd ) - 1 );

		gEngfuncs.pfnClientCmd( szCmd );
		return 1;
	}

	return 0;
}

static int SendServerCmd( const char *szCmdString )
{
	if( EngineHackCmd( szCmdString ))
		return 0;

	return g_pfnEngineServerCmd( szCmdString );
}

static int SendServerCmdUnreliable( const char *szCmdString )
{
	if( strstr( szCmdString, "savehack" ) && EngineHackCmd( szCmdString ))
		return 0;

	return g_pfnEngineServerCmdUnreliable( szCmdString );
}

int DLLEXPORT Initialize( cl_enginefunc_t *pEnginefuncs, int iVersion )
{
	gEngfuncs = *pEnginefuncs;

	if (iVersion != CLDLL_INTERFACE_VERSION)
		return 0;

	memcpy( &gEngfuncs, pEnginefuncs, sizeof(cl_enginefunc_t) - sizeof( void * ) );

	if( gEngfuncs.pfnGetCvarPointer( "cl_filterstuffcmd" ) == 0 )
	{
		gEngfuncs.pfnFilteredClientCmd = gEngfuncs.pfnClientCmd;
	}
	else
	{
		gEngfuncs.pfnFilteredClientCmd = pEnginefuncs->pfnFilteredClientCmd;
	}

	g_pfnEngineServerCmd = gEngfuncs.pfnServerCmd;
	g_pfnEngineServerCmdUnreliable = gEngfuncs.pfnServerCmdUnreliable;
	gEngfuncs.pfnServerCmd = SendServerCmd;
	gEngfuncs.pfnServerCmdUnreliable = SendServerCmdUnreliable;

	EV_HookEvents();

	gEngfuncs.pfnRegisterVariable( "cl_game_build_commit", g_VCSInfo_Commit, 0 );
	gEngfuncs.pfnRegisterVariable( "cl_game_build_branch", g_VCSInfo_Branch, 0 );

	return 1;
}

int *HUD_GetRect( void )
{
	static int extent[4];

	extent[0] = gEngfuncs.GetWindowCenterX() - ScreenWidth / 2;
	extent[1] = gEngfuncs.GetWindowCenterY() - ScreenHeight / 2;
	extent[2] = gEngfuncs.GetWindowCenterX() + ScreenWidth / 2;
	extent[3] = gEngfuncs.GetWindowCenterY() + ScreenHeight / 2;

	return extent;
}


/*
==========================
	HUD_VidInit

Called when the game initializes
and whenever the vid_mode is changed
so the HUD can reinitialize itself.
==========================
*/

// default values of the game cvars
static const struct { const char *name; const char *value; } g_DefaultCvars[] =
{
	{ "bitmapfonts", "1" },
	{ "brightvalue", "0.5" },
	{ "cg_water", "1" },
	{ "cl_bloodeffects", "1" },
	{ "cl_cameraeffect", "0" },
	{ "cl_cheapweaponsmoke", "0" },
	{ "cl_coop_language", "1" },
	{ "cl_fovmultiplier", "1" },
	{ "cl_laddermode", "2" },
	{ "cl_lazyweapon_force", "2" },
	{ "cl_lowflashlight", "0" },
	{ "cl_nodoubletapdodge", "0" },
	{ "cl_noiseamount", "0.5" },
	{ "cl_noiseeffect", "0" },
	{ "cl_propdistance", "0" },
	{ "cl_ripples", "1" },
	{ "cl_smokeparticlelimit", "5" },
	{ "cl_subtitles", "1" },
	{ "cl_vignette", "0" },
	{ "cl_weapon_idle_dampener", "2" },
	{ "cl_weapon_idle_speed", "4" },
	{ "cof_fullbrighttextures", "0.5" },
	{ "cof_reducedlightcolour1", "0.23" },
	{ "cof_reducedlightcolour2", "0.23" },
	{ "cof_reducedlightcolour3", "0.26" },
	{ "cof_reducedlightradius", "60" },
	{ "cof_reducedlightxangle", "50" },
	{ "cof_reducedlightzangle", "90" },
	{ "gl4es_brushents", "0" },
	{ "gl_blurstepsx", "32" },
	{ "gl_blurstepsy", "24" },
	{ "gl_blurtest", "0" },
	{ "gl_brightness", "0" },
	{ "gl_brushents", "1" },
	{ "gl_bump", "1" },
	{ "gl_clientfx", "1" },
	{ "gl_clipplanes", "0" },
	{ "gl_contrast", "1" },
	{ "gl_customdecals", "1" },
	{ "gl_customsky", "1" },
	{ "gl_decalstatus", "0" },
	{ "gl_detailtex", "1" },
	{ "gl_dynlight", "1" },
	{ "gl_dynlight_ent", "1" },
	{ "gl_gamma", "1" },
	{ "gl_glows", "1" },
	{ "gl_glowsdebug", "0" },
	{ "gl_glowstraces", "5" },
	{ "gl_grayscale", "0" },
	{ "gl_highspecular", "1" },
	{ "gl_lambert", "2" },
	{ "gl_modelrenderer", "1" },
	{ "gl_nocombs", "0" },
	{ "gl_noshaders", "0" },
	{ "gl_opacity", "0.5" },
	{ "gl_posteffects", "1" },
	{ "gl_renderer", "1" },
	{ "gl_screenblur", "0" },
	{ "gl_showplane", "0" },
	{ "gl_skybright", "0" },
	{ "gl_specular", "1" },
	{ "gl_twopassdyn", "0" },
	{ "gl_twopassspecular", "0" },
	{ "r_weather", "1" },
};

static cvar_t *cof_defaults_applied;
void FixUnsupportedWavs( void );

// sets the default cvars once, after that the saved values are used
static void LoadDefaultCvars( void )
{
	for( size_t i = 0; i < sizeof( g_DefaultCvars ) / sizeof( g_DefaultCvars[0] ); i++ )
	{
		if( gEngfuncs.pfnGetCvarPointer( g_DefaultCvars[i].name ))
			gEngfuncs.Cvar_SetValue( g_DefaultCvars[i].name, atof( g_DefaultCvars[i].value ));
	}

	if( cof_defaults_applied )
		gEngfuncs.Cvar_SetValue( "cof_defaults_applied", 1.0f );
}

int DLLEXPORT HUD_VidInit( void )
{
	if( cof_defaults_applied && cof_defaults_applied->value < 1.0f )
		LoadDefaultCvars();

	gHUD.VidInit();

	VGui_Startup();

	return 1;
}

/*
==========================
	HUD_Init

Called whenever the client connects
to a server.  Reinitializes all 
the hud variables.
==========================
*/

void DLLEXPORT HUD_Init( void )
{
	SetupLogging (); // buz

	// buz: enable CrashRpt library
	LoadCrashRpt();

	InitInput();
	gHUD.Init();
	Scheme_Init();

	cof_defaults_applied = gEngfuncs.pfnRegisterVariable( "cof_defaults_applied", "0", FCVAR_ARCHIVE );
	gEngfuncs.pfnAddCommand( "cof_defaults", LoadDefaultCvars );
	FixUnsupportedWavs();
}


/*
==========================
	HUD_Redraw

called every screen frame to
redraw the HUD.
===========================
*/

int DLLEXPORT HUD_Redraw( float time, int intermission )
{
	gHUD.Redraw( time, intermission );

	GL_ResetEngineState();

	return 1;
}


/*
==========================
	HUD_UpdateClientData

called every time shared client
dll/engine data gets changed,
and gives the cdll a chance
to modify the data.

returns 1 if anything has been changed, 0 otherwise.
==========================
*/

int DLLEXPORT HUD_UpdateClientData(client_data_t *pcldata, float flTime )
{
	IN_Commands();

	return gHUD.UpdateClientData(pcldata, flTime );
}

/*
==========================
	HUD_Reset

Called at start and end of demos to restore to "non"HUD state.
==========================
*/

void DLLEXPORT HUD_Reset( void )
{
	gHUD.VidInit();
}

/*
==========================
HUD_Frame

Called by engine every frame that client .dll is loaded
==========================
*/

void DLLEXPORT HUD_Frame( double time )
{
	ServersThink( time );

	IrrKlangFrame();
	irrklang::SoundFrame();

	GetClientVoiceMgr()->Frame(time);
}


/*
==========================
HUD_VoiceStatus

Called when a player starts or stops talking.
==========================
*/

void DLLEXPORT HUD_VoiceStatus(int entindex, qboolean bTalking)
{
	GetClientVoiceMgr()->UpdateSpeakerStatus(entindex, bTalking);
}

/*
==========================
HUD_DirectorEvent

Called when a director event message was received
==========================
*/

void DLLEXPORT HUD_DirectorMessage( int iSize, void *pbuf )
{
	 gHUD.m_Spectator.DirectorMessage( iSize, pbuf );
}

// the engine's default touch layout without quick save/load and invnext/invprev,
// plus aim, inventory, weapontoggle and sprint buttons, used when touch is reset to defaults
static void SetupTouchDefaults( void )
{
	unsigned char color[4] = { 255, 255, 255, 255 };

	gMobileEngfuncs->pfnTouchResetDefaultButtons();
	gMobileEngfuncs->pfnTouchAddDefaultButton( "look", "", "_look", 0.500000, 0.000000, 1.000000, 1, color, 0, 0, 0 );
	gMobileEngfuncs->pfnTouchAddDefaultButton( "move", "", "_move", 0.000000, 0.000000, 0.500000, 1, color, 0, 0, 0 );
	gMobileEngfuncs->pfnTouchAddDefaultButton( "use", "touch_default/use", "+use", 0.880000, 0.454457, 1.000000, 0.681685, color, 2, 1, 0 );
	gMobileEngfuncs->pfnTouchAddDefaultButton( "jump", "touch_default/jump", "+jump", 0.880000, 0.227228, 1.000000, 0.454457, color, 2, 1, 0 );
	gMobileEngfuncs->pfnTouchAddDefaultButton( "attack", "touch_default/shoot", "+attack", 0.760000, 0.530200, 0.880000, 0.757428, color, 2, 1, 0 );
	gMobileEngfuncs->pfnTouchAddDefaultButton( "attack2", "touch_default/shoot_alt", "+attack2", 0.760000, 0.302971, 0.880000, 0.530200, color, 2, 1, 0 );
	gMobileEngfuncs->pfnTouchAddDefaultButton( "attack3", "touch_default/attack3", "+attack3", 0.640000, 0.302971, 0.760000, 0.530200, color, 2, 1, 0 );
	gMobileEngfuncs->pfnTouchAddDefaultButton( "messagemode", "touch_default/keyboard", "messagemode", 0.840000, 0.000000, 0.920000, 0.142222, color, 2, 1, 8 );
	gMobileEngfuncs->pfnTouchAddDefaultButton( "reload", "touch_default/reload", "+reload", 0.000000, 0.302971, 0.120000, 0.530200, color, 2, 1, 0 );
	gMobileEngfuncs->pfnTouchAddDefaultButton( "weapontoggle", "touch_default/weapontoggle", "weapontoggle", 0.000000, 0.075742, 0.120000, 0.302971, color, 2, 1, 0 );
	gMobileEngfuncs->pfnTouchAddDefaultButton( "scores", "touch_default/map", "+showscores", 0.760000, 0.000000, 0.840000, 0.142222, color, 2, 1, 8 );
	gMobileEngfuncs->pfnTouchAddDefaultButton( "inventory", "touch_default/inventory", "+inventory", 0.600000, 0.000000, 0.680000, 0.142222, color, 2, 1, 0 );
	gMobileEngfuncs->pfnTouchAddDefaultButton( "sprint", "touch_default/sprint", "+sprint", 0.200000, 0.318000, 0.280000, 0.500000, color, 2, 1, 0 );
	gMobileEngfuncs->pfnTouchAddDefaultButton( "duck", "touch_default/crouch", "+duck", 0.880000, 0.757428, 1.000000, 0.984656, color, 2, 1, 0 );
	gMobileEngfuncs->pfnTouchAddDefaultButton( "tduck", "touch_default/tduck", ";+duck", 0.560000, 0.833171, 0.620000, 0.946785, color, 2, 1, 0 );
	gMobileEngfuncs->pfnTouchAddDefaultButton( "edit", "touch_default/settings", "touch_enableedit", 0.420000, 0.000000, 0.500000, 0.151486, color, 2, 1, 32 );
	gMobileEngfuncs->pfnTouchAddDefaultButton( "menu", "touch_default/menu", "cancelselect", 0.000000, 0.833171, 0.080000, 0.984656, color, 2, 1, 0 );
	gMobileEngfuncs->pfnTouchAddDefaultButton( "spray", "touch_default/spray", "impulse 201", 0.680000, 0.000000, 0.760000, 0.142222, color, 2, 1, 8 );
	gMobileEngfuncs->pfnTouchAddDefaultButton( "voicechat", "touch_default/microphone", "+voicerecord", 0.780000, 0.817778, 0.860000, 0.960000, color, 2, 1, 8 );
}

extern "C" int DLLEXPORT HUD_MobilityInterface( mobile_engfuncs_t *gpMobileEngfuncs )
{
	if( gpMobileEngfuncs->version != MOBILITY_API_VERSION )
		return 1;
	gMobileEngfuncs = gpMobileEngfuncs;
	SetupTouchDefaults();
	return 0;
}

extern "C" int DLLEXPORT HUD_GetRenderInterface( int version, void *renderfuncs, void *callback )
{
	if( version != RENDER_INTERFACE_VERSION || !renderfuncs )
		return 0;

	g_pXashRenderAPI = (cof_render_api_t *)renderfuncs;
	return 1;
}

bool HUD_MessageBox( const char *msg )
{
	gEngfuncs.Con_Printf( msg );

	if( IsXashFWGS() )
	{
		gMobileEngfuncs->pfnSys_Warn( msg );
		return true;
	}

	return false;
}

bool IsXashFWGS()
{
	return gMobileEngfuncs != NULL;
}
