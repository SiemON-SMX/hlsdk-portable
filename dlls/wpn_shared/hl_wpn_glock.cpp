/***
*
*       Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*       This product contains software technology licensed from Id
*       Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*       All Rights Reserved.
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

enum glock_e
{
        GLOCK_IDLE1 = 0,                          // idle1                          (loaded)
        GLOCK_IDLE_NOSHOT,                        // idle_noshot                    (slide back, empty)
        GLOCK_IDLE3,                              // idle3                          (loaded)
        GLOCK_SHOOT,                              // shoot_right
        GLOCK_SHOOT_THEN_EMPTY,                   // shoot_right_then_empty         (last round; ends with slide back)
        GLOCK_RELOAD,                             // reload                         (clip still has rounds)
        GLOCK_RELOAD_FAST,                        // reload_fast
        GLOCK_RELOAD_NOSHOT,                      // reload_noshot                  (slide back / empty)
        GLOCK_RELOAD_NOSHOT_FAST,                 // reload_noshot_fast
        GLOCK_DRAW,                               // draw                           (loaded)
        GLOCK_DRAW_FAST,                          // draw_fast
        GLOCK_DRAW_NOSHOT,                        // draw_noshot                    (empty)
        GLOCK_DRAW_NOSHOT_FAST,                   // draw_noshot_fast
        GLOCK_HOLSTER,                            // holster
        GLOCK_DRAW_FROM_TWIN,                     // draw_from_twin                 (deploying after dropping twin)
        GLOCK_DRAW_FROM_TWIN_FAST,                // draw_from_twin_fast
        GLOCK_DRAW_FROM_TWIN_NOSHOT_BOTH,         // draw_from_twin_noshot_both
        GLOCK_DRAW_FROM_TWIN_NOSHOT_BOTH_FAST,    // draw_from_twin_noshot_both_fast
        GLOCK_DRAW_FROM_TWIN_NOSHOT_LEFT,         // draw_from_twin_noshot_left
        GLOCK_DRAW_FROM_TWIN_NOSHOT_LEFT_FAST,    // draw_from_twin_noshot_left_fast
        GLOCK_DRAW_FROM_TWIN_NOSHOT_RIGHT,        // draw_from_twin_noshot_right
        GLOCK_DRAW_FROM_TWIN_NOSHOT_RIGHT_FAST    // draw_from_twin_noshot_right_fast
};

LINK_ENTITY_TO_CLASS( weapon_glock, CGlock )
LINK_ENTITY_TO_CLASS( weapon_9mmhandgun, CGlock )

void CGlock::Spawn()
{
        pev->classname = MAKE_STRING( "weapon_9mmhandgun" ); // hack to allow for old names
        Precache();
        m_iId = WEAPON_GLOCK;
        SET_MODEL( ENT( pev ), "models/w_9mmhandgun.mdl" );

        m_iDefaultAmmo = GLOCK_DEFAULT_GIVE;

        FallInit();// get ready to fall down.
}

void CGlock::Precache( void )
{
        PRECACHE_MODEL( "models/v_9mmhandgun.mdl" );
        PRECACHE_MODEL( "models/w_9mmhandgun.mdl" );
        PRECACHE_MODEL( "models/p_9mmhandgun.mdl" );

        m_iShell = PRECACHE_MODEL( "models/shell.mdl" );// brass shell

        PRECACHE_SOUND( "items/9mmclip1.wav" );
        PRECACHE_SOUND( "items/9mmclip2.wav" );

        PRECACHE_SOUND( "weapons/pl_gun1.wav" );//silenced handgun
        PRECACHE_SOUND( "weapons/pl_gun2.wav" );//silenced handgun
        PRECACHE_SOUND( "weapons/pl_gun3.wav" );//handgun

        m_usFireGlock1 = PRECACHE_EVENT( 1, "events/glock1.sc" );
        m_usFireGlock2 = PRECACHE_EVENT( 1, "events/glock2.sc" );
}

int CGlock::GetItemInfo( ItemInfo *p )
{
        p->pszName = STRING( pev->classname );
        p->pszAmmo1 = "9mm";
        p->iMaxAmmo1 = _9MM_MAX_CARRY;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = GLOCK_MAX_CLIP;
        p->iSlot = 1;
        p->iPosition = 0;
        p->iFlags = 0;
        p->iId = m_iId = WEAPON_GLOCK;
        p->iWeight = GLOCK_WEIGHT;

        return 1;
}

int CGlock::AddToPlayer( CBasePlayer *pPlayer )
{
        if( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
        {
                MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
                        WRITE_BYTE( m_iId );
                MESSAGE_END();
                return TRUE;
        }
        return FALSE;
}

int CGlock::AddDuplicate( CBasePlayerItem *pOriginal )
{
#ifndef CLIENT_DLL
        if( !pOriginal )
                return CBasePlayerWeapon::AddDuplicate( pOriginal );

        CBasePlayer *pPlayer = static_cast<CBasePlayer *>( pOriginal->m_pPlayer );
        if( pPlayer && !pPlayer->HasNamedPlayerItem( "weapon_9mmhandgun_twin" ) )
        {
                CBaseEntity *pTwin = CBaseEntity::Create( "weapon_9mmhandgun_twin",
                                                          pev->origin,
                                                          g_vecZero, NULL );
                if( pTwin )
                {
                        DispatchTouch( ENT( pTwin->pev ), ENT( pPlayer->pev ) );
                        return TRUE;
                }
        }
#endif
        return CBasePlayerWeapon::AddDuplicate( pOriginal );
}

BOOL CGlock::Deploy()
{
        int iAnim;
        const bool bRightEmpty = ( m_iClip <= 0 );

        bool bFromTwin = false;
        bool bTwinLeftEmpty  = false;
        bool bTwinRightEmpty = bRightEmpty;

        if( m_pPlayer && m_pPlayer->m_pLastItem )
        {
                entvars_t *pPrevPev = m_pPlayer->m_pLastItem->pev;
                if( pPrevPev && pPrevPev->classname && FClassnameIs( pPrevPev, "weapon_9mmhandgun_twin" ) )
                {
                        bFromTwin = true;
                        // Read both clip states from the twin so we can pick the correct
                        // draw_from_twin animation variant (noshot_left / noshot_right /
                        // noshot_both) instead of always assuming both are empty.
                        CGlockTwin *pTwin = static_cast<CGlockTwin *>( m_pPlayer->m_pLastItem );
                        bTwinLeftEmpty  = ( pTwin->GetClipLeft() <= 0 );
                        bTwinRightEmpty = ( pTwin->m_iClip       <= 0 );
                }
        }

        if( bFromTwin )
        {
                if( bTwinRightEmpty && bTwinLeftEmpty )  iAnim = GLOCK_DRAW_FROM_TWIN_NOSHOT_BOTH;
                else if( bTwinLeftEmpty )                iAnim = GLOCK_DRAW_FROM_TWIN_NOSHOT_LEFT;
                else if( bTwinRightEmpty )               iAnim = GLOCK_DRAW_FROM_TWIN_NOSHOT_RIGHT;
                else                                     iAnim = GLOCK_DRAW_FROM_TWIN;
        }
        else
        {
                iAnim = bRightEmpty ? GLOCK_DRAW_NOSHOT : GLOCK_DRAW;
        }

        return DefaultDeploy( "models/v_9mmhandgun.mdl", "models/p_9mmhandgun.mdl", iAnim, "onehanded", /*UseDecrement() ? 1 : 0*/ 0 );
}

