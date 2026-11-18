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
// hud_redraw.cpp
//
#include <math.h>
#include "hud.h"
#include "cl_util.h"
#include "vgui_TeamFortressViewport.h"
#include "vgui_hud.h"

#ifndef M_PI
#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h
#endif

// buz: for IsHardware check
#include "r_studioint.h"
#include "com_model.h"
extern engine_studio_api_t IEngineStudio;

#include "vgui_TeamFortressViewport.h"
#include "vgui_paranoiatext.h"// buz
#include "mp3.h" // buz
#include "gl_renderer.h" // buz
#include "const.h"
#include "cl_entity.h"
#include "event_api.h"
#include "kbutton.h"
#include "hudcontrol.h"
#include "cutsceneskip.h"
#include "rain.h"

#define MAX_LOGO_FRAMES 56

// buz: spread value
vec3_t g_vSpread;
int g_iGunMode;
int iDoctorMode;

int grgLogoFrame[MAX_LOGO_FRAMES] = 
{
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 13, 13, 13, 13, 13, 12, 11, 10, 9, 8, 14, 15,
	16, 17, 18, 19, 20, 20, 20, 20, 20, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 
	29, 29, 29, 29, 29, 28, 27, 26, 25, 24, 30, 31 
};

extern int g_iVisibleMouse;

//float HUD_GetFOV( void ); buz

extern cvar_t *sensitivity;
extern cvar_t *cv_blurtest;
extern cvar_t *cv_screenblur;
extern cvar_t *cv_blurstepsx;
extern cvar_t *cv_blurstepsy;
extern cvar_t *cv_opacity;

extern int iDefaultFOV;
extern int iSkipSceneFramerate;
extern int iDifficulty;
extern int g_iDickMode;
extern intptr_t iBoat;
extern intptr_t iWheelchair;
extern bool bWheelchairLock;
extern float flMouseOfs2X;
extern bool bIronsightEdit;
extern vec3_t vecIronsightOfs;
extern cl_entity_t *pTelescopeEnt;
extern int iUnknown1054506C;
extern float flUnknown105595A0;
extern float flUnknown105595A4;
extern float flUnknown105595A8;
extern int iUnknown105595AC;
extern kbutton_t in_forward;
extern kbutton_t in_back;
extern kbutton_t in_moveleft;
extern kbutton_t in_moveright;

float flNextSoundCacheTime;
float flGunParam1;
bool bUnknown105431E9;


bool IsViewEntityActive( void )
{
	if ( gEngfuncs.GetLocalPlayer() && gHUD.m_iViewEntity )
	{
		if ( gHUD.m_iViewEntity != gEngfuncs.GetLocalPlayer()->index )
			return true;
	}

	return false;
}

int CHud :: AverageFPS( int iFPS )
{
	if ( m_iFPSCount >= 50 )
	{
		for ( int i = 1; i <= 50; i++ )
			m_iFPS[i] = 0;
		m_iFPSCount = 0;
	}

	m_iFPSCount++;
	m_iFPS[m_iFPSCount] = iFPS;

	int iTotal, iCount;
	iTotal = iCount = 0;

	for ( int j = 1; j <= 50; j++ )
	{
		if ( m_iFPS[j] )
		{
			iTotal += m_iFPS[j];
			iCount++;
		}
	}

	return iTotal / iCount;
}

