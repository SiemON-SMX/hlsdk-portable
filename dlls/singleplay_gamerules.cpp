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

#include        "extdll.h"
#include        "util.h"
#include        "cbase.h"
#include        "player.h"
#include        "weapons.h"
#include        "gamerules.h"
#include        "skill.h"
#include        "items.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern DLL_GLOBAL CGameRules    *g_pGameRules;
extern DLL_GLOBAL BOOL  g_fGameOver;
extern int gmsgDeathMsg;        // client dll messages
extern int gmsgScoreInfo;
extern int gmsgMOTD;

int gmsgEndCredits = 0;
int gmsgEndActiv   = 0;
int gmsgEndTitle   = 0;
int gmsgEndTime    = 0;
int gmsgEndScore   = 0;
int gmsgEndStat    = 0;


struct MapCommentaryEntry
{
        char    mapName[64];
        bool    useClassMatcher;        // true → match by className or targetName
        char    classMatcher[64];       // used when useClassMatcher == true
        int     modelIndexMatcher;      // used when useClassMatcher == false; -1 = map start
        char    soundPath[256];
        float   delay;
};

#define MAX_COMMENTARY_ENTRIES 512
static MapCommentaryEntry       s_commentary[MAX_COMMENTARY_ENTRIES];
static int                      s_commentaryCount = 0;
static char                     s_commentaryMap[64] = "";

static void Commentary_TrimRight( char *s )
{
        int n = (int)strlen( s ) - 1;
        while ( n >= 0 && ( s[n] == ' ' || s[n] == '\t' || s[n] == '\r' || s[n] == '\n' ) )
                s[n--] = '\0';
}

static void LoadMapCommentaryConfig( const char *mapName )
{
        s_commentaryCount = 0;
        strncpy( s_commentaryMap, mapName, sizeof( s_commentaryMap ) - 1 );
        s_commentaryMap[sizeof( s_commentaryMap ) - 1] = '\0';

        char gameDir[256];
        GET_GAME_DIR( gameDir );

        char filePath[512];
        snprintf( filePath, sizeof( filePath ), "%s/map_cfg/%s.txt", gameDir, mapName );

        FILE *f = fopen( filePath, "r" );
        if ( !f )
                return;

        bool inSection = false;
        char line[512];

        while ( fgets( line, sizeof( line ), f ) &&
                s_commentaryCount < MAX_COMMENTARY_ENTRIES )
        {
                // Strip inline comment
                char *cmt = strstr( line, "//" );
                if ( cmt ) *cmt = '\0';

                Commentary_TrimRight( line );

                char *trimmed = line;
                while ( *trimmed == ' ' || *trimmed == '\t' ) trimmed++;
                if ( !*trimmed ) continue;

                // Section header
                if ( trimmed[0] == '[' )
                {
                        inSection = ( strncmp( trimmed, "[max_commentary]", 16 ) == 0 );
                        continue;
                }

                if ( !inSection ) continue;

                // Format: mapname matcher soundpath [delay]
                char entryMap[64], matcher[128], soundPath[256];
                float delay = 0.0f;
                int n = sscanf( trimmed, "%63s %127s %255s %f",
                                entryMap, matcher, soundPath, &delay );
                if ( n < 3 ) continue;

                // Only load entries for the current map
                if ( strcmp( entryMap, mapName ) != 0 ) continue;

                MapCommentaryEntry &e = s_commentary[s_commentaryCount++];
                strncpy( e.mapName,   entryMap,  sizeof( e.mapName )   - 1 );
                e.mapName[sizeof( e.mapName ) - 1] = '\0';
                strncpy( e.soundPath, soundPath, sizeof( e.soundPath ) - 1 );
                e.soundPath[sizeof( e.soundPath ) - 1] = '\0';
                e.delay = delay;

                // Determine matcher type: numeric = model index, otherwise class/targetname
                char *endPtr;
                long idx = strtol( matcher, &endPtr, 10 );
                if ( endPtr != matcher && *endPtr == '\0' )
                {
                        e.useClassMatcher   = false;
                        e.modelIndexMatcher = (int)idx;
                        e.classMatcher[0]   = '\0';
                }
                else
                {
                        e.useClassMatcher   = true;
                        e.modelIndexMatcher = 0;
                        strncpy( e.classMatcher, matcher, sizeof( e.classMatcher ) - 1 );
                        e.classMatcher[sizeof( e.classMatcher ) - 1] = '\0';
                }

                // Precache the sound while the world is still loading
                PRECACHE_SOUND( e.soundPath );
        }

        fclose( f );
}

