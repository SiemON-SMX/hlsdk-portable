/***
*
*       Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*       
*       This product contains software technology licensed from Id 
*       Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*       All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/
//=========================================================
// Generic Monster - purely for scripted sequence work.
//=========================================================
#include        "extdll.h"
#include        "util.h"
#include        "cbase.h"
#include        "monsters.h"
#include        "schedule.h"
#include        "animation.h"

// For holograms, make them not solid so the player can walk through them
#define SF_GENERICMONSTER_NOTSOLID                                      4 

//=========================================================
// Monster's Anim Events Go Here
//=========================================================

class CGenericMonster : public CBaseMonster
{
public:
        void Spawn( void );
        void Precache( void );
        void SetYawSpeed( void );
        int  Classify( void );
        void HandleAnimEvent( MonsterEvent_t *pEvent );
        int  ISoundMask( void );
        // Graceful fallback: if the model lacks the requested activity
        // (e.g. rogan_rider.mdl has no ACT_IDLE) use sequence 0 instead
        // of printing an error and leaving the entity in a broken state.
        void SetActivity( Activity NewActivity );
};

LINK_ENTITY_TO_CLASS( monster_generic, CGenericMonster )

//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
int CGenericMonster::Classify( void )
{
        return CLASS_PLAYER_ALLY;
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CGenericMonster::SetYawSpeed( void )
{
        int ys;

        switch( m_Activity )
        {
        case ACT_IDLE:
        default:
                ys = 90;
        }

        pev->yaw_speed = ys;
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CGenericMonster::HandleAnimEvent( MonsterEvent_t *pEvent )
{
        switch( pEvent->event )
        {
        case 0:
        default:
                CBaseMonster::HandleAnimEvent( pEvent );
                break;
        }
}

//=========================================================
// ISoundMask - generic monster can't hear.
//=========================================================
int CGenericMonster::ISoundMask( void )
{
        return 0;
}

//=========================================================
// Spawn
//=========================================================
void CGenericMonster::Spawn()
{
        Precache();

        SET_MODEL( ENT( pev ), STRING( pev->model ) );

        if( FStrEq( STRING( pev->model ), "models/player.mdl" ) || FStrEq( STRING( pev->model ), "models/holo.mdl" ) )
                UTIL_SetSize( pev, VEC_HULL_MIN, VEC_HULL_MAX );
        else
                UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );

        pev->solid = SOLID_SLIDEBOX;
        pev->movetype = MOVETYPE_STEP;
        m_bloodColor = BLOOD_COLOR_RED;
        pev->health = 8;
        m_flFieldOfView = 0.5;
        m_MonsterState = MONSTERSTATE_NONE;

        MonsterInit();

        // Explicit not-solid spawnflag (SF_GENERICMONSTER_NOTSOLID = 4).
        if( pev->spawnflags & SF_GENERICMONSTER_NOTSOLID )
        {
                pev->solid = SOLID_NOT;
                pev->takedamage = DAMAGE_NO;
        }
        // Auto-detect invisible props: rendermode kRenderTransAdd (4) with a
        // black rendercolor (0,0,0) results in a fully invisible entity.
        // Maps like wantintro use this trick for rider/attachment props that
        // sit on top of other entities.  Without SOLID_NOT the entity spawns
        // as a solid box in mid-air and "stuck in wall" errors follow.
        else if( pev->rendermode == 4 &&
                 pev->rendercolor.x == 0 &&
                 pev->rendercolor.y == 0 &&
                 pev->rendercolor.z == 0 )
        {
                pev->solid      = SOLID_NOT;
                pev->takedamage = DAMAGE_NO;
        }
}

//=========================================================
// SetActivity — graceful fallback for models that lack
// standard HL activities (e.g. custom rider/prop models).
// Instead of printing "no sequence for act:N" and leaving
// the monster in a broken state, silently use sequence 0.
//=========================================================
void CGenericMonster::SetActivity( Activity NewActivity )
{
        int iSequence = LookupActivity( NewActivity );
        if( iSequence == ACTIVITY_NOT_AVAILABLE )
        {
                // Model doesn't have this activity — fall back to sequence 0
                // (usually a neutral/bind pose) so the entity stays visible
                // and functional without flooding the console with errors.
                pev->sequence  = 0;
                pev->frame     = 0;
                ResetSequenceInfo();
                m_Activity     = NewActivity;
                m_IdealActivity = NewActivity;
                return;
        }
        CBaseMonster::SetActivity( NewActivity );
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CGenericMonster::Precache()
{
        PRECACHE_MODEL( STRING( pev->model ) );
}

//=========================================================
// AI Schedules Specific to this monster
//=========================================================
