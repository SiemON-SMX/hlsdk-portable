//========= Copyright (c) 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: New version of the slider bar
//
// $NoKeywords: $
//=============================================================================


#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"monsters.h"
#include	"schedule.h"

// For holograms, make them not solid so the player can walk through them
#define	SF_MONSTERPLAYER_NOTSOLID					4 


class CPlayerMonster : public CBaseMonster
{
public:
	void Spawn( void );
	void Precache( void );
	void SetYawSpeed( void );
	int Classify( void );
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	int ISoundMask( void );
};

LINK_ENTITY_TO_CLASS( monster_player, CPlayerMonster )

int CPlayerMonster::Classify( void )
{
	return CLASS_PLAYER_ALLY;
}

void CPlayerMonster::SetYawSpeed( void )
{
	int ys;

	switch( m_Activity )
	{
	case ACT_IDLE:
	default:
		ys = 90;
	}

	pev->yaw_speed = ys;
}

void CPlayerMonster :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
	case 0:
	default:
		CBaseMonster::HandleAnimEvent( pEvent );
		break;
	}
}

int CPlayerMonster::ISoundMask( void )
{
	return 0;
}

void CPlayerMonster::Spawn()
{
	Precache();

	SET_MODEL( ENT( pev ), "models/player.mdl" );
	UTIL_SetSize( pev, VEC_HULL_MIN, VEC_HULL_MAX );

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BLOOD_COLOR_RED;
	pev->health = 8;
	m_flFieldOfView = 0.5;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState = MONSTERSTATE_NONE;

	MonsterInit();
	if( pev->spawnflags & SF_MONSTERPLAYER_NOTSOLID )
	{
		pev->solid = SOLID_NOT;
		pev->takedamage = DAMAGE_NO;
	}
}

void CPlayerMonster::Precache()
{
	PRECACHE_MODEL( "models/player.mdl" );
}