void HalfPayneHookModelIndex( edict_t *edict )
{
        if ( g_pGameRules && !g_pGameRules->IsMultiplayer() )
                static_cast<CHalfLifeRules*>( g_pGameRules )->HookModelIndex( edict );
}

void CHalfLifeRules::HookModelIndex( edict_t *activator )
{
        int         modelIndex = activator ? activator->v.modelindex : -1;
        const char *className  = activator ? STRING( activator->v.classname )  : "";
        const char *targetName = activator ? STRING( activator->v.targetname ) : "on_map_start";
        HookModelIndex( modelIndex, className, targetName );
}

void CHalfLifeRules::HookModelIndex( int modelIndex, const char *className, const char *targetName )
{
        CBasePlayer *pPlayer = static_cast<CBasePlayer*>(
                CBaseEntity::Instance( g_engfuncs.pfnPEntityOfEntIndex( 1 ) ) );
        if ( !pPlayer || !pPlayer->IsPlayer() )
                return;

        char key[320];
        snprintf( key, sizeof( key ), "%s|%d|%s|%s",
                  STRING( gpGlobals->mapname ), modelIndex, className, targetName );

        bool firstTime = !pPlayer->ModelIndexHasBeenHooked( key );
        if ( firstTime )
                pPlayer->RememberHookedModelIndex( ALLOC_STRING( key ) );

        OnHookedModelIndex( pPlayer, modelIndex, className, targetName, firstTime );
}

void CHalfLifeRules::OnHookedModelIndex( CBasePlayer *pPlayer, int modelIndex,
        const char *className, const char *targetName, bool firstTime )
{
        // Commentary sounds fire only on the first encounter
        if ( !firstTime )
                return;

        const char *mapName = STRING( gpGlobals->mapname );

        // Reload config if the map has changed since last load
        if ( strcmp( s_commentaryMap, mapName ) != 0 )
                LoadMapCommentaryConfig( mapName );

        for ( int i = 0; i < s_commentaryCount; i++ )
        {
                const MapCommentaryEntry &e = s_commentary[i];

                bool fits = false;
                if ( e.useClassMatcher )
                {
                        // Match by classname OR targetname
                        fits = ( strcmp( e.classMatcher, className )  == 0 ||
                                 strcmp( e.classMatcher, targetName ) == 0 );
                }
                else
                {
                        // Match by BSP model index (-1 == map start)
                        fits = ( e.modelIndexMatcher == modelIndex );
                }

                if ( fits )
                        pPlayer->AddToSoundQueue( ALLOC_STRING( e.soundPath ), e.delay );
        }
}

CHalfLifeRules::CHalfLifeRules( void )
{
        SERVER_COMMAND( "exec spserver.cfg\n" );
        RefreshSkillData();
        m_bMapStartHookFired = false;
        ended = false;
        m_flMapStartTime = gpGlobals->time;
        LoadMapCommentaryConfig( STRING( gpGlobals->mapname ) );

        if( !gmsgEndCredits )
        {
                gmsgEndCredits = REG_USER_MSG( "EndCredits", 0 );
                gmsgEndActiv   = REG_USER_MSG( "EndActiv",   1  );
                gmsgEndTitle   = REG_USER_MSG( "EndTitle",   -1 );
                gmsgEndTime    = REG_USER_MSG( "EndTime",    -1 );
                gmsgEndScore   = REG_USER_MSG( "EndScore",   -1 );
                gmsgEndStat    = REG_USER_MSG( "EndStat",    -1 );
        }
}

void CHalfLifeRules::End( CBasePlayer *pPlayer )
{
        if( ended )
                return;

        ended = true;

        pPlayer->pev->movetype = MOVETYPE_NONE;
        pPlayer->pev->flags |= FL_NOTARGET | FL_GODMODE;
        pPlayer->RemoveAllItems( true );

        OnEnd( pPlayer );
}

