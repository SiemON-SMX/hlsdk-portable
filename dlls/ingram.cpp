/***
*
*       Ingram MAC-10 (single)
*
*       Server side. Companion file is ingram_twin.cpp.
*       Animation sequence layout matches the v_ingram.mdl from HalfPayne
*       (https://github.com/suXinjke/HalfPayne) - the matching enum lives
*       in cl_dll/ev_hldm.h so both server and client stay in lockstep.
*
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"

#ifndef CLIENT_DLL
extern int gmsgWeapPickup;
#endif

enum ingram_seq_e
{
        ING_TWIN_IDLE = 0,
        ING_TWIN_IDLE_NOSHOT_BOTH,
        ING_TWIN_IDLE_NOSHOT_LEFT,
        ING_TWIN_IDLE_NOSHOT_RIGHT,

        ING_TWIN_SHOOT_BOTH_1,
        ING_TWIN_SHOOT_BOTH_2,
        ING_TWIN_SHOOT_BOTH_3,

        ING_TWIN_SHOOT_BOTH_THEN_EMPTY,
        ING_TWIN_SHOOT_BOTH_THEN_LEFT_EMPTY,
        ING_TWIN_SHOOT_BOTH_THEN_RIGHT_EMPTY,

        ING_TWIN_SHOOT_RIGHT_WHEN_LEFT_EMPTY_1,
        ING_TWIN_SHOOT_RIGHT_WHEN_LEFT_EMPTY_2,
        ING_TWIN_SHOOT_RIGHT_WHEN_LEFT_EMPTY_3,
        ING_TWIN_SHOOT_RIGHT_THEN_EMPTY_WHEN_LEFT_EMPTY,

        ING_TWIN_SHOOT_LEFT_WHEN_RIGHT_EMPTY_1,
        ING_TWIN_SHOOT_LEFT_WHEN_RIGHT_EMPTY_2,
        ING_TWIN_SHOOT_LEFT_WHEN_RIGHT_EMPTY_3,
        ING_TWIN_SHOOT_LEFT_THEN_EMPTY_WHEN_RIGHT_EMPTY,

        ING_TWIN_RELOAD,
        ING_TWIN_RELOAD_FAST,
        ING_TWIN_RELOAD_NOSHOT_BOTH,
        ING_TWIN_RELOAD_NOSHOT_BOTH_FAST,
        ING_TWIN_RELOAD_NOSHOT_LEFT,
        ING_TWIN_RELOAD_NOSHOT_LEFT_FAST,
        ING_TWIN_RELOAD_NOSHOT_RIGHT,
        ING_TWIN_RELOAD_NOSHOT_RIGHT_FAST,

        ING_TWIN_RELOAD_ONLY_LEFT,
        ING_TWIN_RELOAD_ONLY_LEFT_FAST,
        ING_TWIN_RELOAD_ONLY_LEFT_NOSHOT,
        ING_TWIN_RELOAD_ONLY_LEFT_NOSHOT_FAST,

        ING_TWIN_RELOAD_ONLY_RIGHT,
        ING_TWIN_RELOAD_ONLY_RIGHT_FAST,
        ING_TWIN_RELOAD_ONLY_RIGHT_NOSHOT,
        ING_TWIN_RELOAD_ONLY_RIGHT_NOSHOT_FAST,
        ING_TWIN_RELOAD_ONLY_RIGHT_NOSHOT_BOTH,
        ING_TWIN_RELOAD_ONLY_RIGHT_NOSHOT_BOTH_FAST,

        ING_TWIN_DRAW,
        ING_TWIN_DRAW_FAST,
        ING_TWIN_DRAW_NOSHOT_BOTH,
        ING_TWIN_DRAW_NOSHOT_BOTH_FAST,
        ING_TWIN_DRAW_NOSHOT_LEFT,
        ING_TWIN_DRAW_NOSHOT_LEFT_FAST,
        ING_TWIN_DRAW_NOSHOT_RIGHT,
        ING_TWIN_DRAW_NOSHOT_RIGHT_FAST,

        ING_TWIN_DRAW_ONLY_LEFT,
        ING_TWIN_DRAW_ONLY_LEFT_FAST,
        ING_TWIN_DRAW_NOSHOT_BOTH_ONLY_LEFT,
        ING_TWIN_DRAW_NOSHOT_BOTH_ONLY_LEFT_FAST,
        ING_TWIN_DRAW_NOSHOT_LEFT_ONLY_LEFT,
        ING_TWIN_DRAW_NOSHOT_LEFT_ONLY_LEFT_FAST,
        ING_TWIN_DRAW_NOSHOT_RIGHT_ONLY_LEFT,
        ING_TWIN_DRAW_NOSHOT_RIGHT_ONLY_LEFT_FAST,

        ING_IDLE,
        ING_IDLE_NOSHOT,

        ING_SHOOT_1,
        ING_SHOOT_2,
        ING_SHOOT_3,
        ING_SHOOT_EMPTY,

        ING_RELOAD_NOT_EMPTY,
        ING_RELOAD_NOT_EMPTY_FAST,
        ING_RELOAD,
        ING_RELOAD_FAST,

        ING_DRAW,
        ING_DRAW_FAST,
        ING_DRAW_NOSHOT,
        ING_DRAW_NOSHOT_FAST,

        ING_DRAW_FROM_TWIN,
        ING_DRAW_FROM_TWIN_FAST,

        ING_DRAW_FROM_TWIN_NOSHOT_BOTH,
        ING_DRAW_FROM_TWIN_NOSHOT_BOTH_FAST,

        ING_DRAW_FROM_TWIN_NOSHOT_LEFT,
        ING_DRAW_FROM_TWIN_NOSHOT_LEFT_FAST,

        ING_DRAW_FROM_TWIN_NOSHOT_RIGHT,
        ING_DRAW_FROM_TWIN_NOSHOT_RIGHT_FAST
};

LINK_ENTITY_TO_CLASS( weapon_ingram, CIngram )

void CIngram::Spawn( void )
{
        pev->classname = MAKE_STRING( "weapon_ingram" );
        Precache();
        m_iId = WEAPON_INGRAM;
        SET_MODEL( ENT( pev ), "models/w_ingram.mdl" );

        m_iDefaultAmmo = INGRAM_DEFAULT_GIVE;

        FallInit();
}

void CIngram::Precache( void )
{
        PRECACHE_MODEL( "models/v_ingram.mdl" );
        PRECACHE_MODEL( "models/w_ingram.mdl" );
        PRECACHE_MODEL( "models/p_9mmhandgun.mdl" );

        m_iShell = PRECACHE_MODEL( "models/shell.mdl" );

        PRECACHE_SOUND( "items/9mmclip1.wav" );
        PRECACHE_SOUND( "items/9mmclip2.wav" );
        PRECACHE_SOUND( "weapons/ingram_shot.wav" );
        PRECACHE_SOUND( "weapons/ingram_clip_out.wav" );
        PRECACHE_SOUND( "weapons/ingram_bolt_pull.wav" );

        m_usFireIngram = PRECACHE_EVENT( 1, "events/ingram.sc" );
}

int CIngram::GetItemInfo( ItemInfo *p )
{
        p->pszName = STRING( pev->classname );
        p->pszAmmo1 = "ingram9mm";
        p->iMaxAmmo1 = INGRAM_AMMO_MAX_CARRY;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = INGRAM_MAX_CLIP;
        p->iSlot = 2;
        p->iPosition = 3;
        p->iFlags = 0;
        p->iId = m_iId = WEAPON_INGRAM;
        p->iWeight = INGRAM_WEIGHT;

        return 1;
}

int CIngram::AddToPlayer( CBasePlayer *pPlayer )
{
        if( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
        {
                pPlayer->GiveAmmo( INGRAM_RESERVE_GIVE, "ingram9mm", INGRAM_AMMO_MAX_CARRY );

                MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
                        WRITE_BYTE( m_iId );
                MESSAGE_END();
                return TRUE;
        }
        return FALSE;
}

int CIngram::AddDuplicate( CBasePlayerItem *pOriginal )
{
#ifndef CLIENT_DLL
        if( !pOriginal )
                return CBasePlayerWeapon::AddDuplicate( pOriginal );

        CBasePlayer *pPlayer = static_cast<CBasePlayer *>( pOriginal->m_pPlayer );
        if( pPlayer && !pPlayer->HasNamedPlayerItem( "weapon_ingram_twin" ) )
        {
                CBaseEntity *pTwin = CBaseEntity::Create( "weapon_ingram_twin",
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

BOOL CIngram::Deploy( void )
{
        int iAnim = ( m_iClip <= 0 ) ? ING_DRAW_NOSHOT : ING_DRAW;

        // Check whether we are switching from the twin.
        // FNullEnt guards against a dangling m_pLastItem pointer that can occur
        // during client-side prediction resyncs on Android (causes a crash when
        // pev->classname is accessed on a freed entity).
        CIngramTwin *pTwin = NULL;
        if( m_pPlayer && m_pPlayer->m_pLastItem &&
            !FNullEnt( m_pPlayer->m_pLastItem->pev ) )
        {
                entvars_t *pPrevPev = m_pPlayer->m_pLastItem->pev;
                if( pPrevPev->classname && FClassnameIs( pPrevPev, "weapon_ingram_twin" ) )
                        pTwin = static_cast<CIngramTwin *>( m_pPlayer->m_pLastItem );
        }

        if( pTwin )
        {
                // Pick the correct draw-from-twin animation based on both clip states.
                // m_iClip  = right gun (tracked on this weapon via twin's m_iClip).
                // GetClipLeft() = left gun (only the twin knows this value).
                const bool bRightEmpty = ( m_iClip             <= 0 );
                const bool bLeftEmpty  = ( pTwin->GetClipLeft() <= 0 );

                if( bRightEmpty && bLeftEmpty )
                        iAnim = ING_DRAW_FROM_TWIN_NOSHOT_BOTH;
                else if( bLeftEmpty )
                        iAnim = ING_DRAW_FROM_TWIN;               // right loaded, left empty: standard draw
                else if( bRightEmpty )
                        iAnim = ING_DRAW_FROM_TWIN_NOSHOT_RIGHT;
                else
                        iAnim = ING_DRAW_FROM_TWIN;
        }
        // else iAnim already set above (ING_DRAW or ING_DRAW_NOSHOT)

        return DefaultDeploy( "models/v_ingram.mdl", "models/p_9mmhandgun.mdl",
                              iAnim, "onehanded", 0 );
}

void CIngram::Holster( int skiplocal )
{
        m_fInReload = FALSE;
        m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5f;

        m_flNextPrimaryAttack   = UTIL_WeaponTimeBase();
        m_flNextSecondaryAttack = UTIL_WeaponTimeBase();
}

void CIngram::PrimaryAttack( void )
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
        m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash  = NORMAL_GUN_FLASH;

        int flags;
#if CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        Vector vecSrc    = m_pPlayer->GetGunPosition();
        Vector vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

        Vector vecDir = m_pPlayer->FireBulletsPlayer(
                1, vecSrc, vecAiming,
                Vector( 0.02f, 0.02f, 0.02f ),
                8192, BULLET_PLAYER_9MM, 0, 0,
                m_pPlayer->pev, m_pPlayer->random_seed );

        // bparam1 = "clip empty after this shot?"
        PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(),
                m_usFireIngram,
                0.0, g_vecZero, g_vecZero,
                vecDir.x, vecDir.y, 0, 0,
                ( m_iClip == 0 ) ? 1 : 0, 0 );

        // Cycle ~625 RPM (Ingram MAC-10 territory).
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetNextAttackDelay( 0.06f );

        if( !m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 )
                m_pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() +
                UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}

void CIngram::SecondaryAttack( void )
{
        PrimaryAttack();
}

void CIngram::Reload( void )
{
        if( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip == INGRAM_MAX_CLIP )
                return;

        int iAnim;
        float flDur;
        if( m_iClip == 0 )
        {
                iAnim = ING_RELOAD;
                flDur = 1.867f;
        }
        else
        {
                iAnim = ING_RELOAD_NOT_EMPTY;
                flDur = 1.533f;
        }
        const int iResult = DefaultReload( INGRAM_MAX_CLIP, iAnim, flDur );

        if( iResult )
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() +
                        UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}

void CIngram::WeaponIdle( void )
{
        ResetEmptySound();
        m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

        if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
                return;

        SendWeaponAnim( ( m_iClip <= 0 ) ? ING_IDLE_NOSHOT : ING_IDLE );
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0f;
}
