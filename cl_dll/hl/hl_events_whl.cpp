/***
*
* WantedHL - client-side weapon event handlers
*
* Called by the engine for each PLAYBACK_EVENT fired by a WantedHL weapon.
* Since CLIENT_WEAPONS is not defined, flags=0 on all ranged weapons, meaning
* these callbacks fire on the LOCAL player's client too — making them
* responsible for playing the viewmodel fire animation, muzzle flash,
* screen recoil, shell ejection, and weapon sound.
*
* Animation index constants are taken directly from the server-side weapon
* enum definitions (weapon_*.cpp). If the model sequences change, update
* the numeric values here to match.
*
***/

#include "../hud.h"
#include "../cl_util.h"
#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "entity_types.h"
#include "usercmd.h"
#include "pm_defs.h"
#include "pm_materials.h"
#include "eventscripts.h"
#include "ev_hldm.h"
#include "r_efx.h"
#include "event_api.h"
#include "event_args.h"
#include "in_defs.h"

// -------------------------------------------------------------------------
// Animation sequence indices — must match server-side enum definitions
// -------------------------------------------------------------------------

// pistol_e
#define PISTOL_SHOOT           2
#define PISTOL_QUICKFIRE_SHOOT 9

// colts_e
#define COLTS_LEFTFIRE  4
#define COLTS_RIGHTFIRE 5
#define COLTS_DUALFIRE  6

// winchester_e
#define WINCHESTER_FIRE 1

// dbarrel_e  (CShotgun2)
#define DBARREL_SHOOT  5
#define DBARREL_SHOOT2 6

// gattlinggun_e  — animation transitions are driven server-side by SendWeaponAnim.
// The client event only handles muzzle flash, recoil, shell ejection, and fire sound.

// buffalo_e
#define BUFFALO_FIRE 5

// knife_e  (FEV_NOTHOST — only fires on remote clients in MP)
#define KNIFE_ATTACK1HIT  3

// pick_e   (FEV_NOTHOST — only fires on remote clients in MP)
#define PICK_ATTACK1HIT   3

// bow_e  — model[11]="fire", model[4]="idle5" (old wrong value was 4)
#define BOW_FIRE  11
#define BOW_FIRE2 12

// btrap_e
#define BTRAP_THROW 4

// scorp_e
#define SCORP_THROW 5

// V_PunchAxis is implemented in view.cpp and declared in eventscripts.h,
// but the compiler can't resolve it from this subdirectory without the
// forward declaration below.
extern void V_PunchAxis( int axis, float punch );

extern "C"
{

// =========================================================================
// PISTOL
// args: bparam1 = fSecondary (quickfire mode)
// =========================================================================
void EV_WHL_Pistol( struct event_args_s *args )
{
        int    idx = args->entindex;
        vec3_t origin, angles, velocity;
        VectorCopy( args->origin,   origin );
        VectorCopy( args->angles,   angles );
        VectorCopy( args->velocity, velocity );

        int fSecondary = args->bparam1;

        if( EV_IsLocal( idx ) )
        {
                EV_MuzzleFlash();
                gEngfuncs.pEventAPI->EV_WeaponAnimation(
                        fSecondary ? PISTOL_QUICKFIRE_SHOOT : PISTOL_SHOOT, 0 );
                V_PunchAxis( 0, -2.0f );
        }

        gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON,
                gEngfuncs.pfnRandomLong( 0, 1 ) ? "weapons/pistol_shot1.wav"
                                                 : "weapons/pistol_shot2.wav",
                gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0,
                98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
}

// =========================================================================
// COLTS (dual revolvers)
// args: bparam1 = fDualFire, bparam2 = fLeftAttack
// =========================================================================
void EV_WHL_Colts( struct event_args_s *args )
{
        int    idx = args->entindex;
        vec3_t origin, angles, velocity;
        VectorCopy( args->origin,   origin );
        VectorCopy( args->angles,   angles );
        VectorCopy( args->velocity, velocity );

        int fDualFire  = args->bparam1;
        int fLeftAttack = args->bparam2;

        int iAnim;
        if( fDualFire )
                iAnim = COLTS_DUALFIRE;
        else if( fLeftAttack )
                iAnim = COLTS_LEFTFIRE;
        else
                iAnim = COLTS_RIGHTFIRE;

        if( EV_IsLocal( idx ) )
        {
                EV_MuzzleFlash();
                gEngfuncs.pEventAPI->EV_WeaponAnimation( iAnim, 0 );
                V_PunchAxis( 0, -3.0f );
        }

        gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON,
                gEngfuncs.pfnRandomLong( 0, 1 ) ? "weapons/coltsfire1.wav"
                                                 : "weapons/coltsfire2.wav",
                gEngfuncs.pfnRandomFloat( 0.92f, 1.0f ), ATTN_NORM, 0,
                98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
}

// =========================================================================
// WINCHESTER (lever-action rifle)
// =========================================================================
void EV_WHL_Winchester( struct event_args_s *args )
{
        int    idx = args->entindex;
        vec3_t origin, angles, velocity;
        VectorCopy( args->origin,   origin );
        VectorCopy( args->angles,   angles );
        VectorCopy( args->velocity, velocity );

        if( EV_IsLocal( idx ) )
        {
                EV_MuzzleFlash();
                gEngfuncs.pEventAPI->EV_WeaponAnimation( WINCHESTER_FIRE, 0 );
                V_PunchAxis( 0, -5.0f );
        }

        static const char *pSounds[] = {
                "weapons/winchester_fire1.wav",
                "weapons/winchester_fire2.wav",
                "weapons/winchester_fire3.wav",
        };
        gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON,
                pSounds[ gEngfuncs.pfnRandomLong( 0, 2 ) ],
                gEngfuncs.pfnRandomFloat( 0.95f, 1.0f ), ATTN_NORM, 0,
                88 + gEngfuncs.pfnRandomLong( 0, 0x1f ) );
}

