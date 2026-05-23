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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "gamerules.h"

#define KNIFE_BODYHIT_VOLUME 128
#define KNIFE_WALLHIT_VOLUME 512

LINK_ENTITY_TO_CLASS(weapon_knife, CKnife);

enum knife_e {
        KNIFE_IDLE = 0,
        KNIFE_DRAW,
        KNIFE_HOLSTER,
        KNIFE_ATTACK1HIT,
        KNIFE_ATTACK1MISS,
        KNIFE_ATTACK2MISS,
        KNIFE_ATTACK2HIT,
        KNIFE_ATTACK3MISS,
        KNIFE_ATTACK3HIT,
        KNIFE_IDLE2,   // [9]  looping idle  15fps 30fr = 2.00 s
        KNIFE_IDLE3    // [10] looping idle  25fps 17fr = 0.68 s
};

void CKnife::Spawn()
{
        Precache();
        m_iId = WEAPON_KNIFE;
        SET_MODEL(ENT(pev), "models/w_knife.mdl");
        m_iClip = -1;

        FallInit();// get ready to fall down.
}

void CKnife::Precache(void)
{
        PRECACHE_MODEL("models/v_knife.mdl");
        PRECACHE_MODEL("models/w_knife.mdl");
        PRECACHE_MODEL("models/p_knife.mdl");
        PRECACHE_SOUND("weapons/knife_hit1.wav");
        PRECACHE_SOUND("weapons/knife_hit2.wav");
        PRECACHE_SOUND("weapons/knife_hitbod1.wav");
        PRECACHE_SOUND("weapons/knife_hitbod2.wav");
        PRECACHE_SOUND("weapons/knife_hitbod3.wav");
        PRECACHE_SOUND("weapons/knife_miss1.wav");

        m_usKnife = PRECACHE_EVENT(1, "events/knife.sc");
}

int CKnife::GetItemInfo(ItemInfo *p)
{
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = NULL;
        p->iMaxAmmo1 = -1;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = WEAPON_NOCLIP;
        p->iSlot = 0;
        p->iPosition = 0;
        p->iId = WEAPON_KNIFE;
        p->iWeight = KNIFE_WEIGHT;
        return 1;
}

BOOL CKnife::Deploy()
{
        return DefaultDeploy("models/v_knife.mdl", "models/p_knife.mdl", KNIFE_DRAW, "knife");
}

void CKnife::Holster(int skiplocal /* = 0 */)
{
        m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
        SendWeaponAnim(KNIFE_HOLSTER);
}

