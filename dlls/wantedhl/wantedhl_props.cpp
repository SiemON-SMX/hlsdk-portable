/***
*
* WantedHL - Prop / scenery entities and ammo aliases
*
***/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "player.h"
#include "items.h"
#include "gamerules.h"

//=============================================================================
// CWantedProp - generic static decorative prop base
// Uses EF_NODRAW so it is invisible if no proper model exists yet.
// Map designers can assign a custom model via the "model" key.
//=============================================================================
class CWantedProp : public CBaseEntity
{
public:
        void Spawn( void )
        {
                pev->solid    = SOLID_NOT;
                pev->movetype = MOVETYPE_NONE;

                if( !FStringNull( pev->model ) )
                {
                        SET_MODEL( ENT( pev ), STRING( pev->model ) );
                }
                else
                {
                        pev->effects |= EF_NODRAW;
                }
        }
        int ObjectCaps( void ) { return FCAP_DONT_SAVE; }
};

class CItemWheelSide : public CWantedProp
{
public:
        void Spawn( void )
        {
                Precache();
                CWantedProp::Spawn();
        }
        void Precache( void ) {}
};
LINK_ENTITY_TO_CLASS( item_wheelside, CItemWheelSide );

class CItemWagonWheel : public CWantedProp
{
public:
        void Spawn( void )
        {
                Precache();
                CWantedProp::Spawn();
        }
        void Precache( void ) {}
};
LINK_ENTITY_TO_CLASS( item_wagonwheel, CItemWagonWheel );

class CItemBottle : public CWantedProp
{
public:
        void Spawn( void )
        {
                Precache();
                CWantedProp::Spawn();
        }
        void Precache( void ) {}
};
LINK_ENTITY_TO_CLASS( item_bottle, CItemBottle );

class CItemGlass : public CWantedProp
{
public:
        void Spawn( void )
        {
                Precache();
                CWantedProp::Spawn();
        }
        void Precache( void ) {}
};
LINK_ENTITY_TO_CLASS( item_glass, CItemGlass );

class CItemCactus : public CWantedProp
{
public:
        void Spawn( void )
        {
                Precache();
                CWantedProp::Spawn();
        }
        void Precache( void ) {}
};
LINK_ENTITY_TO_CLASS( item_cactus, CItemCactus );

class CItemPass : public CWantedProp
{
public:
        void Spawn( void )
        {
                Precache();
                CWantedProp::Spawn();
        }
        void Precache( void ) {}
};
LINK_ENTITY_TO_CLASS( item_pass, CItemPass );

class CBowAmmoAlias : public CBasePlayerAmmo
{
        void Spawn( void )
        {
                Precache();
                SET_MODEL( ENT( pev ), "models/w_bowammo.mdl" );
                CBasePlayerAmmo::Spawn();
        }
        void Precache( void )
        {
                PRECACHE_MODEL( "models/w_bowammo.mdl" );
                PRECACHE_SOUND( "items/9mmclip1.wav" );
        }
        BOOL AddAmmo( CBaseEntity *pOther )
        {
                if( pOther->GiveAmmo( AMMO_BOW_GIVE, "arrows", BOW_MAX_CARRY ) != -1 )
                {
                        EMIT_SOUND( ENT( pev ), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM );
                        return TRUE;
                }
                return FALSE;
        }
};
LINK_ENTITY_TO_CLASS( ammo_bow, CBowAmmoAlias );
