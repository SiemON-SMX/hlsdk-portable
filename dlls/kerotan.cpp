/***
 *
 *      Kerotan — hidden Metal Gear frog figurine Easter egg.
 *
 *      Ported from the GoldSRC Half-Payne source (kerotan.cpp).
 *      GoldSRC-specific systems removed:
 *        - gameplayMods::kerotanDetector  (Custom Game Mode only)
 *        - CHalfLifeRules::HookModelIndex (Custom Game Mode only)
 *        - TryToPlayMaxCommentary         (Custom Game Mode only)
 *
 ***/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "kerotan.h"

LINK_ENTITY_TO_CLASS( kerotan, CKerotan )

TYPEDESCRIPTION CKerotan::m_SaveData[] =
{
        DEFINE_FIELD( CKerotan, hasBeenFound,      FIELD_BOOLEAN ),
        DEFINE_FIELD( CKerotan, hitsReceived,      FIELD_INTEGER ),
        DEFINE_FIELD( CKerotan, soundsLeft,        FIELD_INTEGER ),
        DEFINE_FIELD( CKerotan, nextSound,         FIELD_TIME    ),
        DEFINE_FIELD( CKerotan, rotationLeft,      FIELD_FLOAT   ),
        DEFINE_FIELD( CKerotan, rotationDirection, FIELD_INTEGER ),
        DEFINE_FIELD( CKerotan, rollAmplitude,     FIELD_FLOAT   ),
        DEFINE_FIELD( CKerotan, rollDirection,     FIELD_INTEGER ),
};

IMPLEMENT_SAVERESTORE( CKerotan, CBaseToggle )

void CKerotan::Spawn( void )
{
        Precache();
        SET_MODEL( ENT( pev ), "models/kerotan.mdl" );

        hasBeenFound      = FALSE;
        hitsReceived      = 0;
        soundsLeft        = 0;
        nextSound         = 0.0f;
        rotationLeft      = 0.0f;
        rotationDirection = 0;
        rollAmplitude     = 0.0f;
        rollDirection     = 0;

        pev->movetype   = MOVETYPE_TOSS;
        pev->solid      = SOLID_SLIDEBOX;
        pev->takedamage = DAMAGE_YES;

        UTIL_SetOrigin( pev, pev->origin );
        UTIL_SetSize( pev, Vector( -2, -2, -2 ), Vector( 2, 2, 2 ) );

        SetThink( &CKerotan::OnThink );
        pev->nextthink = gpGlobals->time + 0.01f;
}

void CKerotan::Precache( void )
{
        PRECACHE_MODEL( "models/kerotan.mdl" );
        PRECACHE_SOUND( "var/kerotan.wav"       );  // normal ribbit
        PRECACHE_SOUND( "var/kerotan_alert.wav" );  // hit reaction
        PRECACHE_SOUND( "var/kerotan_broke.wav" );  // death rattle
        PRECACHE_SOUND( "var/kerotan_short.wav" );  // final shortened ribbit
}

int CKerotan::TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker,
                          float flDamage, int bitsDamageType )
{
        // Only physical damage counts — ignore fire, radiation, drown, etc.
        if( !( bitsDamageType & ( DMG_BULLET | DMG_BLAST | DMG_CLUB | DMG_SLASH ) ) )
                return 1;

        // Spin in a random direction.
        rotationLeft      = RANDOM_FLOAT( 40.0f, 70.0f );
        rotationDirection = RANDOM_LONG( 0, 1 ) ? 1 : -1;

        // Start a wobble roll if not already wobbling.
        if( !rollAmplitude )
        {
                rollAmplitude = RANDOM_FLOAT( 10.0f, 30.0f );
                rollDirection = RANDOM_LONG( 0, 1 ) ? 1 : -1;
        }

        // Mark as found on the first hit.
        hasBeenFound = TRUE;

        if( hitsReceived >= 5 )
        {
                EMIT_SOUND_DYN( ENT( pev ), CHAN_STATIC,
                        "var/kerotan_alert.wav", 1, ATTN_NORM, 0, 100 );
                return 1;
        }

        hitsReceived++;

        EMIT_SOUND_DYN( ENT( pev ), CHAN_STATIC,
                "var/kerotan_alert.wav", 1, ATTN_NORM, 0, 100 );

        // Schedule 10 ribbit sounds spaced ~1.3 s apart.
        if( !soundsLeft )
        {
                soundsLeft = 10;
                nextSound  = gpGlobals->time + 1.0f;
        }

        return 1;
}

void CKerotan::OnThink( void )
{
        // Play scheduled ribbit sounds with pitch degrading on each hit.
        if( nextSound > 0.0f && gpGlobals->time >= nextSound )
        {
                soundsLeft--;

                int pitch = 100 - ( hitsReceived * 9 ) + 16;
                if( pitch > 100 ) pitch = 100;
                if( pitch < 28  ) pitch = 28;

                if( hitsReceived >= 5 && soundsLeft <= 0 )
                {
                        // Final broken ribbit.
                        EMIT_SOUND_DYN( ENT( pev ), CHAN_STATIC,
                                "var/kerotan_short.wav", 1, ATTN_NORM, 0, pitch );
                        EMIT_SOUND( ENT( pev ), CHAN_STATIC,
                                "var/kerotan_broke.wav", 1, ATTN_NORM );
                }
                else
                {
                        EMIT_SOUND_DYN( ENT( pev ), CHAN_STATIC,
                                "var/kerotan.wav", 1, ATTN_NORM, 0, pitch );
                }

                nextSound = ( soundsLeft > 0 )
                        ? gpGlobals->time + 1.3f + ( hitsReceived / 8.0f )
                        : 0.0f;
        }

        // Rotate on the Y axis (yaw spin).
        if( rotationLeft > 0.0f )
        {
                float step = ( rotationLeft < 5.0f ) ? rotationLeft : 5.0f;
                pev->angles.y += step * rotationDirection;
                rotationLeft  -= step;
        }

        // Wobble roll that decays over time.
        pev->angles.z  = rollDirection * rollAmplitude * (float)sin( gpGlobals->time * 16.0 );
        rollAmplitude -= 0.5f;
        if( rollAmplitude < 0.0f )
                rollAmplitude = 0.0f;

        pev->nextthink = gpGlobals->time + 0.01f;
}
