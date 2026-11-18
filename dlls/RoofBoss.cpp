/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/
//=========================================================
//=========================================================

#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"monsters.h"
#include	"weapons.h"
#include	"player.h"

extern DLL_GLOBAL short g_sModelIndexNull;

class CRoofBossHead : public CBaseMonster
{
public:
	void Spawn( void );
	void Precache( void );
	int  Classify ( void );
	int  IRelationship ( CBaseEntity *pTarget );
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	void Killed( entvars_t *pevAttacker, int iGib );

	void EXPORT StartTrack ( void );
	void EXPORT StartDart ( void );
	void EXPORT TrackTarget ( void );
	void EXPORT TrackTouch ( CBaseEntity *pOther );
	void EXPORT DartTouch( CBaseEntity *pOther );
	void EXPORT DieTouch ( CBaseEntity *pOther );

	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );

	float			m_flStopAttack;
	float			m_flFlySpeed;
};

LINK_ENTITY_TO_CLASS( roofboss_head, CRoofBossHead );

//=========================================================
// Save/Restore
//=========================================================
TYPEDESCRIPTION	CRoofBossHead::m_SaveData[] =
{
	DEFINE_FIELD( CRoofBossHead, m_flStopAttack, FIELD_TIME ),
	DEFINE_FIELD( CRoofBossHead, m_flFlySpeed, FIELD_FLOAT ),
};

IMPLEMENT_SAVERESTORE( CRoofBossHead, CBaseMonster );

//=========================================================
//=========================================================
int CRoofBossHead :: TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	// filter these bits a little.
	bitsDamageType &= ~ ( DMG_ALWAYSGIB );
	bitsDamageType |= DMG_NEVERGIB;

	return CBaseMonster :: TakeDamage ( pevInflictor, pevAttacker, flDamage, bitsDamageType );
}

//=========================================================
//=========================================================
void CRoofBossHead :: Killed( entvars_t *pevAttacker, int iGib )
{
	int pitch = RANDOM_LONG( 90, 110 );
	EMIT_SOUND_DYN( edict(), CHAN_BODY, "roofboss/rb_headdeath.wav", 1, ATTN_NORM, 0, pitch );

	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, pev->origin );
		WRITE_BYTE( TE_BLOODSPRITE );
		WRITE_COORD( pev->origin.x );	// pos
		WRITE_COORD( pev->origin.y );
		WRITE_COORD( pev->origin.z );
		WRITE_SHORT( g_sModelIndexNull );
		WRITE_SHORT( g_sModelIndexBloodDrop );
		WRITE_BYTE( BLOOD_COLOR_RED );			// color
		WRITE_BYTE( 5 );
	MESSAGE_END();

	SetThink( NULL );

	STOP_SOUND( edict(), CHAN_VOICE, "roofboss/rb_mouthidle1.wav" );
	STOP_SOUND( edict(), CHAN_VOICE, "roofboss/rb_mouthidle2.wav" );

	SET_MODEL( ENT(pev), "models/null.mdl" );
	pev->solid = SOLID_NOT;

	CBaseMonster::Killed( pevAttacker, iGib );
}

//=========================================================
//=========================================================
void CRoofBossHead :: Spawn( void )
{
	Precache();

	pev->movetype	= MOVETYPE_FLY;
	pev->solid		= SOLID_BBOX;
	pev->takedamage = DAMAGE_AIM;
	pev->flags		|= FL_MONSTER;
	pev->health		= gSkillData.roofbossHeadHealth;

	m_flStopAttack	= gpGlobals->time + 5.0;

	m_flFieldOfView = 0.9; // +- 25 degrees
	m_flFlySpeed = 400;

	m_bloodColor = BLOOD_COLOR_RED;

	if ( CVAR_GET_FLOAT( "sv_joebiden" ) )
		SET_MODEL(ENT( pev ), "models/biden_box.mdl");
	else
		SET_MODEL(ENT( pev ), "models/hookedhead.mdl");

	UTIL_SetSize( pev, Vector( 0, 0, 0 ), Vector( 33, 41, 13 ) );

	SetTouch( &CRoofBossHead::DieTouch );
	SetThink( &CRoofBossHead::StartTrack );

	edict_t *pSoundEnt = pev->owner;
	if ( !pSoundEnt )
		pSoundEnt = edict();

	pev->renderfx = 101;

	pev->dmg = gSkillData.roofbossHeadHit;

	pev->nextthink = gpGlobals->time + 0.1;
	ResetSequenceInfo( );
}


