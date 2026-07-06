/***
*
*       M249 SAW
*
*       Ported from FWGS hlsdk-portable opfor branch (dlls/gearbox/m249.cpp).
*       Adapted to the HalfPayne / hlsdk-portable Half-Life code base:
*       fall back to the standard "models/shell.mdl" for both shell and link
*       (the original opfor saw_shell.mdl / saw_link.mdl are not part of the
*       base Half-Life game).
*
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "soundent.h"
#include "gamerules.h"

enum m249_e
{
        M249_SLOWIDLE = 0,
        M249_IDLE2,
        M249_RELOAD1,           // reload1_1: open belt cover, insert new belt
        M249_RELOAD2,           // reload1_2: close cover, charge bolt
        M249_HOLSTER,
        M249_DRAW,              // draw / deploy
        M249_SHOOT1,
        M249_SHOOT2,
        M249_SHOOT3,
};

LINK_ENTITY_TO_CLASS( weapon_m249, CM249 )

void CM249::Spawn()
{
        pev->classname = MAKE_STRING( "weapon_m249" );
        Precache();
        SET_MODEL( ENT( pev ), "models/w_saw.mdl" );
        m_iId = WEAPON_M249;

        m_iDefaultAmmo      = M249_DEFAULT_GIVE;
        m_bAlternatingEject = false;
        m_flReloadStage2Time = 0.0f;

        m_iClip        = 0;
        m_iVisibleClip = 0;

        FallInit();// get ready to fall down.
}

void CM249::Precache( void )
{
        PRECACHE_MODEL( "models/v_saw.mdl" );
        PRECACHE_MODEL( "models/w_saw.mdl" );
        PRECACHE_MODEL( "models/p_saw.mdl" );

        m_iShell = PRECACHE_MODEL( "models/shell.mdl" );
        m_iLink  = m_iShell;

        PRECACHE_SOUND( "items/9mmclip1.wav" );
        PRECACHE_SOUND( "weapons/saw_fire1.wav" );
        PRECACHE_SOUND( "weapons/357_cock1.wav" );

        m_usM249 = PRECACHE_EVENT( 1, "events/m249.sc" );
}

int CM249::GetItemInfo( ItemInfo *p )
{
        p->pszName    = STRING( pev->classname );
        p->pszAmmo1   = "556";
        p->iMaxAmmo1  = _556_MAX_CARRY;
        p->pszAmmo2   = NULL;
        p->iMaxAmmo2  = -1;
        p->iMaxClip   = M249_MAX_CLIP;
        p->iSlot      = 3;
        p->iPosition  = 4;
        p->iFlags     = 0;
        p->iId        = m_iId = WEAPON_M249;
        p->iWeight    = M249_WEIGHT;

        return 1;
}

int CM249::AddToPlayer( CBasePlayer *pPlayer )
{
        if( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
        {
                if( m_iClip <= 0 )
                {
                        m_iClip        = M249_MAX_CLIP;
                        m_iVisibleClip = M249_MAX_CLIP;
                        UpdateTape();
                }

                pPlayer->GiveAmmo( M249_DEFAULT_GIVE_CARRY, "556", _556_MAX_CARRY );

                MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
                        WRITE_BYTE( m_iId );
                MESSAGE_END();
                return TRUE;
        }
        return FALSE;
}

BOOL CM249::Deploy()
{
        UpdateTape();
        m_flReloadStage2Time = 0.0f;
        return DefaultDeploy( "models/v_saw.mdl", "models/p_saw.mdl", M249_DRAW, "m249", 0, pev->body );
}

void CM249::Holster( int skiplocal /* = 0 */ )
{
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0f;
        SendWeaponAnim( M249_HOLSTER );
}

void CM249::PrimaryAttack()
{
        // don't fire underwater
        if( m_pPlayer->pev->waterlevel == 3 )
        {
                PlayEmptySound();
                m_flNextPrimaryAttack = GetNextAttackDelay( 0.15f );
                return;
        }

        if( m_iClip <= 0 )
        {
                PlayEmptySound();
                m_flNextPrimaryAttack = GetNextAttackDelay( 0.15f );
                return;
        }

        m_pPlayer->pev->punchangle.x = RANDOM_FLOAT( 1.0f, 1.5f );
        m_pPlayer->pev->punchangle.y = RANDOM_FLOAT( -0.5f, -0.2f );

        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash  = NORMAL_GUN_FLASH;

        m_iClip--;
        UpdateTape();
        m_bAlternatingEject = !m_bAlternatingEject;
        m_pPlayer->pev->effects = (int)( m_pPlayer->pev->effects ) | EF_MUZZLEFLASH;

        m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

        Vector vecSrc    = m_pPlayer->GetGunPosition();
        Vector vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );
        Vector vecDir;

        vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming,
                VECTOR_CONE_3DEGREES, 8192, BULLET_PLAYER_MP5, 2, 0,
                m_pPlayer->pev, m_pPlayer->random_seed );

        int flags;
