/***
*
*       Copyright (c) 1996-2001, Valve LLC. All rights reserved.
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
#if !defined( OEM_BUILD ) && !defined( HLDEMO_BUILD )

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "skill.h"
#include "monsters.h"
#include "player.h"
#include "gamerules.h"

// FOV while +attack2 is held — matches python.cpp zoom for consistency
#define BUFFALO_ZOOM_FOV  20

enum buffalo_e {
        BUFFALO_DRAW = 0,
        BUFFALO_HOLSTER,
        BUFFALO_IDLE1,
        BUFFALO_IDLE2,
        BUFFALO_FIDGET,
        BUFFALO_FIRE,
        BUFFALO_DRYFIRE,
        BUFFALO_RELOAD
};

LINK_ENTITY_TO_CLASS(weapon_buffalo, CBuffalo);

int CBuffalo::GetItemInfo(ItemInfo *p)
{
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "buffalo";
        p->iMaxAmmo1 = BUFFALO_MAX_CARRY;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = BUFFALO_MAX_CLIP;
        p->iFlags = 0;
        p->iSlot = 2;
        p->iPosition = 2;
        p->iId = m_iId = WEAPON_BUFFALO;
        p->iWeight = BUFFALO_WEIGHT;

        return 1;
}

int CBuffalo::AddToPlayer(CBasePlayer *pPlayer)
{
        if (CBasePlayerWeapon::AddToPlayer(pPlayer))
        {
                MESSAGE_BEGIN(MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev);
                WRITE_BYTE(m_iId);
                MESSAGE_END();
                return TRUE;
        }
        return FALSE;
}

void CBuffalo::Spawn()
{
        Precache();
        m_iId = WEAPON_BUFFALO;
        SET_MODEL(ENT(pev), "models/w_buffalogun.mdl");

        m_iDefaultAmmo = BUFFALO_DEFAULT_GIVE;

        FallInit();// get ready to fall down.
}

void CBuffalo::Precache(void)
{
        PRECACHE_MODEL("models/v_buffalogun.mdl");
        PRECACHE_MODEL("models/w_buffalogun.mdl");
        PRECACHE_MODEL("models/p_buffalogun.mdl");

        PRECACHE_MODEL("models/w_buffalobox.mdl");
        PRECACHE_SOUND("items/9mmclip1.wav");

        PRECACHE_SOUND("weapons/buffalo_breakopen.wav");
        PRECACHE_SOUND("weapons/buffalo_close.wav");
        PRECACHE_SOUND("weapons/buffalo_dryfire.wav");
        PRECACHE_SOUND("weapons/buffalo_reload.wav");
        PRECACHE_SOUND("weapons/buffalo_shoot1.wav");
        PRECACHE_SOUND("weapons/buffalo_shoot2.wav");
        PRECACHE_SOUND("weapons/357_cock1.wav");

        m_usBuffalo = PRECACHE_EVENT(1, "events/buffalo.sc");
}

BOOL CBuffalo::Deploy()
{
        return DefaultDeploy("models/v_buffalogun.mdl", "models/p_buffalogun.mdl", BUFFALO_DRAW, "buffalo", UseDecrement());
}

void CBuffalo::Holster(int skiplocal /* = 0 */)
{
        m_fInReload = FALSE;// cancel any reload in progress.

        // Always reset zoom when holstering so the next weapon gets normal FOV.
        if( m_fInZoom )
        {
                m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0;
                m_pPlayer->pev->viewmodel = MAKE_STRING( "models/v_buffalogun.mdl" );
                m_fInZoom = FALSE;
        }

        m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.0;
        m_flTimeWeaponIdle = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
        SendWeaponAnim(BUFFALO_HOLSTER);
}

void CBuffalo::SecondaryAttack()
{
        // Toggle zoom: first press zooms in, second press zooms out.
        // A 0.4 s cooldown prevents the state from flipping rapidly while
        // the button is held down.
        if( !m_fInZoom )
        {
                m_pPlayer->pev->fov = m_pPlayer->m_iFOV = BUFFALO_ZOOM_FOV;
                m_pPlayer->pev->viewmodel = iStringNull; // hide viewmodel while scoped
                m_fInZoom = TRUE;
        }
        else
        {
                m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0;
                m_pPlayer->pev->viewmodel = MAKE_STRING( "models/v_buffalogun.mdl" );
                m_fInZoom = FALSE;
        }

        m_flNextSecondaryAttack = GetNextAttackDelay( 0.4f );
}

