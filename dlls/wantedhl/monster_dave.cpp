/***
*
* WantedHL - monster_dave
* Dynamite Dave — friendly ally NPC. Heals the player when nearby.
*
***/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "talkmonster.h"
#include "schedule.h"
#include "defaultai.h"
#include "scripted.h"
#include "animation.h"
#include "soundent.h"
#include "scientist.h"
#include "skill.h"

class CDave : public CScientist
{
public:
        void Spawn( void );
        void Precache( void );
        void TalkInit( void );
        BOOL CanHeal( void );
        Schedule_t *GetSchedule( void );
        void Heal( void );
};

LINK_ENTITY_TO_CLASS( monster_dave, CDave );

void CDave::TalkInit( void )
{
        CTalkMonster::TalkInit();

        // Dave has no voice lines — clear all sentence groups so the
        // scientist head-bodygroup system never changes dave.mdl's head.
        for( int i = 0; i < TLK_CGROUPS; i++ )
                m_szGrp[i] = NULL;

        m_voicePitch = 100;
}

void CDave::Spawn( void )
{
        Precache();

        SET_MODEL( ENT(pev), "models/dyndave.mdl" );
        UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );

        pev->solid      = SOLID_SLIDEBOX;
        pev->movetype   = MOVETYPE_STEP;
        m_bloodColor    = BLOOD_COLOR_RED;
        pev->health     = gSkillData.daveHealth;
        pev->view_ofs   = Vector( 0, 0, 50 );
        m_flFieldOfView = VIEW_FIELD_WIDE;
        m_MonsterState  = MONSTERSTATE_NONE;

        m_afCapability  = bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP;

        MonsterInit();
        SetUse( &CDave::FollowerUse );
}

void CDave::Precache( void )
{
        PRECACHE_MODEL( "models/dyndave.mdl" );
        TalkInit();
        CTalkMonster::Precache();
}

BOOL CDave::CanHeal( void )
{
        if( ( m_healTime > gpGlobals->time ) || ( m_hTargetEnt == NULL ) ||
            ( m_hTargetEnt->pev->health > ( m_hTargetEnt->pev->max_health * 0.5f ) ) )
                return FALSE;
        return TRUE;
}

void CDave::Heal( void )
{
        if( !CanHeal() )
                return;
        Vector vecTarget = m_hTargetEnt->pev->origin - pev->origin;
        if( vecTarget.Length() > 100 )
                return;
        m_hTargetEnt->TakeHealth( gSkillData.daveHeal, DMG_GENERIC );
        m_healTime = gpGlobals->time + 60;
}

class CDeadDave : public CDeadScientist
{
public:
        void Spawn( void );
};

LINK_ENTITY_TO_CLASS( monster_dave_dead, CDeadDave );

void CDeadDave::Spawn( void )
{
        PRECACHE_MODEL( "models/dyndave.mdl" );
        SET_MODEL( ENT(pev), "models/dyndave.mdl" );
        pev->effects  = 0;
        pev->health   = 8;
        m_bloodColor  = BLOOD_COLOR_RED;
        pev->sequence = LookupSequence( "lying_on_stomach" );
        if( pev->sequence == -1 )
                pev->sequence = 0;
        MonsterInitDead();
}

Schedule_t *CDave::GetSchedule( void )
{
        if( CanHeal() ) Heal();
        return CTalkMonster::GetSchedule();
}
