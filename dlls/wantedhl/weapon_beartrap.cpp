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
#include "nodes.h"
#include "player.h"
#include "gamerules.h"

enum btrap_e {
        BTRAP_DRAW = 0,
        BTRAP_HOLSTER,
        BTRAP_IDLE,
        BTRAP_FIDGET,
        BTRAP_THROW,
};

LINK_ENTITY_TO_CLASS( weapon_beartrap, CBeartrap );

void CBeartrap::Spawn( void )
{
        Precache();
        m_iId = WEAPON_BEARTRAP;
        SET_MODEL( ENT( pev ), "models/w_beartrap.mdl" );
        m_iClip = -1;
        m_iDefaultAmmo = BEARTRAP_DEFAULT_GIVE;
        FallInit();
}

void CBeartrap::Precache( void )
{
        PRECACHE_MODEL( "models/v_beartrap.mdl" );
        PRECACHE_MODEL( "models/w_beartrap.mdl" );
        PRECACHE_MODEL( "models/p_beartrap.mdl" );

        PRECACHE_SOUND( "weapons/beartrap_deploy.wav" );
        PRECACHE_SOUND( "weapons/beartrap_fire.wav" );

        UTIL_PrecacheOther( "monster_beartrap" );

        m_usBeartrap = PRECACHE_EVENT( 1, "events/beartrap.sc" );
}

int CBeartrap::GetItemInfo( ItemInfo *p )
{
        p->pszName   = STRING( pev->classname );
        p->pszAmmo1  = "beartraps";
        p->iMaxAmmo1 = BEARTRAP_MAX_CARRY;
        p->pszAmmo2  = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip  = WEAPON_NOCLIP;
        p->iSlot     = 4;
        p->iPosition = 4;
        p->iFlags    = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;
        p->iId       = m_iId = WEAPON_BEARTRAP;
        p->iWeight   = BEARTRAP_WEIGHT;
        return 1;
}

BOOL CBeartrap::Deploy( void )
{
        return DefaultDeploy( "models/v_beartrap.mdl", "models/p_beartrap.mdl",
                              BTRAP_DRAW, "beartrap" );
}

void CBeartrap::Holster( int skiplocal )
{
        m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

        if( !m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] )
        {
                m_pPlayer->pev->weapons &= ~( 1 << WEAPON_BEARTRAP );
                SetThink( &CBeartrap::DestroyItem );
                pev->nextthink = gpGlobals->time + 0.1;
                return;
        }

        SendWeaponAnim( BTRAP_HOLSTER );
}

void CBeartrap::PrimaryAttack( void )
{
        if( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 )
                return;

        UTIL_MakeVectors( m_pPlayer->pev->v_angle );
        TraceResult tr;
        Vector trace_origin = m_pPlayer->pev->origin;

        if( m_pPlayer->pev->flags & FL_DUCKING )
                trace_origin = trace_origin - ( VEC_HULL_MIN - VEC_DUCK_HULL_MIN );

        UTIL_TraceLine( trace_origin + gpGlobals->v_forward * 20,
                        trace_origin + gpGlobals->v_forward * 64,
                        dont_ignore_monsters, NULL, &tr );

        int flags;
#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usBeartrap, 0.0,
                             (float *)&g_vecZero, (float *)&g_vecZero,
                             0, 0, 0, 0, 0, 0 );

        if( tr.fAllSolid == 0 && tr.fStartSolid == 0 && tr.flFraction > 0.25 )
        {
                m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

#ifndef CLIENT_DLL
                CBaseEntity *pTrap = CBaseEntity::Create( "monster_beartrap",
                                                          tr.vecEndPos,
                                                          m_pPlayer->pev->v_angle,
                                                          m_pPlayer->edict() );
                if( pTrap )
                        pTrap->pev->velocity = gpGlobals->v_forward * 100 + m_pPlayer->pev->velocity;
#endif

                m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
                m_fJustThrown = 1;

                m_flNextPrimaryAttack = GetNextAttackDelay( 0.3 );
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0;
        }
}

void CBeartrap::WeaponIdle( void )
{
        if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
                return;

        if( m_fJustThrown )
        {
                m_fJustThrown = 0;

                if( !m_pPlayer->m_rgAmmo[PrimaryAmmoIndex()] )
                {
                        RetireWeapon();
                        return;
                }

                SendWeaponAnim( BTRAP_DRAW );
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase()
                                     + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
                return;
        }

        int iAnim;
        float flRand = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 0, 1 );
        if( flRand <= 0.75 )
        {
                iAnim = BTRAP_IDLE;
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 30.0 / 16.0;
        }
        else
        {
                iAnim = BTRAP_FIDGET;
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 40.0 / 16.0;
        }

        SendWeaponAnim( iAnim );
}

class CBearTrapAmmo : public CBasePlayerAmmo
{
        void Spawn( void )
        {
                Precache();
                // w_beartrapack.mdl is absent from all public WantedHL releases.
                // Fall back to the beartrap weapon world model so the pickup is visible.
                SET_MODEL( ENT( pev ), "models/w_beartrap.mdl" );
                CBasePlayerAmmo::Spawn();
        }
        void Precache( void )
        {
                PRECACHE_MODEL( "models/w_beartrap.mdl" );
                PRECACHE_SOUND( "items/9mmclip1.wav" );
        }
        BOOL AddAmmo( CBaseEntity *pOther )
        {
                if( pOther->GiveAmmo( AMMO_BEARTRAP_GIVE, "beartraps", BEARTRAP_MAX_CARRY ) != -1 )
                {
                        EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM );
                        return TRUE;
                }
                return FALSE;
        }
};
LINK_ENTITY_TO_CLASS( ammo_beartraps, CBearTrapAmmo );