#if CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usM249,
                0.0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, 0, pev->body,
                m_bAlternatingEject ? 1 : 0, 0 );

#if !CLIENT_DLL
        if( m_pPlayer->pev->flags & FL_ONGROUND )
        {
                float flZVel = m_pPlayer->pev->velocity.z;
                m_pPlayer->pev->velocity = m_pPlayer->pev->velocity
                        - gpGlobals->v_forward * ( 40 + ( RANDOM_LONG( 1, 2 ) * 2 ) );
                m_pPlayer->pev->velocity.z = flZVel;
        }
#endif

        if( !m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 )
                m_pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );

        m_flNextPrimaryAttack = GetNextAttackDelay( 0.067f );

        if( m_flNextPrimaryAttack < UTIL_WeaponTimeBase() )
                m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1f;

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.2f;
}

void CM249::Reload( void )
{
        if( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip == M249_MAX_CLIP )
                return;

        if( DefaultReload( M249_MAX_CLIP, M249_RELOAD1, 1.33f, pev->body ) )
        {
                m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 3.78f;
                m_flTimeWeaponIdle    = UTIL_WeaponTimeBase() + 3.78f;
                m_flReloadStage2Time  = UTIL_WeaponTimeBase() + 1.33f;
        }
}

void CM249::ItemPostFrame()
{
        if( !m_fInReload )
                m_iVisibleClip = m_iClip;

        if( m_flReloadStage2Time > 0.0f && UTIL_WeaponTimeBase() >= m_flReloadStage2Time )
        {
                SendWeaponAnim( M249_RELOAD2, UseDecrement(), pev->body );
                m_flReloadStage2Time = 0.0f;
        }

        if( m_fInReload && m_pPlayer->m_flNextAttack <= UTIL_WeaponTimeBase() )
        {
                int maxClip;
#ifndef CLIENT_DLL
                maxClip = iMaxClip();
#else
                ItemInfo itemInfo;
                GetItemInfo( &itemInfo );
                maxClip = itemInfo.iMaxClip;
#endif
                int iWant     = maxClip - m_iClip;
                int iAvail    = m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType];
                int iGive     = ( iWant < iAvail ) ? iWant : iAvail;
                m_iVisibleClip = m_iClip + iGive;
                UpdateTape( m_iVisibleClip );
        }

        CBasePlayerWeapon::ItemPostFrame();
}

void CM249::WeaponIdle( void )
{
        ResetEmptySound();

        m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

        UpdateTape( m_iVisibleClip );

        if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
                return;

        float flRand = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 0.0f, 1.0f );
        int iAnim;
        if( flRand <= 0.8f )
        {
                iAnim = M249_SLOWIDLE;
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5.0f;
        }
        else
        {
                iAnim = M249_IDLE2;
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 155.0f / 25.0f;
        }

        SendWeaponAnim( iAnim, UseDecrement(), pev->body );

        m_flTimeWeaponIdle = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}

void CM249::UpdateTape()
{
        UpdateTape( m_iClip );
        m_iVisibleClip = m_iClip;
}

void CM249::UpdateTape( int clip )
{
        pev->body = BodyFromClip( clip );
}

int CM249::BodyFromClip()
{
        return BodyFromClip( m_iVisibleClip );
}

int CM249::BodyFromClip( int clip )
{
        if( clip == 0 )
                return 8;
        else if( clip > 0 && clip <= 8 )
                return 9 - clip;
        else
                return 0;
}

class CM249AmmoClip : public CBasePlayerAmmo
{
        void Spawn( void )
        {
                Precache();
                SET_MODEL( ENT( pev ), "models/saw_link.mdl" );
                CBasePlayerAmmo::Spawn();
        }
        void Precache( void )
        {
                PRECACHE_MODEL( "models/saw_link.mdl" );
                PRECACHE_SOUND( "items/9mmclip1.wav" );
        }
        BOOL AddAmmo( CBaseEntity *pOther )
        {
                int bResult = ( pOther->GiveAmmo( AMMO_556CLIP_GIVE, "556", _556_MAX_CARRY ) != -1 );
                if( bResult )
                        EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM );
                return bResult;
        }
};

LINK_ENTITY_TO_CLASS( ammo_556, CM249AmmoClip )