void CRoofBossHead :: Precache()
{
	if ( CVAR_GET_FLOAT( "sv_joebiden" ) )
		PRECACHE_MODEL("models/biden_box.mdl");
	else
		PRECACHE_MODEL("models/hookedhead.mdl");

	PRECACHE_SOUND( "roofboss/rb_headdeath.wav" );

	if ( CVAR_GET_FLOAT( "sv_joebiden" ) )
		PRECACHE_SOUND( "roofboss/milkme.wav" );
	else
		PRECACHE_SOUND( "roofboss/rb_headhit.wav" );
}

//=========================================================
//=========================================================
int CRoofBossHead::IRelationship ( CBaseEntity *pTarget )
{
	if ( pTarget->pev->modelindex == pev->modelindex )
	{
		return R_NO;
	}

	return CBaseMonster :: IRelationship( pTarget );
}

//=========================================================
//=========================================================
int CRoofBossHead::Classify ( void )
{
	if ( pev->owner && pev->owner->v.flags & FL_CLIENT)
	{
		return CLASS_PLAYER_BIOWEAPON;
	}

	return	CLASS_ALIEN_BIOWEAPON;
}

//=========================================================
//=========================================================
void CRoofBossHead :: StartTrack ( void )
{
	SetTouch( &CRoofBossHead::TrackTouch );
	SetThink( &CRoofBossHead::TrackTarget );

	pev->nextthink = gpGlobals->time + 0.1;
}

//=========================================================
//=========================================================
void CRoofBossHead :: StartDart ( void )
{
	SetTouch( &CRoofBossHead::DartTouch );

	SetThink( &CRoofBossHead::SUB_Remove );
	pev->nextthink = gpGlobals->time + 4;
}

//=========================================================
//=========================================================
void CRoofBossHead :: TrackTarget ( void )
{
	Vector	vecFlightDir;
	Vector	vecDirToEnemy;
	float	flDelta;

	StudioFrameAdvance( );

	if (gpGlobals->time > m_flStopAttack)
	{
		SetTouch( NULL );
		SetThink( &CRoofBossHead::SUB_Remove );
		pev->nextthink = gpGlobals->time + 0.1;
		return;
	}

	// UNDONE: The player pointer should come back after returning from another level
	if ( m_hEnemy == NULL )
	{// enemy is dead.
		Look( 512 );
		m_hEnemy = BestVisibleEnemy( );
	}

	if ( m_hEnemy != NULL && FVisible( m_hEnemy ))
	{
		m_vecEnemyLKP = m_hEnemy->BodyTarget( pev->origin );
	}
	else
	{
		m_vecEnemyLKP = m_vecEnemyLKP + pev->velocity * m_flFlySpeed * 0.1;
	}

	vecDirToEnemy = ( m_vecEnemyLKP - pev->origin ).Normalize();

	if (pev->velocity.Length() < 0.1)
		vecFlightDir = vecDirToEnemy;
	else
		vecFlightDir = pev->velocity.Normalize();

	// measure how far the turn is, the wider the turn, the slow we'll go this time.
	flDelta = DotProduct ( vecFlightDir, vecDirToEnemy );

	pev->velocity = ( vecFlightDir + vecDirToEnemy).Normalize();

	if ( pev->owner && (pev->owner->v.flags & FL_MONSTER) )
	{

		pev->velocity.x += RANDOM_FLOAT ( -0.10, 0.10 );// scramble the flight dir a bit.
		pev->velocity.y += RANDOM_FLOAT ( -0.10, 0.10 );
		pev->velocity.z += RANDOM_FLOAT ( -0.10, 0.10 );
	}

	float flSpeed = m_flFlySpeed;
	pev->velocity = pev->velocity * flSpeed;
	pev->nextthink = gpGlobals->time + 0.1;

	pev->angles = UTIL_VecToAngles (pev->velocity);

	pev->solid = SOLID_BBOX;

	if ( m_hEnemy != NULL )
	{
		if ( flDelta >= 0.4 && ( pev->origin - m_vecEnemyLKP ).Length() <= 300 )
		{
		}
	}
}

