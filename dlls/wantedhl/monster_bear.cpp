/***
*
* WantedHL - monster_bear
* Large aggressive melee predator. Double claw and pounce attacks.
*
***/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "schedule.h"
#include "soundent.h"
#include "skill.h"

#define BEAR_AE_CLAW   1
#define BEAR_AE_CLAW2  2
#define BEAR_AE_POUNCE 3

class CBear : public CBaseMonster
{
public:
        void        Spawn( void );
        void        Precache( void );
        void        SetYawSpeed( void );
        int         Classify( void );
        BOOL        CheckMeleeAttack1( float flDot, float flDist );
        BOOL        CheckMeleeAttack2( float flDot, float flDist );
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

LINK_ENTITY_TO_CLASS( monster_bear, CBear );

int CBear::Classify( void ) { return CLASS_ALIEN_MONSTER; }
void CBear::SetYawSpeed( void ) { pev->yaw_speed = 90; }

int CBear::ISoundMask( void )
{
        return bits_SOUND_WORLD | bits_SOUND_COMBAT | bits_SOUND_PLAYER |
               bits_SOUND_DANGER | bits_SOUND_MEAT | bits_SOUND_CARCASS;
}

void CBear::IdleSound( void )
{
        EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "bear/bear_idle1.wav", 1.0, ATTN_NORM, 0, 100 );
}
void CBear::AlertSound( void )
{
        EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "bear/bear_alert1.wav", 1.0, ATTN_NORM, 0, 100 );
}
void CBear::PainSound( void )
{
        EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "bear/bear_pain1.wav", 1.0, ATTN_NORM, 0, 100 );
}
void CBear::DeathSound( void )
{
        EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "bear/bear_die1.wav", 1.0, ATTN_NORM, 0, 100 );
}

BOOL CBear::CheckMeleeAttack1( float flDot, float flDist )
{
        return m_hEnemy && flDist <= 80.0f && flDot >= 0.5f;
}
BOOL CBear::CheckMeleeAttack2( float flDot, float flDist )
{
        return m_hEnemy && flDist <= 80.0f && flDot >= 0.5f;
}
BOOL CBear::CheckRangeAttack1( float flDot, float flDist )
{
        return m_hEnemy &&
               flDist > 100.0f && flDist <= 300.0f &&
               flDot >= 0.5f &&
               FVisible( m_hEnemy );
}

void CBear::HandleAnimEvent( MonsterEvent_t *pEvent )
{
        switch( pEvent->event )
        {
        case BEAR_AE_CLAW:
        {
                CBaseEntity *pHurt = CheckTraceHullAttack( 80, gSkillData.bearDmgClaw, DMG_SLASH );
                if( pHurt )
                {
                        pHurt->pev->punchangle.x = 5;
                        EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "bear/bear_claw.wav", 1.0, ATTN_NORM, 0, 100 );
                }
                break;
        }
        case BEAR_AE_CLAW2:
        {
                CBaseEntity *pHurt = CheckTraceHullAttack( 80, gSkillData.bearDmgClaw, DMG_SLASH );
                if( pHurt )
                {
                        pHurt->pev->punchangle.x = 5;
                        EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "bear/bear_claw.wav", 1.0, ATTN_NORM, 0, 80 );
                }
                break;
        }
        case BEAR_AE_POUNCE:
        {
                CBaseEntity *pHurt = CheckTraceHullAttack( 100, gSkillData.bearDmgPounce, DMG_SLASH );
                if( pHurt )
                {
                        pHurt->pev->punchangle.x = 12;
                        pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_forward * 220;
                        EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "bear/bear_claw.wav", 1.0, ATTN_NORM, 0, 120 );
                }
                break;
        }
        default:
                CBaseMonster::HandleAnimEvent( pEvent );
                break;
        }
}

void CBear::StartTask( Task_t *pTask )
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

Schedule_t *CBear::GetSchedule( void )
{
        switch( m_MonsterState )
        {
        case MONSTERSTATE_COMBAT:
                if( HasConditions( bits_COND_CAN_MELEE_ATTACK1 ) )
                        return GetScheduleOfType( SCHED_MELEE_ATTACK1 );
                if( HasConditions( bits_COND_CAN_MELEE_ATTACK2 ) )
                        return GetScheduleOfType( SCHED_MELEE_ATTACK2 );
                if( HasConditions( bits_COND_CAN_RANGE_ATTACK1 ) )
                        return GetScheduleOfType( SCHED_RANGE_ATTACK1 );
                return GetScheduleOfType( SCHED_CHASE_ENEMY );

        default:
                return CBaseMonster::GetSchedule();
        }
}

void CBear::Precache( void )
{
        PRECACHE_MODEL( "models/bear.mdl" );
        PRECACHE_SOUND( "bear/bear_idle1.wav" );
        PRECACHE_SOUND( "bear/bear_alert1.wav" );
        PRECACHE_SOUND( "bear/bear_pain1.wav" );
        PRECACHE_SOUND( "bear/bear_die1.wav" );
        PRECACHE_SOUND( "bear/bear_claw.wav" );
}

void CBear::Spawn( void )
{
        Precache();
        SET_MODEL( ENT(pev), "models/bear.mdl" );
        UTIL_SetSize( pev, Vector(-32,-32,0), Vector(32,32,64) );

        pev->solid      = SOLID_SLIDEBOX;
        pev->movetype   = MOVETYPE_STEP;
        m_bloodColor    = BLOOD_COLOR_RED;
        pev->health     = gSkillData.bearHealth;
        pev->view_ofs   = Vector(0,0,48);
        m_flFieldOfView = 0.5f;
        m_MonsterState  = MONSTERSTATE_NONE;
        m_flGroundSpeed = 150;
        m_afCapability  = bits_CAP_HEAR |
                          bits_CAP_MELEE_ATTACK1 |
                          bits_CAP_MELEE_ATTACK2 |
                          bits_CAP_RANGE_ATTACK1;
        MonsterInit();
}
