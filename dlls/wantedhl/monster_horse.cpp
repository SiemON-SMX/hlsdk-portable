/***
*
* WantedHL - monster_horse
* Ambient large animal — wanders the map when idle.
*
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "soundent.h"
#include "schedule.h"

class CHorse : public CBaseMonster
{
public:
        void        Spawn( void );
        void        Precache( void );
        void        SetYawSpeed( void );
        int         Classify( void );
        void        HandleAnimEvent( MonsterEvent_t *pEvent );
        int         ISoundMask( void );
};

LINK_ENTITY_TO_CLASS( monster_horse, CHorse );

int CHorse::Classify( void ) { return CLASS_PLAYER_ALLY; }
void CHorse::SetYawSpeed( void ) { pev->yaw_speed = 90; }

int CHorse::ISoundMask( void )
{
        return bits_SOUND_WORLD | bits_SOUND_DANGER;
}

void CHorse::HandleAnimEvent( MonsterEvent_t *pEvent )
{
        switch( pEvent->event )
        {
        default:
                CBaseMonster::HandleAnimEvent( pEvent );
                break;
        }
}

void CHorse::Precache( void )
{
        PRECACHE_MODEL( "models/horse.mdl" );
}

void CHorse::Spawn( void )
{
        Precache();
        SET_MODEL( ENT(pev), "models/horse.mdl" );
        UTIL_SetSize( pev, Vector(-24,-24,0), Vector(24,24,72) );

        pev->solid      = SOLID_SLIDEBOX;
        pev->movetype   = MOVETYPE_STEP;
        m_bloodColor    = BLOOD_COLOR_RED;
        pev->health     = gSkillData.horseHealth;
        pev->view_ofs   = Vector(0,0,60);
        m_flFieldOfView = 0.5f;
        m_MonsterState  = MONSTERSTATE_NONE;
        m_flGroundSpeed = 120.0f;
        m_afCapability  = 0;
        MonsterInit();
}
