#include <stdint.h>
#include "hud.h"
#include "usercmd.h"
#include "cvardef.h"
#include "kbutton.h"
#include "keydefs.h"
#include "input_mouse.h"
#include "view.h"
extern cvar_t		*sensitivity;
extern cvar_t		*in_joystick;

extern kbutton_t	in_strafe;
extern kbutton_t	in_mlook;
extern kbutton_t	in_jlook;
extern kbutton_t	in_forward;
extern kbutton_t	in_back;
extern kbutton_t	in_moveleft;
extern kbutton_t	in_moveright;

extern cvar_t	*m_pitch;
extern cvar_t	*m_yaw;
extern cvar_t	*m_forward;
extern cvar_t	*m_side;
extern cvar_t	*lookstrafe;
extern cvar_t	*lookspring;
extern cvar_t	*cl_pitchdown;
extern cvar_t	*cl_pitchup;
extern cvar_t	*cl_yawspeed;
extern cvar_t	*cl_sidespeed;
extern cvar_t	*cl_forwardspeed;
extern cvar_t	*cl_backspeed;
extern cvar_t	*cl_pitchspeed;
cvar_t	*cl_laddermode;


#define F 1U<<0	// Forward
#define B 1U<<1	// Back
#define L 1U<<2	// Left
#define R 1U<<3	// Right
#define T 1U<<4	// Forward stop
#define S 1U<<5	// Side stop

#define BUTTON_DOWN		1
#define IMPULSE_DOWN	2
#define IMPULSE_UP		4

int CL_IsDead( void );
extern Vector dead_viewangles;

extern int iPlayerMode;
extern int iModelAttachEnt;
extern int iModelAttachNum;
extern int iDefaultFOV;
extern cl_entity_t *pTelescopeEnt;
extern vec3_t vecWheelchairAngles;
extern intptr_t iWheelchair;
extern bool bWheelchairLock;
extern float flMouseOfs2X;
extern float flMouseOfs2Y;

/*
===========
IN_GetMouseSensitivity
Get mouse sensitivity with sanitization
===========
*/
float IN_GetMouseSensitivity()
{
	// Absurdly high sensitivity values can cause the game to hang, so clamp
	if( sensitivity->value > 10000.0 )
	{
		gEngfuncs.Cvar_SetValue( "sensitivity", 10000.0 );
	}
	else if( sensitivity->value < 0.01 )
	{
		gEngfuncs.Cvar_SetValue( "sensitivity", 0.01 );
	}
	return sensitivity->value;
}

extern cl_entity_t *pAttachCamEnt;

static bool s_bTouchForward, s_bTouchBack;
static bool s_bSentForward, s_bSentBack;

// touch movement never goes through IN_ForwardDown, so the ladder and other
// camera scenes never got forwardpressed / backpressed from the joystick
static void IN_SyncCameraMove( bool down, bool *sent, const char *pressed, const char *released )
{
	bool want = down && pAttachCamEnt;

	if( want == *sent )
		return;

	*sent = want;

	if( pAttachCamEnt )
		gEngfuncs.pfnServerCmd( want ? pressed : released );
}

void IN_ToggleButtons( float forwardmove, float sidemove )
{
	static unsigned int moveflags = T | S;

	if( forwardmove )
		moveflags &= ~T;
	else
	{
		//if( in_forward.state || in_back.state ) gEngfuncs.Con_Printf("Buttons pressed f%d b%d\n", in_forward.state, in_back.state);
		if( !( moveflags & T ) )
		{
			//IN_ForwardUp();
			//IN_BackUp();
			//gEngfuncs.Con_Printf("Reset forwardmove state f%d b%d\n", in_forward.state, in_back.state);
			in_forward.state &= ~BUTTON_DOWN;
			in_back.state &= ~BUTTON_DOWN;
			moveflags |= T;
		}
	}
	if( sidemove )
		moveflags &= ~S;
	else
	{
		//gEngfuncs.Con_Printf("l%d r%d\n", in_moveleft.state, in_moveright.state);
		//if( in_moveleft.state || in_moveright.state ) gEngfuncs.Con_Printf("Buttons pressed l%d r%d\n", in_moveleft.state, in_moveright.state);
		if( !( moveflags & S ) )
		{
			//IN_MoverightUp();
			//IN_MoveleftUp();
			//gEngfuncs.Con_Printf("Reset sidemove state f%d b%d\n", in_moveleft.state, in_moveright.state);
			in_moveleft.state &= ~BUTTON_DOWN;
			in_moveright.state &= ~BUTTON_DOWN;
			moveflags |= S;
		}
	}

	if( forwardmove > 0.7f && !( moveflags & F ) )
	{
		moveflags |= F;
		in_forward.state |= BUTTON_DOWN;
	}
	if( forwardmove < 0.7f && ( moveflags & F ) )
	{
		moveflags &= ~F;
		in_forward.state &= ~BUTTON_DOWN;
	}
	if( forwardmove < -0.7f && !( moveflags & B ) )
	{
		moveflags |= B;
		in_back.state |= BUTTON_DOWN;
	}
	if( forwardmove > -0.7f && ( moveflags & B ) )
	{
		moveflags &= ~B;
		in_back.state &= ~BUTTON_DOWN;
	}
	if( sidemove > 0.9f && !( moveflags & R ) )
	{
		moveflags |= R;
		in_moveright.state |= BUTTON_DOWN;
	}
	if( sidemove < 0.9f && ( moveflags & R ) )
	{
		moveflags &= ~R;
		in_moveright.state &= ~BUTTON_DOWN;
	}
	if( sidemove < -0.9f && !( moveflags & L ) )
	{
		moveflags |= L;
		in_moveleft.state |= BUTTON_DOWN;
	}
	if( sidemove > -0.9f && ( moveflags & L ) )
	{
		moveflags &= ~L;
		in_moveleft.state &= ~BUTTON_DOWN;
	}

	s_bTouchForward = ( moveflags & F ) != 0;
	s_bTouchBack = ( moveflags & B ) != 0;
}