//=========================================================
//=========================================================
void CRoofBossHead :: TrackTouch ( CBaseEntity *pOther )
{
	if ( pOther->edict() == pev->owner || pOther->pev->modelindex == pev->modelindex )
	{// bumped into the guy that shot it.
		pev->solid = SOLID_NOT;
		return;
	}

	DieTouch( pOther );
}

void CRoofBossHead::DartTouch( CBaseEntity *pOther )
{
	DieTouch( pOther );
}

void CRoofBossHead::DieTouch ( CBaseEntity *pOther )
{
	pev->health = -1;

	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, pev->origin );
		WRITE_BYTE( TE_BLOODSPRITE );
		WRITE_COORD( pev->origin.x );	// pos
		WRITE_COORD( pev->origin.y );
		WRITE_COORD( pev->origin.z );
		WRITE_SHORT( g_sModelIndexNull );
		WRITE_SHORT( g_sModelIndexBloodDrop );
		WRITE_BYTE( BLOOD_COLOR_RED );			// color
		WRITE_BYTE( 5 );
	MESSAGE_END();

	if ( CVAR_GET_FLOAT( "sv_joebiden" ) )
		EMIT_SOUND( ENT(pev), CHAN_VOICE, "roofboss/milkme.wav", 1, ATTN_NORM );
	else
		EMIT_SOUND( ENT(pev), CHAN_VOICE, "roofboss/rb_headhit.wav", 1, ATTN_NORM );

	if ( pOther && pOther->pev->takedamage )
	{// do the damage
		if ( pOther->IsPlayer() )
			pOther->TakeDamage( pev, VARS( pev->owner ), pev->dmg, DMG_BULLET );
	}

	pev->modelindex = 0;// so will disappear for the 0.1 secs we wait until NEXTTHINK gets rid
	pev->solid = SOLID_NOT;

	SetThink ( &CRoofBossHead::SUB_Remove );
	pev->nextthink = gpGlobals->time + 1;// stick around long enough for the sound to finish!
}

//=========================================================
//=========================================================
int UTIL_FindRoofBossTargetsInSphere( CBaseEntity **pList, int listMax, const Vector &center, float radius )
{

	edict_t		*pEdict = g_engfuncs.pfnPEntityOfEntIndex( 1 );
	CBaseEntity *pEntity;
	int			count;
	float		distance, delta;

	count = 0;
	float radiusSquared = radius * radius;

	if ( !pEdict )
		return count;

	for ( int i = 1; i < gpGlobals->maxEntities; i++, pEdict++ )
	{
		if ( pEdict->free )	// Not in use
			continue;

		if ( !pEdict )
			continue;

		if ( pEdict->v.takedamage != 50 )
			continue;

		if ( !FClassnameIs( pEdict, "info_roofboss_target" ) )
			continue;

		// Use origin for X & Y since they are centered for all monsters
		// Now X
		delta = center.x - pEdict->v.origin.x;//(pEdict->v.absmin.x + pEdict->v.absmax.x)*0.5;
		delta *= delta;

		if ( delta > radiusSquared )
			continue;
		distance = delta;

		// Now Y
		delta = center.y - pEdict->v.origin.y;//(pEdict->v.absmin.y + pEdict->v.absmax.y)*0.5;
		delta *= delta;

		distance += delta;
		if ( distance > radiusSquared )
			continue;

		// Now Z
		delta = center.z - (pEdict->v.absmin.z + pEdict->v.absmax.z)*0.5;
		delta *= delta;

		distance += delta;
		if ( distance > radiusSquared )
			continue;

		pEntity = CBaseEntity::Instance(pEdict);
		if ( !pEntity )
			continue;

		pList[ count ] = pEntity;
		count++;

		if ( count >= listMax )
			return count;
	}


	return count;
}

