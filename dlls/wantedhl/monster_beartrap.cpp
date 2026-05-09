/***
*
* WantedHL - monster_beartrap
* A deployable bear trap placed by weapon_beartrap.
* When a player or monster steps on it, it snaps shut dealing damage
* and slowing/trapping the victim briefly.
*
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "soundent.h"
#include "weapons.h"
#include "player.h"
#include "effects.h"
#include "gamerules.h"

#define BEARTRAP_DAMAGE      40.0f
#define BEARTRAP_SLOW_TIME   3.0f
#define BEARTRAP_SLOW_FACTOR 0.3f

class CMonsterBearTrap : public CBaseMonster
{
public:
        int  ISoundMask( void );
        void Spawn( void );
        void Precache( void );
        void EXPORT BearTrapTouch( CBaseEntity *pOther );
        void EXPORT BearTrapThink( void );
        int  Classify( void ) { return CLASS_NONE; }
        int  BloodColor( void ) { return DONT_BLEED; }
        void Killed( entvars_t *pevAttacker, int iGib );

private:
        BOOL m_bTriggered;
        float m_flUnlockTime;
};


int CMonsterBearTrap::ISoundMask( void )
{
        return bits_SOUND_WORLD | bits_SOUND_COMBAT | bits_SOUND_PLAYER | bits_SOUND_DANGER;
}


LINK_ENTITY_TO_CLASS( monster_beartrap, CMonsterBearTrap );


void CMonsterBearTrap::Spawn( void )
{
        Precache();

        pev->movetype  = MOVETYPE_TOSS;
        pev->solid     = SOLID_TRIGGER;

        SET_MODEL( ENT( pev ), "models/w_beartrap.mdl" );
        UTIL_SetSize( pev, Vector( -8, -8, 0 ), Vector( 8, 8, 4 ) );
        UTIL_SetOrigin( pev, pev->origin );

        pev->takedamage = DAMAGE_NO;
        pev->health     = 1;
        pev->flags     |= FL_MONSTER;
        pev->gravity    = 1.0f;
        pev->friction   = 1.0f;

        m_bTriggered    = FALSE;
        m_flUnlockTime  = 0.0f;

        SetTouch( &CMonsterBearTrap::BearTrapTouch );
        SetThink( &CMonsterBearTrap::BearTrapThink );
        pev->nextthink = gpGlobals->time + 0.3f;
}

void CMonsterBearTrap::Precache( void )
{
        PRECACHE_MODEL( "models/w_beartrap.mdl" );
        PRECACHE_SOUND( "weapons/beartrap_fire.wav" );
}

void CMonsterBearTrap::BearTrapThink( void )
{
        StudioFrameAdvance();

        if( pev->flags & FL_ONGROUND )
        {
                pev->movetype = MOVETYPE_NONE;
                pev->velocity = g_vecZero;
                pev->avelocity = g_vecZero;
        }

        if( m_bTriggered && gpGlobals->time > m_flUnlockTime )
        {
                UTIL_Remove( this );
                return;
        }

        pev->nextthink = gpGlobals->time + 0.1f;
}

void CMonsterBearTrap::BearTrapTouch( CBaseEntity *pOther )
{
        if( m_bTriggered )
                return;

        if( !pOther )
                return;

        if( pOther->edict() == pev->owner )
                return;

        if( !pOther->IsPlayer() && !( pOther->pev->flags & FL_MONSTER ) )
                return;

        if( pOther->pev->takedamage == DAMAGE_NO )
                return;

        m_bTriggered   = TRUE;
        m_flUnlockTime = gpGlobals->time + BEARTRAP_SLOW_TIME + 0.5f;

        EMIT_SOUND( ENT( pev ), CHAN_WEAPON, "weapons/beartrap_fire.wav", VOL_NORM, ATTN_NORM );

        entvars_t *pevAttacker = pev->owner ? VARS( pev->owner ) : pev;
        pOther->TakeDamage( pev, pevAttacker, BEARTRAP_DAMAGE, DMG_CRUSH | DMG_NEVERGIB );

        if( pOther->IsPlayer() )
        {
                CBasePlayer *pPlayer = (CBasePlayer *)pOther;
                pPlayer->pev->maxspeed = 60.0f;
                pPlayer->m_flNextAttack = gpGlobals->time + BEARTRAP_SLOW_TIME;
                CLIENT_COMMAND( pPlayer->edict(), "slot10\n" );
        }
        else
        {
                pOther->pev->velocity = pOther->pev->velocity * BEARTRAP_SLOW_FACTOR;
        }

        SetTouch( NULL );
        pev->solid = SOLID_NOT;

        SetThink( &CMonsterBearTrap::BearTrapThink );
        pev->nextthink = gpGlobals->time + BEARTRAP_SLOW_TIME;
}

void CMonsterBearTrap::Killed( entvars_t *pevAttacker, int iGib )
{
        UTIL_Remove( this );
}
