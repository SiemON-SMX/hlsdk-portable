/***
*
* WantedHL - monster_colonel
* The Colonel — story NPC driven by scripted_sequence chains.
*
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "schedule.h"
#include "defaultai.h"
#include "scripted.h"
#include "skill.h"

class CColonel : public CBaseMonster
{
public:
	void        Spawn( void );
	void        Precache( void );
	int         Classify( void );
	void        SetYawSpeed( void );
	Schedule_t *GetSchedule( void );
};

LINK_ENTITY_TO_CLASS( monster_colonel, CColonel );

int CColonel::Classify( void )
{
	return CLASS_PLAYER_ALLY;
}

void CColonel::SetYawSpeed( void )
{
	pev->yaw_speed = 90;
}

Schedule_t *CColonel::GetSchedule( void )
{
	switch( m_MonsterState )
	{
	case MONSTERSTATE_IDLE:
	case MONSTERSTATE_ALERT:
		return GetScheduleOfType( SCHED_IDLE_STAND );
	default:
		return CBaseMonster::GetSchedule();
	}
}

void CColonel::Spawn( void )
{
	Precache();

	SET_MODEL( ENT(pev), "models/colonel.mdl" );
	UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );

	pev->solid      = SOLID_SLIDEBOX;
	pev->movetype   = MOVETYPE_STEP;
	m_bloodColor    = BLOOD_COLOR_RED;
	pev->health     = gSkillData.colonelHealth;
	pev->view_ofs   = Vector( 0, 0, 50 );
	m_flFieldOfView = VIEW_FIELD_WIDE;
	m_MonsterState  = MONSTERSTATE_NONE;
	m_afCapability  = bits_CAP_HEAR | bits_CAP_TURN_HEAD;

	MonsterInit();
}

void CColonel::Precache( void )
{
	PRECACHE_MODEL( "models/colonel.mdl" );
}
