/***
*
* WantedHL - monster_masala
* Masala — friendly shop-keeper NPC, heals the player when nearby.
* CScientist subclass: follows player, flees enemies, heals on approach.
* Silent — no voice lines.
*
* TalkInit fix: calling only CTalkMonster::TalkInit() and then setting
* all m_szGrp[] to NULL prevents the CScientist head-bodygroup animation
* system from firing, which was corrupting the masala.mdl head mesh.
*
****/

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

class CMasala : public CScientist
{
public:
        void Spawn( void );
        void Precache( void );
        void TalkInit( void );
        BOOL CanHeal( void );
        Schedule_t *GetSchedule( void );
        void Heal( void );
};

LINK_ENTITY_TO_CLASS( monster_masala, CMasala );

void CMasala::TalkInit( void )
{
        CTalkMonster::TalkInit();

        // Masala has no voice lines — clear every sentence group so the
        // scientist head-bodygroup animation system never fires, which
        // was causing a corrupted head mesh on masala.mdl.
        for( int i = 0; i < TLK_CGROUPS; i++ )
                m_szGrp[i] = NULL;

        m_voicePitch = 100;
}

void CMasala::Spawn( void )
{
        Precache();

        SET_MODEL( ENT(pev), "models/masala.mdl" );
        UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );

        pev->solid      = SOLID_SLIDEBOX;
        pev->movetype   = MOVETYPE_STEP;
        m_bloodColor    = BLOOD_COLOR_RED;
        pev->health     = gSkillData.masalaHealth;
        pev->view_ofs   = Vector( 0, 0, 50 );
        m_flFieldOfView = VIEW_FIELD_WIDE;
        m_MonsterState  = MONSTERSTATE_NONE;

        m_afCapability  = bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP;

        MonsterInit();
        SetUse( &CMasala::FollowerUse );
}

void CMasala::Precache( void )
{
        PRECACHE_MODEL( "models/masala.mdl" );
        TalkInit();
        CTalkMonster::Precache();
}

BOOL CMasala::CanHeal( void )
{
        if( ( m_healTime > gpGlobals->time ) || ( m_hTargetEnt == NULL ) ||
            ( m_hTargetEnt->pev->health > ( m_hTargetEnt->pev->max_health * 0.5f ) ) )
                return FALSE;
        return TRUE;
}

void CMasala::Heal( void )
{
        if( !CanHeal() )
                return;
        Vector vecTarget = m_hTargetEnt->pev->origin - pev->origin;
        if( vecTarget.Length() > 100 )
                return;
        m_hTargetEnt->TakeHealth( gSkillData.masalaHeal, DMG_GENERIC );
        m_healTime = gpGlobals->time + 60;
}

Schedule_t *CMasala::GetSchedule( void )
{
        if( CanHeal() ) Heal();
        return CTalkMonster::GetSchedule();
}
