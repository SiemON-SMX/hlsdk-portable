/***
*
*Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*This product contains software technology licensed from Id
*Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "skill.h"
#include "nodes.h"
#include "player.h"
#include "gamerules.h"

enum gattlinggun_e {
	GATTLINGGUN_IDLE = 0,
	GATTLINGGUN_IDLE2,
	GATTLINGGUN_IDLE3,
	GATTLINGGUN_SPINUP,
	GATTLINGGUN_FIRE,
	GATTLINGGUN_SPINDOWN,
	GATTLINGGUN_DRAW,
	GATTLINGGUN_HOLSTER,
	GATTLINGGUN_JAMB,
	GATTLINGGUN_UNJAMB,
	GATTLINGGUN_RELOAD,
	GATTLINGGUN_DRYFIRE,
};

#define GATL_STOPPED  0   // barrels at rest, IDLE anim playing
#define GATL_SPINUP   1   // SPINUP anim playing, barrels accelerating
#define GATL_FIRE     2   // FIRE anim looping, barrels at full speed
#define GATL_SPINDOWN 3   // SPINDOWN anim playing, barrels decelerating
#define GATL_JAMMED   4   // JAMB anim — player must unjam via secondary attack

// Spinup duration: sequence[3] = 10 frames @ 6 fps = 1.67 s
#define GATL_SPINUP_TIME ( 10.0f / 6.0f )

// Per-shot jam chance (out of 1000).  Doubles when clip < 20.
#define GATL_JAM_CHANCE 8

LINK_ENTITY_TO_CLASS( weapon_gattlinggun, CGattlingGun );

void CGattlingGun::Spawn( void )
{
	Precache();
	m_iId = WEAPON_GATTLINGGUN;
	SET_MODEL( ENT( pev ), "models/w_gattlinggun.mdl" );
	m_iDefaultAmmo = GATTLINGGUN_DEFAULT_GIVE;
	FallInit();
}

void CGattlingGun::Precache( void )
{
	PRECACHE_MODEL( "models/v_gattlinggun.mdl" );
	PRECACHE_MODEL( "models/w_gattlinggun.mdl" );
	PRECACHE_MODEL( "models/p_gattlinggun.mdl" );

	// Fire — three variants, rotated randomly each shot
	PRECACHE_SOUND( "weapons/gat_shoot1.wav" );
	PRECACHE_SOUND( "weapons/gat_shoot2.wav" );
	PRECACHE_SOUND( "weapons/gat_shoot3.wav" );

	// Barrel spin sounds
	PRECACHE_SOUND( "weapons/gat_spinup.wav" );
	PRECACHE_SOUND( "weapons/gat_spindown.wav" );

	// Dry-fire (barrels spinning but clip empty)
	PRECACHE_SOUND( "weapons/gat_dryfire.wav" );

	// Reload
	PRECACHE_SOUND( "weapons/gat_reload.wav" );

	// Jam and unjam
	PRECACHE_SOUND( "weapons/gat_jamb.wav" );
	PRECACHE_SOUND( "weapons/gat_unjamb.wav" );

	m_iShell = PRECACHE_MODEL( "models/shell.mdl" );

	m_usGattlingGun = PRECACHE_EVENT( 1, "events/gattlinggun.sc" );
}

int CGattlingGun::GetItemInfo( ItemInfo *p )
{
	p->pszName   = STRING( pev->classname );
	p->pszAmmo1  = "gattlinggun";
	p->iMaxAmmo1 = GATTLINGGUN_MAX_CARRY;
	p->pszAmmo2  = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip  = GATTLINGGUN_MAX_CLIP;
	p->iSlot     = 3;
	p->iPosition = 2;
	p->iFlags    = 0;
	p->iId       = m_iId = WEAPON_GATTLINGGUN;
	p->iWeight   = GATTLINGGUN_WEIGHT;
	return 1;
}

BOOL CGattlingGun::Deploy( void )
{
	m_bJammed       = FALSE;
	m_iGatlingState = GATL_STOPPED;
	m_flSpinTime    = 0.0f;
	return DefaultDeploy( "models/v_gattlinggun.mdl", "models/p_gattlinggun.mdl",
	                      GATTLINGGUN_DRAW, "gattlinggun" );
}

void CGattlingGun::Holster( int skiplocal )
{
	m_fInReload     = FALSE;
	m_bJammed       = FALSE;
	m_iGatlingState = GATL_STOPPED;
	m_flSpinTime    = 0.0f;
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.5;
	SendWeaponAnim( GATTLINGGUN_HOLSTER );
}

void CGattlingGun::PrimaryAttack( void )
{
	if( m_pPlayer->pev->waterlevel == 3 )
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = GetNextAttackDelay( 0.15 );
		return;
	}

	// Jammed — show the jamb anim and tell the player to use secondary attack
	if( m_bJammed )
	{
		if( m_iGatlingState != GATL_JAMMED )
		{
			SendWeaponAnim( GATTLINGGUN_JAMB, UseDecrement() );
			m_iGatlingState = GATL_JAMMED;
		}
		m_flNextPrimaryAttack = GetNextAttackDelay( 0.3 );
		return;
	}

		if( m_iClip <= 0 )
	{
		if( m_fFireOnEmpty )
		{
			// Barrels spinning but clip empty — dry_fire loop (seq 11)
			SendWeaponAnim( GATTLINGGUN_DRYFIRE, UseDecrement() );
			EMIT_SOUND_DYN( ENT( m_pPlayer->pev ), CHAN_ITEM,
			                "weapons/gat_dryfire.wav",
			                VOL_NORM, ATTN_NORM, 0, PITCH_NORM );
			m_flNextPrimaryAttack = GetNextAttackDelay( 0.2 );
			m_flTimeWeaponIdle    = UTIL_WeaponTimeBase() + 0.15f;
		}
		else
		{
			Reload();
		}
		return;
	}

	m_iClip--;

	m_pPlayer->pev->effects = (int)( m_pPlayer->pev->effects ) | EF_MUZZLEFLASH;
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash  = BRIGHT_GUN_FLASH;

	// Random jam — chance doubles when ammo is running low
	{
		int jamChance = ( m_iClip < 20 ) ? GATL_JAM_CHANCE * 2 : GATL_JAM_CHANCE;
		if( RANDOM_LONG( 0, 999 ) < jamChance )
		{
			m_bJammed       = TRUE;
			m_iGatlingState = GATL_JAMMED;
			SendWeaponAnim( GATTLINGGUN_JAMB, UseDecrement() );
			EMIT_SOUND_DYN( ENT( m_pPlayer->pev ), CHAN_ITEM,
			                "weapons/gat_jamb.wav",
			                VOL_NORM, ATTN_NORM, 0, PITCH_NORM );
			m_flNextPrimaryAttack   = GetNextAttackDelay( 0.5 );
			m_flNextSecondaryAttack = GetNextAttackDelay( 0.5 );
			m_flTimeWeaponIdle      = UTIL_WeaponTimeBase() + 0.5f;
			return;
		}
	}

	// twice on the local player (once from the server message, once from the event).

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	Vector vecSrc    = m_pPlayer->GetGunPosition();
	Vector vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );
	Vector vecDir;
	vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming,
	                                       VECTOR_CONE_6DEGREES, 8192,
	                                       BULLET_PLAYER_MP5, 0, (int)gSkillData.plrDmgGattlingGunBullet,
	                                       m_pPlayer->pev, m_pPlayer->random_seed );

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usGattlingGun, 0.0,
	                     (float *)&g_vecZero, (float *)&g_vecZero,
	                     vecDir.x, vecDir.y, 0, 0, 1, 0 );

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetNextAttackDelay( 0.07 );
	m_flTimeWeaponIdle    = UTIL_WeaponTimeBase() + 0.15f;
}

void CGattlingGun::SecondaryAttack( void )
{
	if( !m_bJammed )
		return;

	// sequence[9] unjamb  20fr 15fps = 1.33 s
	SendWeaponAnim( GATTLINGGUN_UNJAMB, UseDecrement() );
	EMIT_SOUND_DYN( ENT( m_pPlayer->pev ), CHAN_ITEM,
	                "weapons/gat_unjamb.wav",
	                VOL_NORM, ATTN_NORM, 0, PITCH_NORM );

	m_bJammed       = FALSE;
	m_iGatlingState = GATL_STOPPED;
	m_flSpinTime    = 0.0f;

	float unjamDuration = 20.0f / 15.0f; // 1.33 s
	m_flNextPrimaryAttack   = GetNextAttackDelay( unjamDuration );
	m_flNextSecondaryAttack = GetNextAttackDelay( unjamDuration );
	m_flTimeWeaponIdle      = UTIL_WeaponTimeBase() + unjamDuration + 0.1f;
}

void CGattlingGun::Reload( void )
{
	if( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 )
		return;

	BOOL fRet = DefaultReload( GATTLINGGUN_MAX_CLIP, GATTLINGGUN_RELOAD, 2.5 );

	if( fRet )
	{
		EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, "weapons/gat_reload.wav",
		                VOL_NORM, ATTN_NORM, 0, PITCH_NORM );
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase()
		                     + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
	}
}

void CGattlingGun::WeaponIdle( void )
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

	if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	// Jammed — nothing to do until the player unjams via secondary attack
	if( m_bJammed )
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0f;
		return;
	}

	switch( m_iGatlingState )
	{
	case GATL_SPINUP:
	case GATL_FIRE:
		// +attack released — spin down
		// sequence[5] spindown  4fps 10fr = 2.50 s
		SendWeaponAnim( GATTLINGGUN_SPINDOWN );
		EMIT_SOUND_DYN( ENT( m_pPlayer->pev ), CHAN_ITEM,
		                "weapons/gat_spindown.wav",
		                VOL_NORM, ATTN_NORM, 0, PITCH_NORM );
		m_iGatlingState    = GATL_SPINDOWN;
		m_flSpinTime       = gpGlobals->time;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + ( 10.0f / 4.0f ); // 2.50 s
		break;

	case GATL_SPINDOWN:
		// Spindown finished — barrels fully stopped
		// sequence[0] idle  1fr 30fps
		SendWeaponAnim( GATTLINGGUN_IDLE );
		m_iGatlingState    = GATL_STOPPED;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60.0f;
		break;

	case GATL_STOPPED:
	default:
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60.0f;
		break;
	}
}

class CGattlingGunAmmo : public CBasePlayerAmmo
{
	void Spawn( void )
	{
		Precache();
		SET_MODEL( ENT( pev ), "models/w_gattlinggun_belt.mdl" );
		CBasePlayerAmmo::Spawn();
	}
	void Precache( void )
	{
		PRECACHE_MODEL( "models/w_gattlinggun_belt.mdl" );
		PRECACHE_SOUND( "items/9mmclip1.wav" );
	}
	BOOL AddAmmo( CBaseEntity *pOther )
	{
		if( pOther->GiveAmmo( AMMO_GATTLINGGUN_GIVE, "gattlinggun", GATTLINGGUN_MAX_CARRY ) != -1 )
		{
			EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM );
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_gattlinggun, CGattlingGunAmmo );
