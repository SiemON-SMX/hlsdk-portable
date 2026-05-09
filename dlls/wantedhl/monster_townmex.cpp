/***
*
* WantedHL - monster_townmex / monster_twnwesta / monster_twnwestb
* Civilian NPCs. CTownMex and CTwnWestB are wandering healer allies.
* CTwnWestA ("Zeke") is a story NPC driven by scripted_sequence chains.
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

class CTownMex : public CScientist
{
public:
	void Spawn( void );
	void Precache( void );
	void TalkInit( void );
	int  Classify( void );
	virtual int ObjectCaps( void ) { return CTalkMonster::ObjectCaps() | FCAP_IMPULSE_USE; }
	BOOL CanHeal( void );
	void Heal( void );
};

LINK_ENTITY_TO_CLASS( monster_townmex, CTownMex );

int CTownMex::Classify( void )
{
	return CLASS_PLAYER_ALLY;
}

void CTownMex::TalkInit( void )
{
	CTalkMonster::TalkInit();
	// silent — no sentence groups
}

void CTownMex::Spawn( void )
{
	Precache();
	SET_MODEL( ENT(pev), "models/townmex.mdl" );
	UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );

	pev->solid      = SOLID_SLIDEBOX;
	pev->movetype   = MOVETYPE_STEP;
	m_bloodColor    = BLOOD_COLOR_RED;
	pev->health     = gSkillData.townmexHealth;
	pev->view_ofs   = Vector( 0, 0, 50 );
	m_flFieldOfView = VIEW_FIELD_WIDE;
	m_MonsterState  = MONSTERSTATE_NONE;

	m_afCapability = bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_OPEN_DOORS | bits_CAP_AUTO_DOORS | bits_CAP_USE;

	MonsterInit();
	SetUse( &CTownMex::FollowerUse );
}

void CTownMex::Precache( void )
{
	PRECACHE_MODEL( "models/townmex.mdl" );
	TalkInit();
	CTalkMonster::Precache();
}

BOOL CTownMex::CanHeal( void )
{
	if( ( m_healTime > gpGlobals->time ) || ( m_hTargetEnt == NULL ) ||
	    ( m_hTargetEnt->pev->health > ( m_hTargetEnt->pev->max_health * 0.5f ) ) )
		return FALSE;
	return TRUE;
}

void CTownMex::Heal( void )
{
	if( !CanHeal() )
		return;
	Vector vecTarget = m_hTargetEnt->pev->origin - pev->origin;
	if( vecTarget.Length() > 100 )
		return;
	m_hTargetEnt->TakeHealth( gSkillData.townmexHeal, DMG_GENERIC );
	m_healTime = gpGlobals->time + 60;
}

class CTwnWestA : public CBaseMonster
{
public:
	void        Spawn( void );
	void        Precache( void );
	int         Classify( void );
	void        SetYawSpeed( void );
	Schedule_t *GetSchedule( void );
};

LINK_ENTITY_TO_CLASS( monster_twnwesta, CTwnWestA );

int CTwnWestA::Classify( void )
{
	return CLASS_PLAYER_ALLY;
}

void CTwnWestA::SetYawSpeed( void )
{
	pev->yaw_speed = 90;
}

Schedule_t *CTwnWestA::GetSchedule( void )
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

void CTwnWestA::Spawn( void )
{
	Precache();
	SET_MODEL( ENT(pev), "models/twnwesta.mdl" );
	UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );

	pev->solid      = SOLID_SLIDEBOX;
	pev->movetype   = MOVETYPE_STEP;
	m_bloodColor    = BLOOD_COLOR_RED;
	pev->health     = gSkillData.wtownaHealth;
	pev->view_ofs   = Vector( 0, 0, 50 );
	m_flFieldOfView = VIEW_FIELD_WIDE;
	m_MonsterState  = MONSTERSTATE_NONE;
	m_afCapability  = bits_CAP_HEAR | bits_CAP_TURN_HEAD;

	MonsterInit();
}

void CTwnWestA::Precache( void )
{
	PRECACHE_MODEL( "models/twnwesta.mdl" );
}

class CTwnWestB : public CScientist
{
public:
	void Spawn( void );
	void Precache( void );
	void TalkInit( void );
	int  Classify( void );
	virtual int ObjectCaps( void ) { return CTalkMonster::ObjectCaps() | FCAP_IMPULSE_USE; }
	BOOL CanHeal( void );
	void Heal( void );
};

LINK_ENTITY_TO_CLASS( monster_twnwestb, CTwnWestB );

int CTwnWestB::Classify( void )
{
	return CLASS_PLAYER_ALLY;
}

void CTwnWestB::TalkInit( void )
{
	CTalkMonster::TalkInit();
	// silent — no sentence groups
}

void CTwnWestB::Spawn( void )
{
	Precache();
	SET_MODEL( ENT(pev), "models/twnwesta.mdl" ); // twnwestb.mdl not in release
	UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );

	pev->solid      = SOLID_SLIDEBOX;
	pev->movetype   = MOVETYPE_STEP;
	m_bloodColor    = BLOOD_COLOR_RED;
	pev->health     = gSkillData.wtownbHealth;
	pev->view_ofs   = Vector( 0, 0, 50 );
	m_flFieldOfView = VIEW_FIELD_WIDE;
	m_MonsterState  = MONSTERSTATE_NONE;

	m_afCapability = bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_OPEN_DOORS | bits_CAP_AUTO_DOORS | bits_CAP_USE;

	MonsterInit();
	SetUse( &CTwnWestB::FollowerUse );
}

void CTwnWestB::Precache( void )
{
	PRECACHE_MODEL( "models/twnwesta.mdl" );
	TalkInit();
	CTalkMonster::Precache();
}

BOOL CTwnWestB::CanHeal( void )
{
	if( ( m_healTime > gpGlobals->time ) || ( m_hTargetEnt == NULL ) ||
	    ( m_hTargetEnt->pev->health > ( m_hTargetEnt->pev->max_health * 0.5f ) ) )
		return FALSE;
	return TRUE;
}

void CTwnWestB::Heal( void )
{
	if( !CanHeal() )
		return;
	Vector vecTarget = m_hTargetEnt->pev->origin - pev->origin;
	if( vecTarget.Length() > 100 )
		return;
	m_hTargetEnt->TakeHealth( gSkillData.wtownbHeal, DMG_GENERIC );
	m_healTime = gpGlobals->time + 60;
}