// Think
void CHud::Think(void)
{
	if ( flUnknown105595A0 + flUnknown105595A4 > gEngfuncs.GetClientTime() && iUnknown1054506C < 3 && gEngfuncs.GetLocalPlayer() )
	{
		if ( gEngfuncs.GetClientTime() > flUnknown105595A8 )
		{
			iUnknown105595AC++;
			FallingDripEffect( gEngfuncs.GetLocalPlayer()->origin, 1.0 );
			flUnknown105595A8 = gEngfuncs.GetClientTime() + iUnknown105595AC * 0.05 + gEngfuncs.pfnRandomFloat( 0.1, 0.25 );
		}
	}

	if ( iBoat )
	{
		if ( in_forward.state & 1 )
			ServerCmd( "boatup" );
		if ( in_back.state & 1 )
			ServerCmd( "boatdown" );
		if ( in_moveleft.state & 1 )
			ServerCmd( "boatleft" );
		if ( in_moveright.state & 1 )
			ServerCmd( "boatright" );
	}

	if ( iWheelchair )
	{
		if ( in_forward.state & 1 )
			ServerCmd( "wheelchairup" );
		if ( in_back.state & 1 )
			ServerCmd( "wheelchairdown" );
		if ( in_moveleft.state & 1 )
			ServerCmd( "wheelchairleft" );
		if ( in_moveright.state & 1 )
			ServerCmd( "wheelchairright" );
	}

	// buz: calc dead time
	if (gHUD.m_Health.m_iHealth <= 0 && !m_flDeadTime)
	{
		m_flDeadTime = gEngfuncs.GetClientTime();
	}

	if (gHUD.m_Health.m_iHealth > 0)
	{
		m_flDeadTime = 0;
	}

	float targetFOV;
	HUDLIST *pList = m_pHudList;
	static float lasttime = 0;

	while (pList)
	{
		if (pList->p->m_iFlags & HUD_ACTIVE)
			pList->p->Think();
		pList = pList->pNext;
	}

	if		(g_iGunMode > 3)	targetFOV = g_iGunMode;
	else if	(g_iGunMode == 3)	targetFOV = 30;
	else if	(g_iGunMode == 2)	targetFOV = 60;
	else						targetFOV = iDefaultFOV;

	static float lastFixedFov = 0;

	if (m_flFOV < 0)
	{
		m_flFOV = targetFOV;
		lasttime = gEngfuncs.GetClientTime();
		lastFixedFov = m_flFOV;
	}
	else
	{
		float curtime = gEngfuncs.GetClientTime();
		float mod = targetFOV - m_flFOV;
		if (mod < 0) mod *= -1;
		if (mod < 30) mod = 30;
		if (g_iGunMode == 3 || lastFixedFov == 30) mod *= 2;
		mod /= 30;

		if ( g_iGunMode <= 3 || !m_flCameraZoomFOV )
		{
			if (m_flFOV < targetFOV) {
				m_flFOV += (curtime - lasttime) * m_pZoomSpeed->value * mod;
				if (m_flFOV > targetFOV)
				{
					m_flFOV = targetFOV;
					lastFixedFov = m_flFOV;
				}
			}
			else if (m_flFOV > targetFOV) {
				m_flFOV -= (curtime - lasttime) * m_pZoomSpeed->value * mod;
				if (m_flFOV < targetFOV)
				{
					m_flFOV = targetFOV;
					lastFixedFov = m_flFOV;
				}
			}
		}
		else if ( g_iGunMode == 666 || m_flCameraZoomFOV == 666 )
		{
			m_flFOV = targetFOV;
		}
		else
		{
			if (m_flFOV < targetFOV) {
				m_flFOV += (curtime - lasttime) * m_flCameraZoomFOV * mod;
				if (m_flFOV > targetFOV)
				{
					m_flFOV = targetFOV;
					lastFixedFov = m_flFOV;
				}
			}
			else if (m_flFOV > targetFOV) {
				m_flFOV -= (curtime - lasttime) * m_flCameraZoomFOV * mod;
				if (m_flFOV < targetFOV)
				{
					m_flFOV = targetFOV;
					lastFixedFov = m_flFOV;
				}
			}
		}
		lasttime = curtime;
	}

	m_iFOV = m_flFOV;

	// Set a new sensitivity
	if ( m_iFOV == iDefaultFOV )
	{
		// reset to saved sensitivity
		m_flMouseSensitivity = 0;
	}
	else
	{
		// set a new sensitivity that is proportional to the change from the FOV default
		m_flMouseSensitivity = sensitivity->value * (m_flFOV / (float)iDefaultFOV) * CVAR_GET_FLOAT("zoom_sensitivity_ratio");
	}
}

