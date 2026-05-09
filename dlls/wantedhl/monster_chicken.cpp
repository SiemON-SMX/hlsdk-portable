/***
*
* WantedHL - monster_chicken
* Simple ambient animal NPC — wanders the map randomly.
*
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "soundent.h"
#include "schedule.h"

class CChicken : public CBaseMonster
{
public:
        int         ISoundMask( void );
        void        Spawn( void );
        void        Precache( void );
        void        SetYawSpeed( void );
        int         Classify( void );
        void        HandleAnimEvent( MonsterEvent_t *pEvent );
};

LINK_ENTITY_TO_CLASS( monster_chicken, CChicken );

int CChicken::Classify( void ) { return CLASS_INSECT; }
void CChicken::SetYawSpeed( void ) { pev->yaw_speed = 120; }

int CChicken::ISoundMask( void )
{
        return bits_SOUND_WORLD | bits_SOUND_COMBAT;
}

void CChicken::HandleAnimEvent( MonsterEvent_t *pEvent )
{
        switch( pEvent->event )
        {
        default:
                CBaseMonster::HandleAnimEvent( pEvent );
                break;
        }
}

void CChicken::Precache( void )
{
        PRECACHE_MODEL( "models/chicken.mdl" );
}

void CChicken::Spawn( void )
{
        Precache();
        SET_MODEL( ENT(pev), "models/chicken.mdl" );
        UTIL_SetSize( pev, Vector(-6,-6,0), Vector(6,6,12) );

        pev->solid      = SOLID_SLIDEBOX;
        pev->movetype   = MOVETYPE_STEP;
        m_bloodColor    = BLOOD_COLOR_RED;
        pev->health     = gSkillData.chickenHealth;
        pev->view_ofs   = Vector(0,0,10);
        m_flFieldOfView = 0.5f;
        m_MonsterState  = MONSTERSTATE_NONE;
        m_flGroundSpeed = 80.0f;
        m_afCapability  = 0;
        MonsterInit();
}
