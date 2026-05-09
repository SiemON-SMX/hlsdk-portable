/***
*
* WantedHL - monster_snake
* Ground serpent. Bites the player when close. Poison damage.
*
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "soundent.h"
#include "schedule.h"
#include "skill.h"

#define SNAKE_AE_BITE 1

class CSnake : public CBaseMonster
{
public:
        int         ISoundMask( void );
        void        Spawn( void );
        void        Precache( void );
        void        SetYawSpeed( void );
        int         Classify( void );
        BOOL        CheckMeleeAttack1( float flDot, float flDist );
        void        HandleAnimEvent( MonsterEvent_t *pEvent );
        void        StartTask( Task_t *pTask );
        Schedule_t *GetSchedule( void );
};

LINK_ENTITY_TO_CLASS( monster_snake, CSnake );

int CSnake::Classify( void ) { return CLASS_ALIEN_MONSTER; }
void CSnake::SetYawSpeed( void ) { pev->yaw_speed = 90; }

int CSnake::ISoundMask( void )
{
        return bits_SOUND_WORLD | bits_SOUND_COMBAT | bits_SOUND_PLAYER | bits_SOUND_DANGER;
}

BOOL CSnake::CheckMeleeAttack1( float flDot, float flDist )
{
        return m_hEnemy && flDist <= 70.0f && flDot >= 0.5f;
}

void CSnake::HandleAnimEvent( MonsterEvent_t *pEvent )
{
        switch( pEvent->event )
        {
        case SNAKE_AE_BITE:
        {
                if( m_hEnemy )
                {
                        float flDist = ( m_hEnemy->pev->origin - pev->origin ).Length2D();
                        if( flDist <= 70.0f )
                        {
                                m_hEnemy->TakeDamage( pev, pev, gSkillData.snakeDmgBite, DMG_POISON );
                                EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "snake/snake_bite.wav", 1.0, ATTN_NORM, 0, 100 );
                        }
                }
                break;
        }
        default:
                CBaseMonster::HandleAnimEvent( pEvent );
                break;
        }
}

void CSnake::StartTask( Task_t *pTask )
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

Schedule_t *CSnake::GetSchedule( void )
{
        switch( m_MonsterState )
        {
        case MONSTERSTATE_COMBAT:
                if( HasConditions( bits_COND_CAN_MELEE_ATTACK1 ) )
                        return GetScheduleOfType( SCHED_MELEE_ATTACK1 );
                return GetScheduleOfType( SCHED_CHASE_ENEMY );

        default:
                return CBaseMonster::GetSchedule();
        }
}

void CSnake::Precache( void )
{
        PRECACHE_MODEL( "models/snake.mdl" );
        PRECACHE_SOUND( "snake/snake_bite.wav" );
        PRECACHE_SOUND( "snake/snake_idle.wav" );
}

void CSnake::Spawn( void )
{
        Precache();
        SET_MODEL( ENT(pev), "models/snake.mdl" );
        UTIL_SetSize( pev, Vector(-4,-4,0), Vector(4,4,6) );

        pev->solid      = SOLID_SLIDEBOX;
        pev->movetype   = MOVETYPE_STEP;
        m_bloodColor    = BLOOD_COLOR_RED;
        pev->health     = gSkillData.snakeHealth;
        pev->view_ofs   = Vector(0,0,4);
        m_flFieldOfView = 0.5f;
        m_MonsterState  = MONSTERSTATE_NONE;
        m_flGroundSpeed = 80;
        m_afCapability  = bits_CAP_HEAR | bits_CAP_MELEE_ATTACK1;
        MonsterInit();
}