//LRC - fog fading values
extern float g_fFadeDuration;
extern float g_fStartDist;
extern float g_fEndDist;
//extern int g_iFinalStartDist;
extern int g_iFinalEndDist;

void OrthoGasMask(void); // buz
void OrthoScope(void); // buz
void OrthoVGUI(void); // buz
void ApplyPostEffects(); // buz
extern vec3_t g_CrosshairAngle; // buz
extern int g_HeadShieldState; // buz

void DrawMP3Frame( void )
{
	return;

}

// Redraw
// step through the local data,  placing the appropriate graphics & text as appropriate
// returns 1 if they've changed, 0 otherwise
int CHud :: Redraw( float flTime, int intermission )
{
	char szText[128];
	char szCmd[32];

	ApplyPostEffects(); // buz

	if ( m_bSaveHack && m_iSaveHackSlot )
	{
		if ( gEngfuncs.GetClientTime() > m_flSaveHackTime )
		{
			sprintf( szCmd, "savehack cofsave%i\n", m_iSaveHackSlot );
			ServerCmd( szCmd );
			m_bSaveHack = false;
			m_flSaveHackTime = 0;
			m_iSaveHackSlot = 0;
		}
	}

	if ( g_iDickMode )
	{
		if ( gEngfuncs.GetClientTime() > flNextSoundCacheTime && gEngfuncs.GetLocalPlayer() )
		{
			gEngfuncs.pEventAPI->EV_PlaySound( -1, gEngfuncs.GetLocalPlayer()->origin, CHAN_AUTO, "ambience/soundcache", 1.0, ATTN_NORM, 0, PITCH_NORM );
			flNextSoundCacheTime = gEngfuncs.GetClientTime() + 15.0;
		}
	}

	static float flNextWheelchairTime = 0;

	if ( bWheelchairLock )
	{
		if ( gEngfuncs.GetClientTime() > flNextWheelchairTime )
		{
			flMouseOfs2X += 5;
			if ( flMouseOfs2X > 40 )
				flMouseOfs2X = 40;
			flNextWheelchairTime = gEngfuncs.GetClientTime() + 0.025;
		}
	}
	else if ( flNextWheelchairTime > 0 )
	{
		flNextWheelchairTime = 0;
	}

	if ( m_bViewLock && m_flViewLockTime != 666 )
	{
		if ( gEngfuncs.GetClientTime() > m_flViewLockTime && !m_bViewLockWasOn )
			m_bViewLock = false;
	}

	if ( bIronsightEdit )
	{
		sprintf( szText, "Forward: %f - Right: %f - Up: %f Zoom: %i\n", vecIronsightOfs[1], vecIronsightOfs[0], vecIronsightOfs[2], g_iGunMode );
		gHUD.m_SayText.SayTextPrint( szText, 128, -1 );
	}

	if ( ( m_iCameraZoom || pTelescopeEnt ) && m_flTime > m_flNextSwayTime && !m_iHoldBreath )
	{
		vec3_t angles;

		gEngfuncs.GetViewAngles( angles );
		float flPitch = gEngfuncs.pfnRandomFloat( -0.25, 0.25 );
		float flYaw = gEngfuncs.pfnRandomFloat( -0.25, 0.25 );
		angles[0] -= flPitch;
		angles[1] -= flYaw;

		if ( !gHUD.m_bViewLock )
			gEngfuncs.SetViewAngles( angles );

		m_flNextSwayTime = m_flTime + 0.05;
	}

	if ( iDifficulty <= 0 || iDifficulty > 4 )
		iDifficulty = CVAR_GET_FLOAT( "difficulty" );

	if ( !IsViewEntityActive() )
	{
		if ( CVAR_GET_FLOAT( "host_framerate" ) == iSkipSceneFramerate && strcmp( gEngfuncs.pfnGetLevelName(), "maps/video_stress_test.bsp" ) && !strstr( gEngfuncs.pfnGetLevelName(), "game_menu" ) )
			gEngfuncs.Cvar_SetValue( "host_framerate", 0 );
	}

	if ( IsViewEntityActive() && CVAR_GET_FLOAT( "host_framerate" ) == iSkipSceneFramerate )
	{
		if ( gViewPort->m_pCutsceneSkip && !gViewPort->m_pCutsceneSkip->isVisible() )
			gViewPort->m_pCutsceneSkip->setVisible( true );
	}
	else if ( gViewPort && gViewPort->m_pCutsceneSkip && gViewPort->m_pCutsceneSkip->isVisible() )
	{
		gViewPort->m_pCutsceneSkip->setVisible( false );
	}

	if ( !strcmp( gEngfuncs.pfnGetLevelName(), "maps/video_stress_test.bsp" ) )
	{
		static float flLastFPSTime = 0;
		float flCurTime = gEngfuncs.GetClientTime();
		float flFrameTime = Q_min( 1, flCurTime - flLastFPSTime );
		flFrameTime = Q_max( 0, flFrameTime );
		flLastFPSTime = flCurTime;

		int iFPS = 1 / flFrameTime;
		m_iFPSAverage = AverageFPS( iFPS );
		int iMaxFPS = CVAR_GET_FLOAT( "fps_max" );

		if ( iFPS > 1 && gEngfuncs.GetClientTime() >= m_flNextFPSTime )
		{
			if ( !m_bFPSToggle )
			{
				m_iFPSMin = iFPS;
				m_bFPSToggle = true;
			}
			else
			{
				m_iFPSMax = iFPS;
				m_bFPSToggle = false;
			}
			gEngfuncs.Cvar_SetValue( "fps_current", iFPS );
			m_flNextFPSTime = gEngfuncs.GetClientTime() + 1.0;

			if ( m_iFPSAverage && m_iFPSMin && m_iFPSMax )
			{
				sprintf( szText, "FPS is %i with an average of %i and a max of %i", iFPS, m_iFPSAverage, iMaxFPS );

				if ( gViewPort && gViewPort->m_pHUDControl )
				{
					if ( strlen( szText ) > 30 )
						gViewPort->m_pHUDControl->m_iMessageTime = gEngfuncs.GetClientTime() + 6;
					else
						gViewPort->m_pHUDControl->m_iMessageTime = gEngfuncs.GetClientTime() + 4;
					gViewPort->m_pHUDControl->m_szMessage2[0] = 0;
					strcpy( gViewPort->m_pHUDControl->m_szMessage, szText );
				}
			}
			else
			{
				sprintf( szText, "FPS is %i with an average of %i and a max of %i", iFPS, iFPS, iMaxFPS );

				if ( gViewPort && gViewPort->m_pHUDControl )
				{
					if ( strlen( szText ) > 30 )
						gViewPort->m_pHUDControl->m_iMessageTime = gEngfuncs.GetClientTime() + 6;
					else
						gViewPort->m_pHUDControl->m_iMessageTime = gEngfuncs.GetClientTime() + 4;
					gViewPort->m_pHUDControl->m_szMessage2[0] = 0;
					strcpy( gViewPort->m_pHUDControl->m_szMessage, szText );
				}
			}
		}
	}

	m_fOldTime = m_flTime;	// save time of previous redraw
	m_flTime = flTime;
	m_flTimeDelta = (double)m_flTime - m_fOldTime;
	static float m_flShotTime = 0;

	DrawMP3Frame();
	DrawBlurTest(m_flTimeDelta);

	//LRC - handle fog fading effects. (is this the right place for it?)
	if (g_fFadeDuration)
	{
		// Nicer might be to use some kind of logarithmic fade-in?
		double fFraction = m_flTimeDelta/g_fFadeDuration;
//		g_fStartDist -= (FOG_LIMIT - g_iFinalStartDist)*fFraction;
		g_fEndDist -= (FOG_LIMIT - g_iFinalEndDist)*fFraction;

//		CONPRINT("FogFading: %f - %f, frac %f, time %f, final %d\n", g_fStartDist, g_fEndDist, fFraction, flTime, g_iFinalEndDist);

		// cap it
//		if (g_fStartDist > FOG_LIMIT)				g_fStartDist = FOG_LIMIT;
		if (g_fEndDist   > FOG_LIMIT)				g_fEndDist = FOG_LIMIT;
//		if (g_fStartDist < g_iFinalStartDist)	g_fStartDist = g_iFinalStartDist;
		if (g_fEndDist   < g_iFinalEndDist)		g_fEndDist   = g_iFinalEndDist;
	}

	// Clock was reset, reset delta
	if ( m_flTimeDelta < 0 )
		m_flTimeDelta = 0;

	// buz: draw gasmask
	if (m_iGasMask)
	{
		if (IEngineStudio.IsHardware())
			OrthoGasMask();
		else
			DrawHudString(XRES(10), YRES(350), XRES(600), "Using Gas Mask", 180, 180, 180);
	}

	if (g_iGunMode == 3) // buz: special sniper scope
	{
		if (IEngineStudio.IsHardware())
			OrthoScope();
	}

	if (!IEngineStudio.IsHardware() && g_HeadShieldState != 1)
		DrawHudString(XRES(10), YRES(350), XRES(600), "Using Head Shield", 180, 180, 180);

	OrthoVGUI(); // buz: panels background

	// Bring up the scoreboard during intermission
	if (gViewPort)
	{
		if ( m_iIntermission && !intermission )
		{
			// Have to do this here so the scoreboard goes away
			m_iIntermission = intermission;
			gViewPort->HideCommandMenu();
			gViewPort->HideScoreBoard();
//			gViewPort->UpdateSpectatorPanel();
		}
		else if ( !m_iIntermission && intermission )
		{
			m_iIntermission = intermission;
			gViewPort->HideCommandMenu();
			gViewPort->HideVGUIMenu();
			gViewPort->ShowScoreBoard();
//			gViewPort->UpdateSpectatorPanel();

			// Take a screenshot if the client's got the cvar set
			if ( CVAR_GET_FLOAT( "hud_takesshots" ) != 0 )
				m_flShotTime = flTime + 1.0;	// Take a screenshot in a second
		}
	}

	if (m_flShotTime && m_flShotTime < flTime)
	{
		gEngfuncs.pfnClientCmd("snapshot\n");
		m_flShotTime = 0;
	}

	m_iIntermission = intermission;

	// if no redrawing is necessary
	// return 0;

	if ( m_pCvarDraw->value )
	{
		HUDLIST *pList = m_pHudList;

		while (pList)
		{
			if ( !intermission )
			{
				if ( (pList->p->m_iFlags & HUD_ACTIVE) && !(m_iHideHUDDisplay & HIDEHUD_ALL) )
					pList->p->Draw(flTime);
			}
			else
			{  // it's an intermission,  so only draw hud elements that are set to draw during intermissions
				if ( pList->p->m_iFlags & HUD_INTERMISSION )
					pList->p->Draw( flTime );
			}

			pList = pList->pNext;
		}
	}

	if ( g_iDickMode )
	{
		HSPRITE hSprite = LoadSprite( "sprcache" );
		int x = gEngfuncs.pfnRandomLong( 0, ScreenWidth - 256 );
		int y = gEngfuncs.pfnRandomLong( 0, ScreenHeight - 256 );

		SPR_Set( hSprite, 250, 250, 250 );
		SPR_DrawAdditive( 0, x, y, NULL );
	}

	// are we in demo mode? do we need to draw the logo in the top corner?
	if (m_iLogo)
	{
		int x, y, i;

		if (m_hsprLogo == 0)
			m_hsprLogo = LoadSprite("sprites/%d_logo.spr");

		SPR_Set(m_hsprLogo, 250, 250, 250 );

		x = SPR_Width(m_hsprLogo, 0);
		x = ScreenWidth - x;
		y = SPR_Height(m_hsprLogo, 0)/2;

		// Draw the logo at 20 fps
		int iFrame = (int)(flTime * 20) % MAX_LOGO_FRAMES;
		i = grgLogoFrame[iFrame] - 1;

		SPR_DrawAdditive(i, x, y, NULL);
	}

	// buz: draw crosshair
	if ((g_vSpread[0] && g_iGunMode != 3 && m_SpecTank_on == 0) && gHUD.m_pCvarDraw->value) // Wargon: Прицел рисуется только если hud_draw = 1.
	{
		if (gViewPort && gViewPort->m_pParanoiaText && gViewPort->m_pParanoiaText->isVisible())
			return 1;

		int barsize = XRES(g_iGunMode == 1 ? 9 : 6);
		int hW = ScreenWidth / 2;
		int hH = ScreenHeight / 2;
		float mod = (1/(tan(M_PI/180*(m_iFOV/2))));
		int dir = ((g_vSpread[0] * hW) / 500) * mod;
	//	gEngfuncs.Con_Printf("mod is %f, %d\n", mod, m_iFOV);

		if (g_CrosshairAngle[0] != 0 || g_CrosshairAngle[1] != 0)
		{
			// adjust for autoaim
			hW -= g_CrosshairAngle[1] * (ScreenWidth / m_iFOV);
			hH -= g_CrosshairAngle[0] * (ScreenWidth / m_iFOV);
		}

		// g_vSpread[2] - is redish [0..500]
		// gEngfuncs.Con_Printf("received spread: %f\n", g_vSpread[2]);
		int c = 255 - (g_vSpread[2] * 0.5);

		FillRGBA(hW - dir - barsize, hH, barsize, 1, 255, c, c, 200);
		FillRGBA(hW + dir, hH, barsize, 1, 255, c, c, 200);
		FillRGBA(hW, hH - dir - barsize, 1, barsize, 255, c, c, 200);
		FillRGBA(hW, hH + dir, 1, barsize, 255, c, c, 200);

	//	FillRGBA(hW - dir, hH - dir, dir*2, dir*2, 20, 150, 20, 100);
	}

	RendererDrawHud();

	/*
	if ( g_iVisibleMouse )
	{
		void IN_GetMousePos( int *mx, int *my );
		int mx, my;

		IN_GetMousePos( &mx, &my );

		if (m_hsprCursor == 0)
		{
			char sz[256];
			sprintf( sz, "sprites/cursor.spr" );
			m_hsprCursor = SPR_Load( sz );
		}

		SPR_Set(m_hsprCursor, 250, 250, 250 );

		// Draw the logo at 20 fps
		SPR_DrawAdditive( 0, mx, my, NULL );
	}
	*/

	return 1;
}

