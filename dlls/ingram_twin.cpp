/***
*
*       Twin Ingrams (akimbo MAC-10s)
*
*       Companion to ingram.cpp. Tracks two independent clips (right + left)
*       and fires from BOTH guns simultaneously per trigger pull, mirroring the
*       HalfPayne behavior. Animation enum lives in cl_dll/ev_hldm.h.
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
extern int gmsgTwinClip;
#endif

// Sequence indices for v_ingram.mdl (mirrored from ingram.cpp).
enum ingram_twin_seq_e
{
        ITWIN_IDLE = 0,
        ITWIN_IDLE_NOSHOT_BOTH,
        ITWIN_IDLE_NOSHOT_LEFT,
        ITWIN_IDLE_NOSHOT_RIGHT,

        ITWIN_SHOOT_BOTH_1,
        ITWIN_SHOOT_BOTH_2,
        ITWIN_SHOOT_BOTH_3,

        ITWIN_SHOOT_BOTH_THEN_EMPTY,
        ITWIN_SHOOT_BOTH_THEN_LEFT_EMPTY,
        ITWIN_SHOOT_BOTH_THEN_RIGHT_EMPTY,

        ITWIN_SHOOT_RIGHT_WHEN_LEFT_EMPTY_1,
        ITWIN_SHOOT_RIGHT_WHEN_LEFT_EMPTY_2,
        ITWIN_SHOOT_RIGHT_WHEN_LEFT_EMPTY_3,
        ITWIN_SHOOT_RIGHT_THEN_EMPTY_WHEN_LEFT_EMPTY,

        ITWIN_SHOOT_LEFT_WHEN_RIGHT_EMPTY_1,
        ITWIN_SHOOT_LEFT_WHEN_RIGHT_EMPTY_2,
        ITWIN_SHOOT_LEFT_WHEN_RIGHT_EMPTY_3,
        ITWIN_SHOOT_LEFT_THEN_EMPTY_WHEN_RIGHT_EMPTY,

        ITWIN_RELOAD,
        ITWIN_RELOAD_FAST,
        ITWIN_RELOAD_NOSHOT_BOTH,
        ITWIN_RELOAD_NOSHOT_BOTH_FAST,
        ITWIN_RELOAD_NOSHOT_LEFT,
        ITWIN_RELOAD_NOSHOT_LEFT_FAST,
        ITWIN_RELOAD_NOSHOT_RIGHT,
        ITWIN_RELOAD_NOSHOT_RIGHT_FAST,

        ITWIN_RELOAD_ONLY_LEFT,
        ITWIN_RELOAD_ONLY_LEFT_FAST,
        ITWIN_RELOAD_ONLY_LEFT_NOSHOT,
        ITWIN_RELOAD_ONLY_LEFT_NOSHOT_FAST,

        ITWIN_RELOAD_ONLY_RIGHT,
        ITWIN_RELOAD_ONLY_RIGHT_FAST,
        ITWIN_RELOAD_ONLY_RIGHT_NOSHOT,
        ITWIN_RELOAD_ONLY_RIGHT_NOSHOT_FAST,
        ITWIN_RELOAD_ONLY_RIGHT_NOSHOT_BOTH,
        ITWIN_RELOAD_ONLY_RIGHT_NOSHOT_BOTH_FAST,

        ITWIN_DRAW,
        ITWIN_DRAW_FAST,
        ITWIN_DRAW_NOSHOT_BOTH,
        ITWIN_DRAW_NOSHOT_BOTH_FAST,
        ITWIN_DRAW_NOSHOT_LEFT,
        ITWIN_DRAW_NOSHOT_LEFT_FAST,
        ITWIN_DRAW_NOSHOT_RIGHT,
        ITWIN_DRAW_NOSHOT_RIGHT_FAST,

        ITWIN_DRAW_ONLY_LEFT,
        ITWIN_DRAW_ONLY_LEFT_FAST,
        ITWIN_DRAW_NOSHOT_BOTH_ONLY_LEFT,
        ITWIN_DRAW_NOSHOT_BOTH_ONLY_LEFT_FAST,
        ITWIN_DRAW_NOSHOT_LEFT_ONLY_LEFT,
        ITWIN_DRAW_NOSHOT_LEFT_ONLY_LEFT_FAST,
        ITWIN_DRAW_NOSHOT_RIGHT_ONLY_LEFT,
        ITWIN_DRAW_NOSHOT_RIGHT_ONLY_LEFT_FAST
};

LINK_ENTITY_TO_CLASS( weapon_ingram_twin, CIngramTwin )

void CIngramTwin::Spawn( void )
{
        pev->classname = MAKE_STRING( "weapon_ingram_twin" );
        Precache();
        m_iId = WEAPON_INGRAM_TWIN;
        SET_MODEL( ENT( pev ), "models/w_ingram.mdl" );

        m_iDefaultAmmo = INGRAM_TWIN_DEFAULT_GIVE;
        m_iClipLeft = 0;
        m_iLastSentClipLeft = -1;

        FallInit();
}

void CIngramTwin::Precache( void )
{
        PRECACHE_MODEL( "models/v_ingram.mdl" );
        PRECACHE_MODEL( "models/w_ingram.mdl" );
        PRECACHE_MODEL( "models/p_9mmhandgun.mdl" );

        m_iShell = PRECACHE_MODEL( "models/shell.mdl" );

        PRECACHE_SOUND( "items/9mmclip1.wav" );
        PRECACHE_SOUND( "items/9mmclip2.wav" );
        PRECACHE_SOUND( "weapons/ingram_shot.wav" );
        PRECACHE_SOUND( "weapons/ingram_clip_out.wav" );
        PRECACHE_SOUND( "weapons/ingram_clip_out_twin.wav" );
        PRECACHE_SOUND( "weapons/ingram_bolt_pull.wav" );

        m_usFireIngramTwin       = PRECACHE_EVENT( 1, "events/ingram_twin.sc" );
        m_usFireIngramTwinTracer = PRECACHE_EVENT( 1, "events/ingram_twin_tracer.sc" );
}

int CIngramTwin::GetItemInfo( ItemInfo *p )
{
        p->pszName = STRING( pev->classname );
        p->pszAmmo1 = "ingram9mm";
        p->iMaxAmmo1 = INGRAM_AMMO_MAX_CARRY;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = INGRAM_TWIN_MAX_CLIP;   // right; left tracked via m_iClipLeft
        p->iSlot = 2;
        p->iPosition = 4;
        p->iFlags = 0;
        p->iId = m_iId = WEAPON_INGRAM_TWIN;
        p->iWeight = INGRAM_TWIN_WEIGHT;

        return 1;
}

int CIngramTwin::AddToPlayer( CBasePlayer *pPlayer )
{
        if( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
        {
                if( m_iClipLeft <= 0 )
                        m_iClipLeft = INGRAM_TWIN_MAX_CLIP;
                if( m_iClip <= 0 )
                        m_iClip = INGRAM_TWIN_MAX_CLIP;

                pPlayer->GiveAmmo( INGRAM_TWIN_RESERVE_GIVE, "ingram9mm",
                                   INGRAM_AMMO_MAX_CARRY );

                MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
                        WRITE_BYTE( m_iId );
                MESSAGE_END();
                return TRUE;
        }
        return FALSE;
}

int CIngramTwin::PickIdleAnim( void )
{
        const bool bRightEmpty = ( m_iClip     <= 0 );
        const bool bLeftEmpty  = ( m_iClipLeft <= 0 );

        if( bRightEmpty && bLeftEmpty ) return ITWIN_IDLE_NOSHOT_BOTH;
        if( bLeftEmpty )                return ITWIN_IDLE_NOSHOT_LEFT;
        if( bRightEmpty )               return ITWIN_IDLE_NOSHOT_RIGHT;
        return ITWIN_IDLE;
}

BOOL CIngramTwin::Deploy( void )
{
        bool bFromSingle = false;
        if( m_pPlayer && m_pPlayer->m_pLastItem )
        {
                entvars_t *pPrevPev = m_pPlayer->m_pLastItem->pev;
                if( pPrevPev && pPrevPev->classname )
                        bFromSingle = FClassnameIs( pPrevPev, "weapon_ingram" );
        }

        const bool bRightEmpty = ( m_iClip     <= 0 );
        const bool bLeftEmpty  = ( m_iClipLeft <= 0 );

        int iAnim;
        if( bFromSingle )
        {
                if( bRightEmpty && bLeftEmpty )      iAnim = ITWIN_DRAW_NOSHOT_BOTH_ONLY_LEFT;
                else if( bRightEmpty )               iAnim = ITWIN_DRAW_NOSHOT_RIGHT_ONLY_LEFT;
                else if( bLeftEmpty )                iAnim = ITWIN_DRAW_NOSHOT_LEFT_ONLY_LEFT;
                else                                 iAnim = ITWIN_DRAW_ONLY_LEFT;
        }
        else
        {
                if( bRightEmpty && bLeftEmpty )      iAnim = ITWIN_DRAW_NOSHOT_BOTH;
                else if( bRightEmpty )               iAnim = ITWIN_DRAW_NOSHOT_RIGHT;
                else if( bLeftEmpty )                iAnim = ITWIN_DRAW_NOSHOT_LEFT;
                else                                 iAnim = ITWIN_DRAW;
        }

        return DefaultDeploy( "models/v_ingram.mdl", "models/p_9mmhandgun.mdl",
                              iAnim, "onehanded", 0 );
}

void CIngramTwin::Holster( int skiplocal )
{
        m_fInReload = FALSE;
        m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5f;

        m_flNextPrimaryAttack   = UTIL_WeaponTimeBase();
        m_flNextSecondaryAttack = UTIL_WeaponTimeBase();
}

void CIngramTwin::PrimaryAttack( void )
{
        const bool bRightHasAmmo = ( m_iClip     > 0 );
        const bool bLeftHasAmmo  = ( m_iClipLeft > 0 );

        if( !bRightHasAmmo && !bLeftHasAmmo )
        {
                if( m_fFireOnEmpty )
                {
                        PlayEmptySound();
                        m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetNextAttackDelay( 0.2f );
                        SendWeaponAnim( PickIdleAnim() );
                }
                return;
        }

        // Both pistols fire on the same trigger pull when loaded.
        if( bRightHasAmmo )
                m_iClip--;
        if( bLeftHasAmmo )
                m_iClipLeft--;

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
        Vector vecDirR( 0, 0, 0 ), vecDirL( 0, 0, 0 );

        if( bRightHasAmmo )
                vecDirR = m_pPlayer->FireBulletsPlayer(
                        1, vecSrc, vecAiming,
                        Vector( 0.025f, 0.025f, 0.025f ),
                        8192, BULLET_PLAYER_9MM, 0, 0,
                        m_pPlayer->pev, m_pPlayer->random_seed );
        if( bLeftHasAmmo )
                vecDirL = m_pPlayer->FireBulletsPlayer(
                        1, vecSrc, vecAiming,
                        Vector( 0.025f, 0.025f, 0.025f ),
                        8192, BULLET_PLAYER_9MM, 0, 0,
                        m_pPlayer->pev, m_pPlayer->random_seed + 22 );

        const int iSide = ( bRightHasAmmo && bLeftHasAmmo ) ? 2
                        : ( bLeftHasAmmo ? 1 : 0 );

        // iparam1 packs per-side post-shot empty state: bit0=right empty, bit1=left empty.
        int iEmptyFlags = 0;
        if( m_iClip     == 0 ) iEmptyFlags |= 1;
        if( m_iClipLeft == 0 ) iEmptyFlags |= 2;

        PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(),
                m_usFireIngramTwin,
                0.0, g_vecZero, g_vecZero,
                0.0f, 0.0f, iEmptyFlags, 0,
                0, iSide );

        if( bRightHasAmmo )
                PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(),
                        m_usFireIngramTwinTracer,
                        0.0, g_vecZero, g_vecZero,
                        vecDirR.x, vecDirR.y, 0, 0,
                        0, 1 /* right */ );
        if( bLeftHasAmmo )
                PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(),
                        m_usFireIngramTwinTracer,
                        0.0, g_vecZero, g_vecZero,
                        vecDirL.x, vecDirL.y, 0, 0,
                        0, 0 /* left */ );

        m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetNextAttackDelay( 0.07f );

        if( !m_iClip && !m_iClipLeft && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 )
                m_pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 );

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() +
                UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}

