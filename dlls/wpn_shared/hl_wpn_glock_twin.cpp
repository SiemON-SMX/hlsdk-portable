
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"

extern int gmsgTwinClip;

enum glock_twin_e
{
        GTWIN_IDLE = 0,                              // idle                          (both loaded)
        GTWIN_IDLE_NOSHOT_BOTH,                      // idle_noshot_both              (both empty)
        GTWIN_IDLE_NOSHOT_LEFT,                      // idle_noshot_left              (left empty)
        GTWIN_IDLE_NOSHOT_RIGHT,                     // idle_noshot_right             (right empty)

        GTWIN_SHOOT_RIGHT,
        GTWIN_SHOOT_RIGHT_FAST,
        GTWIN_SHOOT_RIGHT_WHEN_LEFT_EMPTY,
        GTWIN_SHOOT_RIGHT_WHEN_LEFT_EMPTY_FAST,
        GTWIN_SHOOT_RIGHT_THEN_EMPTY,
        GTWIN_SHOOT_RIGHT_THEN_EMPTY_FAST,
        GTWIN_SHOOT_RIGHT_THEN_EMPTY_WHEN_LEFT,
        GTWIN_SHOOT_RIGHT_THEN_EMPTY_WHEN_LEFT_FAST,

        GTWIN_SHOOT_LEFT,
        GTWIN_SHOOT_LEFT_FAST,
        GTWIN_SHOOT_LEFT_WHEN_RIGHT_EMPTY,
        GTWIN_SHOOT_LEFT_WHEN_RIGHT_EMPTY_FAST,
        GTWIN_SHOOT_LEFT_THEN_EMPTY,
        GTWIN_SHOOT_LEFT_THEN_EMPTY_FAST,
        GTWIN_SHOOT_LEFT_THEN_EMPTY_WHEN_RIGHT,
        GTWIN_SHOOT_LEFT_THEN_EMPTY_WHEN_RIGHT_FAST,

        GTWIN_RELOAD,                                // reload                        (full reload, both guns loaded)
        GTWIN_RELOAD_FAST,
        GTWIN_RELOAD_NOSHOT_BOTH,                    // reload_noshot_both            (both slides back)
        GTWIN_RELOAD_NOSHOT_BOTH_FAST,
        GTWIN_RELOAD_NOSHOT_LEFT,
        GTWIN_RELOAD_NOSHOT_LEFT_FAST,
        GTWIN_RELOAD_NOSHOT_RIGHT,
        GTWIN_RELOAD_NOSHOT_RIGHT_FAST,

        GTWIN_RELOAD_ONLY_LEFT,
        GTWIN_RELOAD_ONLY_LEFT_FAST,
        GTWIN_RELOAD_ONLY_LEFT_NOSHOT,
        GTWIN_RELOAD_ONLY_LEFT_NOSHOT_FAST,
        GTWIN_RELOAD_ONLY_LEFT_NOSHOT_BOTH,
        GTWIN_RELOAD_ONLY_LEFT_NOSHOT_BOTH_FAST,
        GTWIN_RELOAD_ONLY_RIGHT,
        GTWIN_RELOAD_ONLY_RIGHT_FAST,
        GTWIN_RELOAD_ONLY_RIGHT_NOSHOT,
        GTWIN_RELOAD_ONLY_RIGHT_NOSHOT_FAST,
        GTWIN_RELOAD_ONLY_RIGHT_NOSHOT_BOTH,
        GTWIN_RELOAD_ONLY_RIGHT_NOSHOT_BOTH_FAST,

        GTWIN_DRAW,                                  // draw
        GTWIN_DRAW_FAST,
        GTWIN_DRAW_NOSHOT_BOTH,
        GTWIN_DRAW_NOSHOT_BOTH_FAST,
        GTWIN_DRAW_NOSHOT_LEFT,
        GTWIN_DRAW_NOSHOT_LEFT_FAST,
        GTWIN_DRAW_NOSHOT_RIGHT,
        GTWIN_DRAW_NOSHOT_RIGHT_FAST,

