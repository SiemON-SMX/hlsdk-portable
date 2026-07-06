/***
*
*       Copyright (c) 1996-2002, Valve LLC. All rights reserved.
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
#include "animation.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "items.h"
#include "gamerules.h"
#include <algorithm>
#include <random>

extern int gmsgItemPickup;

class CHealthKit : public CItem
{
public:
        void Spawn( void );
        void Precache( void );
        BOOL MyTouch( CBasePlayer *pPlayer );

        BOOL pickable;

        virtual int             Save( CSave &save );
        virtual int             Restore( CRestore &restore );

        static  TYPEDESCRIPTION m_SaveData[];
};

LINK_ENTITY_TO_CLASS( item_healthkit, CHealthKit )

TYPEDESCRIPTION CHealthKit::m_SaveData[] =
{
        DEFINE_FIELD( CHealthKit, pickable, FIELD_BOOLEAN ),
};

IMPLEMENT_SAVERESTORE( CHealthKit, CItem )

void CHealthKit::Spawn( void )
{
        Precache();
        SET_MODEL( ENT( pev ), "models/w_medkit.mdl" );

        pickable = TRUE;

        CItem::Spawn();
}

void CHealthKit::Precache( void )
{
        PRECACHE_MODEL( "models/w_medkit.mdl" );
        PRECACHE_SOUND( "items/smallmedkit1.wav" );
        PRECACHE_SOUND( "items/pills.wav" );
        PRECACHE_SOUND( "items/pills_use.wav" );
}

BOOL CHealthKit::MyTouch( CBasePlayer *pPlayer )
{
        if( pPlayer->pev->deadflag != DEAD_NO
                || !( pPlayer->pev->weapons & ( 1 << WEAPON_SUIT ) )
                || !pickable )
        {
                return FALSE;
        }

        if( pPlayer->TakePainkiller() )
        {
                if( g_pGameRules->ItemShouldRespawn( this ) )
                {
                        Respawn();
                }
                else
                {
                        UTIL_Remove( this );
                }

                return TRUE;
        }

        return FALSE;
}

class CWallHealth : public CBaseToggle
{
public:
        void Spawn();
        void Precache( void );
        void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
        virtual int ObjectCaps( void ) { return ( CBaseToggle::ObjectCaps() | FCAP_CONTINUOUS_USE ); }

        void EXPORT WaitUntilOpened( void );

        enum WALL_HEALTH_ANIM
        {
                IDLE        = 0,
                OPENING     = 1,
                IDLE_OPENED = 2
        };
};

LINK_ENTITY_TO_CLASS( func_healthcharger, CWallHealth )

void CWallHealth::Spawn()
{
        Precache();

        pev->solid    = SOLID_SLIDEBOX;
        pev->movetype = MOVETYPE_PUSH;

        UTIL_SetOrigin( pev, pev->origin );
        UTIL_SetSize( pev, pev->mins, pev->maxs );
        SET_MODEL( ENT( pev ), STRING( pev->model ) );

        bool horizontallyPlaced = pev->size.x > pev->size.y;

        // Corner of the brush at middle height — used for wall-side trace
        Vector beginPos = Vector( pev->mins.x, pev->mins.y, pev->mins.z + pev->size.z );

        TraceResult tr1, tr2;
        if( horizontallyPlaced )
        {
                UTIL_TraceLine( beginPos, beginPos + Vector( 0,  13, 0 ), ignore_monsters, ENT( pev ), &tr1 );
                UTIL_TraceLine( beginPos, beginPos - Vector( 0,  13, 0 ), ignore_monsters, ENT( pev ), &tr2 );
        }
        else
        {
                UTIL_TraceLine( beginPos, beginPos - Vector( 13, 0, 0 ), ignore_monsters, ENT( pev ), &tr1 );
                UTIL_TraceLine( beginPos, beginPos + Vector( 13, 0, 0 ), ignore_monsters, ENT( pev ), &tr2 );
        }

        float tr1Length = ( beginPos - tr1.vecEndPos ).Length();
        float tr2Length = ( beginPos - tr2.vecEndPos ).Length();

        if( ( ( tr1Length > tr2Length ) && tr1.fInOpen ) || !tr2.fInOpen )
        {
                if( horizontallyPlaced )
                        pev->angles.y += 90;
                else
                        pev->angles.y += 180;
        }
        else
        {
                if( horizontallyPlaced )
                        pev->angles.y -= 90;
        }

        Vector realPos  = pev->origin + ( pev->mins + pev->maxs ) * 0.5f;
        realPos.z      -= pev->size.z / 3.0f;

        SET_MODEL( ENT( pev ), "models/w_med_cabinet.mdl" );

        UTIL_SetOrigin( pev, realPos );
        if( horizontallyPlaced )
                UTIL_SetSize( pev, Vector( -5, -6, 0 ), Vector( 5,  6,  50 ) );
        else
                UTIL_SetSize( pev, Vector( -6, -5, 0 ), Vector( 6,  5,  50 ) );

        // Cap to 4: arrays below hold exactly 4 spawn positions
        int painkillersToSpawn = (int)ceil( gSkillData.healthchargerCapacity / 10.0f ) - 1;
        if( painkillersToSpawn > 4 ) painkillersToSpawn = 4;
        if( painkillersToSpawn < 0 ) painkillersToSpawn = 0;

        float diversity  = 1.5f;
        float diversity2 = 3.5f;

        Vector horizontalSpots[4] =
        {
                realPos + Vector( -5 + RANDOM_FLOAT( -diversity2, diversity2 ), RANDOM_FLOAT( -diversity, diversity ), 13.3f ),
                realPos + Vector(  5 + RANDOM_FLOAT( -diversity2, diversity2 ), RANDOM_FLOAT( -diversity, diversity ), 13.3f ),
                realPos + Vector( -5 + RANDOM_FLOAT( -diversity2, diversity2 ), RANDOM_FLOAT( -diversity, diversity ),  0.5f ),
                realPos + Vector(  5 + RANDOM_FLOAT( -diversity2, diversity2 ), RANDOM_FLOAT( -diversity, diversity ),  0.5f )
        };

        Vector verticalSpots[4] =
        {
                realPos + Vector( RANDOM_FLOAT( -diversity,  diversity ),  -5 + RANDOM_FLOAT( -diversity2, diversity2 ), 13.3f ),
                realPos + Vector( RANDOM_FLOAT( -diversity2, diversity2 ),  5 + RANDOM_FLOAT( -diversity,  diversity  ), 13.3f ),
                realPos + Vector( RANDOM_FLOAT( -diversity,  diversity ),  -5 + RANDOM_FLOAT( -diversity2, diversity2 ),  0.6f ),
                realPos + Vector( RANDOM_FLOAT( -diversity2, diversity2 ),  5 + RANDOM_FLOAT( -diversity,  diversity  ),  0.6f )
        };

        static std::random_device rd;
        static std::mt19937 rng( rd() );

        std::shuffle( std::begin( horizontalSpots ), std::end( horizontalSpots ), rng );
        std::shuffle( std::begin( verticalSpots ),   std::end( verticalSpots ),   rng );

        for( int i = 0; i < painkillersToSpawn; i++ )
        {
                CHealthKit *healthKit = (CHealthKit *)CBaseEntity::Create(
                        "item_healthkit", realPos, Vector( 0, RANDOM_FLOAT( 0, 360 ), 0 ), NULL );
                healthKit->Spawn();
                UTIL_SetOrigin( healthKit->pev, horizontallyPlaced ? horizontalSpots[i] : verticalSpots[i] );

                // Kits are locked until the cabinet animation finishes opening
                healthKit->pev->movetype = MOVETYPE_NONE;
                healthKit->pickable      = FALSE;
        }
}

void CWallHealth::Precache()
{
        PRECACHE_MODEL( "models/w_medkit.mdl" );
        PRECACHE_MODEL( "models/w_med_cabinet.mdl" );

        PRECACHE_SOUND( "items/medshot4.wav" );
        PRECACHE_SOUND( "items/medshotno1.wav" );
        PRECACHE_SOUND( "items/medcharge4.wav" );
        PRECACHE_SOUND( "items/med_cabinet_open.wav" );
}

void CWallHealth::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
        if( !pActivator )
                return;
        if( !pActivator->IsPlayer() )
                return;

        if( pev->sequence == OPENING || pev->sequence == IDLE_OPENED )
                return;

        EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/med_cabinet_open.wav", 1, ATTN_NORM );

        pev->sequence = OPENING;
        ResetSequenceInfo();
        SetThink( &CWallHealth::WaitUntilOpened );
        pev->nextthink = pev->ltime + 0.1f;
}

void CWallHealth::WaitUntilOpened( void )
{
        pev->nextthink = pev->ltime + 0.1f;

        float flInterval = StudioFrameAdvance( 0.1f );
        DispatchAnimEvents( flInterval );

        if( m_fSequenceFinished )
        {
                pev->sequence = IDLE_OPENED;
                ResetSequenceInfo();

                Vector vecSrc        = pev->origin;
                CBaseEntity *pEntity = NULL;
                while( ( pEntity = UTIL_FindEntityInSphere( pEntity, vecSrc, 60.0f ) ) != NULL )
                {
                        if( FClassnameIs( pEntity->pev, "item_healthkit" ) )
                        {
                                CHealthKit *healthKit = (CHealthKit *)pEntity;
                                healthKit->pickable = TRUE;
                        }
                }
        }
}