void CHalfLifeRules::OnEnd( CBasePlayer *pPlayer )
{
        // Credits scroll
        MESSAGE_BEGIN( MSG_ONE, gmsgEndCredits, NULL, pPlayer->pev );
        MESSAGE_END();

        // End stats screen — title
        MESSAGE_BEGIN( MSG_ONE, gmsgEndTitle, NULL, pPlayer->pev );
        WRITE_STRING( "LEVEL COMPLETE" );
        MESSAGE_END();

        // Time played since map load
        float timePlayed = gpGlobals->time - m_flMapStartTime;
        MESSAGE_BEGIN( MSG_ONE, gmsgEndTime, NULL, pPlayer->pev );
        WRITE_STRING( "TIME" );
        WRITE_LONG( (int)timePlayed );
        WRITE_LONG( 0 );  // no record in singleplayer
        WRITE_BYTE( 0 );      // record not beaten
        MESSAGE_END();

        // Kill count stat (if any)
        int kills = (int)pPlayer->pev->frags;
        if( kills > 0 )
        {
                char statBuf[64];
                sprintf( statBuf, "KILLS|%d", kills );
                MESSAGE_BEGIN( MSG_ONE, gmsgEndStat, NULL, pPlayer->pev );
                WRITE_STRING( statBuf );
                MESSAGE_END();
        }

        // Activate the end screen (not cheated)
        MESSAGE_BEGIN( MSG_ONE, gmsgEndActiv, NULL, pPlayer->pev );
        WRITE_BYTE( 0 );
        MESSAGE_END();
}

void CHalfLifeRules::Think( void )
{
}

BOOL CHalfLifeRules::IsMultiplayer( void )
{
        return FALSE;
}

BOOL CHalfLifeRules::IsDeathmatch( void )
{
        return FALSE;
}

BOOL CHalfLifeRules::IsCoOp( void )
{
        return FALSE;
}

BOOL CHalfLifeRules::FShouldSwitchWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon )
{
        if( !pPlayer->m_pActiveItem )
        {
                // player doesn't have an active item!
                return TRUE;
        }

        if( !pPlayer->m_iAutoWepSwitch )
        {
                return FALSE;
        }

        if( pPlayer->m_iAutoWepSwitch == 2
            && pPlayer->m_afButtonLast & ( IN_ATTACK | IN_ATTACK2 ) )
        {
                return FALSE;
        }

        if( !pPlayer->m_pActiveItem->CanHolster() )
        {
                return FALSE;
        }

        return TRUE;
}

BOOL HLGetNextBestWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pCurrentWeapon )
{
        CBasePlayerItem *pCheck;
        CBasePlayerItem *pBest;// this will be used in the event that we don't find a weapon in the same category.
        int iBestWeight;
        int i;

        iBestWeight = -1;// no weapon lower than -1 can be autoswitched to
        pBest = NULL;

        if( !pCurrentWeapon->CanHolster() )
        {
                // can't put this gun away right now, so can't switch.
                return FALSE;
        }

        for( i = 0; i < MAX_ITEM_TYPES; i++ )
        {
                pCheck = pPlayer->m_rgpPlayerItems[i];

                while( pCheck )
                {
                        if( !FBitSet( pCheck->iFlags(), ITEM_FLAG_NOAUTOSWITCHTO ))
                        {
                                if( pCheck->iWeight() > -1 && pCheck->iWeight() == pCurrentWeapon->iWeight() && pCheck != pCurrentWeapon )
                                {
                                        // this weapon is from the same category.
                                        if ( pCheck->CanDeploy() )
                                        {
                                                if ( pPlayer->SwitchWeapon( pCheck ) )
                                                {
                                                        return TRUE;
                                                }
                                        }
                                }
                                else if( pCheck->iWeight() > iBestWeight && pCheck != pCurrentWeapon )// don't reselect the weapon we're trying to get rid of
                                {
                                        if( pCheck->CanDeploy() )
                                        {
                                                // if this weapon is useable, flag it as the best
                                                iBestWeight = pCheck->iWeight();
                                                pBest = pCheck;
                                        }
                                }
                        }

                        pCheck = pCheck->m_pNext;
                }
        }


        if( !pBest )
        {
                return FALSE;
        }

        pPlayer->SwitchWeapon( pBest );

        return TRUE;
}

BOOL CHalfLifeRules::GetNextBestWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pCurrentWeapon )
{
        if( pCurrentWeapon && FBitSet( pCurrentWeapon->iFlags(), ITEM_FLAG_EXHAUSTIBLE ))
                return HLGetNextBestWeapon( pPlayer, pCurrentWeapon );
        return FALSE;
}

BOOL CHalfLifeRules::ClientConnected( edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[128] )
{
        return TRUE;
}

void CHalfLifeRules::InitHUD( CBasePlayer *pl )
{
}

void CHalfLifeRules::ClientDisconnected( edict_t *pClient )
{
}