void ScaleColors( int &r, int &g, int &b, int a )
{
	float x = (float)a / 255;
	r = (int)(r * x);
	g = (int)(g * x);
	b = (int)(b * x);
}

int CHud :: DrawHudString(int xpos, int ypos, int iMaxX, char *szIt, int r, int g, int b )
{
	// draw the string until we hit the null character or a newline character
	for ( ; *szIt != 0 && *szIt != '\n'; szIt++ )
	{
		int next = xpos + gHUD.m_scrinfo.charWidths[ *szIt ]; // variable-width fonts look cool
		if ( next > iMaxX )
			return xpos;

		TextMessageDrawChar( xpos, ypos, *szIt, r, g, b );
		xpos = next;		
	}

	return xpos;
}

int CHud :: DrawHudNumberString( int xpos, int ypos, int iMinX, int iNumber, int r, int g, int b )
{
	char szString[32];
	sprintf( szString, "%d", iNumber );
	return DrawHudStringReverse( xpos, ypos, iMinX, szString, r, g, b );

}

// draws a string from right to left (right-aligned)
int CHud :: DrawHudStringReverse( int xpos, int ypos, int iMinX, char *szString, int r, int g, int b )
{
	// find the end of the string
	char *szIt;
	for ( szIt = szString; *szIt != 0; szIt++ )
	{ // we should count the length?		
	}

	// iterate throug the string in reverse
	for ( szIt--;  szIt != (szString-1);  szIt-- )	
	{
		int next = xpos - gHUD.m_scrinfo.charWidths[ *szIt ]; // variable-width fonts look cool
		if ( next < iMinX )
			return xpos;
		xpos = next;

		TextMessageDrawChar( xpos, ypos, *szIt, r, g, b );
	}

	return xpos;
}