void CIngramTwin::SecondaryAttack( void )
{
        PrimaryAttack();
}

void CIngramTwin::Reload( void )
{
        if( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 )
                return;

        const int iNeedRight = INGRAM_TWIN_MAX_CLIP - m_iClip;
        const int iNeedLeft  = INGRAM_TWIN_MAX_CLIP - m_iClipLeft;

        if( iNeedRight <= 0 && iNeedLeft <= 0 )
                return;

        const bool bRightEmpty = ( m_iClip     <= 0 );
        const bool bLeftEmpty  = ( m_iClipLeft <= 0 );

        int   iAnim;
        float flDuration;

        if( iNeedRight > 0 && iNeedLeft > 0 )
        {
                if( bRightEmpty && bLeftEmpty )
                { iAnim = ITWIN_RELOAD_NOSHOT_BOTH;  flDuration = 3.233f; }
                else if( bLeftEmpty )
                { iAnim = ITWIN_RELOAD_NOSHOT_LEFT;  flDuration = 2.900f; }
                else if( bRightEmpty )
                { iAnim = ITWIN_RELOAD_NOSHOT_RIGHT; flDuration = 2.900f; }
                else
                { iAnim = ITWIN_RELOAD;              flDuration = 2.567f; }
        }
        else if( iNeedLeft > 0 )
        {
                if( bLeftEmpty )
                { iAnim = ITWIN_RELOAD_ONLY_LEFT_NOSHOT; flDuration = 2.233f; }
                else
                { iAnim = ITWIN_RELOAD_ONLY_LEFT;        flDuration = 1.900f; }
        }
        else
        {
                if( bRightEmpty && bLeftEmpty )
                { iAnim = ITWIN_RELOAD_ONLY_RIGHT_NOSHOT_BOTH; flDuration = 2.233f; }
                else if( bRightEmpty )
                { iAnim = ITWIN_RELOAD_ONLY_RIGHT_NOSHOT;      flDuration = 2.233f; }
                else
                { iAnim = ITWIN_RELOAD_ONLY_RIGHT;             flDuration = 1.900f; }
        }

        // Manual ammo move (DefaultReload only refills one clip).
        int iAvailable = m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType];
        int iGiveRight = ( iNeedRight > 0 ) ? ( iNeedRight < iAvailable ? iNeedRight : iAvailable ) : 0;
        iAvailable -= iGiveRight;
        int iGiveLeft  = ( iNeedLeft  > 0 ) ? ( iNeedLeft  < iAvailable ? iNeedLeft  : iAvailable ) : 0;

        m_iClip                                 += iGiveRight;
        m_iClipLeft                             += iGiveLeft;
        m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= ( iGiveRight + iGiveLeft );

        SendWeaponAnim( iAnim );

        m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + flDuration;
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle =
                UTIL_WeaponTimeBase() + flDuration;
}

void CIngramTwin::ItemPostFrame( void )
{
#ifndef CLIENT_DLL
        if( m_pPlayer )
        {
                // Use a per-instance member instead of a static local so that
                // multiple CIngramTwin instances (e.g. multiplayer or after
                // respawn) each track their own last-sent value independently.
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

void CIngramTwin::WeaponIdle( void )
{
        ResetEmptySound();
        m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

        if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
                return;

        SendWeaponAnim( PickIdleAnim(), 1 );
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0f;
}