void FWGSInput::IN_ClientMoveEvent( float forwardmove, float sidemove )
{
	//gEngfuncs.Con_Printf("IN_MoveEvent\n");

	ac_forwardmove += forwardmove;
	ac_sidemove += sidemove;
	ac_movecount++;
}

void FWGSInput::IN_ClientLookEvent( float relyaw, float relpitch )
{
	rel_yaw += relyaw;
	rel_pitch += relpitch;
}

// Rotate camera and add move values to usercmd
void FWGSInput::IN_Move( float frametime, usercmd_t *cmd )
{
	Vector viewangles;
	bool fLadder = false;

	if( gHUD.m_iIntermission )
		return; // we can't move during intermission

	if( cl_laddermode->value != 2 )
	{
		cl_entity_t *pplayer = gEngfuncs.GetLocalPlayer();
		if( pplayer )
			fLadder = pplayer->curstate.movetype == MOVETYPE_FLY;
	}
	//if(ac_forwardmove || ac_sidemove)
	//gEngfuncs.Con_Printf("Move: %f %f %f %f\n", ac_forwardmove, ac_sidemove, rel_pitch, rel_yaw);
	V_StopPitchDrift();
	if( CL_IsDead() )
	{
		viewangles = dead_viewangles; // HACKHACK: see below
	}
	else
	{
		gEngfuncs.GetViewAngles( viewangles );
	}
	float mouse_sensitivity = gHUD.GetSensitivity() != 0 ? gHUD.GetSensitivity() : IN_GetMouseSensitivity();
	rel_yaw *= mouse_sensitivity;
	rel_pitch *= mouse_sensitivity;

	bool bLookBlocked = ( iModelAttachEnt != -1 && iModelAttachNum != -1 ) || ( iPlayerMode > 0 && iPlayerMode <= 4 );

	if( bLookBlocked )
	{
		rel_yaw = 0;
		rel_pitch = 0;
	}

	if( gHUD.m_iFOV < iDefaultFOV )
	{
		rel_yaw *= 0.5f;
		rel_pitch *= 0.5f;
	}

	if( CL_IsDead() )
	{
		rel_yaw = 0;
		rel_pitch = 0;
	}

	float cof_look_yaw = rel_yaw;
	float cof_look_pitch = rel_pitch;

	viewangles[YAW] += rel_yaw;
	if( fLadder )
	{
		if( cl_laddermode->value == 1 )
			viewangles[YAW] -= ac_sidemove * 5;
		ac_sidemove = 0;
	}
	viewangles[PITCH] += rel_pitch;

	if( viewangles[PITCH] > cl_pitchdown->value )
		viewangles[PITCH] = cl_pitchdown->value;
	if( viewangles[PITCH] < -cl_pitchup->value )
		viewangles[PITCH] = -cl_pitchup->value;
	
	// HACKHACK: change viewangles directly in viewcode, 
	// so viewangles when player is dead will not be changed on server
	if( !CL_IsDead() && !bLookBlocked && !gHUD.m_bViewLock && !pTelescopeEnt )
	{
		gEngfuncs.SetViewAngles( viewangles );
	}

	if( pTelescopeEnt && !bLookBlocked )
	{
		vec3_t angles;

		flMouseOfs2X += cof_look_pitch;
		flMouseOfs2Y += -cof_look_yaw;

		if( flMouseOfs2X > 30 )
			flMouseOfs2X = 30;
		else if( flMouseOfs2X < -60 )
			flMouseOfs2X = -60;

		if( flMouseOfs2Y > 60 )
			flMouseOfs2Y = 60;
		if( flMouseOfs2Y < -60 )
			flMouseOfs2Y = -60;

		angles[0] = flMouseOfs2X;
		angles[1] = flMouseOfs2Y * -1;
		angles[2] = 0;
		gEngfuncs.SetViewAngles( (float *)angles );
	}

	if( iWheelchair && !bWheelchairLock && !bLookBlocked )
	{
		vecWheelchairAngles[0] += cof_look_pitch;
		vecWheelchairAngles[1] += -cof_look_yaw;

		if( vecWheelchairAngles[0] > 30 )
			vecWheelchairAngles[0] = 30;
		else if( vecWheelchairAngles[0] < -60 )
			vecWheelchairAngles[0] = -60;

		if( vecWheelchairAngles[1] > 60 )
			vecWheelchairAngles[1] = 60;
		else if( vecWheelchairAngles[1] < -60 )
			vecWheelchairAngles[1] = -60;
	}

	dead_viewangles = viewangles; // keep them actual
	if( ac_movecount )
	{
		// pm halves forwardmove while strafing, so drop the small axis like a keyboard would
		if( fabs( ac_sidemove ) < fabs( ac_forwardmove ) * 0.4f )
			ac_sidemove = 0;
		else if( fabs( ac_forwardmove ) < fabs( ac_sidemove ) * 0.4f )
			ac_forwardmove = 0;

		IN_ToggleButtons( ac_forwardmove / ac_movecount, ac_sidemove / ac_movecount );

		if( ac_forwardmove )
			cmd->forwardmove = ac_forwardmove * ( ac_forwardmove < 0 ? cl_backspeed->value : cl_forwardspeed->value ) / ac_movecount;
		if( ac_sidemove )
			cmd->sidemove  = ac_sidemove * cl_sidespeed->value / ac_movecount;
	}

	IN_SyncCameraMove( s_bTouchForward, &s_bSentForward, "forwardpressed", "forwardreleased" );
	IN_SyncCameraMove( s_bTouchBack, &s_bSentBack, "backpressed", "backreleased" );

	ac_sidemove = ac_forwardmove = rel_pitch = rel_yaw = 0;
	ac_movecount = 0;
}