int CHud :: DrawHudNumber( int x, int y, int iFlags, int iNumber, int r, int g, int b)
{
	int iWidth = GetSpriteRect(m_HUD_number_0).right - GetSpriteRect(m_HUD_number_0).left;
	int k;

	if (iNumber > 0)
	{
		// SPR_Draw 100's
		if (iNumber >= 100)
		{
			 k = iNumber/100;
			SPR_Set(GetSprite(m_HUD_number_0 + k), r, g, b );
			SPR_DrawAdditive( 0, x, y, &GetSpriteRect(m_HUD_number_0 + k));
			x += iWidth;
		}
		else if (iFlags & (DHN_3DIGITS))
		{
			//SPR_DrawAdditive( 0, x, y, &rc );
			x += iWidth;
		}

		// SPR_Draw 10's
		if (iNumber >= 10)
		{
			k = (iNumber % 100)/10;
			SPR_Set(GetSprite(m_HUD_number_0 + k), r, g, b );
			SPR_DrawAdditive( 0, x, y, &GetSpriteRect(m_HUD_number_0 + k));
			x += iWidth;
		}
		else if (iFlags & (DHN_3DIGITS | DHN_2DIGITS))
		{
			//SPR_DrawAdditive( 0, x, y, &rc );
			x += iWidth;
		}

		// SPR_Draw ones
		k = iNumber % 10;
		SPR_Set(GetSprite(m_HUD_number_0 + k), r, g, b );
		SPR_DrawAdditive(0,  x, y, &GetSpriteRect(m_HUD_number_0 + k));
		x += iWidth;
	} 
	else if (iFlags & DHN_DRAWZERO) 
	{
		SPR_Set(GetSprite(m_HUD_number_0), r, g, b );

		// SPR_Draw 100's
		if (iFlags & (DHN_3DIGITS))
		{
			//SPR_DrawAdditive( 0, x, y, &rc );
			x += iWidth;
		}

		if (iFlags & (DHN_3DIGITS | DHN_2DIGITS))
		{
			//SPR_DrawAdditive( 0, x, y, &rc );
			x += iWidth;
		}

		// SPR_Draw ones
		
		SPR_DrawAdditive( 0,  x, y, &GetSpriteRect(m_HUD_number_0));
		x += iWidth;
	}

	return x;
}


