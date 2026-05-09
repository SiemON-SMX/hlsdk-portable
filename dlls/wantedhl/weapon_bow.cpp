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
#include "projectile_bow.h"

enum bow_e {
        BOW_IDLE1 = 0,
        BOW_IDLE2,
        BOW_IDLE3,
        BOW_IDLE4,
        BOW_IDLE5,
        BOW_IDLE6,
        BOW_DRAW,         BOW_HOLSTER,      // = 7  (was 3 — broken, played idle4)
        BOW_RELOAD,       // = 8  (was 6 — broken, played draw)
        BOW_PULLBACK,     // = 9
        BOW_PULLBACK_IDLE,// = 10
        BOW_FIRE,         BOW_DRYFIRE,      // = 12
};

LINK_ENTITY_TO_CLASS( weapon_bow, CBow );

void CBow::Spawn( void )
{
        Precache();
        m_iId = WEAPON_BOW;
        SET_MODEL( ENT( pev ), "models/w_bow.mdl" );
        m_iDefaultAmmo = BOW_DEFAULT_GIVE;
        FallInit();
}

void CBow::Precache( void )
{
        PRECACHE_MODEL( "models/v_bow.mdl" );
        PRECACHE_MODEL( "models/w_bow.mdl" );
        PRECACHE_MODEL( "models/p_bow.mdl" );

        PRECACHE_SOUND( "weapons/bow_fire1.wav" );
        PRECACHE_SOUND( "weapons/bow_draw.wav" );
        PRECACHE_SOUND( "weapons/bow_reload.wav" );
        PRECACHE_SOUND( "weapons/357_cock1.wav" );

        PRECACHE_MODEL( "models/arrow.mdl" );

        m_usBow = PRECACHE_EVENT( 1, "events/bow.sc" );
}

int CBow::GetItemInfo( ItemInfo *p )
{
        p->pszName   = STRING( pev->classname );
        p->pszAmmo1  = "arrows";
        p->iMaxAmmo1 = BOW_MAX_CARRY;
        p->pszAmmo2  = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip  = BOW_MAX_CLIP;
        p->iSlot     = 2;
        p->iPosition = 3;
        p->iFlags    = 0;
        p->iId       = m_iId = WEAPON_BOW;
        p->iWeight   = BOW_WEIGHT;
        return 1;
}

BOOL CBow::Deploy( void )
{
        return DefaultDeploy( "models/v_bow.mdl", "models/p_bow.mdl", BOW_DRAW, "bow" );
}

void CBow::Holster( int skiplocal )
{
        m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
        SendWeaponAnim( BOW_HOLSTER );
}

void CBow::PrimaryAttack( void )
{
        if( m_pPlayer->pev->waterlevel == 3 )
        {
                PlayEmptySound();
                m_flNextPrimaryAttack = GetNextAttackDelay( 0.15 );
                return;
        }

        if( m_iClip <= 0 )
        {
                Reload();
                if( m_iClip == 0 )
                {
                        // sequence[12] = dryfire  18fps 30fr = 1.67 s
                        SendWeaponAnim( BOW_DRYFIRE, UseDecrement() );
                        PlayEmptySound();
                }
                return;
        }

        m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash  = DIM_GUN_FLASH;

        m_iClip--;

        m_pPlayer->pev->effects = (int)( m_pPlayer->pev->effects ) | EF_MUZZLEFLASH;
        m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

        // Fire animation — BOW_FIRE=11 → model[11]=fire (direct, always plays on Android)
        SendWeaponAnim( BOW_FIRE, UseDecrement() );

        // Fire sound — direct so it always plays regardless of event system
        EMIT_SOUND_DYN( ENT( m_pPlayer->pev ), CHAN_WEAPON, "weapons/bow_fire1.wav",
                        1, ATTN_NORM, 0, 100 + RANDOM_LONG( 0, 0xf ) );

        int flags;
#if defined( CLIENT_WEAPONS )
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

        Vector vecSrc    = m_pPlayer->GetGunPosition();
        Vector vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );
        Vector vecDir;
        vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming,
                                               VECTOR_CONE_1DEGREES, 8192,
                                               BULLET_PLAYER_BUCKSHOT, 0, (int)gSkillData.plrDmgBowArrow,
                                               m_pPlayer->pev, m_pPlayer->random_seed );

#ifndef CLIENT_DLL
        CArrow::Shoot( m_pPlayer->pev, vecSrc, vecAiming * 1000 );
#endif

        PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usBow, 0.0,
                             (float *)&g_vecZero, (float *)&g_vecZero,
                             vecDir.x, vecDir.y, 0, 0, 0, 0 );

        m_flNextPrimaryAttack = GetNextAttackDelay( 1.0 );
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase()
                             + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );

        if( m_iClip == 0 && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0 )
                Reload();
}

void CBow::Reload( void )
{
        if( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 )
                return;

        if( m_iClip >= BOW_MAX_CLIP )
                return;

        DefaultReload( BOW_MAX_CLIP, BOW_RELOAD, 1.0 );
}

void CBow::WeaponIdle( void )
{
        ResetEmptySound();
        m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

        if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
                return;

        int iAnim;
        float flRand = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 0, 1 );
        if( flRand <= 0.75 )
        {
                iAnim = BOW_IDLE1;
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 40.0 / 16.0;
        }
        else
        {
                iAnim = BOW_IDLE2;  // was BOW_FIDGET (removed from enum — index 1 = idle2)
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60.0 / 16.0;
        }

        SendWeaponAnim( iAnim );
}

class CBowAmmoBox : public CBasePlayerAmmo
{
        void Spawn( void )
        {
                Precache();
                // w_arrowbox.mdl is absent from all public WantedHL releases.
                // Fall back to the bow's own world model so the pickup is visible.
                SET_MODEL( ENT( pev ), "models/w_bow.mdl" );
                CBasePlayerAmmo::Spawn();
        }
        void Precache( void )
        {
                PRECACHE_MODEL( "models/w_bow.mdl" );
                PRECACHE_SOUND( "items/9mmclip1.wav" );
        }
        BOOL AddAmmo( CBaseEntity *pOther )
        {
                if( pOther->GiveAmmo( AMMO_BOW_GIVE, "arrows", BOW_MAX_CARRY ) != -1 )
                {
                        EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM );
                        return TRUE;
                }
                return FALSE;
        }
};
LINK_ENTITY_TO_CLASS( ammo_arrows, CBowAmmoBox );