// =========================================================================
// SHOTGUN (double-barrel)
// args: iparam1 = 1 (single barrel) or 2 (double barrel)
//       bparam1 = fSecondary
// =========================================================================
void EV_WHL_Shotgun( struct event_args_s *args )
{
        int    idx = args->entindex;
        vec3_t origin, angles, velocity;
        vec3_t ShellVelocity, ShellOrigin;
        vec3_t up, right, forward;

        VectorCopy( args->origin,   origin );
        VectorCopy( args->angles,   angles );
        VectorCopy( args->velocity, velocity );

        int iMode      = args->iparam1; // 1=single, 2=double
        int fSecondary = args->bparam1;

        AngleVectors( angles, forward, right, up );

        int shell = gEngfuncs.pEventAPI->EV_FindModelIndex( "models/shotgunshell.mdl" );

        if( EV_IsLocal( idx ) )
        {
                EV_MuzzleFlash();
                gEngfuncs.pEventAPI->EV_WeaponAnimation(
                        ( iMode == 2 ) ? DBARREL_SHOOT2 : DBARREL_SHOOT, 0 );
                V_PunchAxis( 0, ( iMode == 2 ) ? -10.0f : -5.0f );
        }

        int shells = ( iMode == 2 ) ? 2 : 1;
        for( int j = 0; j < shells; j++ )
        {
                EV_GetDefaultShellInfo( args, origin, velocity,
                        ShellVelocity, ShellOrigin,
                        forward, right, up, 32, -12, 6 );
                EV_EjectBrass( ShellOrigin, ShellVelocity, angles[1], shell, TE_BOUNCE_SHOTSHELL );
        }

        gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON,
                ( iMode == 2 ) ? "weapons/dbarrel1.wav" : "weapons/sbarrel1.wav",
                gEngfuncs.pfnRandomFloat( 0.95f, 1.0f ), ATTN_NORM, 0,
                85 + gEngfuncs.pfnRandomLong( 0, 0x1f ) );
}

// =========================================================================
// GATTLING GUN
// =========================================================================
void EV_WHL_GattlingGun( struct event_args_s *args )
{
        int    idx = args->entindex;
        vec3_t origin, angles, velocity;
        vec3_t ShellVelocity, ShellOrigin;
        vec3_t up, right, forward;

        VectorCopy( args->origin,   origin );
        VectorCopy( args->angles,   angles );
        VectorCopy( args->velocity, velocity );

        AngleVectors( angles, forward, right, up );

        int shell = gEngfuncs.pEventAPI->EV_FindModelIndex( "models/shell.mdl" );

        // NOTE: Do NOT call EV_WeaponAnimation here.
        // EV_WeaponAnimation always resets the sequence to frame 0. Calling it on
        // every shot (every ~0.07 s) would reset the 22-fps FIRE loop before it
        // can advance past frame 1-2, making the barrel appear frozen.
        // All viewmodel animation transitions (spinup → fire → spindown → idle)
        // are driven exclusively by the server via SendWeaponAnim at state changes.

        if( EV_IsLocal( idx ) )
        {
                EV_MuzzleFlash();
                V_PunchAxis( 0, -1.5f );
        }

        EV_GetDefaultShellInfo( args, origin, velocity,
                ShellVelocity, ShellOrigin,
                forward, right, up, 20, -12, 4 );
        EV_EjectBrass( ShellOrigin, ShellVelocity, angles[1], shell, TE_BOUNCE_SHELL );

        // Rotate randomly through three fire sound variants each shot
        static const char *s_fireSounds[3] =
        {
                "weapons/gat_shoot1.wav",
                "weapons/gat_shoot2.wav",
                "weapons/gat_shoot3.wav",
        };
        gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON,
                s_fireSounds[gEngfuncs.pfnRandomLong( 0, 2 )],
                gEngfuncs.pfnRandomFloat( 0.95f, 1.0f ), ATTN_NORM, 0,
                95 + gEngfuncs.pfnRandomLong( 0, 0x1f ) );
}