int CHud::GetNumWidth( int iNumber, int iFlags )
{
	if (iFlags & (DHN_3DIGITS))
		return 3;

	if (iFlags & (DHN_2DIGITS))
		return 2;

	if (iNumber <= 0)
	{
		if (iFlags & (DHN_DRAWZERO))
			return 1;
		else
			return 0;
	}

	if (iNumber < 10)
		return 1;

	if (iNumber < 100)
		return 2;

	return 3;

}	

extern int g_iBlurTexture;
extern float g_flBlurTexScale[2];
void CaptureBlurTexture( int width, int height, int iUnused1, int iUnused2 );

// buz
void CHud :: DrawBlurTest( float frametime )
{
	if ( !cv_screenblur->value )
		return;

	bool bDamaged = false;
	if ( m_flDamageTime > gEngfuncs.GetClientTime() )
		bDamaged = true;

	if ( ( cv_blurtest->value || gHUD.m_iBlur || iUnknown1054506C > 2 || bDamaged ) && frametime )
	{
		GLint viewport[4];
		gl.glGetIntegerv( GL_VIEWPORT, viewport );

		int iScreenWide = viewport[2];
		int iScreenTall = viewport[3];

		if ( iScreenWide <= 0 || iScreenTall <= 0 )
			return;

		CaptureBlurTexture( iScreenWide, iScreenTall, 0, 0 );

		if ( !g_iBlurTexture )
			return;

		glDisable( GL_TEXTURE_GEN_S );
		glDisable( GL_TEXTURE_GEN_T );

		float flScaleX = (float)iScreenWide / ScreenWidth;
		float flScaleY = (float)iScreenTall / ScreenHeight;

		int iWidth = XRES( 32 ) * flScaleX;
		int iHeight = YRES( 24 ) * flScaleY;

		int iStepX = XRES( cv_blurstepsx->value ) * flScaleX;
		int iStepY = YRES( cv_blurstepsy->value ) * flScaleY;

		if ( iStepX < 1 )
			iStepX = 1;
		if ( iStepY < 1 )
			iStepY = 1;

		int iSteps = iScreenWide / iStepX + 1;

		float s = g_flBlurTexScale[0];
		float t = g_flBlurTexScale[1];

		for ( int i = 0; i < iSteps; i++ )
		{
			gl.glBindTexture( GL_TEXTURE_2D, g_iBlurTexture );
			gl.glEnable( GL_BLEND );
			gl.glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
			gl.glColor4f( 1, 1, 1, cv_opacity->value );

			iHeight += iStepY;
			iWidth += iStepX;

			glBegin( GL_QUADS );
			glTexCoord2f( 0, t );
			glVertex2f( iScreenWide / 2 - iWidth / 2, iScreenTall / 2 - iHeight / 2 );
			glTexCoord2f( s, t );
			glVertex2f( iScreenWide / 2 + iWidth / 2, iScreenTall / 2 - iHeight / 2 );
			glTexCoord2f( s, 0 );
			glVertex2f( iScreenWide / 2 + iWidth / 2, iScreenTall / 2 + iHeight / 2 );
			glTexCoord2f( 0, 0 );
			glVertex2f( iScreenWide / 2 - iWidth / 2, iScreenTall / 2 + iHeight / 2 );
			glEnd();

			gl.glDisable( GL_BLEND );
		}
	}
}