//=========================================================
//=========================================================
class CRoofBoss : public CBaseMonster
{
public:
	void Spawn( void );
	void Precache( void );
	void SetYawSpeed( void );
	int  Classify ( void );
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	int  ISoundMask( void );
	void MonsterThink( void );

	int m_iZHeight;

	void Killed( entvars_t *pevAttacker, int iGib );
	void PainSound( void );
	void DeathSound( void );

	bool m_bHasAttacked;
	float m_fNextAttackTime;
	float m_flNextShoot;
	bool m_bStartingAttack;
	float m_fNextAttackSoundTime;
	bool m_bHasJustAttacked;
	float m_flNextIdleSoundTime;
	float m_flNextMoveTime;
	Vector m_vMoveTarget;
	bool m_bMoving;
	float m_fMoveTime;

	void UpdateOnRemove( void );

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];
};

TYPEDESCRIPTION	CRoofBoss::m_SaveData[] =
{
	DEFINE_FIELD( CRoofBoss, m_fNextAttackTime, FIELD_TIME ),
	DEFINE_FIELD( CRoofBoss, m_bHasAttacked, FIELD_BOOLEAN ),
	DEFINE_FIELD( CRoofBoss, m_flNextShoot, FIELD_TIME ),
	DEFINE_FIELD( CRoofBoss, m_bStartingAttack, FIELD_BOOLEAN ),
	DEFINE_FIELD( CRoofBoss, m_vMoveTarget, FIELD_VECTOR ),
	DEFINE_FIELD( CRoofBoss, m_bMoving, FIELD_BOOLEAN ),
	DEFINE_FIELD( CRoofBoss, m_fMoveTime, FIELD_TIME ),
	DEFINE_FIELD( CRoofBoss, m_flNextMoveTime, FIELD_TIME ),
	DEFINE_FIELD( CRoofBoss, m_flNextIdleSoundTime, FIELD_TIME ),
	DEFINE_FIELD( CRoofBoss, m_fNextAttackSoundTime, FIELD_TIME ),
	DEFINE_FIELD( CRoofBoss, m_bHasJustAttacked, FIELD_BOOLEAN ),
	DEFINE_FIELD( CRoofBoss, m_iZHeight, FIELD_INTEGER ),
};

IMPLEMENT_SAVERESTORE( CRoofBoss, CBaseMonster );

LINK_ENTITY_TO_CLASS( monster_roofboss, CRoofBoss );

//=========================================================
// Classify - indicates this monster's place in the
// relationship table.
//=========================================================
int	CRoofBoss :: Classify ( void )
{
	return m_iClass?m_iClass:CLASS_ALIEN_MONSTER;
}