void CGlock::Holster( int skiplocal /* = 0 */ )
{
        m_fInReload = FALSE;

        // Play the holster animation (16 frames @ 20 fps = 0.800 s).
        SendWeaponAnim( GLOCK_HOLSTER );
        m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.8f;

        m_flNextPrimaryAttack   = UTIL_WeaponTimeBase();
        m_flNextSecondaryAttack = UTIL_WeaponTimeBase();
}

void CGlock::SecondaryAttack( void )
{
        GlockFire( 0.1f, 0.2f, FALSE );
}

void CGlock::PrimaryAttack( void )
{
        GlockFire( 0.01f, 0.3f, TRUE );
}

void CGlock::GlockFire( float flSpread, float flCycleTime, BOOL fUseAutoAim )
{
        if( m_iClip <= 0 )
        {
                if( m_fFireOnEmpty )
                {
                        PlayEmptySound();
                        m_flNextPrimaryAttack = GetNextAttackDelay( 0.2f );
                }

                return;
        }

        m_iClip--;

        m_pPlayer->pev->effects = (int)( m_pPlayer->pev->effects ) | EF_MUZZLEFLASH;

        int flags;
#if CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif
        // player "shoot" animation
        m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

        // silenced
        if( pev->body == 1 )
        {
                m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;
                m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;
        }
        else
        {
                // non-silenced
                m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
                m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;
        }

        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecAiming;

        if( fUseAutoAim )
        {
                vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );
        }
        else
        {
                vecAiming = gpGlobals->v_forward;
        }

        Vector vecDir;
        vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, Vector( flSpread, flSpread, flSpread ), 8192, BULLET_PLAYER_9MM, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );

        PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), fUseAutoAim ? m_usFireGlock1 : m_usFireGlock2, 0.0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, 0, 0, ( m_iClip == 0 ) ? 1 : 0, 0 );

        m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetNextAttackDelay( flCycleTime );

        if( !m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 )
                // HEV suit - indicate out of ammo condition
                m_pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}