// =========================================================================
// BUFFALO RIFLE (large-calibre single shot)
// =========================================================================
void EV_WHL_Buffalo( struct event_args_s *args )
{
        int    idx = args->entindex;
        vec3_t origin, angles, velocity;
        vec3_t ShellVelocity, ShellOrigin;
        vec3_t up, right, forward;

        VectorCopy( args->origin,   origin );
        VectorCopy( args->angles,   angles );
        VectorCopy( args->velocity, velocity );

        AngleVectors( angles, forward, right, up );

        int shell = gEngfuncs.pEventAPI->EV_FindModelIndex( "models/shotgunshell.mdl" );

        if( EV_IsLocal( idx ) )
        {
                EV_MuzzleFlash();
                gEngfuncs.pEventAPI->EV_WeaponAnimation( BUFFALO_FIRE, 0 );
                V_PunchAxis( 0, -8.0f );
        }

        EV_GetDefaultShellInfo( args, origin, velocity,
                ShellVelocity, ShellOrigin,
                forward, right, up, 32, -12, 6 );
        EV_EjectBrass( ShellOrigin, ShellVelocity, angles[1], shell, TE_BOUNCE_SHOTSHELL );

        // Sound is emitted directly from PrimaryAttack (server-side) for reliability.
        // This event handles visuals only: muzzle flash, shell eject, and view punch.
}

// =========================================================================
// BOW (projectile — no muzzle flash, no shell)
// =========================================================================
void EV_WHL_Bow( struct event_args_s *args )
{
        int    idx = args->entindex;
        vec3_t origin;
        VectorCopy( args->origin, origin );

        if( EV_IsLocal( idx ) )
        {
                gEngfuncs.pEventAPI->EV_WeaponAnimation( BOW_FIRE, 0 );
                V_PunchAxis( 0, -1.0f );
        }

        gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON,
                "weapons/bow_fire1.wav",
                gEngfuncs.pfnRandomFloat( 0.95f, 1.0f ), ATTN_NORM, 0, PITCH_NORM );
}

// =========================================================================
// BEARTRAP (thrown/placed — no muzzle flash, no shell)
// =========================================================================
void EV_WHL_Beartrap( struct event_args_s *args )
{
        int    idx = args->entindex;
        vec3_t origin;
        VectorCopy( args->origin, origin );

        if( EV_IsLocal( idx ) )
        {
                gEngfuncs.pEventAPI->EV_WeaponAnimation( BTRAP_THROW, 0 );
        }

        gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON,
                "weapons/beartrap_deploy.wav",
                VOL_NORM, ATTN_NORM, 0, PITCH_NORM );
}

// =========================================================================
// SCORPION (thrown — no muzzle flash, no shell)
// =========================================================================
void EV_WHL_Scorpion( struct event_args_s *args )
{
        int    idx = args->entindex;
        vec3_t origin;
        VectorCopy( args->origin, origin );

        if( EV_IsLocal( idx ) )
        {
                gEngfuncs.pEventAPI->EV_WeaponAnimation( SCORP_THROW, 0 );
        }

        // Scorpion deploy click played server-side; client plays the deploy
        // sound for the predicted throw so there is no perceptible delay.
        gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON,
                "scorpion/scorp_deploy1.wav",
                VOL_NORM, ATTN_NORM, 0, PITCH_NORM );
}

// =========================================================================
// KNIFE  — FEV_NOTHOST: fires on remote clients only (not local SP player).
// Server calls SendWeaponAnim for the local player. This handler plays the
// swing sound for players watching in multiplayer.
// =========================================================================
void EV_WHL_Knife( struct event_args_s *args )
{
        int    idx = args->entindex;
        vec3_t origin;
        VectorCopy( args->origin, origin );

        if( EV_IsLocal( idx ) )
        {
                // Should never fire for local player (FEV_NOTHOST), but guard anyway
                gEngfuncs.pEventAPI->EV_WeaponAnimation( KNIFE_ATTACK1HIT, 0 );
        }

        gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON,
                "weapons/knife_hit1.wav",
                VOL_NORM, ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
}

// =========================================================================
// PICK   — FEV_NOTHOST: same rationale as knife above.
// =========================================================================
void EV_WHL_Pick( struct event_args_s *args )
{
        int    idx = args->entindex;
        vec3_t origin;
        VectorCopy( args->origin, origin );

        if( EV_IsLocal( idx ) )
        {
                gEngfuncs.pEventAPI->EV_WeaponAnimation( PICK_ATTACK1HIT, 0 );
        }

        gEngfuncs.pEventAPI->EV_PlaySound( idx, origin, CHAN_WEAPON,
                "weapons/pick_hit1.wav",
                VOL_NORM, ATTN_NORM, 0, 98 + gEngfuncs.pfnRandomLong( 0, 3 ) );
}

} // extern "C"