        GTWIN_DRAW_ONLY_LEFT,                        // draw_only_left                (right is already in hand from single)
        GTWIN_DRAW_ONLY_LEFT_FAST,
        GTWIN_DRAW_NOSHOT_BOTH_ONLY_LEFT,
        GTWIN_DRAW_NOSHOT_BOTH_ONLY_LEFT_FAST,
        GTWIN_DRAW_NOSHOT_LEFT_ONLY_LEFT,
        GTWIN_DRAW_NOSHOT_LEFT_ONLY_LEFT_FAST,
        GTWIN_DRAW_NOSHOT_RIGHT_ONLY_LEFT,
        GTWIN_DRAW_NOSHOT_RIGHT_ONLY_LEFT_FAST
};

LINK_ENTITY_TO_CLASS( weapon_9mmhandgun_twin, CGlockTwin )
LINK_ENTITY_TO_CLASS( weapon_glock_twin,      CGlockTwin )

void CGlockTwin::Spawn()
{
        pev->classname = MAKE_STRING( "weapon_9mmhandgun_twin" );
        Precache();
        m_iId = WEAPON_GLOCK_TWIN;
        SET_MODEL( ENT( pev ), "models/w_9mmhandgun.mdl" );

        m_iDefaultAmmo = GLOCK_TWIN_DEFAULT_GIVE;
        m_iClipLeft = 0;
        m_iLastSentClipLeft = -1;
        m_bFireLeftNext = FALSE;

        FallInit();
}

void CGlockTwin::Precache( void )
{
        PRECACHE_MODEL( "models/v_9mmhandgun_twin.mdl" );
        PRECACHE_MODEL( "models/w_9mmhandgun.mdl" );
        PRECACHE_MODEL( "models/p_9mmhandgun.mdl" );

        m_iShell = PRECACHE_MODEL( "models/shell.mdl" );

        PRECACHE_SOUND( "items/9mmclip1.wav" );
        PRECACHE_SOUND( "items/9mmclip2.wav" );

        PRECACHE_SOUND( "weapons/pl_gun1.wav" );
        PRECACHE_SOUND( "weapons/pl_gun2.wav" );
        PRECACHE_SOUND( "weapons/pl_gun3.wav" );

        m_usFireGlockTwin = PRECACHE_EVENT( 1, "events/glock_twin.sc" );
}

int CGlockTwin::GetItemInfo( ItemInfo *p )
{
        p->pszName = STRING( pev->classname );
        p->pszAmmo1 = "9mm";
        p->iMaxAmmo1 = _9MM_MAX_CARRY;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = GLOCK_TWIN_MAX_CLIP;
        p->iSlot = 1;
        p->iPosition = 2;
        p->iFlags = 0;
        p->iId = m_iId = WEAPON_GLOCK_TWIN;
        p->iWeight = GLOCK_TWIN_WEIGHT;

        return 1;
}