void CBuffalo::PrimaryAttack()
{
        // don't fire underwater
        if (m_pPlayer->pev->waterlevel == 3)
        {
                PlayEmptySound();
                m_flNextPrimaryAttack = GetNextAttackDelay( 0.15 );
                return;
        }

        if (m_iClip <= 0)
        {
                // Dry fire only when BOTH clip AND reserve ammo are empty (0/0)
                if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
                {
                        if (m_fFireOnEmpty)
                        {
                                // sequence[6] = dryfire  15fps 10fr = 0.67 s
                                SendWeaponAnim( BUFFALO_DRYFIRE, UseDecrement() );
                                EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/buffalo_dryfire.wav", 0.8, ATTN_NORM);
                                m_flNextPrimaryAttack = GetNextAttackDelay( 0.67 );
                        }
                }
                else
                {
                        Reload();
                }

                return;
        }

        m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

        m_iClip--;

        m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

        // Player body shoot animation
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);

        // Fire animation — direct so it always plays on Android
        SendWeaponAnim(BUFFALO_FIRE, UseDecrement());

        // Play fire sound directly — reliable on all platforms including Android.
        // The event (EV_WHL_Buffalo) handles visuals only (muzzle flash, shell, view punch).
        EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON,
                RANDOM_LONG(0, 1) ? "weapons/buffalo_shoot1.wav" : "weapons/buffalo_shoot2.wav",
                RANDOM_FLOAT(0.95f, 1.0f), ATTN_NORM, 0, 88 + RANDOM_LONG(0, 0x1f));

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

        Vector vecDir;
        vecDir = m_pPlayer->FireBulletsPlayer(1, vecSrc, vecAiming, VECTOR_CONE_1DEGREES, 8192, BULLET_PLAYER_357, 0, (int)gSkillData.plrDmgBuffalo, m_pPlayer->pev, m_pPlayer->random_seed);

        int flags;
#if defined( CLIENT_WEAPONS )
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usBuffalo, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0);

#ifndef CLIENT_DLL
        Vector vecSmokeOrigin;
        UTIL_MakeAimVectors(m_pPlayer->pev->v_angle);

        vecSmokeOrigin = m_pPlayer->GetGunPosition() +
                                         gpGlobals->v_forward * 16 +
                                         gpGlobals->v_right * 4 +
                                         gpGlobals->v_up * -8;

        MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
                WRITE_BYTE(TE_SMOKE);
                WRITE_COORD(vecSmokeOrigin.x);
                WRITE_COORD(vecSmokeOrigin.y);
                WRITE_COORD(vecSmokeOrigin.z);
                WRITE_SHORT(g_sModelIndexSmoke);
                WRITE_BYTE(12);
                WRITE_BYTE(12); // framerate
        MESSAGE_END();
#endif

        // Clear zoom on fire — the recoil breaks the aim anyway, and this ensures
        // the FOV is always restored even if secondary attack was used to zoom in.
        if( m_fInZoom )
        {
                m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0;
                m_pPlayer->pev->viewmodel = MAKE_STRING( "models/v_buffalogun.mdl" );
                m_fInZoom = FALSE;
        }

        m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetNextAttackDelay( 3.0f );
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15);
        // NOTE: do NOT set m_pPlayer->m_flNextAttack here.
        // That timer blocks weapon switching — setting it to 3.0f caused the player
        // to be frozen out of switching weapons for 3 seconds after every shot.
        // The fire rate is already enforced by m_flNextPrimaryAttack above.
}

void CBuffalo::Reload(void)
{
        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
                return;

        DefaultReload(BUFFALO_MAX_CLIP, BUFFALO_RELOAD, 3.6);
}

void CBuffalo::WeaponIdle(void)
{
        ResetEmptySound();

        m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

        if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
                return;

        int iAnim;
        float flRand = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0, 1);
        if (flRand <= 0.5)
        {
                iAnim = BUFFALO_IDLE1;
        }
        else
        {
                iAnim = BUFFALO_IDLE2;
        }

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + (20.0 / 9.0);
        SendWeaponAnim(iAnim, UseDecrement());
}

class CBuffaloAmmo : public CBasePlayerAmmo
{
        void Spawn(void)
        {
                Precache();
                SET_MODEL(ENT(pev), "models/w_buffalobox.mdl");
                CBasePlayerAmmo::Spawn();
        }
        void Precache(void)
        {
                PRECACHE_MODEL("models/w_buffalobox.mdl");
                PRECACHE_SOUND("items/9mmclip1.wav");
        }
        BOOL AddAmmo(CBaseEntity *pOther)
        {
                if (pOther->GiveAmmo(AMMO_BUFFALO_GIVE, "buffalo", BUFFALO_MAX_CARRY) != -1)
                {
                        EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
                        return TRUE;
                }
                return FALSE;
        }
};
LINK_ENTITY_TO_CLASS(ammo_buffalo, CBuffaloAmmo);

#endif