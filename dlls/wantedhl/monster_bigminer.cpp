/***
*
* WantedHL - monster_bigminer
* Large aggressive miner enemy. Heavy melee attacks.
*
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "schedule.h"
#include "skill.h"

#define BIGMINER_AE_KICK   3
#define BIGMINER_AE_SWING1 7
#define BIGMINER_AE_SWING2 8

class CBigMiner : public CBaseMonster
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

LINK_ENTITY_TO_CLASS( monster_bigminer, CBigMiner );

int CBigMiner::Classify( void ) { return CLASS_HUMAN_MILITARY; }
void CBigMiner::SetYawSpeed( void ) { pev->yaw_speed = 70; }

void CBigMiner::AlertSound( void )
{
	EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "bigminer/bm_alert1.wav", 1.0, ATTN_NORM, 0, 100 );
}
void CBigMiner::PainSound( void )
{
	EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "bigminer/bm_pain1.wav", 1.0, ATTN_NORM, 0, 100 );
}
void CBigMiner::DeathSound( void )
{
	EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "bigminer/bm_die1.wav", 1.0, ATTN_NORM, 0, 100 );
}

BOOL CBigMiner::CheckMeleeAttack1( float flDot, float flDist )
{
	return m_hEnemy && flDist <= 64.0f && flDot >= 0.7f;
}

BOOL CBigMiner::CheckRangeAttack2( float flDot, float flDist )
{
	return m_hEnemy && flDist > 64.0f && flDist <= 130.0f && flDot >= 0.5f;
}

Schedule_t *CBigMiner::GetSchedule( void )
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

void CBigMiner::HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
	case BIGMINER_AE_KICK:
	{
		CBaseEntity *pHurt = CheckTraceHullAttack( 64, gSkillData.bigminerPunch, DMG_CLUB );
		if( pHurt )
			EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "bigminer/bm_punch.wav", 1.0, ATTN_NORM, 0, 100 );
		break;
	}
	case BIGMINER_AE_SWING1:
	case BIGMINER_AE_SWING2:
	{
		CBaseEntity *pHurt = CheckTraceHullAttack( 90, gSkillData.bigminerPunch, DMG_CLUB );
		if( pHurt )
		{
			pHurt->pev->punchangle.x = 10;
			pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_forward * 200;
			EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "bigminer/bm_punch.wav", 1.0, ATTN_NORM, 0, 100 );
		}
		break;
	}
	default:
		CBaseMonster::HandleAnimEvent( pEvent );
		break;
	}
}

void CBigMiner::Precache( void )
{
	PRECACHE_MODEL( "models/bigminer.mdl" );
	PRECACHE_SOUND( "bigminer/bm_alert1.wav" );
	PRECACHE_SOUND( "bigminer/bm_pain1.wav" );
	PRECACHE_SOUND( "bigminer/bm_die1.wav" );
	PRECACHE_SOUND( "bigminer/bm_punch.wav" );
}

void CBigMiner::Spawn( void )
{
	Precache();
	SET_MODEL( ENT(pev), "models/bigminer.mdl" );
	UTIL_SetSize( pev, Vector(-16,-16,0), Vector(16,16,72) );

	pev->solid      = SOLID_SLIDEBOX;
	pev->movetype   = MOVETYPE_STEP;
	m_bloodColor    = BLOOD_COLOR_RED;
	pev->health     = gSkillData.bigminerHealth;
	pev->view_ofs   = Vector(0,0,64);
	m_flFieldOfView = 0.2f;
	m_MonsterState  = MONSTERSTATE_NONE;
	m_flGroundSpeed = 100;
	m_afCapability  = bits_CAP_HEAR | bits_CAP_MELEE_ATTACK1 | bits_CAP_RANGE_ATTACK2;
	MonsterInit();
}