void FWGSInput::IN_MouseEvent( int mstate )
{
	static int mouse_oldbuttonstate;
	// perform button actions
	for( int i = 0; i < 5; i++ )
	{
		if( ( mstate & ( 1 << i ) ) && !( mouse_oldbuttonstate & ( 1 << i ) ) )
		{
			gEngfuncs.Key_Event( K_MOUSE1 + i, 1 );
		}

		if( !( mstate & ( 1 << i ) ) && ( mouse_oldbuttonstate & ( 1 << i ) ) )
		{
			gEngfuncs.Key_Event( K_MOUSE1 + i, 0 );
		}
	}	

	mouse_oldbuttonstate = mstate;
}

// Stubs

void FWGSInput::IN_ClearStates( void )
{
	//gEngfuncs.Con_Printf( "IN_ClearStates\n" );
}

void FWGSInput::IN_ActivateMouse( void )
{
	//gEngfuncs.Con_Printf( "IN_ActivateMouse\n" );
}

void FWGSInput::IN_DeactivateMouse( void )
{
	//gEngfuncs.Con_Printf( "IN_DeactivateMouse\n" );
}

void FWGSInput::IN_Accumulate( void )
{
	//gEngfuncs.Con_Printf( "IN_Accumulate\n" );
}

void FWGSInput::IN_Commands( void )
{
	//gEngfuncs.Con_Printf( "IN_Commands\n" );
}

void FWGSInput::IN_Shutdown( void )
{
}

// Register cvars and reset data
void FWGSInput::IN_Init( void )
{
	sensitivity = gEngfuncs.pfnRegisterVariable( "sensitivity", "3", FCVAR_ARCHIVE | FCVAR_FILTERSTUFFTEXT );
	in_joystick = gEngfuncs.pfnRegisterVariable( "joystick", "0", FCVAR_ARCHIVE );
	cl_laddermode = gEngfuncs.pfnRegisterVariable( "cl_laddermode", "2", FCVAR_ARCHIVE );
	ac_forwardmove = ac_sidemove = rel_yaw = rel_pitch = 0;
}