int CGlockTwin::AddToPlayer( CBasePlayer *pPlayer )
{
        if( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
        {
                if( m_iClipLeft <= 0 )
                        m_iClipLeft = GLOCK_TWIN_MAX_CLIP;
                if( m_iClip <= 0 )
                        m_iClip = GLOCK_TWIN_MAX_CLIP;

                MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
                        WRITE_BYTE( m_iId );
                MESSAGE_END();
                return TRUE;
        }
        return FALSE;
}

BOOL CGlockTwin::Deploy()
{
        int iAnim;
        const bool bRightEmpty = ( m_iClip <= 0 );
        const bool bLeftEmpty  = ( m_iClipLeft <= 0 );

        bool bFromSingle = false;
        if( m_pPlayer && m_pPlayer->m_pLastItem )
        {
                entvars_t *pPrevPev = m_pPlayer->m_pLastItem->pev;
                if( pPrevPev && pPrevPev->classname )
                        bFromSingle = FClassnameIs( pPrevPev, "weapon_9mmhandgun" );
        }

        if( bFromSingle )
        {
                if( bRightEmpty && bLeftEmpty )      iAnim = GTWIN_DRAW_NOSHOT_BOTH_ONLY_LEFT;
                else if( bLeftEmpty )                iAnim = GTWIN_DRAW_NOSHOT_LEFT_ONLY_LEFT;
                else if( bRightEmpty )               iAnim = GTWIN_DRAW_NOSHOT_RIGHT_ONLY_LEFT;
                else                                 iAnim = GTWIN_DRAW_ONLY_LEFT;
        }
        else
        {
                if( bRightEmpty && bLeftEmpty )      iAnim = GTWIN_DRAW_NOSHOT_BOTH;
                else if( bLeftEmpty )                iAnim = GTWIN_DRAW_NOSHOT_LEFT;
                else if( bRightEmpty )               iAnim = GTWIN_DRAW_NOSHOT_RIGHT;
                else                                 iAnim = GTWIN_DRAW;
        }

        return DefaultDeploy( "models/v_9mmhandgun_twin.mdl", "models/p_9mmhandgun.mdl", iAnim, "onehanded", 0 );
}

void CGlockTwin::Holster( int skiplocal /* = 0 */ )
{
        m_fInReload = FALSE;

        m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.8f;

        m_flNextPrimaryAttack   = UTIL_WeaponTimeBase();
        m_flNextSecondaryAttack = UTIL_WeaponTimeBase();
}

int CGlockTwin::PickIdleAnim( void )
{
        const bool bRightEmpty = ( m_iClip <= 0 );
        const bool bLeftEmpty  = ( m_iClipLeft <= 0 );

        if( bRightEmpty && bLeftEmpty ) return GTWIN_IDLE_NOSHOT_BOTH;
        if( bLeftEmpty )                return GTWIN_IDLE_NOSHOT_LEFT;
        if( bRightEmpty )               return GTWIN_IDLE_NOSHOT_RIGHT;
        return GTWIN_IDLE;
}

void CGlockTwin::PrimaryAttack( void )
{
        FireOne( 0.01f, 0.2f,  TRUE );
}

void CGlockTwin::SecondaryAttack( void )
{
        FireOne( 0.1f, 0.15f,  FALSE );
}

void CGlockTwin::FireOne( float flSpread, float flCycleTime, BOOL fUseAutoAim )
{
        const bool bRightHasAmmo = ( m_iClip      > 0 );
        const bool bLeftHasAmmo  = ( m_iClipLeft  > 0 );

        if( !bRightHasAmmo && !bLeftHasAmmo )
        {
                if( m_fFireOnEmpty )
                {
                        PlayEmptySound();
                        m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetNextAttackDelay( 0.2f );
                }
                return;
        }

        bool bFireLeft;
        if( m_bFireLeftNext && bLeftHasAmmo )
                bFireLeft = true;
        else if( !m_bFireLeftNext && bRightHasAmmo )
                bFireLeft = false;
        else
                bFireLeft = bLeftHasAmmo;

        m_bFireLeftNext = !bFireLeft;

        int  iSide;            // 0 = right, 1 = left  (matches event bparam2)
        bool bThisSideEmpty;
        bool bOtherSideEmpty;

        if( bFireLeft )
        {
                m_iClipLeft--;
                iSide           = 1;
                bThisSideEmpty  = ( m_iClipLeft == 0 );
                bOtherSideEmpty = ( m_iClip     == 0 );
        }
        else
        {
                m_iClip--;
                iSide           = 0;
                bThisSideEmpty  = ( m_iClip     == 0 );
                bOtherSideEmpty = ( m_iClipLeft == 0 );
        }

        int iAnim;
        if( bFireLeft )
        {
                if( bThisSideEmpty && bOtherSideEmpty )       iAnim = GTWIN_SHOOT_LEFT_THEN_EMPTY_WHEN_RIGHT;
                else if( bThisSideEmpty )                     iAnim = GTWIN_SHOOT_LEFT_THEN_EMPTY;
                else if( bOtherSideEmpty )                    iAnim = GTWIN_SHOOT_LEFT_WHEN_RIGHT_EMPTY;
                else                                          iAnim = GTWIN_SHOOT_LEFT;
        }
        else
        {
                if( bThisSideEmpty && bOtherSideEmpty )       iAnim = GTWIN_SHOOT_RIGHT_THEN_EMPTY_WHEN_LEFT;
                else if( bThisSideEmpty )                     iAnim = GTWIN_SHOOT_RIGHT_THEN_EMPTY;
                else if( bOtherSideEmpty )                    iAnim = GTWIN_SHOOT_RIGHT_WHEN_LEFT_EMPTY;
                else                                          iAnim = GTWIN_SHOOT_RIGHT;
        }

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

        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecAiming = fUseAutoAim
                ? m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES )
                : gpGlobals->v_forward;

        Vector vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming,
                Vector( flSpread, flSpread, flSpread ), 8192, BULLET_PLAYER_9MM, 0, 0,
                m_pPlayer->pev, m_pPlayer->random_seed + ( bFireLeft ? 1 : 0 ) );

        // bparam1 = "this side now empty?"   bparam2 = side (0=right, 1=left)
        PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(),
                m_usFireGlockTwin,
                0.0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, 0, 0,
                bThisSideEmpty ? 1 : 0, iSide );

        SendWeaponAnim( iAnim );

        m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetNextAttackDelay( flCycleTime );

        if( !m_iClip && !m_iClipLeft && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 )
                m_pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}