void CHud :: DrawNewFade( float flTime )
{
	if ( !gl.IsGLAllowed() )
		return;

	if ( m_flFadeStart + m_flFadeTime > gEngfuncs.GetClientTime() || m_bFadeOut || m_flFadeEnd >= gEngfuncs.GetClientTime() )
	{
		float flMaxAlpha = m_iFadeAlpha / 255.0f;
		float flAlpha = ( gEngfuncs.GetClientTime() - m_flFadeStart ) / m_flFadeTime;

		if ( flAlpha > 1 )
			flAlpha = 1;
		if ( flAlpha > flMaxAlpha )
			flAlpha = flMaxAlpha;
		if ( flAlpha < 0 )
			flAlpha = 0;

		float color[3];
		color[0] = m_iFadeR / 255.0f;
		color[1] = m_iFadeG / 255.0f;
		color[2] = m_iFadeB / 255.0f;

		glEnable( GL_BLEND );
		glDisable( GL_TEXTURE_2D );

		if ( !m_bFadeHold )
			glColor4f( color[0], color[1], color[2], flMaxAlpha - flAlpha );
		else
			glColor4f( color[0], color[1], color[2], flAlpha );
		glBegin( GL_QUADS );
		glVertex2f( 0, 0 );
		glVertex2f( ScreenWidth, 0 );
		glVertex2f( ScreenWidth, ScreenHeight );
		glVertex2f( 0, ScreenHeight );
		glEnd();
		glColor4f( 1, 1, 1, 1 );

		glEnable( GL_TEXTURE_2D );
		glDisable( GL_BLEND );
	}
}