void CKnife::WeaponIdle( void )
{
        if( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
                return;

        // Use the two looping idle animations for visual variety instead of
        // the one-shot idle1 played by the base CCrowbar::WeaponIdle.
        // [9]  idle2  LOOP 15fps 30fr = 2.00 s
        // [10] idle3  LOOP 25fps 17fr = 0.68 s
        int   iAnim;
        float flNext;
        if( UTIL_SharedRandomFloat( m_pPlayer->random_seed, 0.0f, 1.0f ) <= 0.6f )
        {
                iAnim  = KNIFE_IDLE2;
                flNext = 30.0f / 15.0f; // 2.00 s
        }
        else
        {
                iAnim  = KNIFE_IDLE3;
                flNext = 17.0f / 25.0f; // 0.68 s
        }
        SendWeaponAnim( iAnim );
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + flNext;
}

void FindHullIntersection(const Vector &vecSrc, TraceResult &tr, float *mins, float *maxs, edict_t *pEntity);

void CKnife::PrimaryAttack()
{
        if (!Swing(1))
        {
                SetThink(&CKnife::SwingAgain);
                pev->nextthink = gpGlobals->time + 0.1;
        }
}

int CKnife::Swing(int fFirst)
{
        int fDidHit = FALSE;

        TraceResult tr;

        UTIL_MakeVectors(m_pPlayer->pev->v_angle);
        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecEnd = vecSrc + gpGlobals->v_forward * 32;

        UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

#ifndef CLIENT_DLL
        if (tr.flFraction >= 1.0)
        {
                UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(m_pPlayer->pev), &tr);
                if (tr.flFraction < 1.0)
                {
                        // Calculate the point of intersection of the line (or hull) and the object we hit
                        // This is and approximation of the "best" intersection
                        CBaseEntity *pHit = CBaseEntity::Instance(tr.pHit);
                        if (!pHit || pHit->IsBSPModel())
                                FindHullIntersection(vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, m_pPlayer->edict());
                        vecEnd = tr.vecEndPos;  // This is the point on the actual surface (the hull could have hit space)
                }
        }
#endif

        PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usKnife,
                0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, 0,
                0.0, 0, 0.0);

        if (tr.flFraction >= 1.0)
        {
                if (fFirst)
                {
                        // miss
                        m_flNextPrimaryAttack = GetNextAttackDelay(0.5);

                        switch (((m_iSwing++) % 3))
                        {
                        case 0: SendWeaponAnim(KNIFE_ATTACK1MISS); break;
                        case 1: SendWeaponAnim(KNIFE_ATTACK2MISS); break;
                        default: SendWeaponAnim(KNIFE_ATTACK3MISS); break;
                        }

                        // player "shoot" animation
                        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
                }
        }
        else
        {
                switch (((m_iSwing++) % 2) + 1)
                {
                case 0:
                        SendWeaponAnim(KNIFE_ATTACK1HIT); break;
                case 1:
                        SendWeaponAnim(KNIFE_ATTACK2HIT); break;
                case 2:
                        SendWeaponAnim(KNIFE_ATTACK3HIT); break;
                }

                // player "shoot" animation
                m_pPlayer->SetAnimation(PLAYER_ATTACK1);

#ifndef CLIENT_DLL

                // hit
                fDidHit = TRUE;
                CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);

                ClearMultiDamage();

                if( pEntity )
                {
                if ((m_flNextPrimaryAttack + 1 < UTIL_WeaponTimeBase()) || g_pGameRules->IsMultiplayer())
                {
                        // first swing does full damage
                        pEntity->TraceAttack(m_pPlayer->pev, gSkillData.plrDmgKnife, gpGlobals->v_forward, &tr, DMG_SLASH);
                }
                else
                {
                        // subsequent swings do half
                        pEntity->TraceAttack(m_pPlayer->pev, gSkillData.plrDmgKnife / 2, gpGlobals->v_forward, &tr, DMG_SLASH);
                }
                }
                ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

                // play thwack, smack, or dong sound
                float flVol = 1.0;
                int fHitWorld = TRUE;

                if (pEntity)
                {
                        if (pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE)
                        {
                                // play thwack or smack sound
                                switch (RANDOM_LONG(0, 2))
                                {
                                case 0:
                                        EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/knife_hitbod1.wav", 1, ATTN_NORM); break;
                                case 1:
                                        EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/knife_hitbod2.wav", 1, ATTN_NORM); break;
                                case 2:
                                        EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/knife_hitbod3.wav", 1, ATTN_NORM); break;
                                }
                                m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
                                if (!pEntity->IsAlive())
                                        return TRUE;
                                else
                                        flVol = 0.1;

                                fHitWorld = FALSE;
                        }
                }

                // play texture hit sound
                // UNDONE: Calculate the correct point of intersection when we hit with the hull instead of the line

                if (fHitWorld)
                {
                        float fvolbar = TEXTURETYPE_PlaySound(&tr, vecSrc, vecSrc + (vecEnd - vecSrc) * 2, BULLET_PLAYER_CROWBAR);

                        if (g_pGameRules->IsMultiplayer())
                        {
                                // override the volume here, cause we don't play texture sounds in multiplayer, 
                                // and fvolbar is going to be 0 from the above call.

                                fvolbar = 1;
                        }

                        // also play crowbar strike
                        switch (RANDOM_LONG(0, 1))
                        {
                        case 0:
                                EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/knife_hit1.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
                                break;
                        case 1:
                                EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/knife_hit2.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
                                break;
                        }

                        // delay the decal a bit
                        m_trHit = tr;
                }

                m_pPlayer->m_iWeaponVolume = flVol * KNIFE_WALLHIT_VOLUME;
#endif
                m_flNextPrimaryAttack = GetNextAttackDelay(0.25);

                SetThink(&CKnife::Smack);
                pev->nextthink = UTIL_WeaponTimeBase() + 0.2;

        }
        return fDidHit;
}

#ifndef CLIENT_DLL

class CFlyingKnife : public CBaseEntity
{
public:
        void    Spawn( void );
        void    Precache( void );
        void EXPORT BubbleThink( void );
        void EXPORT SpinTouch( CBaseEntity *pOther );
        EHANDLE m_hOwner;
};
LINK_ENTITY_TO_CLASS( flying_knife, CFlyingKnife );

void CFlyingKnife::Precache( void )
{
        PRECACHE_MODEL( "models/w_knife.mdl" );
        PRECACHE_SOUND( "weapons/knife_hitbod1.wav" );
        PRECACHE_SOUND( "weapons/knife_hit1.wav" );
        PRECACHE_SOUND( "weapons/knife_miss1.wav" );
}

void CFlyingKnife::Spawn( void )
{
        Precache();

        pev->movetype = MOVETYPE_TOSS;
        pev->solid    = SOLID_BBOX;
        pev->dmg      = 50;

        SET_MODEL( ENT( pev ), "models/w_knife.mdl" );
        UTIL_SetOrigin( pev, pev->origin );
        UTIL_SetSize( pev, Vector( -4, -4, -4 ), Vector( 4, 4, 4 ) );

        // Store owner so damage can be attributed even after pev->owner is cleared.
        if( pev->owner )
                m_hOwner = Instance( pev->owner );

        SetThink( &CFlyingKnife::BubbleThink );
        pev->nextthink = gpGlobals->time + 0.25f;

        SetTouch( &CFlyingKnife::SpinTouch );
}

void CFlyingKnife::BubbleThink( void )
{
        pev->owner     = NULL;
        pev->nextthink = gpGlobals->time + 0.25f;

        if( pev->waterlevel )
                UTIL_BubbleTrail( pev->origin - pev->velocity * 0.1f, pev->origin, 1 );
}