void CGlockTwin::Reload( void )
{
        if( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 )
                return;

        const int iNeedRight = GLOCK_MAX_CLIP - m_iClip;
        const int iNeedLeft  = GLOCK_MAX_CLIP - m_iClipLeft;

        if( iNeedRight <= 0 && iNeedLeft <= 0 )
                return;

        const bool bRightEmpty = ( m_iClip <= 0 );
        const bool bLeftEmpty  = ( m_iClipLeft <= 0 );

        int iAnim;
        float flDuration;

        if( iNeedRight > 0 && iNeedLeft > 0 )
        {
                if( bRightEmpty && bLeftEmpty )      iAnim = GTWIN_RELOAD_NOSHOT_BOTH;
                else if( bLeftEmpty )                iAnim = GTWIN_RELOAD_NOSHOT_LEFT;
                else if( bRightEmpty )               iAnim = GTWIN_RELOAD_NOSHOT_RIGHT;
                else                                 iAnim = GTWIN_RELOAD;
                flDuration = 3.208f;
        }
        else if( iNeedLeft > 0 )
        {
                if( bLeftEmpty && bRightEmpty )      iAnim = GTWIN_RELOAD_ONLY_LEFT_NOSHOT_BOTH;
                else if( bLeftEmpty )                iAnim = GTWIN_RELOAD_ONLY_LEFT_NOSHOT;
                else                                 iAnim = GTWIN_RELOAD_ONLY_LEFT;
                flDuration = 2.375f;
        }
        else
        {
                if( bRightEmpty && bLeftEmpty )      iAnim = GTWIN_RELOAD_ONLY_RIGHT_NOSHOT_BOTH;
                else if( bRightEmpty )               iAnim = GTWIN_RELOAD_ONLY_RIGHT_NOSHOT;
                else                                 iAnim = GTWIN_RELOAD_ONLY_RIGHT;
                flDuration = 2.375f;
        }

        int iAvailable = m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType];
        int iGiveRight = ( iNeedRight > 0 ) ? ( iNeedRight < iAvailable ? iNeedRight : iAvailable ) : 0;
        iAvailable -= iGiveRight;
        int iGiveLeft  = ( iNeedLeft  > 0 ) ? ( iNeedLeft  < iAvailable ? iNeedLeft  : iAvailable ) : 0;

        m_iClip                                  += iGiveRight;
        m_iClipLeft                              += iGiveLeft;
        m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]  -= ( iGiveRight + iGiveLeft );

        SendWeaponAnim( iAnim );

        m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + flDuration;
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle =
                UTIL_WeaponTimeBase() + flDuration;
}

void CGlockTwin::ItemPostFrame( void )
{
#ifndef CLIENT_DLL
        if( m_pPlayer )
        {
                // m_iClipLeft == N, the second player's HUD never gets updated
                // On respawn / weapon re-pickup the static also suppresses the
                if( m_iClipLeft != m_iLastSentClipLeft && gmsgTwinClip )
                {
                        m_iLastSentClipLeft = m_iClipLeft;
                        MESSAGE_BEGIN( MSG_ONE, gmsgTwinClip, NULL, m_pPlayer->pev );
                                WRITE_BYTE( m_iClipLeft );
                        MESSAGE_END();
                }
        }
#endif

        CBasePlayerWeapon::ItemPostFrame();
}

void CGlockTwin::WeaponIdle( void )
{
        ResetEmptySound();
        m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

        if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
                return;

        SendWeaponAnim( PickIdleAnim(), 1 );
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0f;
}
