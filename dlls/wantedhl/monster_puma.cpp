/***
*
* WantedHL - monster_puma
* Aggressive large cat. Claw and pounce attacks.
*
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "schedule.h"
#include "soundent.h"
#include "skill.h"

#define PUMA_AE_CLAW   1
#define PUMA_AE_POUNCE 2

class CPuma : public CBaseMonster
{
public:
        void        Spawn( void );
        void        Precache( void );
        void        SetYawSpeed( void );
        int         Classify( void );
        BOOL        CheckMeleeAttack1( float flDot, float flDist );
        BOOL        CheckRangeAttack1( float flDot, float flDist );
        void        HandleAnimEvent( MonsterEvent_t *pEvent );
        int         ISoundMask( void );
        void        IdleSound( void );
        void        AlertSound( void );
        void        PainSound( void );
        void        DeathSound( void );
        void        StartTask( Task_t *pTask );
        Schedule_t *GetSchedule( void );
};

LINK_ENTITY_TO_CLASS( monster_puma, CPuma );

int CPuma::Classify( void ) { return CLASS_ALIEN_MONSTER; }
void CPuma::SetYawSpeed( void ) { pev->yaw_speed = 120; }

int CPuma::ISoundMask( void )
{
        return bits_SOUND_WORLD | bits_SOUND_COMBAT | bits_SOUND_PLAYER |
               bits_SOUND_DANGER | bits_SOUND_MEAT | bits_SOUND_CARCASS;
}

void CPuma::IdleSound( void )
{
        EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "puma/puma_idle1.wav", 1.0, ATTN_NORM, 0, 100 );
}
void CPuma::AlertSound( void )
{
        EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "puma/puma_alert1.wav", 1.0, ATTN_NORM, 0, 100 );
}
void CPuma::PainSound( void )
{
        EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "puma/puma_pain1.wav", 1.0, ATTN_NORM, 0, 100 );
}
void CPuma::DeathSound( void )
{
        EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "puma/puma_die1.wav", 1.0, ATTN_NORM, 0, 100 );
}

BOOL CPuma::CheckMeleeAttack1( float flDot, float flDist )
{
        return m_hEnemy && flDist <= 80.0f && flDot >= 0.5f;
}

BOOL CPuma::CheckRangeAttack1( float flDot, float flDist )
{
        return m_hEnemy && flDist > 100.0f && flDist <= 280.0f && flDot >= 0.5f;
}

void CPuma::HandleAnimEvent( MonsterEvent_t *pEvent )
{
        switch( pEvent->event )
        {
        case PUMA_AE_CLAW:
        {
                CBaseEntity *pHurt = CheckTraceHullAttack( 80, gSkillData.pumaDmgClaw, DMG_SLASH );
                if( pHurt )
                        EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "puma/puma_claw.wav", 1.0, ATTN_NORM, 0, 100 );
                break;
        }
        case PUMA_AE_POUNCE:
        {
                if( m_hEnemy )
                {
                        Vector vecDir = ( m_hEnemy->pev->origin - pev->origin ).Normalize();
                        m_hEnemy->TakeDamage( pev, pev, gSkillData.pumaDmgPounce, DMG_SLASH );
                        if( m_hEnemy )
                                m_hEnemy->pev->velocity = m_hEnemy->pev->velocity + vecDir * 300;
                        EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "puma/puma_pounce.wav", 1.0, ATTN_NORM, 0, 100 );
                }
                break;
        }
        default:
                CBaseMonster::HandleAnimEvent( pEvent );
                break;
        }
}

void CPuma::StartTask( Task_t *pTask )
{
        switch( pTask->iTask )
        {
        case TASK_GET_PATH_TO_ENEMY:
        {
                CBaseEntity *pEnemy = m_hEnemy;
                if( !pEnemy ) { TaskFail(); break; }
                if( BuildRoute( pEnemy->pev->origin, bits_MF_TO_ENEMY, pEnemy ) )
                        { TaskComplete(); break; }
                if( BuildNearestRoute( pEnemy->pev->origin, pEnemy->pev->view_ofs, 0,
                        ( pEnemy->pev->origin - pev->origin ).Length() ) )
                        { TaskComplete(); break; }
                TaskComplete();
                break;
        }
        default:
                CBaseMonster::StartTask( pTask );
                break;
        }
}

Schedule_t *CPuma::GetSchedule( void )
{
        switch( m_MonsterState )
        {
        case MONSTERSTATE_COMBAT:
                if( HasConditions( bits_COND_CAN_MELEE_ATTACK1 ) )
                        return GetScheduleOfType( SCHED_MELEE_ATTACK1 );
                if( HasConditions( bits_COND_CAN_RANGE_ATTACK1 ) )
                        return GetScheduleOfType( SCHED_RANGE_ATTACK1 );
                return GetScheduleOfType( SCHED_CHASE_ENEMY );

        default:
                return CBaseMonster::GetSchedule();
        }
}

void CPuma::Precache( void )
{
        PRECACHE_MODEL( "models/puma.mdl" );
        PRECACHE_SOUND( "puma/puma_idle1.wav" );
        PRECACHE_SOUND( "puma/puma_alert1.wav" );
        PRECACHE_SOUND( "puma/puma_pain1.wav" );
        PRECACHE_SOUND( "puma/puma_die1.wav" );
        PRECACHE_SOUND( "puma/puma_claw.wav" );
        PRECACHE_SOUND( "puma/puma_pounce.wav" );
}

void CPuma::Spawn( void )
{
        Precache();
        SET_MODEL( ENT(pev), "models/puma.mdl" );
        UTIL_SetSize( pev, Vector(-24,-24,0), Vector(24,24,48) );

        pev->solid      = SOLID_SLIDEBOX;
        pev->movetype   = MOVETYPE_STEP;
        m_bloodColor    = BLOOD_COLOR_RED;
        pev->health     = gSkillData.pumaHealth;
        pev->view_ofs   = Vector(0,0,40);
        m_flFieldOfView = 0.5f;
        m_MonsterState  = MONSTERSTATE_NONE;
        m_flGroundSpeed = 200;
        m_afCapability  = bits_CAP_HEAR | bits_CAP_MELEE_ATTACK1 | bits_CAP_RANGE_ATTACK1;
        MonsterInit();
}
