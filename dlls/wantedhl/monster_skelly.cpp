/***
*
* WantedHL - monster_skelly / monster_skellydance
* Skeleton NPCs used in WantedHL maps.
*
***/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "soundent.h"
#include "schedule.h"

class CSkelly : public CBaseMonster
{
public:
        int  ISoundMask( void );
        void Spawn( void );
        void Precache( void );
        int  Classify( void );
};

int CSkelly::ISoundMask( void )
{
        return bits_SOUND_WORLD | bits_SOUND_COMBAT | bits_SOUND_PLAYER | bits_SOUND_DANGER;
}

LINK_ENTITY_TO_CLASS( monster_skelly, CSkelly );

int CSkelly::Classify( void ) { return CLASS_NONE; }

void CSkelly::Precache( void )
{
        PRECACHE_MODEL( "models/skeleton.mdl" );
}

void CSkelly::Spawn( void )
{
        Precache();
        SET_MODEL( ENT(pev), "models/skeleton.mdl" );
        UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );

        pev->solid      = SOLID_NOT;
        pev->movetype   = MOVETYPE_NONE;
        m_bloodColor    = DONT_BLEED;
        pev->health     = 1;
        pev->takedamage = DAMAGE_NO;
        m_MonsterState  = MONSTERSTATE_NONE;

        m_afCapability = 0;
        MonsterInit();
}

class CSkellyDance : public CSkelly
{
public:
        void Spawn( void );
};

LINK_ENTITY_TO_CLASS( monster_skellydance, CSkellyDance );

void CSkellyDance::Spawn( void )
{
        CSkelly::Spawn();
        // Dance sequence played by scripted_sequence entities in the map
}

class CTiedColonel : public CBaseMonster
{
public:
        void Spawn( void );
        void Precache( void );
        int  Classify( void );
        void PainSound( void ) {}
        void DeathSound( void ) {}
        Schedule_t *GetSchedule( void )
        {
                return GetScheduleOfType( SCHED_IDLE_STAND );
        }
};

LINK_ENTITY_TO_CLASS( monster_tied_colonel, CTiedColonel );

int CTiedColonel::Classify( void ) { return CLASS_PLAYER_ALLY; }

void CTiedColonel::Precache( void )
{
        PRECACHE_MODEL( "models/colonel.mdl" );
}

void CTiedColonel::Spawn( void )
{
        Precache();
        SET_MODEL( ENT(pev), "models/colonel.mdl" );
        UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );
        pev->solid      = SOLID_SLIDEBOX;
        pev->movetype   = MOVETYPE_NONE;
        m_bloodColor    = BLOOD_COLOR_RED;
        pev->health     = 100;
        pev->view_ofs   = Vector(0,0,50);
        m_flFieldOfView = 0.5f;
        m_MonsterState  = MONSTERSTATE_NONE;
        m_afCapability  = 0;
        MonsterInit();
}

class CEagleFlock : public CBaseEntity
{
public:
        void Spawn( void );
        int  ObjectCaps( void ) { return FCAP_DONT_SAVE; }
};

LINK_ENTITY_TO_CLASS( monster_eagle_flock, CEagleFlock );

void CEagleFlock::Spawn( void )
{
        pev->solid    = SOLID_NOT;
        pev->movetype = MOVETYPE_NONE;
        pev->effects |= EF_NODRAW;
}
