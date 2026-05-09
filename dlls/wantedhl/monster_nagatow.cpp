/***
*
* WantedHL - monster_nagatow
* Nagatow Indian NPC. Friendly ally, heals the player.
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

class CNagatow : public CScientist
{
public:
        void Spawn( void );
        void Precache( void );
        void TalkInit( void );
        BOOL CanHeal( void );
Schedule_t *GetSchedule( void );
        void Heal( void );
};

LINK_ENTITY_TO_CLASS( monster_nagatow, CNagatow );

void CNagatow::TalkInit( void )
{
        CTalkMonster::TalkInit();

        // Nagatow has no voice lines — clear all sentence groups so the
        // CScientist head-bodygroup animation system never fires, which
        // would corrupt the nagatow.mdl head mesh (same fix as masala/dave).
        for( int i = 0; i < TLK_CGROUPS; i++ )
                m_szGrp[i] = NULL;

        m_voicePitch = 100;
}

void CNagatow::Spawn( void )
{
        Precache();

        SET_MODEL( ENT(pev), "models/nagatow.mdl" );
        UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );

        pev->solid      = SOLID_SLIDEBOX;
        pev->movetype   = MOVETYPE_STEP;
        m_bloodColor    = BLOOD_COLOR_RED;
        pev->health     = gSkillData.nagatowHealth;
        pev->view_ofs   = Vector( 0, 0, 50 );
        m_flFieldOfView = VIEW_FIELD_WIDE;
        m_MonsterState  = MONSTERSTATE_NONE;

        m_afCapability = bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP;

        MonsterInit();
        SetUse( &CNagatow::FollowerUse );
}

void CNagatow::Precache( void )
{
        PRECACHE_MODEL( "models/nagatow.mdl" );
        TalkInit();
        CTalkMonster::Precache();
}

BOOL CNagatow::CanHeal( void )
{
        if( ( m_healTime > gpGlobals->time ) || ( m_hTargetEnt == NULL ) ||
            ( m_hTargetEnt->pev->health > ( m_hTargetEnt->pev->max_health * 0.5f ) ) )
                return FALSE;
        return TRUE;
}

void CNagatow::Heal( void )
{
        if( !CanHeal() )
                return;
        Vector vecTarget = m_hTargetEnt->pev->origin - pev->origin;
        if( vecTarget.Length() > 100 )
                return;
        m_hTargetEnt->TakeHealth( gSkillData.nagatowHeal, DMG_GENERIC );
        m_healTime = gpGlobals->time + 60;
}

Schedule_t *CNagatow::GetSchedule( void )
{
        if( CanHeal() ) Heal();
        return CTalkMonster::GetSchedule();
}