void CFlyingKnife::SpinTouch( CBaseEntity *pOther )
{
        if( !pOther )
                return;

        // Don't hit the original thrower during the first 0.25 s window.
        if( ENT( pOther->pev ) == pev->owner )
                return;

        if( pOther->pev->takedamage )
        {
                TraceResult tr = UTIL_GetGlobalTrace();
                ClearMultiDamage();
                pOther->TraceAttack( pev, pev->dmg, pev->velocity.Normalize(), &tr, DMG_SLASH );
                if( m_hOwner != NULL )
                        ApplyMultiDamage( pev, m_hOwner->pev );
                else
                        ApplyMultiDamage( pev, pev );
        }

        if( pOther->IsPlayer() )
                EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, "weapons/knife_hitbod1.wav", 1.0f, ATTN_NORM, 0, 100 );
        else
                EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, "weapons/knife_hit1.wav",    1.0f, ATTN_NORM, 0, 100 );

        // Hide the flying knife model immediately.
        pev->effects |= EF_NODRAW;
        pev->solid    = SOLID_NOT;

        // Spawn a weapon_knife pickup so the player can retrieve it.
        CBasePlayerWeapon *pItem = (CBasePlayerWeapon *)Create( "weapon_knife", pev->origin, pev->angles, edict() );
        pItem->pev->nextthink    = gpGlobals->time + 240.0f; // vanish after 4 minutes
        pItem->SetThink( &CBasePlayerWeapon::Kill );
        pItem->pev->angles.x     = 0;
        pItem->pev->angles.z     = 0;
        pItem->pev->solid        = SOLID_TRIGGER;
        pItem->pev->spawnflags  |= SF_NORESPAWN;
        UTIL_SetSize( pItem->pev, Vector( 0, 0, 0 ), Vector( 0, 0, 0 ) );

        // Bounce the pickup slightly along the impact normal.
        TraceResult tr;
        Vector vecDir = pev->velocity.Normalize();
        UTIL_TraceLine( pev->origin, pev->origin + vecDir * 100, dont_ignore_monsters, ENT( pev ), &tr );
        pItem->pev->velocity = tr.vecPlaneNormal * 100;

        // Clear the think function before removing so BubbleThink cannot
        // reschedule this entity after UTIL_Remove marks it for deletion.
        // A dangling scheduled think on a removed entity corrupts save files.
        SetThink( NULL );
        SetTouch( NULL );
        UTIL_Remove( this );
}

#endif // !CLIENT_DLL

void CKnife::SecondaryAttack( void )
{
        if( m_pPlayer->pev->waterlevel == 3 )
                return;

        SendWeaponAnim( KNIFE_ATTACK1MISS );
        m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
        EMIT_SOUND_DYN( ENT( m_pPlayer->pev ), CHAN_WEAPON, "weapons/knife_miss1.wav",
                        1.0f, ATTN_NORM, 0, 94 + RANDOM_LONG( 0, 0xF ) );

        // Defer the actual throw by 0.2 s (matches crowbar CreateThink delay)
        // so the animation frame has time to show before the weapon disappears.
#ifndef CLIENT_DLL
        SetThink( &CKnife::CreateThink );
        pev->nextthink = gpGlobals->time + 0.2f;
#endif
        m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.25f;
}

#ifndef CLIENT_DLL
void CKnife::CreateThink( void )
{
        UTIL_MakeVectors( m_pPlayer->pev->v_angle );
        Vector vecSrc = m_pPlayer->GetGunPosition()
                        + gpGlobals->v_right * 8
                        + gpGlobals->v_forward * 16;

        Vector vecAng  = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;
        vecAng.z       = -90; // tip the blade forward

        int ang = RANDOM_LONG( -1000, -500 ); // random tumble rate

        CFlyingKnife *pFlyKnife = (CFlyingKnife *)Create( "flying_knife", vecSrc,
                                                          Vector( 0, 0, 0 ),
                                                          m_pPlayer->edict() );
        pFlyKnife->pev->velocity  = gpGlobals->v_forward * (int)( -( ang / 2 ) )
                                  + m_pPlayer->pev->velocity;
        pFlyKnife->pev->angles    = vecAng;
        pFlyKnife->pev->avelocity = Vector( ang, RANDOM_LONG( -50, 50 ), RANDOM_LONG( -25, 25 ) );
        pFlyKnife->pev->gravity   = 0.5f;
        pFlyKnife->pev->dmg       = -(ang / 10);
        pFlyKnife->m_hOwner       = m_pPlayer;

        // Remove the knife from the player's inventory entirely.
        // The player regains it by walking over the weapon_knife pickup.
        m_pPlayer->RemovePlayerItem( this, FALSE );
        m_pPlayer->pev->weapons &= ~( 1 << m_iId );
        DestroyItem();
        m_pPlayer->m_pActiveItem = NULL;

        SetThink( NULL );
}
#endif // !CLIENT_DLL