void CGlock::Reload( void )
{
        if( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip == GLOCK_MAX_CLIP )
                return;

        int iResult;

        if( m_iClip == 0 )
                iResult = DefaultReload( GLOCK_MAX_CLIP, GLOCK_RELOAD_NOSHOT, 1.583f );
        else
                iResult = DefaultReload( GLOCK_MAX_CLIP, GLOCK_RELOAD, 1.583f );

        if( iResult )
        {
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
        }
}

void CGlock::WeaponIdle( void )
{
        ResetEmptySound();

        m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

        if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
                return;

        int iAnim;

        if( m_iClip == 0 )
        {
                // Slide is locked back; only the empty idle is valid.
                iAnim = GLOCK_IDLE_NOSHOT;
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0f;
        }
        else
        {
                float flRand = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 0.0f, 1.0f );

                if( flRand <= 0.5f )
                {
                        iAnim = GLOCK_IDLE1;
                        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60.0f / 24.0f;
                }
                else
                {
                        iAnim = GLOCK_IDLE3;
                        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 49.0f / 24.0f;
                }
        }

        SendWeaponAnim( iAnim, 1 );
}

class CGlockAmmo : public CBasePlayerAmmo
{
        void Spawn( void )
        {
                Precache();
                SET_MODEL( ENT( pev ), "models/w_9mmclip.mdl" );
                CBasePlayerAmmo::Spawn();
        }

        void Precache( void )
        {
                PRECACHE_MODEL( "models/w_9mmclip.mdl" );
                PRECACHE_SOUND( "items/9mmclip1.wav" );
        }

        BOOL AddAmmo( CBaseEntity *pOther )
        {
                if( pOther->GiveAmmo( AMMO_GLOCKCLIP_GIVE, "9mm", _9MM_MAX_CARRY ) != -1 )
                {
                        EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM );
                        return TRUE;
                }
                return FALSE;
        }
};

LINK_ENTITY_TO_CLASS( ammo_glockclip, CGlockAmmo )
LINK_ENTITY_TO_CLASS( ammo_9mmclip, CGlockAmmo )