void CRoofBoss :: UpdateOnRemove( void )
{
	STOP_SOUND( edict(), CHAN_VOICE, "roofboss/rb_mouthidle1.wav" );
	STOP_SOUND( edict(), CHAN_VOICE, "roofboss/rb_mouthidle2.wav" );

	CBaseEntity::UpdateOnRemove();
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CRoofBoss :: SetYawSpeed ( void )
{
	int ys;

	switch ( m_Activity )
	{
	case ACT_IDLE:
	default:
		ys = 90;
		break;
	}

	pev->yaw_speed = ys;
}

//=========================================================
// PainSound
//=========================================================
void CRoofBoss :: PainSound ( void )
{
	switch ( RANDOM_LONG( 0, 2 ) )
	{
	case 0:
		EMIT_SOUND_DYN( edict(), CHAN_BODY, "roofboss/rb_hurt1.wav", 1, ATTN_NONE, 0, 100 );
		break;
	case 1:
		EMIT_SOUND_DYN( edict(), CHAN_BODY, "roofboss/rb_hurt2.wav", 1, ATTN_NONE, 0, 100 );
		break;
	case 2:
		EMIT_SOUND_DYN( edict(), CHAN_BODY, "roofboss/rb_hurt3.wav", 1, ATTN_NONE, 0, 100 );
		break;
	}
}

void CRoofBoss :: DeathSound ( void )
{
	EMIT_SOUND_DYN( edict(), CHAN_BODY, "roofboss/rb_death.wav", 1, ATTN_NONE, 0, 100 );

	EMIT_SOUND_DYN( edict(), CHAN_VOICE, "common/null.wav", 1, ATTN_NONE, 0, 100 );
}

void CRoofBoss :: Killed( entvars_t *pevAttacker, int iGib )
{
	unsigned int	cCount = 0;
	BOOL			fDone = FALSE;

	if ( HasMemory( bits_MEMORY_KILLED ) )
	{
		if ( ShouldGibMonster( iGib ) )
			CallGibMonster();
		return;
	}

	CBaseEntity *pAttacker = CBaseEntity::Instance( pevAttacker );

	if ( pAttacker && pAttacker->IsPlayer() )
	{
		if ( m_iLastDamageType & (1 << 26) )
			((CBasePlayer *)pAttacker)->playerstats.monsters_killed_melee++;
		else
			((CBasePlayer *)pAttacker)->playerstats.monsters_killed_shooting++;

		GetPointsForKill( (char *)STRING( pev->classname ), (CBasePlayer *)pAttacker );
	}

	Remember( bits_MEMORY_KILLED );

	// clear the deceased's sound channels.(may have been firing or reloading when killed)
	EMIT_SOUND(ENT(pev), CHAN_WEAPON, "common/null.wav", 1, ATTN_NORM);
	m_IdealMonsterState = MONSTERSTATE_DEAD;
	// Make sure this condition is fired too (TakeDamage breaks out before this happens on death)
	SetConditions( bits_COND_LIGHT_DAMAGE );

	// tell owner ( if any ) that we're dead.This is mostly for MonsterMaker functionality.
	CBaseEntity *pOwner = CBaseEntity::Instance(pev->owner);
	if ( pOwner )
	{
		pOwner->DeathNotice( pev );
	}

	if	( ShouldGibMonster( iGib ) )
	{
		CallGibMonster();
		return;
	}
	else if ( pev->flags & FL_MONSTER )
	{
		SetTouch( NULL );
		BecomeDead();
	}

	// don't let the status bar glitch for players.with <0 health.
	if (pev->health < -99)
	{
		pev->health = 0;
	}

	//pev->enemy = ENT( pevAttacker );//why? (sjb)

	m_IdealMonsterState = MONSTERSTATE_DEAD;
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CRoofBoss :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
		case 50:
		{
			m_bHasAttacked = true;
			m_fNextAttackTime = gpGlobals->time + 20;
		}
		break;

		default:
			CBaseMonster::HandleAnimEvent( pEvent );
			break;
	}
}

//=========================================================
//=========================================================
int CRoofBoss :: ISoundMask ( void )
{
	return 0;
}

