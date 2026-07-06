/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "nodes.h"
#include "monsters.h"
#include "animation.h"
#include "saverestore.h"
#include "soundent.h"

void CBaseMonster::SetState( MONSTERSTATE State )
{
/*
	if( State != m_MonsterState )
	{
		ALERT( at_aiconsole, "State Changed to %d\n", State );
	}
*/
	switch( State )
	{

	// Drop enemy pointers when going to idle
	case MONSTERSTATE_IDLE:
		if( m_hEnemy != 0 )
		{
			m_hEnemy = NULL;// not allowed to have an enemy anymore.
			ALERT( at_aiconsole, "Stripped\n" );
		}
		break;
	default:
		break;
	}

	m_MonsterState = State;
	m_IdealMonsterState = State;
}

void CBaseMonster::RunAI( void )
{

	if( ( m_MonsterState == MONSTERSTATE_IDLE || m_MonsterState == MONSTERSTATE_ALERT ) && RANDOM_LONG( 0, 99 ) == 0 && !( pev->spawnflags & SF_MONSTER_GAG ) )
	{
		IdleSound();
	}

	if( m_MonsterState != MONSTERSTATE_NONE &&
		 m_MonsterState != MONSTERSTATE_PRONE &&
		 m_MonsterState != MONSTERSTATE_DEAD )// don't bother with this crap if monster is prone. 
	{
		if( !FNullEnt( FIND_CLIENT_IN_PVS( edict() ) ) || ( m_MonsterState == MONSTERSTATE_COMBAT ) )
		{
			Look( m_flDistLook );
			Listen();// check for audible sounds. 

			// now filter conditions.
			ClearConditions( IgnoreConditions() );

			GetEnemy();
		}

		// do these calculations if monster has an enemy.
		if( m_hEnemy != 0 )
		{
			CheckEnemy( m_hEnemy );
		}

		CheckAmmo();
	}

	FCheckAITrigger();

	PrescheduleThink();

	MaintainSchedule();

	m_afConditions &= ~( bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE );
}

MONSTERSTATE CBaseMonster::GetIdealState( void )
{
	int iConditions;

	iConditions = IScheduleFlags();

	// If no schedule conditions, the new ideal state is probably the reason we're in here.
	switch( m_MonsterState )
	{
	case MONSTERSTATE_IDLE:
		/*
		IDLE goes to ALERT upon hearing a sound
		-IDLE goes to ALERT upon being injured
		IDLE goes to ALERT upon seeing food
		-IDLE goes to COMBAT upon sighting an enemy
		IDLE goes to HUNT upon smelling food
		*/
		{
			if( iConditions & bits_COND_NEW_ENEMY )			
			{
				m_IdealMonsterState = MONSTERSTATE_COMBAT;
			}
			else if( iConditions & bits_COND_LIGHT_DAMAGE )
			{
				MakeIdealYaw( m_vecEnemyLKP );
				m_IdealMonsterState = MONSTERSTATE_ALERT;
			}
			else if( iConditions & bits_COND_HEAVY_DAMAGE )
			{
				MakeIdealYaw( m_vecEnemyLKP );
				m_IdealMonsterState = MONSTERSTATE_ALERT;
			}
			else if( iConditions & bits_COND_HEAR_SOUND )
			{
				CSound *pSound;

				pSound = PBestSound();
				ASSERT( pSound != NULL );
				if( pSound )
				{
					MakeIdealYaw( pSound->m_vecOrigin );
					if( pSound->m_iType & ( bits_SOUND_COMBAT|bits_SOUND_DANGER ) )
						m_IdealMonsterState = MONSTERSTATE_ALERT;
				}
			}
			else if( iConditions & ( bits_COND_SMELL | bits_COND_SMELL_FOOD ) )
			{
				m_IdealMonsterState = MONSTERSTATE_ALERT;
			}

			break;
		}
	case MONSTERSTATE_ALERT:
		/*
		ALERT goes to IDLE upon becoming bored
		-ALERT goes to COMBAT upon sighting an enemy
		ALERT goes to HUNT upon hearing a noise
		*/
		{
			if( iConditions & ( bits_COND_NEW_ENEMY | bits_COND_SEE_ENEMY ) )			
			{
				// see an enemy we MUST attack
				m_IdealMonsterState = MONSTERSTATE_COMBAT;
			}
			else if( iConditions & bits_COND_HEAR_SOUND )
			{
				m_IdealMonsterState = MONSTERSTATE_ALERT;
				CSound *pSound = PBestSound();
				ASSERT( pSound != NULL );
				if( pSound )
					MakeIdealYaw( pSound->m_vecOrigin );
			}
			break;
		}
	case MONSTERSTATE_COMBAT:
		/*
		COMBAT goes to HUNT upon losing sight of enemy
		COMBAT goes to ALERT upon death of enemy
		*/
		{
			if( m_hEnemy == 0 )
			{
				m_IdealMonsterState = MONSTERSTATE_ALERT;
				// pev->effects = EF_BRIGHTFIELD;
				ALERT( at_aiconsole, "***Combat state with no enemy!\n" );
			}
			break;
		}
	case MONSTERSTATE_HUNT:
		/*
		HUNT goes to ALERT upon seeing food
		HUNT goes to ALERT upon being injured
		HUNT goes to IDLE if goal touched
		HUNT goes to COMBAT upon seeing enemy
		*/
		{
			break;
		}
	case MONSTERSTATE_SCRIPT:
		if( iConditions & ( bits_COND_TASK_FAILED | bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE ) )
		{
			ExitScriptedSequence();	// This will set the ideal state
		}
		break;
	case MONSTERSTATE_DEAD:
		m_IdealMonsterState = MONSTERSTATE_DEAD;
		break;
	default:
		break;
	}

	return m_IdealMonsterState;
}
