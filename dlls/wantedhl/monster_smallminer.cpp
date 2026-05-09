/***
*
* WantedHL - monster_smallminer
* Small pickaxe-wielding miner enemy.
*
***/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "schedule.h"
#include "skill.h"

#define SMALLMINER_AE_KICK   3   // frontkick ev3  — close kick
#define SMALLMINER_AE_SWING1 7   // throwgrenade ev7 — medium swing
#define SMALLMINER_AE_SWING2 8   // launchgrenade ev8 — medium swing

class CSmallMiner : public CBaseMonster
{
public:
        void  Spawn( void );
        void  Precache( void );
        void  SetYawSpeed( void );
        int   Classify( void );
        BOOL  CheckMeleeAttack1( float flDot, float flDist );
        BOOL  CheckRangeAttack2( float flDot, float flDist );
        void  HandleAnimEvent( MonsterEvent_t *pEvent );
        Schedule_t *GetSchedule( void );
        void  AlertSound( void );
        void  PainSound( void );
        void  DeathSound( void );
};

LINK_ENTITY_TO_CLASS( monster_smallminer, CSmallMiner );

int CSmallMiner::Classify( void ) { return CLASS_HUMAN_MILITARY; }
void CSmallMiner::SetYawSpeed( void ) { pev->yaw_speed = 80; }

void CSmallMiner::AlertSound( void )
{
        EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "smallminer/sm_alert1.wav", 1.0, ATTN_NORM, 0, 100 );
}
void CSmallMiner::PainSound( void )
{
        EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "smallminer/sm_pain1.wav", 1.0, ATTN_NORM, 0, 100 );
}
void CSmallMiner::DeathSound( void )
{
        EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "smallminer/sm_die1.wav", 1.0, ATTN_NORM, 0, 100 );
}

BOOL CSmallMiner::CheckMeleeAttack1( float flDot, float flDist )
{
        return m_hEnemy && flDist <= 64.0f && flDot >= 0.7f;
}

// Medium-range swing attack (launchgrenade animation).
BOOL CSmallMiner::CheckRangeAttack2( float flDot, float flDist )
{
        return m_hEnemy && flDist > 64.0f && flDist <= 120.0f && flDot >= 0.5f;
}

// Block combatidle (gun crouch-idle) — only chase or attack in combat.
Schedule_t *CSmallMiner::GetSchedule( void )
{
        if( m_MonsterState == MONSTERSTATE_COMBAT )
        {
                ClearConditions( bits_COND_TASK_FAILED );

                if( HasConditions( bits_COND_ENEMY_DEAD ) )
                        return GetScheduleOfType( SCHED_VICTORY_DANCE );

                if( HasConditions( bits_COND_CAN_MELEE_ATTACK1 ) )
                        return GetScheduleOfType( SCHED_MELEE_ATTACK1 );

                if( HasConditions( bits_COND_CAN_RANGE_ATTACK2 ) )
                        return GetScheduleOfType( SCHED_RANGE_ATTACK2 );

                return GetScheduleOfType( SCHED_CHASE_ENEMY );
        }
        return CBaseMonster::GetSchedule();
}

void CSmallMiner::HandleAnimEvent( MonsterEvent_t *pEvent )
{
        switch( pEvent->event )
        {
        case SMALLMINER_AE_KICK:   // frontkick ev3@f14 — close crouching kick
        {
                CBaseEntity *pHurt = CheckTraceHullAttack( 64, gSkillData.smallminerPick, DMG_SLASH );
                if( pHurt )
                        EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "smallminer/sm_pick.wav", 1.0, ATTN_NORM, 0, 100 );
                break;
        }
        case SMALLMINER_AE_SWING1: // throwgrenade ev7@f36
        case SMALLMINER_AE_SWING2: // launchgrenade ev8@f16 — engine picks this for RANGE_ATTACK2
        {
                CBaseEntity *pHurt = CheckTraceHullAttack( 80, gSkillData.smallminerPick, DMG_SLASH );
                if( pHurt )
                        EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "smallminer/sm_pick.wav", 1.0, ATTN_NORM, 0, 100 );
                break;
        }
        default:
                CBaseMonster::HandleAnimEvent( pEvent );
                break;
        }
}

void CSmallMiner::Precache( void )
{
        PRECACHE_MODEL( "models/smallminer.mdl" );
        PRECACHE_SOUND( "smallminer/sm_alert1.wav" );
        PRECACHE_SOUND( "smallminer/sm_pain1.wav" );
        PRECACHE_SOUND( "smallminer/sm_die1.wav" );
        PRECACHE_SOUND( "smallminer/sm_pick.wav" );
}

void CSmallMiner::Spawn( void )
{
        Precache();
        SET_MODEL( ENT(pev), "models/smallminer.mdl" );
        UTIL_SetSize( pev, Vector(-16,-16,0), Vector(16,16,64) );

        pev->solid      = SOLID_SLIDEBOX;
        pev->movetype   = MOVETYPE_STEP;
        m_bloodColor    = BLOOD_COLOR_RED;
        pev->health     = gSkillData.smallminerHealth;
        pev->view_ofs   = Vector(0,0,56);
        m_flFieldOfView = 0.2f;
        m_MonsterState  = MONSTERSTATE_NONE;
        m_flGroundSpeed = 120;
        // MELEE_ATTACK1 → frontkick (close kick, ev3)
        // RANGE_ATTACK2 → launchgrenade (medium swing, ev8)
        m_afCapability  = bits_CAP_HEAR | bits_CAP_MELEE_ATTACK1 | bits_CAP_RANGE_ATTACK2;
        MonsterInit();
}