//=========================================================
// Spawn
//=========================================================
void CRoofBoss :: Spawn()
{
	Precache( );

	SET_MODEL(ENT(pev), "models/carcassboss.mdl");
	UTIL_SetSize( pev, Vector( -12, -12, 0 ), Vector( 12, 12, 72 ) );

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_FLY;
	pev->flags			|= FL_FLY;
	m_bloodColor		= BLOOD_COLOR_RED;
	pev->health			= gSkillData.roofbossHealth;
	pev->view_ofs		= Vector ( 0, 0, -2 );// position of the eyes relative to monster's origin.
	m_flFieldOfView		= -0.7;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;

	pev->renderfx = 101;

	if ( CVAR_GET_FLOAT( "sv_joebiden" ) )
	{
		pev->rendermode = kRenderTransAdd;
		pev->renderamt = 0;
	}

	MonsterInit();

	m_iZHeight = pev->origin.z;
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CRoofBoss :: Precache()
{
	PRECACHE_MODEL("models/carcassboss.mdl");
	UTIL_PrecacheOther( "roofboss_head" );

	PRECACHE_SOUND("roofboss/rb_attack1.wav");
	PRECACHE_SOUND("roofboss/rb_attack2.wav");
	PRECACHE_SOUND("roofboss/rb_death.wav");
	PRECACHE_SOUND("roofboss/rb_headshoot.wav");
	PRECACHE_SOUND("roofboss/rb_hurt1.wav");
	PRECACHE_SOUND("roofboss/rb_hurt2.wav");
	PRECACHE_SOUND("roofboss/rb_hurt3.wav");
	PRECACHE_SOUND("roofboss/rb_mouthidle1.wav");
	PRECACHE_SOUND("roofboss/rb_mouthidle2.wav");
	PRECACHE_SOUND("roofboss/rb_stomclose.wav");
	PRECACHE_SOUND("roofboss/rb_stomopen.wav");
}

void CRoofBoss :: MonsterThink( void )
{
	if ( !IsAlive() )
		return;

	if ( pev->origin.z != m_iZHeight && IsAlive() )
		pev->origin.z = m_iZHeight;

	if ( m_flNextIdleSoundTime < gpGlobals->time )
	{
		switch ( RANDOM_LONG( 0, 1 ) )
		{
		case 0:
			EMIT_SOUND_DYN( edict(), CHAN_VOICE, "roofboss/rb_mouthidle1.wav", 1, ATTN_NONE, 0, 100 );
			m_flNextIdleSoundTime = gpGlobals->time + 7.49;
			break;

		case 1:
			EMIT_SOUND_DYN( edict(), CHAN_VOICE, "roofboss/rb_mouthidle2.wav", 1, ATTN_NONE, 0, 100 );
			m_flNextIdleSoundTime = gpGlobals->time + 9.43;
			break;
		}
	}

	if ( m_bMoving )
	{

		if ( m_fMoveTime < gpGlobals->time )
		{
			m_vMoveTarget = g_vecZero;
			m_bMoving = false;

		}

		if ( m_vMoveTarget != g_vecZero )
		{
			UTIL_MoveToOrigin( ENT(pev), m_vMoveTarget, 14, MOVE_STRAFE );

			MakeIdealYaw( m_vMoveTarget );
			SetYawSpeed();
			ChangeYaw( pev->yaw_speed );


			CBaseEntity *pEntity = NULL;
			while ( ( pEntity = UTIL_FindEntityInSphere( pEntity, m_vMoveTarget, 80 ) ) != NULL )
			{
				if ( FClassnameIs( pEntity->edict(), "monster_roofboss" ) )
				{
					int nextmoveseconds = RANDOM_LONG( 2, 7 );

					CBaseEntity *pPlayer = UTIL_FindEntityByClassname( NULL, "player" );

					if ( pPlayer )
					{
						MakeIdealYaw( pPlayer->pev->origin );
						SetYawSpeed();
						ChangeYaw( pev->yaw_speed );
					}


					m_flNextMoveTime = gpGlobals->time + nextmoveseconds;

					// done
					m_vMoveTarget = g_vecZero;
					m_bMoving = false;
				}
			}
		}
		else
			m_bMoving = false;
	}
	else
	{
		CBaseEntity *pPlayer = UTIL_FindEntityByClassname( NULL, "player" );

		if ( pPlayer && FVisible( pPlayer ) )
		{
			MakeIdealYaw( pPlayer->pev->origin );
			SetYawSpeed();
			ChangeYaw( pev->yaw_speed );
		}

		if ( m_flNextMoveTime > gpGlobals->time && !m_bMoving )
		{


			if ( !m_bHasAttacked )
				m_bHasAttacked = false;

			if ( !m_bStartingAttack )
				m_bStartingAttack = false;
		}
		else
		{
			int attack;

			if ( m_bHasJustAttacked )
				attack = RANDOM_LONG( 0, 7 );
			else
				attack = RANDOM_LONG( 0, 2 );

			if ( attack == 1 )
			{
				CBaseEntity *pPlayer = UTIL_FindEntityByClassname( NULL, "player" );

				if ( pPlayer && FVisible( pPlayer ) && !m_bStartingAttack )
				{
					m_bStartingAttack = true;
					SetSequenceByName( "attack" );

					m_bHasJustAttacked = true;

					int pitch = 100 + RANDOM_LONG( -10, 10 );
					EMIT_SOUND_DYN( edict(), CHAN_ITEM, "roofboss/rb_stomopen.wav", 1, ATTN_NONE, 0, pitch );

					m_bHasAttacked = true;
					m_fNextAttackTime = gpGlobals->time + gSkillData.roofbossAttackTime;
				}
			}
			else if ( !m_bHasAttacked && !m_bStartingAttack )
			{
				CBaseEntity *pList[32];
				int count = UTIL_FindRoofBossTargetsInSphere( pList, 96, pev->origin, 4096 );
				int target = 1;


				if ( count > 1 )
					target = RANDOM_LONG( 1, count - 1 );

				if ( pList[target] )
				{
					m_fMoveTime = gpGlobals->time + 10;

					m_vMoveTarget = pList[target]->pev->origin;
					m_bMoving = true;


					m_bHasJustAttacked = false;
				}
			}
		}
	}

	if ( m_bHasAttacked && m_fNextAttackTime < gpGlobals->time )
	{
		m_bHasAttacked = false;
		m_bStartingAttack = false;

		int pitch = 100 + RANDOM_LONG( -10, 10 );
		EMIT_SOUND_DYN( edict(), CHAN_ITEM, "roofboss/rb_stomclose.wav", 1, ATTN_NONE, 0, pitch );

		SetSequenceByName( "idle" );
	}

	if ( m_bHasAttacked && m_fNextAttackTime > gpGlobals->time )
	{
		if ( m_flNextShoot < gpGlobals->time )
		{
			m_flNextShoot = gpGlobals->time + 1;

			Vector vecSrc, vecAng;
			GetAttachment( 0, vecSrc, vecAng );

			Vector vecDirToEnemy;
			Vector angDir;

			if ( m_fNextAttackSoundTime < gpGlobals->time )
			{
				if ( RANDOM_LONG( 0, 1 ) == 1 )
				{
					switch ( RANDOM_LONG( 0, 1 ) )
					{
					case 0:
						EMIT_SOUND_DYN( edict(), CHAN_BODY, "roofboss/rb_attack1.wav", 1, ATTN_NONE, 0, 100 );
						m_fNextAttackSoundTime = gpGlobals->time + 1.66;
						break;

					case 1:
						EMIT_SOUND_DYN( edict(), CHAN_BODY, "roofboss/rb_attack2.wav", 1, ATTN_NONE, 0, 100 );
						m_fNextAttackSoundTime = gpGlobals->time + 2.93;
						break;
					}
				}
			}

			CBaseEntity *pHead = CBaseEntity::Create( "roofboss_head", vecSrc, UTIL_VecToAngles( vecDirToEnemy ), edict() );
			UTIL_MakeVectors ( pHead->pev->angles );
			pHead->pev->velocity = gpGlobals->v_forward * 300;

			CBaseMonster *pHeadMonster = pHead->MyMonsterPointer();

			if ( pHeadMonster )
			{
				CBaseEntity *pPlayer = UTIL_FindEntityByClassname( NULL, "player" );

				if ( pPlayer )
					pHeadMonster->m_hEnemy = pPlayer;
			}
		}
	}

	CBaseMonster::MonsterThink();
}

//=========================================================
//=========================================================
class CInfoRoofBossTarget : public CPointEntity
{
public:
	void Spawn( void );
	void Precache( void );
};

LINK_ENTITY_TO_CLASS( info_roofboss_target, CInfoRoofBossTarget );

//=========================================================
//=========================================================
void CInfoRoofBossTarget :: Spawn( void )
{
	pev->solid = SOLID_NOT;

	pev->takedamage = 50;

	if ( pev->spawnflags & 1 )
	{
		PRECACHE_MODEL( "sprites/null.spr" );
		SET_MODEL( ENT(pev), "sprites/null.spr" );
		UTIL_SetSize( pev, g_vecZero, g_vecZero );
	}
}

void CInfoRoofBossTarget :: Precache( void )
{
	if ( pev->spawnflags & 1 )
		PRECACHE_MODEL( "sprites/null.spr" );
}
