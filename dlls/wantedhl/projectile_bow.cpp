/***
*
*Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*This product contains software technology licensed from Id
*Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*All Rights Reserved.
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

#define ARROW_DAMAGE 40
#include "skill.h"
#include "projectile_bow.h"


LINK_ENTITY_TO_CLASS( arrow, CArrow );

void CArrow::Spawn( void )
{
        Precache();

        pev->movetype  = MOVETYPE_FLY;
        pev->solid     = SOLID_BBOX;

        SET_MODEL( ENT( pev ), "models/arrow.mdl" );
        UTIL_SetSize( pev, Vector( -1, -1, -1 ), Vector( 1, 1, 1 ) );

        pev->gravity     = 0.5f;
        pev->movetype    = MOVETYPE_TOSS;

        SetTouch( &CArrow::ArrowTouch );
        SetThink( &CArrow::ArrowThink );
        pev->nextthink   = gpGlobals->time + 6.0f;

        m_iDamage = (int)gSkillData.plrDmgBowArrow;
        m_bFired  = FALSE;
}

void CArrow::Precache( void )
{
        PRECACHE_MODEL( "models/arrow.mdl" );
        PRECACHE_SOUND( "weapons/bow_hit1.wav" );
        PRECACHE_SOUND( "weapons/bow_hitbod1.wav" );
}

CArrow *CArrow::Shoot( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity )
{
        CArrow *pArrow = GetClassPtr( (CArrow *)NULL );
        if( !pArrow )
                return NULL;
        pArrow->Spawn();

        UTIL_SetOrigin( pArrow->pev, vecStart );
        pArrow->pev->velocity = vecVelocity;
        pArrow->pev->owner    = ENT( pevOwner );

        // Orient the arrow along its velocity
        pArrow->pev->angles = UTIL_VecToAngles( vecVelocity );

        return pArrow;
}

void CArrow::ArrowTouch( CBaseEntity *pOther )
{
        if( !pOther || ( pOther->pev->solid == SOLID_BSP
                         || pOther->pev->movetype == MOVETYPE_PUSHSTEP ) )
        {
                // Hit world geometry — stick
                pev->movetype = MOVETYPE_FLY;
                pev->velocity = g_vecZero;
                pev->avelocity = g_vecZero;
                pev->gravity = 0.0f;
                SetTouch( NULL );
                SetThink( NULL );

                EMIT_SOUND( ENT( pev ), CHAN_WEAPON, "weapons/bow_hit1.wav", 1.0, ATTN_NORM );
                return;
        }

        if( pOther->pev->takedamage != DAMAGE_NO )
        {
                TraceResult tr;
                UTIL_TraceLine( pev->origin, pev->origin + pev->velocity * 0.1,
                                dont_ignore_monsters, ENT( pev->owner ), &tr );

                ClearMultiDamage();
                pOther->TraceAttack( pev->owner ? VARS( pev->owner ) : pev,
                                     (float)m_iDamage, pev->velocity.Normalize(), &tr, DMG_BULLET );
                ApplyMultiDamage( pev, pev->owner ? VARS( pev->owner ) : pev );

                EMIT_SOUND( ENT( pev ), CHAN_WEAPON, "weapons/bow_hitbod1.wav", 1.0, ATTN_NORM );
        }

        UTIL_Remove( this );
}

void CArrow::ArrowThink( void )
{
        UTIL_Remove( this );
}