float CHalfLifeRules::FlPlayerFallDamage( CBasePlayer *pPlayer )
{
        pPlayer->m_flFallVelocity -= PLAYER_MAX_SAFE_FALL_SPEED;
        return pPlayer->m_flFallVelocity * DAMAGE_FOR_FALL_SPEED;
}

void CHalfLifeRules::PlayerSpawn( CBasePlayer *pPlayer )
{
        if ( !m_bMapStartHookFired )
        {
                m_bMapStartHookFired = true;
                // Reload config if map changed since constructor (changelevel)
                if ( strcmp( s_commentaryMap, STRING( gpGlobals->mapname ) ) != 0 )
                        LoadMapCommentaryConfig( STRING( gpGlobals->mapname ) );
                HookModelIndex( (edict_t *)NULL );      // fires model index -1 / "on_map_start"
        }
}

BOOL CHalfLifeRules::AllowAutoTargetCrosshair( void )
{
        return ( g_iSkillLevel == SKILL_EASY );
}

void CHalfLifeRules::PlayerThink( CBasePlayer *pPlayer )
{
}

BOOL CHalfLifeRules::FPlayerCanRespawn( CBasePlayer *pPlayer )
{
        return TRUE;
}

float CHalfLifeRules::FlPlayerSpawnTime( CBasePlayer *pPlayer )
{
        return gpGlobals->time;//now!
}

int CHalfLifeRules::IPointsForKill( CBasePlayer *pAttacker, CBasePlayer *pKilled )
{
        return 1;
}

void CHalfLifeRules::PlayerKilled( CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor )
{
}

void CHalfLifeRules::DeathNotice( CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor )
{
}

void CHalfLifeRules::PlayerGotWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon )
{
        HookModelIndex( pWeapon->edict() );
}

float CHalfLifeRules::FlWeaponRespawnTime( CBasePlayerItem *pWeapon )
{
        return -1;
}

float CHalfLifeRules::FlWeaponTryRespawn( CBasePlayerItem *pWeapon )
{
        return 0;
}

Vector CHalfLifeRules::VecWeaponRespawnSpot( CBasePlayerItem *pWeapon )
{
        return pWeapon->pev->origin;
}

int CHalfLifeRules::WeaponShouldRespawn( CBasePlayerItem *pWeapon )
{
        return GR_WEAPON_RESPAWN_NO;
}

BOOL CHalfLifeRules::CanHaveItem( CBasePlayer *pPlayer, CItem *pItem )
{
        return TRUE;
}

void CHalfLifeRules::PlayerGotItem( CBasePlayer *pPlayer, CItem *pItem )
{
        HookModelIndex( pItem->edict() );
}

int CHalfLifeRules::ItemShouldRespawn( CItem *pItem )
{
        return GR_ITEM_RESPAWN_NO;
}

float CHalfLifeRules::FlItemRespawnTime( CItem *pItem )
{
        return -1;
}

Vector CHalfLifeRules::VecItemRespawnSpot( CItem *pItem )
{
        return pItem->pev->origin;
}

BOOL CHalfLifeRules::IsAllowedToSpawn( CBaseEntity *pEntity )
{
        return TRUE;
}

void CHalfLifeRules::PlayerGotAmmo( CBasePlayer *pPlayer, char *szName, int iCount )
{
}

int CHalfLifeRules::AmmoShouldRespawn( CBasePlayerAmmo *pAmmo )
{
        return GR_AMMO_RESPAWN_NO;
}

float CHalfLifeRules::FlAmmoRespawnTime( CBasePlayerAmmo *pAmmo )
{
        return -1;
}

Vector CHalfLifeRules::VecAmmoRespawnSpot( CBasePlayerAmmo *pAmmo )
{
        return pAmmo->pev->origin;
}

float CHalfLifeRules::FlHealthChargerRechargeTime( void )
{
        return 0;// don't recharge
}

int CHalfLifeRules::DeadPlayerWeapons( CBasePlayer *pPlayer )
{
        return GR_PLR_DROP_GUN_NO;
}

int CHalfLifeRules::DeadPlayerAmmo( CBasePlayer *pPlayer )
{
        return GR_PLR_DROP_AMMO_NO;
}

int CHalfLifeRules::PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget )
{
        // why would a single player in half life need this? 
        return GR_NOTTEAMMATE;
}

BOOL CHalfLifeRules::FAllowMonsters( void )
{
        return TRUE;
}
