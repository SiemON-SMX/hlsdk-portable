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

/*

===== client.cpp ========================================================

  client/server game specific stuff

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "saverestore.h"
#include "player.h"
#include "spectator.h"
#include "client.h"
#include "soundent.h"
#include "gamerules.h"
#include "game.h"
#include "customentity.h"
#include "weapons.h"
#include "weaponinfo.h"
#include "usercmd.h"
#include "netadr.h"
#include "pm_shared.h"

extern DLL_GLOBAL ULONG         g_ulModelIndexPlayer;
extern DLL_GLOBAL BOOL          g_fGameOver;
extern DLL_GLOBAL int           g_iSkillLevel;
extern DLL_GLOBAL ULONG         g_ulFrameCount;

extern void CopyToBodyQue( entvars_t* pev );
extern int giPrecacheGrunt;
extern int gmsgSayText;

extern cvar_t allow_spectators;
extern cvar_t multibyte_only;

extern int g_teamplay;

void LinkUserMessages( void );

/*
 * used by kill command and disconnect command
 * ROBIN: Moved here from player.cpp, to allow multiple player models
 */
void set_suicide_frame( entvars_t *pev )
{       
        if( !FStrEq( STRING( pev->model ), "models/player.mdl" ) )
                return; // allready gibbed

        //pev->frame = $deatha11;
        pev->solid = SOLID_NOT;
        pev->movetype = MOVETYPE_TOSS;
        pev->deadflag = DEAD_DEAD;
        pev->nextthink = -1.0f;
}


/*
===========
ClientConnect

called when a player connects to a server
============
*/
BOOL ClientConnect( edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[128] )
{
        return g_pGameRules->ClientConnected( pEntity, pszName, pszAddress, szRejectReason );

}

/*
===========
ClientDisconnect

called when a player disconnects from a server

GLOBALS ASSUMED SET:  g_fGameOver
============
*/
void ClientDisconnect( edict_t *pEntity )
{
        if( g_fGameOver )
                return;

        char text[256] = "";
        if( pEntity->v.netname )
                safe_snprintf( text, sizeof( text ), "- %s has left the game\n", STRING( pEntity->v.netname ));

        MESSAGE_BEGIN( MSG_ALL, gmsgSayText, NULL );
                WRITE_BYTE( ENTINDEX( pEntity ) );
                WRITE_STRING( text );
        MESSAGE_END();

        CSound *pSound = CSoundEnt::SoundPointerForIndex( CSoundEnt::ClientSoundIndex( pEntity ) );

        // since this client isn't around to think anymore, reset their sound. 
        if( pSound )
        {
                pSound->Reset();
        }

        // since the edict doesn't get deleted, fix it so it doesn't interfere.
        pEntity->v.takedamage = DAMAGE_NO;// don't attract autoaim
        pEntity->v.solid = SOLID_NOT;// nonsolid
        pEntity->v.effects = 0;// clear any effects
        pEntity->v.flags = 0;// clear any flags
        UTIL_SetOrigin( &pEntity->v, pEntity->v.origin );

        g_pGameRules->ClientDisconnected( pEntity );
}

// called by ClientKill and DeadThink
void respawn( entvars_t *pev, BOOL fCopyCorpse )
{
        if( gpGlobals->coop || gpGlobals->deathmatch )
        {
                if( fCopyCorpse )
                {
                        // make a copy of the dead body for appearances sake
                        CopyToBodyQue( pev );
                }

                // respawn player
                GetClassPtr( (CBasePlayer *)pev )->Spawn();
        }
        else
        {       // restart the entire server
                SERVER_COMMAND( "reload\n" );
        }
}

/*
============
ClientKill

Player entered the suicide command

GLOBALS ASSUMED SET:  g_ulModelIndexPlayer
============
*/
void ClientKill( edict_t *pEntity )
{
        entvars_t *pev = &pEntity->v;

        CBasePlayer *pl = (CBasePlayer*)CBasePlayer::Instance( pev );

        if( pl->m_fNextSuicideTime > gpGlobals->time )
                return;  // prevent suiciding too ofter

        pl->m_fNextSuicideTime = gpGlobals->time + 1.0f;  // don't let them suicide for 5 seconds after suiciding

        // have the player kill themself
        pev->health = 0;
        pl->Killed( pev, GIB_NEVER );

}

/*
===========
ClientPutInServer

called each time a player is spawned
============
*/

class CMapMusicCue : public CBaseEntity
{
public:
        void Spawn( void );
        void EXPORT FireMusic( void );
        int ObjectCaps( void ) { return FCAP_DONT_SAVE; }

        char m_szTrack[192]; // empty string = stop music only
        bool m_bLoop;
};

LINK_ENTITY_TO_CLASS( map_music_cue, CMapMusicCue )

void CMapMusicCue::Spawn( void )
{
        pev->classname = MAKE_STRING( "map_music_cue" );
        pev->solid     = SOLID_NOT;
        pev->movetype  = MOVETYPE_NONE;
        pev->effects  |= EF_NODRAW;

        SetThink( &CMapMusicCue::FireMusic );
        pev->nextthink = gpGlobals->time + 1.0f;
}

void CMapMusicCue::FireMusic( void )
{
        edict_t *pClient = g_engfuncs.pfnPEntityOfEntIndex( 1 );
        if( !pClient || pClient->free || ( pClient->v.flags & FL_DORMANT ) )
        {
                // Client not ready yet; retry shortly.
                pev->nextthink = gpGlobals->time + 0.25f;
                return;
        }

        CLIENT_COMMAND( pClient, "mp3 stop\n" );

        if( m_szTrack[0] )
        {
                char cmd[256];
                if( m_bLoop )
                        snprintf( cmd, sizeof( cmd ), "mp3 loop %s\n", m_szTrack );
                else
                        snprintf( cmd, sizeof( cmd ), "mp3 play %s\n", m_szTrack );
                CLIENT_COMMAND( pClient, "%s", cmd );
        }

        UTIL_Remove( this );
}

struct MapMusicEntry
{
        const char *map;
        const char *track;
        bool        loop;
};

static const MapMusicEntry g_mapMusicTable[] =
{
        // Train intro sequence — plays once (no looping flag in config)
        { "c0a0",    "sound/music/c0a0_1.mp3",                       false },
        { "c0a0a",   "sound/music/c0a0_2.mp3",                       false },
        { "c0a0b",   "sound/music/c0a0_3.mp3",                       false },
        { "c0a0c",   "sound/music/c0a0_4.mp3",                       false },
        { "c0a0d",   "sound/music/c0a0_5.mp3",                       false },

        // Chapter 1
        { "c1a0c",   "sound/music/payne3/ambient1.mp3",              true  },
        { "c1a1",    "sound/music/payne3/ambient1.mp3",              true  },
        { "c1a2",    "sound/music/payne3/ambient2.mp3",              true  },
        { "c1a2a",   "sound/music/payne3/ambient2.mp3",              true  },
        { "c1a2b",   "sound/music/payne3/ambient2.mp3",              true  },
        { "c1a4",    "sound/music/payne3/ambient3.mp3",              true  },
        { "c1a4b",   "sound/music/payne3/ambient3.mp3",              true  },
        { "c1a4e",   "sound/music/payne3/ambient3.mp3",              true  },
        { "c1a4f",   "sound/music/payne3/ambient3.mp3",              true  },
        { "c1a4g",   "sound/music/payne3/no_garg.mp3",               true  },
        { "c1a4i",   "sound/music/payne3/ambient3.mp3",              true  },

        // Chapter 2 — On A Rail / Apprehension / Residue Processing
        { "c2a1a",   "sound/music/payne3/power.mp3",                 true  },
        { "c2a2",    "sound/music/payne3/no_garg.mp3",               true  },
        { "c2a2a",   "sound/music/payne3/train_calm.mp3",            true  },
        { "c2a2b2",  "sound/music/payne3/train_calm.mp3",            true  },
        { "c2a2c",   "sound/music/payne3/train.mp3",                 true  },
        { "c2a2d",   "sound/music/payne3/train.mp3",                 true  },
        { "c2a2e",   "sound/music/payne3/train.mp3",                 true  },
        { "c2a2f",   "sound/music/payne3/train.mp3",                 true  },
        { "c2a2g",   "sound/music/payne3/train_calm_very.mp3",       true  },
        { "c2a3",    "sound/music/payne3/apprehension.mp3",          true  },
        { "c2a3b",   "sound/music/payne1/ms_valkyr.mp3",             true  },
        { "c2a3c",   "sound/music/payne1/just_forget_about_it.mp3",  true  },
        { "c2a3d",   "sound/music/payne1/just_forget_about_it.mp3",  true  },
        { "c2a4a",   "sound/music/payne1/byzantine_power_game.mp3",  true  },
        { "c2a4b",   "sound/music/payne1/byzantine_power_game.mp3",  true  },
        { "c2a4c",   "sound/music/payne1/byzantine_power_game.mp3",  true  },
        { "c2a4d",   "sound/music/payne3/ethics.mp3",                true  },
        { "c2a4e",   "sound/music/payne3/ethics.mp3",                true  },
        { "c2a4f",   "sound/music/payne3/ethics.mp3",                true  },
        { "c2a4g",   "valve/media/Half-Life02.mp3",                  false },
        { "c2a5",    "valve/media/Half-Life02.mp3",                  false },
        { "c2a5b",   "sound/music/payne1/killer_suits.mp3",          false },
        { "c2a5e",   "sound/music/payne3/dead.mp3",                  true  },
        { "c2a5f",   "sound/music/payne3/dead.mp3",                  true  },

        // Chapter 3 — Forget About Freeman / Lambda Core
        { "c3a2",    "sound/music/payne3/future.mp3",                true  },
        { "c3a2a",   "sound/music/payne3/future.mp3",                true  },
        { "c3a2b",   "sound/music/payne3/future.mp3",                true  },
        { "c3a2c",   "sound/music/payne3/future.mp3",                true  },

        // Chapter 4 — Xen
        { "c4a1",    "sound/music/payne3/ambient4.mp3",              false },
        { "c4a1a",   "sound/music/payne3/ambient5.mp3",              true  },
        { "c4a1b",   "sound/music/payne3/ambient5.mp3",              true  },
        { "c4a1d",   "sound/music/payne3/alien_factory.mp3",         true  },
        { "c4a1e",   "sound/music/payne3/alien_factory.mp3",         true  },

        // Special / Nightmare
        { "nightmare", "sound/music/payne1/nightmare.mp3",           true  },
};

static void HP_SendMapMusic( void )
{
        const char *szMap = STRING( gpGlobals->mapname );
        if( !szMap || !szMap[0] )
                return;

        const MapMusicEntry *entry = NULL;
        for( unsigned i = 0; i < sizeof( g_mapMusicTable ) / sizeof( g_mapMusicTable[0] ); i++ )
        {
                if( strcmp( szMap, g_mapMusicTable[i].map ) == 0 )
                {
                        entry = &g_mapMusicTable[i];
                        break;
                }
        }

        CMapMusicCue *pCue = GetClassPtr( (CMapMusicCue *)NULL );
        if( !pCue )
                return;

        if( entry )
        {
                strncpy( pCue->m_szTrack, entry->track, sizeof( pCue->m_szTrack ) - 1 );
                pCue->m_szTrack[sizeof( pCue->m_szTrack ) - 1] = '\0';
                pCue->m_bLoop = entry->loop;
        }
        else
        {
                pCue->m_szTrack[0] = '\0';
                pCue->m_bLoop = false;
        }

        pCue->Spawn();
}


struct KerotanEntry
{
        const char *map;
        float x, y, z;
        float yaw;
};

static const KerotanEntry g_kerotanTable[] =
{
        // --- Chapter 1: Unforeseen Consequences ---
        { "c1a0",   -814.38f,   587.13f,   -99.80f, -90.0f },
        { "c1a0a",   546.13f,   700.75f,   192.00f, 141.0f },
        { "c1a0b",   527.75f,   468.63f,  -143.88f,-131.0f },
        { "c1a0c",   527.75f,   468.63f,  -143.88f,-131.0f },
        { "c1a0d", -3350.63f,  1463.25f,  -212.00f,  25.0f },

        // --- Chapter 2: We've Got Hostiles ---
        { "c1a1",    546.13f,   700.75f,   192.00f, 141.0f },
        { "c1a1a", -3350.63f,  1466.25f,  -212.00f,  25.0f },
        { "c1a1b",  1678.00f, -1039.00f,  -147.88f, 180.0f },
        { "c1a1c",   168.25f, -1337.88f, -2511.88f,  80.0f },
        { "c1a1d",  1288.75f,   693.25f,    16.00f, -74.0f },
        { "c1a1f",  -814.38f,   587.13f,   -99.80f, -90.0f },

        // --- Chapter 3: Office Complex ---
        { "c1a2",    198.88f,  -867.13f,  -375.88f, 127.0f },
        { "c1a2a",    87.25f,  -935.63f,  -219.88f, 110.0f },
        { "c1a2b",  1053.50f,  -738.63f,   220.00f,  33.0f },
        { "c1a2c", -1746.25f, -1655.13f,  -231.88f, 180.0f },
        { "c1a2d",   625.38f, -2351.50f,  -357.88f,  90.0f },

        // --- Chapter 4: "Forget About Freeman!" ---
        { "c1a3",    313.38f,  1681.13f,   146.00f, -90.0f },
        { "c1a3a", -1242.38f, -2829.38f,    32.00f,-161.0f },
        { "c1a3b", -1468.88f, -1634.13f,   980.00f,  45.0f },
        { "c1a3c",  -594.00f, -1529.88f,   224.00f, 150.0f },
        { "c1a3d",  1421.25f,  -256.13f,   876.00f,   0.0f },

        // --- Chapter 5: Lambda Core ---
        { "c1a4",  -1136.63f, -3105.25f, -3077.88f,  24.0f },
        { "c1a4b",  -591.63f,  -508.38f, -1266.88f,  80.0f },
        { "c1a4d",  1304.50f,   911.38f, -1255.88f,  90.0f },
        { "c1a4e",  1423.25f,  1118.50f,   320.00f,-145.0f },
        { "c1a4f",     1.63f,   -30.38f,  1296.00f, -61.0f },
        { "c1a4g",  1269.25f, -1362.88f, -2207.88f,  90.0f },
        { "c1a4i",  -392.50f,  -842.63f, -1411.88f, 111.0f },
        { "c1a4j",   757.25f, -2112.50f,   181.75f,  60.0f },
        { "c1a4k", -2440.50f, -1701.63f, -3259.88f, 120.0f },

        // --- Chapter 6: Residue Processing ---
        { "c2a1",   -800.38f,   241.75f,   272.00f, -90.0f },
        { "c2a1a",   379.88f,  2552.13f,   528.00f,  90.0f },
        { "c2a1b",  1214.13f,  1315.38f,     0.00f, -48.0f },

        // --- Chapter 7: Questionable Ethics ---
        { "c2a2",  -1778.63f, -3280.50f, -1175.88f, -90.0f },
        { "c2a2a", -1904.13f,   763.75f,   124.00f,-120.0f },
        { "c2a2b1",-3599.25f,  -729.75f,   364.00f, -90.0f },
        { "c2a2b2", 1560.50f,   567.13f,   280.00f,-153.0f },
        { "c2a2c",   -19.00f, -1152.50f,   932.00f,-120.0f },
        { "c2a2d",  2415.25f,  2827.75f,  -293.88f, 180.0f },
        { "c2a2e",  3019.25f, -1440.38f,  -571.88f,  70.0f },
        { "c2a2f",  -957.13f,  -794.63f,   192.00f,  70.0f },
        { "c2a2g", -1078.13f, -1488.88f,  -487.88f,  16.0f },
        { "c2a2h",  1654.38f,   258.38f,  -651.88f, 180.0f },

        // --- Chapter 8: Surface Tension ---
        { "c2a3",  -1061.50f,  -824.75f,  1310.00f, -90.0f },
        { "c2a3a",   752.38f,   968.38f,  1269.00f, -90.0f },
        { "c2a3b",  1574.75f,  1777.00f,   848.00f, -54.0f },
        { "c2a3c",  2828.88f,  1717.88f,   914.00f,   0.0f },
        { "c2a3d",   476.75f,  -715.00f,   836.00f,  90.0f },
        { "c2a3e", -2905.25f,   434.88f,   712.00f,   0.0f },

        // --- Chapter 9: "Forget About Freeman!" (Interloper) ---
        { "c2a4",   -438.63f,   545.88f,   288.00f, -90.0f },
        { "c2a4a",  -594.13f,  1243.63f,     0.00f,  30.0f },
        { "c2a4b",  -982.75f,  -461.50f,  -799.88f,   0.0f },
        { "c2a4c",  2234.75f,    99.63f,  -831.88f, -45.0f },
        { "c2a4d",  2169.88f,  1285.63f,    52.00f, 180.0f },
        { "c2a4e",   263.50f,  -175.00f,   160.00f,  90.0f },
        { "c2a4f",  -173.00f,  -823.25f,   704.00f,-130.0f },
        { "c2a4g",  -577.88f,  -858.13f,   416.00f, -50.0f },

        // --- Chapter 10: Interloper ---
        { "c2a5",  -1637.63f,   131.25f,   304.00f, -20.0f },
        { "c2a5a",   700.63f,   252.75f,  -367.88f, -70.0f },
        { "c2a5b", -1337.38f,   961.50f,   192.00f,   0.0f },
        { "c2a5c",  1227.63f,   971.88f,  -149.88f,-110.0f },
        { "c2a5d",   134.00f,   666.00f,  -135.88f, -90.0f },
        { "c2a5e",   190.88f,   894.63f, -1595.88f,-118.0f },
        { "c2a5f",  -475.13f,  -669.50f, -1679.88f, 180.0f },
        { "c2a5g",    89.00f,   522.63f, -1471.88f,  90.0f },
        { "c2a5w", -1704.25f,  -778.63f,  1121.13f,  15.0f },
        { "c2a5x",   964.75f,   450.00f,   929.00f,-110.0f },

        // --- Chapter 11: Gonarch's Lair ---
        { "c3a1",  -2232.75f,   732.50f,   448.00f, -90.0f },
        { "c3a1a", -3251.63f, -1210.50f,   376.00f,  90.0f },
        { "c3a1b",  -977.13f,   847.25f,   -95.88f, -90.0f },

        // --- Chapter 12: Interloper (Xen) ---
        { "c3a2",   -464.13f,  1630.13f,   336.00f,  90.0f },
        { "c3a2a", -2296.13f,   301.50f,  -215.88f, 180.0f },
        { "c3a2b",  -109.63f,   494.63f,   640.00f, 141.0f },
        { "c3a2c",  1393.25f,  -789.38f, -1625.88f, 140.0f },
        { "c3a2d",   -16.00f,  -328.25f,    64.00f, 180.0f },
        { "c3a2e",  1776.50f,  1646.38f,  2143.00f,  90.0f },
        { "c3a2f",   -42.63f,  -672.50f, -1791.88f,  90.0f },

        // --- Chapter 13: Nihilanth ---
        { "c4a1",   -319.50f, -1840.38f,  -241.25f, -90.0f },
        { "c4a1a",  1253.75f,  -382.75f,  -275.50f,-126.0f },
        { "c4a1b",  1595.75f, -1627.75f,   480.00f, 173.0f },
        { "c4a1c",  -712.38f, -1639.50f, -1023.88f, -70.0f },
        { "c4a1d",  1029.50f, -1329.38f,  -223.88f,-120.0f },
        { "c4a1e",   756.13f, -1692.25f,    88.00f,-140.0f },
        { "c4a1f",  1831.88f, -2008.38f,   210.88f, -45.0f },

        // --- Chapter 14: Endgame ---
        { "c4a2",   1445.38f,   460.88f,   240.00f,-140.0f },
        { "c4a2a", -1015.63f,   109.13f,  -255.88f, -90.0f },
        { "c4a2b",  -202.38f,   538.63f,  -479.88f,  40.0f },
        { "c4a3",   1864.25f,  -664.00f,  2260.00f, 150.0f },
};

#define NUM_KEROTANS ( (int)( sizeof( g_kerotanTable ) / sizeof( g_kerotanTable[0] ) ) )

// Spawn the Kerotan for the current map (if one is defined and not yet present).
static void HP_SpawnKerotan( void )
{
        const char *szMap = STRING( gpGlobals->mapname );
        if( !szMap || !szMap[0] )
                return;

        // Find the table entry for this map.
        const KerotanEntry *entry = NULL;
        for( int i = 0; i < NUM_KEROTANS; i++ )
        {
                if( strcmp( szMap, g_kerotanTable[i].map ) == 0 )
                {
                        entry = &g_kerotanTable[i];
                        break;
                }
        }

        if( !entry )
                return; // No kerotan on this map (e.g. c0a0 intro maps).

        // Guard: if a kerotan already exists (save/reload) don't spawn another.
        if( !FNullEnt( FIND_ENTITY_BY_CLASSNAME( ENT(0), "kerotan" ) ) )
                return;

        Vector origin( entry->x, entry->y, entry->z );
        Vector angles( 0.0f, entry->yaw, 0.0f );

        CBaseEntity *pKerotan = CBaseEntity::Create( "kerotan", origin, angles, NULL );
        if( pKerotan )
                DROP_TO_FLOOR( ENT( pKerotan->pev ) );
}

void ClientPutInServer( edict_t *pEntity )
{
        CBasePlayer *pPlayer;

        entvars_t *pev = &pEntity->v;

        pPlayer = GetClassPtr( (CBasePlayer *)pev );
        pPlayer->SetCustomDecalFrames( -1 ); // Assume none;
        pPlayer->SetPrefsFromUserinfo( g_engfuncs.pfnGetInfoKeyBuffer( pEntity ) );

        // Allocate a CBasePlayer for pev, and call spawn
        pPlayer->Spawn();

        // Reset interpolation during first frame
        pPlayer->pev->effects |= EF_NOINTERP;

        pPlayer->pev->iuser1 = 0;
        pPlayer->pev->iuser2 = 0;

}

#if !NO_VOICEGAMEMGR
#include "voice_gamemgr.h"
extern CVoiceGameMgr g_VoiceGameMgr;
#endif

bool Q_IsValidUChar32( unsigned int uVal )
{
        return ( ( uVal - 0x0u ) < 0x110000u ) && ( (uVal - 0x00D800u) > 0x7FFu ) && ( (uVal & 0xFFFFu) < 0xFFFEu ) && ( ( uVal - 0x00FDD0u ) > 0x1Fu );
}

int Q_UTF8ToUChar32( const char *pUTF8_, unsigned int &uValueOut, bool &bErrorOut )
{
        const unsigned char *pUTF8 = (const unsigned char*)pUTF8_;

        int nBytes = 1;
        unsigned int uValue = pUTF8[0];
        unsigned int uMinValue = 0;

        // 0....... single byte
        if( uValue < 0x80 )
                goto decodeFinishedNoCheck;

        // Expecting at least a two-byte sequence with 0xC0 <= first <= 0xF7 (110...... and 11110...)
        if( ( uValue - 0xC0u ) > 0x37u || ( pUTF8[1] & 0xC0 ) != 0x80 )
                goto decodeError;

        uValue = ( uValue << 6 ) - ( 0xC0 << 6 ) + pUTF8[1] - 0x80;
        nBytes = 2;
        uMinValue = 0x80;

        // 110..... two-byte lead byte
        if( !( uValue & ( 0x20 << 6 ) ) )
                goto decodeFinished;

        // Expecting at least a three-byte sequence
        if( ( pUTF8[2] & 0xC0 ) != 0x80 )
                goto decodeError;

        uValue = ( uValue << 6 ) - ( 0x20 << 12 ) + pUTF8[2] - 0x80;
        nBytes = 3;
        uMinValue = 0x800;

        // 1110.... three-byte lead byte
decodeFinished:
        if( uValue >= uMinValue && Q_IsValidUChar32( uValue ) )
        {
decodeFinishedNoCheck:
                uValueOut = uValue;
                bErrorOut = false;
                return nBytes;
        }
decodeError:
        uValueOut = '?';
        bErrorOut = true;
        return nBytes;
#if 0
decodeFinishedMaybeCESU8:
        if( ( uValue - 0xD800u ) < 0x400u && pUTF8[3] == 0xED && (unsigned char)( pUTF8[4] - 0xB0 ) < 0x10 && ( pUTF8[5] & 0xC0 ) == 0x80 )
        {
                uValue = 0x10000 + ( ( uValue - 0xD800u ) << 10 ) + ( (unsigned char)( pUTF8[4] - 0xB0 ) << 6 ) + pUTF8[5] - 0x80;
                nBytes = 6;
                uMinValue = 0x10000;
        }
        goto decodeFinished;
#endif
}

bool Q_UnicodeValidate( const char *pUTF8 )
{
        bool bError = false;

        if( !multibyte_only.value )
                return true;

        while( *pUTF8 )
        {
                unsigned int uVal;
                int nCharSize = Q_UTF8ToUChar32( pUTF8, uVal, bError );
                if( bError || nCharSize == 6 )
                        return false;
                pUTF8 += nCharSize;
        }
        return true;
}


void Host_Say( edict_t *pEntity, int teamonly )
{
        CBasePlayer *client;
        int             j;
        char    *p; //, *pc;
        char    text[128];
        char    szTemp[256];
        const char *cpSay = "say";
        const char *cpSayTeam = "say_team";
        const char *pcmd = CMD_ARGV( 0 );

        // We can get a raw string now, without the "say " prepended
        if( CMD_ARGC() == 0 )
                return;

        entvars_t *pev = &pEntity->v;
        CBasePlayer* player = GetClassPtr( (CBasePlayer *)pev );

        //Not yet.
        if( player->m_flNextChatTime > gpGlobals->time )
                 return;

        if( !stricmp( pcmd, cpSay ) || !stricmp( pcmd, cpSayTeam ) )
        {
                if( CMD_ARGC() >= 2 )
                {
                        p = (char *)CMD_ARGS();
                }
                else
                {
                        // say with a blank message, nothing to do
                        return;
                }
        }
        else  // Raw text, need to prepend argv[0]
        {
                if( CMD_ARGC() >= 2 )
                {
                        safe_snprintf( szTemp, sizeof( szTemp ), "%s %s", (char *)pcmd, (char *)CMD_ARGS() );
                }
                else
                {
                        // Just a one word command, use the first word...sigh
                        strlcpy( szTemp, (char *)pcmd, sizeof( szTemp ));
                }

                p = szTemp;
        }

        // remove quotes if present
        if( p && *p == '"' )
        {
                p++;
                p[strlen( p ) - 1] = 0;
        }

        if( !p || !p[0] || !Q_UnicodeValidate ( p ) )
                return;  // no character found, so say nothing

        // turn on color set 2  (color on,  no sound)
        if( player->IsObserver() && ( teamonly ) )
                safe_snprintf( text, sizeof( text ), "%c(SPEC) %s: ", 2, STRING( pEntity->v.netname ) );
        else if( teamonly )
                safe_snprintf( text, sizeof( text ), "%c(TEAM) %s: ", 2, STRING( pEntity->v.netname ) );
        else
                safe_snprintf( text, sizeof( text ), "%c%s: ", 2, STRING( pEntity->v.netname ) );

        j = sizeof( text ) - 2 - strlen( text );  // -2 for /n and null terminator
        if( (int)strlen( p ) > j )
                p[j] = 0;

        strcat( text, p );
        strcat( text, "\n" );

        player->m_flNextChatTime = gpGlobals->time + CHAT_INTERVAL;


        client = NULL;
        while( ( ( client = (CBasePlayer*)UTIL_FindEntityByClassname( client, "player" ) ) != NULL ) && ( !FNullEnt( client->edict() ) ) ) 
        {
                if( !client->pev )
                        continue;

                if( client->edict() == pEntity )
                        continue;

                if( !( client->IsNetClient() ) )        // Not a client ? (should never be true)
                        continue;
#if !NO_VOICEGAMEMGR
                // can the receiver hear the sender? or has he muted him?
                if( g_VoiceGameMgr.PlayerHasBlockedPlayer( client, player ) )
                        continue;
#endif
                if( !player->IsObserver() && teamonly && g_pGameRules->PlayerRelationship( client, CBaseEntity::Instance( pEntity ) ) != GR_TEAMMATE )
                        continue;

                // Spectators can only talk to other specs
                if( player->IsObserver() && teamonly )
                        if ( !client->IsObserver() )
                                continue;

                MESSAGE_BEGIN( MSG_ONE, gmsgSayText, NULL, client->pev );
                        WRITE_BYTE( ENTINDEX( pEntity ) );
                        WRITE_STRING( text );
                MESSAGE_END();
        }

        // print to the sending client
        MESSAGE_BEGIN( MSG_ONE, gmsgSayText, NULL, &pEntity->v );
                WRITE_BYTE( ENTINDEX( pEntity ) );
                WRITE_STRING( text );
        MESSAGE_END();

        // echo to server console
        g_engfuncs.pfnServerPrint( text );

        const char *temp;
        if( teamonly )
                temp = "say_team";
        else
                temp = "say";

        // team match?
        if( g_teamplay )
        {
                UTIL_LogPrintf( "\"%s<%i><%s><%s>\" %s \"%s\"\n", 
                        STRING( pEntity->v.netname ), 
                        GETPLAYERUSERID( pEntity ),
                        GETPLAYERAUTHID( pEntity ),
                        g_engfuncs.pfnInfoKeyValue( g_engfuncs.pfnGetInfoKeyBuffer( pEntity ), "model" ),
                        temp,
                        p );
        }
        else
        {
                UTIL_LogPrintf( "\"%s<%i><%s><%i>\" %s \"%s\"\n", 
                        STRING( pEntity->v.netname ), 
                        GETPLAYERUSERID( pEntity ),
                        GETPLAYERAUTHID( pEntity ),
                        GETPLAYERUSERID( pEntity ),
                        temp,
                        p );
        }
}

/*
===========
ClientCommand
called each time a player uses a "cmd" command
============
*/
extern cvar_t *g_enable_cheats;

// Use CMD_ARGV,  CMD_ARGV, and CMD_ARGC to get pointers the character string command.
void ClientCommand( edict_t *pEntity )
{
        const char *pcmd = CMD_ARGV( 0 );
        const char *pstr;

        // Is the client spawned yet?
        if( !pEntity->pvPrivateData )
                return;

        entvars_t *pev = &pEntity->v;

        if( FStrEq( pcmd, "say" ) )
        {
                Host_Say( pEntity, 0 );
        }
        else if( FStrEq( pcmd, "say_team" ) )
        {
                Host_Say( pEntity, 1 );
        }
        else if( FStrEq( pcmd, "fullupdate" ) )
        {
                GetClassPtr( (CBasePlayer *)pev )->ForceClientDllUpdate(); 
        }
        else if( FStrEq(pcmd, "give" ) )
        {
                if( g_enable_cheats->value != 0 )
                {
                        int iszItem = ALLOC_STRING( CMD_ARGV( 1 ) );    // Make a copy of the classname
                        GetClassPtr( (CBasePlayer *)pev )->GiveNamedItem( STRING( iszItem ) );
                }
        }
        else if( FStrEq( pcmd, "fire" ) )
        {
                if( g_enable_cheats->value != 0 )
                {
                        CBaseEntity *pPlayer = CBaseEntity::Instance( pEntity );
                        if( CMD_ARGC() > 1 )
                        {
                                FireTargets( CMD_ARGV( 1 ), pPlayer, pPlayer, USE_TOGGLE, 0 );
                        }
                        else
                        {
                                TraceResult tr;
                                UTIL_MakeVectors( pev->v_angle );
                                UTIL_TraceLine(
                                        pev->origin + pev->view_ofs,
                                        pev->origin + pev->view_ofs + gpGlobals->v_forward * 1000,
                                        dont_ignore_monsters, pEntity, &tr
                                );

                                if( tr.pHit )
                                {
                                        CBaseEntity *pHitEnt = CBaseEntity::Instance( tr.pHit );
                                        if( pHitEnt )
                                        {
                                                pHitEnt->Use( pPlayer, pPlayer, USE_TOGGLE, 0 );
                                                ClientPrint( &pEntity->v, HUD_PRINTCONSOLE, UTIL_VarArgs( "Fired %s \"%s\"\n", STRING( pHitEnt->pev->classname ), STRING( pHitEnt->pev->targetname ) ) );
                                        }
                                }
                        }
                }
        }
        else if( FStrEq( pcmd, "drop" ) )
        {
                // player is dropping an item. 
                GetClassPtr( (CBasePlayer *)pev )->DropPlayerItem( (char *)CMD_ARGV( 1 ) );
        }
        else if( FStrEq( pcmd, "fov" ) )
        {
                if( g_enable_cheats->value != 0 && CMD_ARGC() > 1 )
                {
                        GetClassPtr( (CBasePlayer *)pev )->m_iFOV = atoi( CMD_ARGV( 1 ) );
                }
                else
                {
                        CLIENT_PRINTF( pEntity, print_console, UTIL_VarArgs( "\"fov\" is \"%d\"\n", (int)GetClassPtr( (CBasePlayer *)pev )->m_iFOV ) );
                }
        }
        else if( FStrEq( pcmd, "use" ) )
        {
                GetClassPtr( (CBasePlayer *)pev )->SelectItem( (char *)CMD_ARGV( 1 ) );
        }
        else if( FStrEq( pcmd, "usepills" ) )
        {
                GetClassPtr( (CBasePlayer *)pev )->UsePainkiller();
        }
        else if( FStrEq( pcmd, "toggleslowmo" ) )
        {
                GetClassPtr( (CBasePlayer *)pev )->ToggleSlowMotion();
        }
        else if( ( ( pstr = strstr( pcmd, "weapon_" ) ) != NULL ) && ( pstr == pcmd ) )
        {
                GetClassPtr( (CBasePlayer *)pev )->SelectItem( pcmd );
        }
        else if( FStrEq( pcmd, "lastinv" ) )
        {
                GetClassPtr( (CBasePlayer *)pev )->SelectLastItem();
        }
        else if( FStrEq( pcmd, "spectate" ) ) // clients wants to become a spectator
        {
                CBasePlayer *pPlayer = GetClassPtr( (CBasePlayer *)pev );
                if( !pPlayer->IsObserver() )
                {
                        // always allow proxies to become a spectator
                        if( ( pev->flags & FL_PROXY ) || allow_spectators.value )
                        {
                                edict_t *pentSpawnSpot = g_pGameRules->GetPlayerSpawnSpot( pPlayer );
                                pPlayer->StartObserver( pev->origin, VARS( pentSpawnSpot )->angles );

                                // notify other clients of player switching to spectator mode
                                UTIL_ClientPrintAll( HUD_PRINTNOTIFY, UTIL_VarArgs( "%s switched to spectator mode\n",
                                                ( pev->netname && ( STRING( pev->netname ) )[0] != 0 ) ? STRING( pev->netname ) : "unconnected" ) );
                        }
                        else
                                ClientPrint( pev, HUD_PRINTCONSOLE, "Spectator mode is disabled.\n" );
                }
                else
                {
                        pPlayer->StopObserver();

                        // notify other clients of player left spectators
                        UTIL_ClientPrintAll( HUD_PRINTNOTIFY, UTIL_VarArgs( "%s has left spectator mode\n",
                                        ( pev->netname && ( STRING( pev->netname ) )[0] != 0 ) ? STRING( pev->netname ) : "unconnected" ) );
                }
        }
        else if( FStrEq( pcmd, "specmode" ) ) // new spectator mode
        {
                CBasePlayer *pPlayer = GetClassPtr( (CBasePlayer *)pev );

                if( pPlayer->IsObserver() )
                        pPlayer->Observer_SetMode( atoi( CMD_ARGV( 1 ) ) );
        }
        else if( FStrEq( pcmd, "closemenus" ) )
        {
                // just ignore it
        }
        else if( FStrEq( pcmd, "follownext" ) ) // follow next player
        {
                CBasePlayer *pPlayer = GetClassPtr( (CBasePlayer *)pev );

                if( pPlayer->IsObserver() )
                        pPlayer->Observer_FindNextPlayer( atoi( CMD_ARGV( 1 ) ) ? true : false );
        }
        else if( g_pGameRules->ClientCommand( GetClassPtr( (CBasePlayer *)pev ), pcmd ) )
        {
                // MenuSelect returns true only if the command is properly handled,  so don't print a warning
        }
        else if( FStrEq( pcmd, "VModEnable" ) )
        {
                // clear 'Unknown command: VModEnable' in singleplayer
                return;
        }
        else
        {
                // tell the user they entered an unknown command
                char command[128];

                strlcpy( command, pcmd, sizeof( command ));

                // First parse the name and remove any %'s
                for( char *pApersand = command; *pApersand; pApersand++ )
                {
                        // Replace it with a space
                        if( *pApersand == '%' )
                                *pApersand = ' ';
                }

                // tell the user they entered an unknown command
                ClientPrint( &pEntity->v, HUD_PRINTCONSOLE, UTIL_VarArgs( "Unknown command: %s\n", command ) );
        }
}

/*
========================
ClientUserInfoChanged

called after the player changes
userinfo - gives dll a chance to modify it before
it gets sent into the rest of the engine.
========================
*/
void ClientUserInfoChanged( edict_t *pEntity, char *infobuffer )
{
        // Is the client spawned yet?
        if( !pEntity->pvPrivateData )
                return;

        // msg everyone if someone changes their name,  and it isn't the first time (changing no name to current name)
        if( pEntity->v.netname && ( STRING( pEntity->v.netname ) )[0] != 0 && !FStrEq( STRING( pEntity->v.netname ), g_engfuncs.pfnInfoKeyValue( infobuffer, "name" ) ) )
        {
                char sName[256];
                char *pName = g_engfuncs.pfnInfoKeyValue( infobuffer, "name" );
                strlcpy( sName, pName, sizeof( sName ));

                // First parse the name and remove any %'s
                for( char *pApersand = sName; pApersand != NULL && *pApersand != 0; pApersand++ )
                {
                        // Replace it with a space
                        if( *pApersand == '%' )
                                *pApersand = ' ';
                }

                // Set the name
                g_engfuncs.pfnSetClientKeyValue( ENTINDEX( pEntity ), infobuffer, "name", sName );

                if( gpGlobals->maxClients > 1 )
                {
                        char text[256];
                        safe_snprintf( text, sizeof( text ), "* %s changed name to %s\n", STRING( pEntity->v.netname ), g_engfuncs.pfnInfoKeyValue( infobuffer, "name" ) );
                        MESSAGE_BEGIN( MSG_ALL, gmsgSayText, NULL );
                                WRITE_BYTE( ENTINDEX( pEntity ) );
                                WRITE_STRING( text );
                        MESSAGE_END();
                }

                // team match?
                if( g_teamplay )
                {
                        UTIL_LogPrintf( "\"%s<%i><%s><%s>\" changed name to \"%s\"\n", 
                                STRING( pEntity->v.netname ), 
                                GETPLAYERUSERID( pEntity ), 
                                GETPLAYERAUTHID( pEntity ),
                                g_engfuncs.pfnInfoKeyValue( infobuffer, "model" ), 
                                g_engfuncs.pfnInfoKeyValue( infobuffer, "name" ) );
                }
                else
                {
                        UTIL_LogPrintf( "\"%s<%i><%s><%i>\" changed name to \"%s\"\n", 
                                STRING( pEntity->v.netname ), 
                                GETPLAYERUSERID( pEntity ), 
                                GETPLAYERAUTHID( pEntity ),
                                GETPLAYERUSERID( pEntity ), 
                                g_engfuncs.pfnInfoKeyValue( infobuffer, "name" ) );
                }
        }

        g_pGameRules->ClientUserInfoChanged( GetClassPtr( (CBasePlayer *)&pEntity->v ), infobuffer );
}

static int g_serveractive = 0;

void ServerDeactivate( void )
{
        //ALERT( at_console, "ServerDeactivate()\n" );

        if( g_serveractive != 1 )
        {
                return;
        }

        g_serveractive = 0;

}

void ServerActivate( edict_t *pEdictList, int edictCount, int clientMax )
{
        int             i;
        CBaseEntity     *pClass;

        //ALERT( at_console, "ServerActivate()\n" );

        // Every call to ServerActivate should be matched by a call to ServerDeactivate
        g_serveractive = 1;

        // Clients have not been initialized yet
        for( i = 0; i < edictCount; i++ )
        {
                if( pEdictList[i].free )
                        continue;

                // Clients aren't necessarily initialized until ClientPutInServer()
                if( (i > 0 && i <= clientMax) || !pEdictList[i].pvPrivateData )
                        continue;

                pClass = CBaseEntity::Instance( &pEdictList[i] );
                // Activate this entity if it's got a class & isn't dormant
                if( pClass && !( pClass->pev->flags & FL_DORMANT ) )
                {
                        pClass->Activate();
                }
                else
                {
                        ALERT( at_console, "Can't instance %s\n", STRING( pEdictList[i].v.classname ) );
                }
        }

        // Link user messages here to make sure first client can get them...
        LinkUserMessages();

        // Spawn per-map music cue and kerotan on every map load.
        HP_SendMapMusic();
        HP_SpawnKerotan();
}

/*
================
PlayerPreThink

Called every frame before physics are run
================
*/
void PlayerPreThink( edict_t *pEntity )
{
        //ALERT( at_console, "PreThink( %g, frametime %g )\n", gpGlobals->time, gpGlobals->frametime );

        CBasePlayer *pPlayer = (CBasePlayer *)GET_PRIVATE( pEntity );

        if( pPlayer )
                pPlayer->PreThink();
}

/*
================
PlayerPostThink

Called every frame after physics are run
================
*/
void PlayerPostThink( edict_t *pEntity )
{
        //ALERT( at_console, "PostThink( %g, frametime %g )\n", gpGlobals->time, gpGlobals->frametime );

        CBasePlayer *pPlayer = (CBasePlayer *)GET_PRIVATE( pEntity );

        if( pPlayer )
                pPlayer->PostThink();
}

void ParmsNewLevel( void )
{
}

void ParmsChangeLevel( void )
{
        // retrieve the pointer to the save data
        SAVERESTOREDATA *pSaveData = (SAVERESTOREDATA *)gpGlobals->pSaveData;

        if( pSaveData )
                pSaveData->connectionCount = BuildChangeList( pSaveData->levelList, MAX_LEVEL_CONNECTIONS );
}

void StartFrame( void )
{
        //ALERT( at_console, "SV_Physics( %g, frametime %g )\n", gpGlobals->time, gpGlobals->frametime );

        if( g_pGameRules )
                g_pGameRules->Think();

        if( g_fGameOver )
                return;

        gpGlobals->teamplay = teamplay.value;
        g_ulFrameCount++;
}

void ClientPrecache( void )
{
        // setup precaches always needed
        PRECACHE_SOUND( "player/sprayer.wav" );                 // spray paint sound for PreAlpha

        // PRECACHE_SOUND( "player/pl_jumpland2.wav" );         // UNDONE: play 2x step sound

        PRECACHE_SOUND( "player/pl_fallpain2.wav" );
        PRECACHE_SOUND( "player/pl_fallpain3.wav" );

        PRECACHE_SOUND( "player/pl_step1.wav" );                // walk on concrete
        PRECACHE_SOUND( "player/pl_step2.wav" );
        PRECACHE_SOUND( "player/pl_step3.wav" );
        PRECACHE_SOUND( "player/pl_step4.wav" );

        PRECACHE_SOUND( "common/npc_step1.wav" );               // NPC walk on concrete
        PRECACHE_SOUND( "common/npc_step2.wav" );
        PRECACHE_SOUND( "common/npc_step3.wav" );
        PRECACHE_SOUND( "common/npc_step4.wav" );

        PRECACHE_SOUND( "player/pl_metal1.wav" );               // walk on metal
        PRECACHE_SOUND( "player/pl_metal2.wav" );
        PRECACHE_SOUND( "player/pl_metal3.wav" );
        PRECACHE_SOUND( "player/pl_metal4.wav" );

        PRECACHE_SOUND( "player/pl_dirt1.wav" );                // walk on dirt
        PRECACHE_SOUND( "player/pl_dirt2.wav" );
        PRECACHE_SOUND( "player/pl_dirt3.wav" );
        PRECACHE_SOUND( "player/pl_dirt4.wav" );

        PRECACHE_SOUND( "player/pl_duct1.wav" );                // walk in duct
        PRECACHE_SOUND( "player/pl_duct2.wav" );
        PRECACHE_SOUND( "player/pl_duct3.wav" );
        PRECACHE_SOUND( "player/pl_duct4.wav" );

        PRECACHE_SOUND( "player/pl_grate1.wav" );               // walk on grate
        PRECACHE_SOUND( "player/pl_grate2.wav" );
        PRECACHE_SOUND( "player/pl_grate3.wav" );
        PRECACHE_SOUND( "player/pl_grate4.wav" );

        PRECACHE_SOUND( "player/pl_slosh1.wav" );               // walk in shallow water
        PRECACHE_SOUND( "player/pl_slosh2.wav" );
        PRECACHE_SOUND( "player/pl_slosh3.wav" );
        PRECACHE_SOUND( "player/pl_slosh4.wav" );

        PRECACHE_SOUND( "player/pl_tile1.wav" );                // walk on tile
        PRECACHE_SOUND( "player/pl_tile2.wav" );
        PRECACHE_SOUND( "player/pl_tile3.wav" );
        PRECACHE_SOUND( "player/pl_tile4.wav" );
        PRECACHE_SOUND( "player/pl_tile5.wav" );

        PRECACHE_SOUND( "player/pl_swim1.wav" );                // breathe bubbles
        PRECACHE_SOUND( "player/pl_swim2.wav" );
        PRECACHE_SOUND( "player/pl_swim3.wav" );
        PRECACHE_SOUND( "player/pl_swim4.wav" );

        PRECACHE_SOUND( "player/pl_ladder1.wav" );      // climb ladder rung
        PRECACHE_SOUND( "player/pl_ladder2.wav" );
        PRECACHE_SOUND( "player/pl_ladder3.wav" );
        PRECACHE_SOUND( "player/pl_ladder4.wav" );

        PRECACHE_SOUND( "player/pl_wade1.wav" );                // wade in water
        PRECACHE_SOUND( "player/pl_wade2.wav" );
        PRECACHE_SOUND( "player/pl_wade3.wav" );
        PRECACHE_SOUND( "player/pl_wade4.wav" );

        PRECACHE_SOUND( "debris/wood1.wav" );                   // hit wood texture
        PRECACHE_SOUND( "debris/wood2.wav" );
        PRECACHE_SOUND( "debris/wood3.wav" );

        PRECACHE_SOUND( "plats/train_use1.wav" );               // use a train

        PRECACHE_SOUND( "buttons/spark5.wav" );         // hit computer texture
        PRECACHE_SOUND( "buttons/spark6.wav" );
        PRECACHE_SOUND( "debris/glass1.wav" );
        PRECACHE_SOUND( "debris/glass2.wav" );
        PRECACHE_SOUND( "debris/glass3.wav" );

        PRECACHE_SOUND( SOUND_FLASHLIGHT_ON );
        PRECACHE_SOUND( SOUND_FLASHLIGHT_OFF );

        // player gib sounds
        PRECACHE_SOUND( "common/bodysplat.wav" );

        // Half Payne death sound (ported from GoldSRC)
        PRECACHE_SOUND( "var/death.wav" );

        // Half Payne slow motion sounds (ported from GoldSRC)
        PRECACHE_SOUND( "slowmo/slowmo_start.wav" );
        PRECACHE_SOUND( "slowmo/slowmo_end.wav" );
        PRECACHE_SOUND( "slowmo/slowmo_heartbeat.wav" );

        // player pain sounds
        PRECACHE_SOUND( "player/pl_pain2.wav" );
        PRECACHE_SOUND( "player/pl_pain4.wav" );
        PRECACHE_SOUND( "player/pl_pain5.wav" );
        PRECACHE_SOUND( "player/pl_pain6.wav" );
        PRECACHE_SOUND( "player/pl_pain7.wav" );

        PRECACHE_SOUND( "max/pain/MINOR_PAIN_1.wav" );
        PRECACHE_SOUND( "max/pain/MINOR_PAIN_2.wav" );
        PRECACHE_SOUND( "max/pain/MINOR_PAIN_3.wav" );
        PRECACHE_SOUND( "max/pain/MINOR_PAIN_4.wav" );
        PRECACHE_SOUND( "max/pain/MINOR_PAIN_5.wav" );
        PRECACHE_SOUND( "max/pain/MINOR_PAIN_6.wav" );
        PRECACHE_SOUND( "max/pain/MINOR_PAIN_7.wav" );
        PRECACHE_SOUND( "max/pain/MINOR_PAIN_8.wav" );
        PRECACHE_SOUND( "max/pain/MINOR_PAIN_9.wav" );
        PRECACHE_SOUND( "max/pain/MINOR_PAIN_10.wav" );
        PRECACHE_SOUND( "max/pain/MINOR_PAIN_11.wav" );
        PRECACHE_SOUND( "max/pain/MINOR_PAIN_12.wav" );
        PRECACHE_SOUND( "max/pain/MINOR_PAIN_13.wav" );
        PRECACHE_SOUND( "max/pain/MINOR_PAIN_14.wav" );
        PRECACHE_SOUND( "max/pain/MINOR_PAIN_15.wav" );
        PRECACHE_SOUND( "max/pain/MINOR_PAIN_16.wav" );

        PRECACHE_SOUND( "max/pain/SERIOUS_PAIN_1.wav" );
        PRECACHE_SOUND( "max/pain/SERIOUS_PAIN_2.wav" );
        PRECACHE_SOUND( "max/pain/SERIOUS_PAIN_3.wav" );
        PRECACHE_SOUND( "max/pain/SERIOUS_PAIN_4.wav" );
        PRECACHE_SOUND( "max/pain/SERIOUS_PAIN_5.wav" );
        PRECACHE_SOUND( "max/pain/SERIOUS_PAIN_6.wav" );
        PRECACHE_SOUND( "max/pain/SERIOUS_PAIN_7.wav" );
        PRECACHE_SOUND( "max/pain/SERIOUS_PAIN_8.wav" );
        PRECACHE_SOUND( "max/pain/SERIOUS_PAIN_9.wav" );
        PRECACHE_SOUND( "max/pain/SERIOUS_PAIN_10.wav" );
        PRECACHE_SOUND( "max/pain/SERIOUS_PAIN_11.wav" );
        PRECACHE_SOUND( "max/pain/SERIOUS_PAIN_12.wav" );
        PRECACHE_SOUND( "max/pain/SERIOUS_PAIN_13.wav" );

        PRECACHE_SOUND( "max/pain/SELF_PAIN_1.wav" );
        PRECACHE_SOUND( "max/pain/SELF_PAIN_2.wav" );
        PRECACHE_SOUND( "max/pain/SELF_PAIN_3.wav" );
        PRECACHE_SOUND( "max/pain/SELF_PAIN_4.wav" );
        PRECACHE_SOUND( "max/pain/SELF_PAIN_5.wav" );
        PRECACHE_SOUND( "max/pain/SELF_PAIN_6.wav" );
        PRECACHE_SOUND( "max/pain/SELF_PAIN_7.wav" );
        PRECACHE_SOUND( "max/pain/SELF_PAIN_8.wav" );
        PRECACHE_SOUND( "max/pain/SELF_PAIN_9.wav" );
        PRECACHE_SOUND( "max/pain/SELF_PAIN_10.wav" );
        PRECACHE_SOUND( "max/pain/SELF_PAIN_11.wav" );
        PRECACHE_SOUND( "max/pain/SELF_PAIN_12.wav" );
        PRECACHE_SOUND( "max/pain/SELF_PAIN_13.wav" );
        PRECACHE_SOUND( "max/pain/SELF_PAIN_14.wav" );
        PRECACHE_SOUND( "max/pain/SELF_PAIN_15.wav" );
        PRECACHE_SOUND( "max/pain/SELF_PAIN_16.wav" );
        PRECACHE_SOUND( "max/pain/SELF_PAIN_17.wav" );
        PRECACHE_SOUND( "max/pain/SELF_PAIN_18.wav" );
        PRECACHE_SOUND( "max/pain/SELF_PAIN_19.wav" );
        PRECACHE_SOUND( "max/pain/SELF_PAIN_20.wav" );

        PRECACHE_SOUND( "max/painkiller/FIND_PILLS_1.wav" );
        PRECACHE_SOUND( "max/painkiller/FIND_PILLS_2.wav" );
        PRECACHE_SOUND( "max/painkiller/FIND_PILLS_3.wav" );
        PRECACHE_SOUND( "max/painkiller/FIND_PILLS_4.wav" );
        PRECACHE_SOUND( "max/painkiller/FIND_PILLS_5.wav" );
        PRECACHE_SOUND( "max/painkiller/FIND_PILLS_6.wav" );
        PRECACHE_SOUND( "max/painkiller/FIND_PILLS_7.wav" );
        PRECACHE_SOUND( "max/painkiller/FIND_PILLS_8.wav" );
        PRECACHE_SOUND( "max/painkiller/FIND_PILLS_9.wav" );
        PRECACHE_SOUND( "max/painkiller/FIND_PILLS_10.wav" );
        PRECACHE_SOUND( "max/painkiller/FIND_PILLS_11.wav" );
        PRECACHE_SOUND( "max/painkiller/FIND_PILLS_12.wav" );
        PRECACHE_SOUND( "max/painkiller/FIND_PILLS_13.wav" );
        PRECACHE_SOUND( "max/painkiller/FIND_PILLS_14.wav" );
        PRECACHE_SOUND( "max/painkiller/FIND_PILLS_15.wav" );
        PRECACHE_SOUND( "max/painkiller/FIND_PILLS_16.wav" );
        PRECACHE_SOUND( "max/painkiller/FIND_PILLS_17.wav" );
        PRECACHE_SOUND( "max/painkiller/FIND_PILLS_18.wav" );

        PRECACHE_SOUND( "max/painkiller/TAKE_PILLS_1.wav" );
        PRECACHE_SOUND( "max/painkiller/TAKE_PILLS_2.wav" );
        PRECACHE_SOUND( "max/painkiller/TAKE_PILLS_3.wav" );
        PRECACHE_SOUND( "max/painkiller/TAKE_PILLS_4.wav" );
        PRECACHE_SOUND( "max/painkiller/TAKE_PILLS_5.wav" );
        PRECACHE_SOUND( "max/painkiller/TAKE_PILLS_6.wav" );
        PRECACHE_SOUND( "max/painkiller/TAKE_PILLS_7.wav" );
        PRECACHE_SOUND( "max/painkiller/TAKE_PILLS_8.wav" );

        PRECACHE_SOUND( "max/painkiller/HAS_PILLS_1.wav" );
        PRECACHE_SOUND( "max/painkiller/HAS_PILLS_2.wav" );
        PRECACHE_SOUND( "max/painkiller/HAS_PILLS_3.wav" );
        PRECACHE_SOUND( "max/painkiller/HAS_PILLS_4.wav" );
        PRECACHE_SOUND( "max/painkiller/HAS_PILLS_5.wav" );
        PRECACHE_SOUND( "max/painkiller/HAS_PILLS_6.wav" );

        PRECACHE_SOUND( "max/painkiller/NO_PILLS_1.wav" );
        PRECACHE_SOUND( "max/painkiller/NO_PILLS_2.wav" );
        PRECACHE_SOUND( "max/painkiller/NO_PILLS_3.wav" );
        PRECACHE_SOUND( "max/painkiller/NO_PILLS_4.wav" );
        PRECACHE_SOUND( "max/painkiller/NO_PILLS_5.wav" );
        PRECACHE_SOUND( "max/painkiller/NO_PILLS_6.wav" );
        PRECACHE_SOUND( "max/painkiller/NO_PILLS_7.wav" );
        PRECACHE_SOUND( "max/painkiller/NO_PILLS_8.wav" );
        PRECACHE_SOUND( "max/painkiller/NO_PILLS_9.wav" );

        PRECACHE_SOUND( "max/innocent_killed/INNOCENT_KILLED_1.wav" );
        PRECACHE_SOUND( "max/innocent_killed/INNOCENT_KILLED_2.wav" );
        PRECACHE_SOUND( "max/innocent_killed/INNOCENT_KILLED_3.wav" );
        PRECACHE_SOUND( "max/innocent_killed/INNOCENT_KILLED_4.wav" );
        PRECACHE_SOUND( "max/innocent_killed/INNOCENT_KILLED_5.wav" );

        PRECACHE_SOUND( "max/no_ammo/NO_AMMO_1.wav" );
        PRECACHE_SOUND( "max/no_ammo/NO_AMMO_2.wav" );
        PRECACHE_SOUND( "max/no_ammo/NO_AMMO_3.wav" );
        PRECACHE_SOUND( "max/no_ammo/NO_AMMO_4.wav" );
        PRECACHE_SOUND( "max/no_ammo/NO_AMMO_5.wav" );
        PRECACHE_SOUND( "max/no_ammo/NO_AMMO_6.wav" );
        PRECACHE_SOUND( "max/no_ammo/NO_AMMO_7.wav" );
        PRECACHE_SOUND( "max/no_ammo/NO_AMMO_8.wav" );
        PRECACHE_SOUND( "max/no_ammo/NO_AMMO_9.wav" );
        PRECACHE_SOUND( "max/no_ammo/NO_AMMO_10.wav" );
        PRECACHE_SOUND( "max/no_ammo/NO_AMMO_11.wav" );
        PRECACHE_SOUND( "max/no_ammo/NO_AMMO_12.wav" );
        PRECACHE_SOUND( "max/no_ammo/NO_AMMO_13.wav" );
        PRECACHE_SOUND( "max/no_ammo/NO_AMMO_14.wav" );
        PRECACHE_SOUND( "max/no_ammo/NO_AMMO_15.wav" );
        PRECACHE_SOUND( "max/no_ammo/NO_AMMO_16.wav" );
        PRECACHE_SOUND( "max/no_ammo/NO_AMMO_17.wav" );
        PRECACHE_SOUND( "max/no_ammo/NO_AMMO_18.wav" );
        PRECACHE_SOUND( "max/no_ammo/NO_AMMO_19.wav" );
        PRECACHE_SOUND( "max/no_ammo/NO_AMMO_20.wav" );

        PRECACHE_SOUND( "max/dumb_shoot/SHOOT_THING_1.wav" );
        PRECACHE_SOUND( "max/dumb_shoot/SHOOT_THING_2.wav" );
        PRECACHE_SOUND( "max/dumb_shoot/SHOOT_THING_3.wav" );
        PRECACHE_SOUND( "max/dumb_shoot/SHOOT_THING_4.wav" );
        PRECACHE_SOUND( "max/dumb_shoot/SHOOT_THING_5.wav" );
        PRECACHE_SOUND( "max/dumb_shoot/SHOOT_THING_6.wav" );
        PRECACHE_SOUND( "max/dumb_shoot/SHOOT_THING_7.wav" );
        PRECACHE_SOUND( "max/dumb_shoot/SHOOT_THING_8.wav" );
        PRECACHE_SOUND( "max/dumb_shoot/SHOOT_THING_9.wav" );
        PRECACHE_SOUND( "max/dumb_shoot/SHOOT_THING_10.wav" );
        PRECACHE_SOUND( "max/dumb_shoot/SHOOT_THING_11.wav" );
        PRECACHE_SOUND( "max/dumb_shoot/SHOOT_THING_12.wav" );
        PRECACHE_SOUND( "max/dumb_shoot/SHOOT_THING_13.wav" );
        PRECACHE_SOUND( "max/dumb_shoot/SHOOT_THING_14.wav" );
        PRECACHE_SOUND( "max/dumb_shoot/SHOOT_THING_15.wav" );
        PRECACHE_SOUND( "max/dumb_shoot/SHOOT_THING_16.wav" );
        PRECACHE_SOUND( "max/dumb_shoot/SHOOT_THING_17.wav" );
        PRECACHE_SOUND( "max/dumb_shoot/SHOOT_THING_18.wav" );
        PRECACHE_SOUND( "max/dumb_shoot/SHOOT_THING_19.wav" );
        PRECACHE_SOUND( "max/dumb_shoot/SHOOT_THING_20.wav" );
        PRECACHE_SOUND( "max/dumb_shoot/SHOOT_THING_21.wav" );
        PRECACHE_SOUND( "max/dumb_shoot/SHOOT_THING_22.wav" );

        // Max Payne commentary / finale voice lines (c5a1 desperation system)
        PRECACHE_SOUND( "comment/onedowntwotogo.wav" );
        PRECACHE_SOUND( "comment/twodownonetogo.wav" );
        PRECACHE_SOUND( "comment/finalewon.wav" );
        PRECACHE_SOUND( "comment/execute.wav" );
        PRECACHE_SOUND( "comment/finalespeech.wav" );

        // Map commentary sounds (from map_cfg/*.txt [max_commentary] sections)
        PRECACHE_SOUND( "comment/abouttentacle.wav" );
        PRECACHE_SOUND( "comment/aboutxen0.wav" );
        PRECACHE_SOUND( "comment/aboutxen1.wav" );
        PRECACHE_SOUND( "comment/aboutxen2.wav" );
        PRECACHE_SOUND( "comment/aboutxen3.wav" );
        PRECACHE_SOUND( "comment/allin.wav" );
        PRECACHE_SOUND( "comment/baddream.wav" );
        PRECACHE_SOUND( "comment/badfeeling.wav" );
        PRECACHE_SOUND( "comment/buildingexplosives.wav" );
        PRECACHE_SOUND( "comment/creep.wav" );
        PRECACHE_SOUND( "comment/crushed.wav" );
        PRECACHE_SOUND( "comment/dontblowup.wav" );
        PRECACHE_SOUND( "comment/finally.wav" );
        PRECACHE_SOUND( "comment/focus.wav" );
        PRECACHE_SOUND( "comment/followwallaround.wav" );
        PRECACHE_SOUND( "comment/getout.wav" );
        PRECACHE_SOUND( "comment/getout2.wav" );
        PRECACHE_SOUND( "comment/great.wav" );
        PRECACHE_SOUND( "comment/gross.wav" );
        PRECACHE_SOUND( "comment/harmony.wav" );
        PRECACHE_SOUND( "comment/hellbreakingloose.wav" );
        PRECACHE_SOUND( "comment/hewasdead.wav" );
        PRECACHE_SOUND( "comment/hivehand.wav" );
        PRECACHE_SOUND( "comment/interesting.wav" );
        PRECACHE_SOUND( "comment/killedgarg.wav" );
        PRECACHE_SOUND( "comment/killingme.wav" );
        PRECACHE_SOUND( "comment/lowrenthell.wav" );
        PRECACHE_SOUND( "comment/moveon.wav" );
        PRECACHE_SOUND( "comment/nightmare.wav" );
        PRECACHE_SOUND( "comment/nightmare2.wav" );
        PRECACHE_SOUND( "comment/nosense.wav" );
        PRECACHE_SOUND( "comment/notprettysight.wav" );
        PRECACHE_SOUND( "comment/notprettysight2.wav" );
        PRECACHE_SOUND( "comment/notrainpower.wav" );
        PRECACHE_SOUND( "comment/performancereview.wav" );
        PRECACHE_SOUND( "comment/pickupcrossbow.wav" );
        PRECACHE_SOUND( "comment/pickupcrowbar.wav" );
        PRECACHE_SOUND( "comment/pickupdeagle.wav" );
        PRECACHE_SOUND( "comment/pickupegon.wav" );
        PRECACHE_SOUND( "comment/pickupgauss.wav" );
        PRECACHE_SOUND( "comment/pickuprpg.wav" );
        PRECACHE_SOUND( "comment/pickupsatchel.wav" );
        PRECACHE_SOUND( "comment/pickupsmg.wav" );
        PRECACHE_SOUND( "comment/pickupsnark.wav" );
        PRECACHE_SOUND( "comment/pickuptripmine.wav" );
        PRECACHE_SOUND( "comment/powerbackon.wav" );
        PRECACHE_SOUND( "comment/regret.wav" );
        PRECACHE_SOUND( "comment/showover.wav" );
        PRECACHE_SOUND( "comment/showsucks.wav" );
        PRECACHE_SOUND( "comment/sickexperiment.wav" );
        PRECACHE_SOUND( "comment/sowrong.wav" );
        PRECACHE_SOUND( "comment/suresoundsgood.wav" );
        PRECACHE_SOUND( "comment/thatwastheway.wav" );
        PRECACHE_SOUND( "comment/theybroughtguns.wav" );
        PRECACHE_SOUND( "comment/theycareless.wav" );
        PRECACHE_SOUND( "comment/uglywall.wav" );
        PRECACHE_SOUND( "comment/usestairs.wav" );
        PRECACHE_SOUND( "comment/vorts.wav" );
        PRECACHE_SOUND( "comment/weird1.wav" );
        PRECACHE_SOUND( "comment/wentrealwrong.wav" );
        PRECACHE_SOUND( "comment/whatdoing.wav" );
        PRECACHE_SOUND( "comment/wicked.wav" );
        PRECACHE_SOUND( "comment/winodownstairs.wav" );
        PRECACHE_SOUND( "comment/y2k.wav" );

        PRECACHE_MODEL( "models/player.mdl" );

        // hud sounds
        PRECACHE_SOUND( "common/wpn_hudoff.wav" );
        PRECACHE_SOUND( "common/wpn_hudon.wav" );
        PRECACHE_SOUND( "common/wpn_moveselect.wav" );
        PRECACHE_SOUND( "common/wpn_select.wav" );
        PRECACHE_SOUND( "common/wpn_denyselect.wav" );

        // geiger sounds
        PRECACHE_SOUND( "player/geiger6.wav" );
        PRECACHE_SOUND( "player/geiger5.wav" );
        PRECACHE_SOUND( "player/geiger4.wav" );
        PRECACHE_SOUND( "player/geiger3.wav" );
        PRECACHE_SOUND( "player/geiger2.wav" );
        PRECACHE_SOUND( "player/geiger1.wav" );

        if( giPrecacheGrunt )
                UTIL_PrecacheOther( "monster_human_grunt" );
}

/*
===============
GetGameDescription

Returns the descriptive name of this .dll.  E.g., Half-Life, or Team Fortress 2
===============
*/
const char *GetGameDescription()
{
        if( g_pGameRules ) // this function may be called before the world has spawned, and the game rules initialized
                return g_pGameRules->GetGameDescription();
        else
                return "Half-Life";
}

/*
================
Sys_Error

Engine is going to shut down, allows setting a breakpoint in game .dll to catch that occasion
================
*/
void Sys_Error( const char *error_string )
{
        // Default case, do nothing.  MOD AUTHORS:  Add code ( e.g., _asm { int 3 }; here to cause a breakpoint for debugging your game .dlls
}

/*
================
PlayerCustomization

A new player customization has been registered on the server
UNDONE:  This only sets the # of frames of the spray can logo
animation right now.
================
*/
void PlayerCustomization( edict_t *pEntity, customization_t *pCust )
{
        CBasePlayer *pPlayer = (CBasePlayer *)GET_PRIVATE( pEntity );

        if( !pPlayer )
        {
                ALERT( at_console, "PlayerCustomization:  Couldn't get player!\n" );
                return;
        }

        if( !pCust )
        {
                ALERT( at_console, "PlayerCustomization:  NULL customization!\n" );
                return;
        }

        switch( pCust->resource.type )
        {
        case t_decal:
                pPlayer->SetCustomDecalFrames( pCust->nUserData2 ); // Second int is max # of frames.
                break;
        case t_sound:
        case t_skin:
        case t_model:
                // Ignore for now.
                break;
        default:
                ALERT( at_console, "PlayerCustomization:  Unknown customization type!\n" );
                break;
        }
}

/*
================
SpectatorConnect

A spectator has joined the game
================
*/
void SpectatorConnect( edict_t *pEntity )
{
        CBaseSpectator *pPlayer = (CBaseSpectator *)GET_PRIVATE( pEntity );

        if( pPlayer )
                pPlayer->SpectatorConnect();
}

/*
================
SpectatorConnect

A spectator has left the game
================
*/
void SpectatorDisconnect( edict_t *pEntity )
{
        CBaseSpectator *pPlayer = (CBaseSpectator *)GET_PRIVATE( pEntity );

        if( pPlayer )
                pPlayer->SpectatorDisconnect();
}

/*
================
SpectatorConnect

A spectator has sent a usercmd
================
*/
void SpectatorThink( edict_t *pEntity )
{
        CBaseSpectator *pPlayer = (CBaseSpectator *)GET_PRIVATE( pEntity );

        if( pPlayer )
                pPlayer->SpectatorThink();
}


/*
================
SetupVisibility

A client can have a separate "view entity" indicating that his/her view should depend on the origin of that
view entity.  If that's the case, then pViewEntity will be non-NULL and will be used.  Otherwise, the current
entity's origin is used.  Either is offset by the view_ofs to get the eye position.

From the eye position, we set up the PAS and PVS to use for filtering network messages to the client.  At this point, we could
 override the actual PAS or PVS values, or use a different origin.

NOTE:  Do not cache the values of pas and pvs, as they depend on reusable memory in the engine, they are only good for this one frame
================
*/
void SetupVisibility( edict_t *pViewEntity, edict_t *pClient, unsigned char **pvs, unsigned char **pas )
{
        Vector org;
        edict_t *pView = pClient;

        // Find the client's PVS
        if( pViewEntity )
        {
                pView = pViewEntity;
        }

        if( pClient->v.flags & FL_PROXY )
        {
                *pvs = NULL;    // the spectator proxy sees
                *pas = NULL;    // and hears everything
                return;
        }

        if( pView->v.effects & EF_MERGE_VISIBILITY )
        {
                if( FClassnameIs( pView, "env_sky" ) )
                {
                        org = pView->v.origin;
                }
                else return; // don't merge pvs
        }
        else
        {
                org = pView->v.origin + pView->v.view_ofs;
                if( pView->v.flags & FL_DUCKING )
                {
                        org = org + ( VEC_HULL_MIN - VEC_DUCK_HULL_MIN );
                }
        }

        *pvs = ENGINE_SET_PVS( org );
        *pas = ENGINE_SET_PAS( org );
}

#include "entity_state.h"

/*
AddToFullPack

Return 1 if the entity state has been filled in for the ent and the entity will be propagated to the client, 0 otherwise

state is the server maintained copy of the state info that is transmitted to the client
a MOD could alter values copied into state to send the "host" a different look for a particular entity update, etc.
e and ent are the entity that is being added to the update, if 1 is returned
host is the player's edict of the player whom we are sending the update to
player is 1 if the ent/e is a player and 0 otherwise
pSet is either the PAS or PVS that we previous set up.  We can use it to ask the engine to filter the entity against the PAS or PVS.
we could also use the pas/ pvs that we set in SetupVisibility, if we wanted to.  Caching the value is valid in that case, but still only for the current frame
*/
int AddToFullPack( struct entity_state_s *state, int e, edict_t *ent, edict_t *host, int hostflags, int player, unsigned char *pSet )
{
        int i;
        CBaseEntity *Entity;

        // don't send if flagged for NODRAW and it's not the host getting the message
        if( ( ent->v.effects & EF_NODRAW ) && ( ent != host ) )
                return 0;

        // Ignore ents without valid / visible models
        if( !ent->v.modelindex || !STRING( ent->v.model ) )
                return 0;

        // Don't send spectators to other players
        if( ( ent->v.flags & FL_SPECTATOR ) && ( ent != host ) )
        {
                return 0;
        }

        if( ent != host )
        {
                if( !ENGINE_CHECK_VISIBILITY( (const struct edict_s *)ent, pSet ) )
                {
                        // env_sky is visible always
                        if( !FClassnameIs( ent, "env_sky" ) )
                        {
                                return 0;
                        }
                }
        }

        // Don't send entity to local client if the client says it's predicting the entity itself.
        if( ent->v.flags & FL_SKIPLOCALHOST )
        {
                if( hostflags & 4 )
                        return 0; // it's a portal pass

                if( ( hostflags & 1 ) && ( ent->v.owner == host ) )
                        return 0;
        }
        
        if( host->v.groupinfo )
        {
                UTIL_SetGroupTrace( host->v.groupinfo, GROUP_OP_AND );

                // Should always be set, of course
                if( ent->v.groupinfo )
                {
                        if( g_groupop == GROUP_OP_AND )
                        {
                                if( !( ent->v.groupinfo & host->v.groupinfo ) )
                                        return 0;
                        }
                        else if( g_groupop == GROUP_OP_NAND )
                        {
                                if( ent->v.groupinfo & host->v.groupinfo )
                                        return 0;
                        }
                }

                UTIL_UnsetGroupTrace();
        }

        memset( state, 0, sizeof(*state) );

        state->number = e;
        state->entityType = ENTITY_NORMAL;

        // Flag custom entities.
        if( ent->v.flags & FL_CUSTOMENTITY )
        {
                state->entityType = ENTITY_BEAM;
        }


        // Round animtime to nearest millisecond
        state->animtime = (int)( 1000.0f * ent->v.animtime ) / 1000.0f;

        memcpy( state->origin, ent->v.origin, 3 * sizeof(float) );
        memcpy( state->angles, ent->v.angles, 3 * sizeof(float) );
        memcpy( state->mins, ent->v.mins, 3 * sizeof(float) );
        memcpy( state->maxs, ent->v.maxs, 3 * sizeof(float) );

        memcpy( state->startpos, ent->v.startpos, 3 * sizeof(float) );
        memcpy( state->endpos, ent->v.endpos, 3 * sizeof(float) );
        memcpy( state->velocity, ent->v.velocity, 3 * sizeof(float) );

        state->impacttime = ent->v.impacttime;
        state->starttime = ent->v.starttime;

        state->modelindex = ent->v.modelindex;

        state->frame = ent->v.frame;

        state->skin = ent->v.skin;
        state->effects = ent->v.effects;

        if( !player &&
                 ent->v.animtime &&
                 ent->v.velocity[0] == 0 && 
                 ent->v.velocity[1] == 0 && 
                 ent->v.velocity[2] == 0 )
        {
                state->eflags |= EFLAG_SLERP;
        }

        state->scale            = ent->v.scale;
        state->solid            = ent->v.solid;
        state->colormap         = ent->v.colormap;

        state->movetype         = ent->v.movetype;
        state->sequence         = ent->v.sequence;
        state->framerate        = ent->v.framerate;
        state->body             = ent->v.body;

        for( i = 0; i < 4; i++ )
        {
                state->controller[i] = ent->v.controller[i];
        }

        for( i = 0; i < 2; i++ )
        {
                state->blending[i] = ent->v.blending[i];
        }

        state->rendermode       = ent->v.rendermode;
        state->renderamt        = (int)ent->v.renderamt; 
        state->renderfx         = ent->v.renderfx;
        state->rendercolor.r    = (byte)ent->v.rendercolor.x;
        state->rendercolor.g    = (byte)ent->v.rendercolor.y;
        state->rendercolor.b    = (byte)ent->v.rendercolor.z;

        state->aiment = 0;
        if( ent->v.aiment )
        {
                state->aiment = ENTINDEX( ent->v.aiment );
        }

        state->owner = 0;
        if( ent->v.owner )
        {
                int owner = ENTINDEX( ent->v.owner );

                // Only care if owned by a player
                if( owner >= 1 && owner <= gpGlobals->maxClients )
                {
                        state->owner = owner;   
                }
        }

        state->onground = 0;
        if( ent->v.groundentity )
        {
                state->onground = ENTINDEX( ent->v.groundentity );
        }

        if( !player )
        {
                state->playerclass  = ent->v.playerclass;
        }

        // Special stuff for players only
        if( player )
        {
                memcpy( state->basevelocity, ent->v.basevelocity, 3 * sizeof(float) );

                state->weaponmodel      = MODEL_INDEX( STRING( ent->v.weaponmodel ) );
                state->gaitsequence     = ent->v.gaitsequence;
                state->spectator        = ent->v.flags & FL_SPECTATOR;
                state->friction         = ent->v.friction;

                state->gravity          = ent->v.gravity;
                //state->team           = ent->v.team;

                state->usehull          = ( ent->v.flags & FL_DUCKING ) ? 1 : 0;
                state->health           = (int)ent->v.health;
        }

        if( ( Entity = CBaseEntity::Instance( ent ))
            && Entity->Classify() != CLASS_NONE
            && Entity->Classify() != CLASS_MACHINE )
        {
                SetBits( state->eflags, EFLAG_FLESH_SOUND );
        }
        else
        {
                ClearBits( state->eflags, EFLAG_FLESH_SOUND );
        }

        return 1;
}

// defaults for clientinfo messages
#define DEFAULT_VIEWHEIGHT      28

/*
===================
CreateBaseline

Creates baselines used for network encoding, especially for player data since players are not spawned until connect time.
===================
*/
void CreateBaseline( int player, int eindex, struct entity_state_s *baseline, struct edict_s *entity, int playermodelindex, vec3_t player_mins, vec3_t player_maxs )
{
        baseline->origin                = entity->v.origin;
        baseline->angles                = entity->v.angles;
        baseline->frame                 = entity->v.frame;
        baseline->skin                  = (short)entity->v.skin;

        // render information
        baseline->rendermode            = (byte)entity->v.rendermode;
        baseline->renderamt             = (byte)entity->v.renderamt;
        baseline->rendercolor.r         = (byte)entity->v.rendercolor.x;
        baseline->rendercolor.g         = (byte)entity->v.rendercolor.y;
        baseline->rendercolor.b         = (byte)entity->v.rendercolor.z;
        baseline->renderfx              = (byte)entity->v.renderfx;

        if( player )
        {
                baseline->mins          = player_mins;
                baseline->maxs          = player_maxs;

                baseline->colormap      = eindex;
                baseline->modelindex    = playermodelindex;
                baseline->friction      = 1.0;
                baseline->movetype      = MOVETYPE_WALK;

                baseline->scale         = entity->v.scale;
                baseline->solid         = SOLID_SLIDEBOX;
                baseline->framerate     = 1.0;
                baseline->gravity       = 1.0;

        }
        else
        {
                baseline->mins          = entity->v.mins;
                baseline->maxs          = entity->v.maxs;

                baseline->colormap      = 0;
                baseline->modelindex    = entity->v.modelindex;//SV_ModelIndex(pr_strings + entity->v.model);
                baseline->movetype      = entity->v.movetype;

                baseline->scale         = entity->v.scale;
                baseline->solid         = entity->v.solid;
                baseline->framerate     = entity->v.framerate;
                baseline->gravity       = entity->v.gravity;
        }
}

typedef struct
{
        char name[32];
        int field;
} entity_field_alias_t;

#define FIELD_ORIGIN0                   0
#define FIELD_ORIGIN1                   1
#define FIELD_ORIGIN2                   2
#define FIELD_ANGLES0                   3
#define FIELD_ANGLES1                   4
#define FIELD_ANGLES2                   5

static entity_field_alias_t entity_field_alias[] =
{
        { "origin[0]",                  0 },
        { "origin[1]",                  0 },
        { "origin[2]",                  0 },
        { "angles[0]",                  0 },
        { "angles[1]",                  0 },
        { "angles[2]",                  0 },
};

void Entity_FieldInit( struct delta_s *pFields )
{
        entity_field_alias[FIELD_ORIGIN0].field         = DELTA_FINDFIELD( pFields, entity_field_alias[FIELD_ORIGIN0].name );
        entity_field_alias[FIELD_ORIGIN1].field         = DELTA_FINDFIELD( pFields, entity_field_alias[FIELD_ORIGIN1].name );
        entity_field_alias[FIELD_ORIGIN2].field         = DELTA_FINDFIELD( pFields, entity_field_alias[FIELD_ORIGIN2].name );
        entity_field_alias[FIELD_ANGLES0].field         = DELTA_FINDFIELD( pFields, entity_field_alias[FIELD_ANGLES0].name );
        entity_field_alias[FIELD_ANGLES1].field         = DELTA_FINDFIELD( pFields, entity_field_alias[FIELD_ANGLES1].name );
        entity_field_alias[FIELD_ANGLES2].field         = DELTA_FINDFIELD( pFields, entity_field_alias[FIELD_ANGLES2].name );
}

/*
==================
Entity_Encode

Callback for sending entity_state_t info over network. 
FIXME:  Move to script
==================
*/
void Entity_Encode( struct delta_s *pFields, const unsigned char *from, const unsigned char *to )
{
        entity_state_t *f, *t;
        int localplayer = 0;
        static int initialized = 0;

        if( !initialized )
        {
                Entity_FieldInit( pFields );
                initialized = 1;
        }

        f = (entity_state_t *)from;
        t = (entity_state_t *)to;

        // Never send origin to local player, it's sent with more resolution in clientdata_t structure
        localplayer = ( t->number - 1 ) == ENGINE_CURRENT_PLAYER();
        if( localplayer )
        {
                DELTA_UNSETBYINDEX( pFields, entity_field_alias[FIELD_ORIGIN0].field );
                DELTA_UNSETBYINDEX( pFields, entity_field_alias[FIELD_ORIGIN1].field );
                DELTA_UNSETBYINDEX( pFields, entity_field_alias[FIELD_ORIGIN2].field );
        }

        if( ( t->impacttime != 0 ) && ( t->starttime != 0 ) )
        {
                DELTA_UNSETBYINDEX( pFields, entity_field_alias[FIELD_ORIGIN0].field );
                DELTA_UNSETBYINDEX( pFields, entity_field_alias[FIELD_ORIGIN1].field );
                DELTA_UNSETBYINDEX( pFields, entity_field_alias[FIELD_ORIGIN2].field );

                DELTA_UNSETBYINDEX( pFields, entity_field_alias[FIELD_ANGLES0].field );
                DELTA_UNSETBYINDEX( pFields, entity_field_alias[FIELD_ANGLES1].field );
                DELTA_UNSETBYINDEX( pFields, entity_field_alias[FIELD_ANGLES2].field );
        }

        if( ( t->movetype == MOVETYPE_FOLLOW ) &&
                ( t->aiment != 0 ) )
        {
                DELTA_UNSETBYINDEX( pFields, entity_field_alias[FIELD_ORIGIN0].field );
                DELTA_UNSETBYINDEX( pFields, entity_field_alias[FIELD_ORIGIN1].field );
                DELTA_UNSETBYINDEX( pFields, entity_field_alias[FIELD_ORIGIN2].field );
        }
        else if( t->aiment != f->aiment )
        {
                DELTA_SETBYINDEX( pFields, entity_field_alias[FIELD_ORIGIN0].field );
                DELTA_SETBYINDEX( pFields, entity_field_alias[FIELD_ORIGIN1].field );
                DELTA_SETBYINDEX( pFields, entity_field_alias[FIELD_ORIGIN2].field );
        }
}

static entity_field_alias_t player_field_alias[] =
{
        { "origin[0]",                  0 },
        { "origin[1]",                  0 },
        { "origin[2]",                  0 },
};

void Player_FieldInit( struct delta_s *pFields )
{
        player_field_alias[FIELD_ORIGIN0].field         = DELTA_FINDFIELD( pFields, player_field_alias[FIELD_ORIGIN0].name );
        player_field_alias[FIELD_ORIGIN1].field         = DELTA_FINDFIELD( pFields, player_field_alias[FIELD_ORIGIN1].name );
        player_field_alias[FIELD_ORIGIN2].field         = DELTA_FINDFIELD( pFields, player_field_alias[FIELD_ORIGIN2].name );
}

/*
==================
Player_Encode

Callback for sending entity_state_t for players info over network. 
==================
*/
void Player_Encode( struct delta_s *pFields, const unsigned char *from, const unsigned char *to )
{
        entity_state_t *f, *t;
        int localplayer = 0;
        static int initialized = 0;

        if( !initialized )
        {
                Player_FieldInit( pFields );
                initialized = 1;
        }

        f = (entity_state_t *)from;
        t = (entity_state_t *)to;

        // Never send origin to local player, it's sent with more resolution in clientdata_t structure
        localplayer = ( t->number - 1 ) == ENGINE_CURRENT_PLAYER();
        if( localplayer )
        {
                DELTA_UNSETBYINDEX( pFields, player_field_alias[FIELD_ORIGIN0].field );
                DELTA_UNSETBYINDEX( pFields, player_field_alias[FIELD_ORIGIN1].field );
                DELTA_UNSETBYINDEX( pFields, player_field_alias[FIELD_ORIGIN2].field );
        }

        if( ( t->movetype == MOVETYPE_FOLLOW ) &&
                 ( t->aiment != 0 ) )
        {
                DELTA_UNSETBYINDEX( pFields, player_field_alias[FIELD_ORIGIN0].field );
                DELTA_UNSETBYINDEX( pFields, player_field_alias[FIELD_ORIGIN1].field );
                DELTA_UNSETBYINDEX( pFields, player_field_alias[FIELD_ORIGIN2].field );
        }
        else if( t->aiment != f->aiment )
        {
                DELTA_SETBYINDEX( pFields, player_field_alias[FIELD_ORIGIN0].field );
                DELTA_SETBYINDEX( pFields, player_field_alias[FIELD_ORIGIN1].field );
                DELTA_SETBYINDEX( pFields, player_field_alias[FIELD_ORIGIN2].field );
        }
}

#define CUSTOMFIELD_ORIGIN0                     0
#define CUSTOMFIELD_ORIGIN1                     1
#define CUSTOMFIELD_ORIGIN2                     2
#define CUSTOMFIELD_ANGLES0                     3
#define CUSTOMFIELD_ANGLES1                     4
#define CUSTOMFIELD_ANGLES2                     5
#define CUSTOMFIELD_SKIN                        6
#define CUSTOMFIELD_SEQUENCE                    7
#define CUSTOMFIELD_ANIMTIME                    8

entity_field_alias_t custom_entity_field_alias[] =
{
        { "origin[0]",                  0 },
        { "origin[1]",                  0 },
        { "origin[2]",                  0 },
        { "angles[0]",                  0 },
        { "angles[1]",                  0 },
        { "angles[2]",                  0 },
        { "skin",                       0 },
        { "sequence",                   0 },
        { "animtime",                   0 },
};

void Custom_Entity_FieldInit( struct delta_s *pFields )
{
        custom_entity_field_alias[CUSTOMFIELD_ORIGIN0].field = DELTA_FINDFIELD( pFields, custom_entity_field_alias[CUSTOMFIELD_ORIGIN0].name );
        custom_entity_field_alias[CUSTOMFIELD_ORIGIN1].field = DELTA_FINDFIELD( pFields, custom_entity_field_alias[CUSTOMFIELD_ORIGIN1].name );
        custom_entity_field_alias[CUSTOMFIELD_ORIGIN2].field = DELTA_FINDFIELD( pFields, custom_entity_field_alias[CUSTOMFIELD_ORIGIN2].name );
        custom_entity_field_alias[CUSTOMFIELD_ANGLES0].field = DELTA_FINDFIELD( pFields, custom_entity_field_alias[CUSTOMFIELD_ANGLES0].name );
        custom_entity_field_alias[CUSTOMFIELD_ANGLES1].field = DELTA_FINDFIELD( pFields, custom_entity_field_alias[CUSTOMFIELD_ANGLES1].name );
        custom_entity_field_alias[CUSTOMFIELD_ANGLES2].field = DELTA_FINDFIELD( pFields, custom_entity_field_alias[CUSTOMFIELD_ANGLES2].name );
        custom_entity_field_alias[CUSTOMFIELD_SKIN].field = DELTA_FINDFIELD( pFields, custom_entity_field_alias[CUSTOMFIELD_SKIN].name );
        custom_entity_field_alias[CUSTOMFIELD_SEQUENCE].field = DELTA_FINDFIELD( pFields, custom_entity_field_alias[CUSTOMFIELD_SEQUENCE].name );
        custom_entity_field_alias[CUSTOMFIELD_ANIMTIME].field = DELTA_FINDFIELD( pFields, custom_entity_field_alias[CUSTOMFIELD_ANIMTIME].name );
}

/*
==================
Custom_Encode

Callback for sending entity_state_t info ( for custom entities ) over network. 
FIXME:  Move to script
==================
*/
void Custom_Encode( struct delta_s *pFields, const unsigned char *from, const unsigned char *to )
{
        entity_state_t *f, *t;
        int beamType;
        static int initialized = 0;

        if( !initialized )
        {
                Custom_Entity_FieldInit( pFields );
                initialized = 1;
        }

        f = (entity_state_t *)from;
        t = (entity_state_t *)to;

        beamType = t->rendermode & 0x0f;

        if( beamType != BEAM_POINTS && beamType != BEAM_ENTPOINT )
        {
                DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[CUSTOMFIELD_ORIGIN0].field );
                DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[CUSTOMFIELD_ORIGIN1].field );
                DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[CUSTOMFIELD_ORIGIN2].field );
        }

        if( beamType != BEAM_POINTS )
        {
                DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[CUSTOMFIELD_ANGLES0].field );
                DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[CUSTOMFIELD_ANGLES1].field );
                DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[CUSTOMFIELD_ANGLES2].field );
        }

        if( beamType != BEAM_ENTS && beamType != BEAM_ENTPOINT )
        {
                DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[CUSTOMFIELD_SKIN].field );
                DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[CUSTOMFIELD_SEQUENCE].field );
        }

        if( (int)f->animtime == (int)t->animtime )
        {
                DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[CUSTOMFIELD_ANIMTIME].field );
        }
}

/*
=================
RegisterEncoders

Allows game .dll to override network encoding of certain types of entities and tweak values, etc.
=================
*/
void RegisterEncoders( void )
{
        DELTA_ADDENCODER( "Entity_Encode", Entity_Encode );
        DELTA_ADDENCODER( "Custom_Encode", Custom_Encode );
        DELTA_ADDENCODER( "Player_Encode", Player_Encode );
}

int GetWeaponData( struct edict_s *player, struct weapon_data_s *info )
{
        memset( info, 0, MAX_WEAPONS * sizeof(weapon_data_t) );
#if CLIENT_WEAPONS
        int i;
        weapon_data_t *item;
        entvars_t *pev = &player->v;
        CBasePlayer *pl = (CBasePlayer *)CBasePlayer::Instance( pev );
        CBasePlayerWeapon *gun;

        if( !pl )
                return 1;

        // go through all of the weapons and make a list of the ones to pack
        for( i = 0; i < MAX_ITEM_TYPES; i++ )
        {
                if( pl->m_rgpPlayerItems[i] )
                {
                        // there's a weapon here. Should I pack it?
                        CBasePlayerItem *pPlayerItem = pl->m_rgpPlayerItems[i];

                        while( pPlayerItem )
                        {
                                gun = (CBasePlayerWeapon *)pPlayerItem->GetWeaponPtr();
                                if( gun && gun->UseDecrement() )
                                {
                                        ItemInfo II = {0};
                                        // Get The ID.
                                        gun->GetItemInfo( &II );

                                        if( II.iId >= 0 && II.iId < MAX_WEAPONS )
                                        {
                                                item = &info[II.iId];
                                                
                                                item->m_iId                     = II.iId;
                                                item->m_iClip                   = gun->m_iClip;

                                                item->m_flTimeWeaponIdle        = Q_max( gun->m_flTimeWeaponIdle, -0.001f );
                                                item->m_flNextPrimaryAttack     = Q_max( gun->m_flNextPrimaryAttack, -0.001f );
                                                item->m_flNextSecondaryAttack   = Q_max( gun->m_flNextSecondaryAttack, -0.001f );
                                                item->m_fInReload               = gun->m_fInReload;
                                                item->m_fInSpecialReload        = gun->m_fInSpecialReload;
                                                item->fuser1                    = Q_max( gun->pev->fuser1, -0.001f );
                                                item->fuser2                    = gun->m_flStartThrow;
                                                item->fuser3                    = gun->m_flReleaseThrow;
                                                item->iuser1                    = gun->m_chargeReady;
                                                item->iuser2                    = gun->m_fInAttack;
                                                item->iuser3                    = gun->m_fireState;

                                                //item->m_flPumpTime            = max( gun->m_flPumpTime, -0.001 );
                                        }
                                }
                                pPlayerItem = pPlayerItem->m_pNext;
                        }
                }
        }
#endif
        return 1;
}

/*
=================
UpdateClientData

Data sent to current client only
engine sets cd to 0 before calling.
=================
*/
void UpdateClientData( const struct edict_s *ent, int sendweapons, struct clientdata_s *cd )
{
        if( !ent || !ent->pvPrivateData )
                return;
        entvars_t *pev = (entvars_t *)&ent->v;
        CBasePlayer *pl = (CBasePlayer *)( CBasePlayer::Instance( pev ) );
        entvars_t *pevOrg = NULL;

        // if user is spectating different player in First person, override some vars
        if( pl && pl->pev->iuser1 == OBS_IN_EYE )
        {
                if( pl->m_hObserverTarget )
                {
                        pevOrg = pev;
                        pev = pl->m_hObserverTarget->pev;
                        pl = (CBasePlayer *)(CBasePlayer::Instance( pev ) );
                }
        }

        cd->flags               = pev->flags;
        cd->health              = pev->health;

        cd->viewmodel           = MODEL_INDEX( STRING( pev->viewmodel ) );

        cd->waterlevel          = pev->waterlevel;
        cd->watertype           = pev->watertype;
        cd->weapons             = pev->weapons;

        // Vectors
        cd->origin              = pev->origin;
        cd->velocity            = pev->velocity;
        cd->view_ofs            = pev->view_ofs;
        cd->punchangle          = pev->punchangle;

        cd->bInDuck             = pev->bInDuck;
        cd->flTimeStepSound     = pev->flTimeStepSound;
        cd->flDuckTime          = pev->flDuckTime;
        cd->flSwimTime          = pev->flSwimTime;
        cd->waterjumptime       = pev->teleport_time;

        strcpy( cd->physinfo, ENGINE_GETPHYSINFO( ent ) );

        cd->maxspeed            = pev->maxspeed;
        cd->fov                 = pev->fov;
        cd->weaponanim          = pev->weaponanim;

        cd->pushmsec            = pev->pushmsec;

        // Spectator mode
        if( pevOrg != NULL )
        {
                // don't use spec vars from chased player
                cd->iuser1              = pevOrg->iuser1;
                cd->iuser2              = pevOrg->iuser2;
        }
        else
        {
                cd->iuser1              = pev->iuser1;
                cd->iuser2              = pev->iuser2;
        }
#if CLIENT_WEAPONS
        if( sendweapons )
        {
                if( pl )
                {
                        cd->m_flNextAttack = pl->m_flNextAttack;
                        cd->fuser2 = pl->m_flNextAmmoBurn;
                        cd->fuser3 = pl->m_flAmmoStartCharge;
                        cd->vuser1.x = pl->ammo_9mm;
                        cd->vuser1.y = pl->ammo_357;
                        cd->vuser1.z = pl->ammo_argrens;
                        cd->ammo_nails = pl->ammo_bolts;
                        cd->ammo_shells = pl->ammo_buckshot;
                        cd->ammo_rockets = pl->ammo_rockets;
                        cd->ammo_cells = pl->ammo_uranium;
                        cd->vuser2.x = pl->ammo_hornets;

                        if( pl->m_pActiveItem )
                        {
                                CBasePlayerWeapon *gun;
                                gun = (CBasePlayerWeapon *)pl->m_pActiveItem->GetWeaponPtr();
                                if( gun && gun->UseDecrement() )
                                {
                                        ItemInfo II = {0};
                                        gun->GetItemInfo( &II );

                                        cd->m_iId = II.iId;

                                        cd->vuser3.z = gun->m_iSecondaryAmmoType;
                                        cd->vuser4.x = gun->m_iPrimaryAmmoType;
                                        cd->vuser4.y = pl->m_rgAmmo[gun->m_iPrimaryAmmoType];
                                        cd->vuser4.z = pl->m_rgAmmo[gun->m_iSecondaryAmmoType];

                                        if( pl->m_pActiveItem->m_iId == WEAPON_RPG )
                                        {
                                                cd->vuser2.y = ( (CRpg *)pl->m_pActiveItem )->m_fSpotActive;
                                                cd->vuser2.z = ( (CRpg *)pl->m_pActiveItem )->m_cActiveRockets;
                                        }
                                }
                        }
                }
        }
#endif
}

/*
=================
CmdStart

We're about to run this usercmd for the specified player.  We can set up groupinfo and masking here, etc.
This is the time to examine the usercmd for anything extra.  This call happens even if think does not.
=================
*/
void CmdStart( const edict_t *player, const struct usercmd_s *cmd, unsigned int random_seed )
{
        entvars_t *pev = (entvars_t *)&player->v;
        CBasePlayer *pl = (CBasePlayer *)CBasePlayer::Instance( pev );

        if( !pl )
                return;

        if( pl->pev->groupinfo != 0 )
        {
                UTIL_SetGroupTrace( pl->pev->groupinfo, GROUP_OP_AND );
        }

        pl->random_seed = random_seed;
}

/*
=================
CmdEnd

Each cmdstart is exactly matched with a cmd end, clean up any group trace flags, etc. here
=================
*/
void CmdEnd( const edict_t *player )
{
        entvars_t *pev = (entvars_t *)&player->v;
        CBasePlayer *pl = (CBasePlayer *)CBasePlayer::Instance( pev );

        if( !pl )
                return;
        if( pl->pev->groupinfo != 0 )
        {
                UTIL_UnsetGroupTrace();
        }
}

/*
================================
ConnectionlessPacket

 Return 1 if the packet is valid.  Set response_buffer_size if you want to send a response packet.  Incoming, it holds the max
  size of the response_buffer, so you must zero it out if you choose not to respond.
================================
*/
int ConnectionlessPacket( const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size )
{

        *response_buffer_size = 0;

        return 0;
}

/*
================================
GetHullBounds

  Engine calls this to enumerate player collision hulls, for prediction.  Return 0 if the hullnumber doesn't exist.
================================
*/
int GetHullBounds( int hullnumber, float *mins, float *maxs )
{
        int iret = 0;

        switch( hullnumber )
        {
        case 0:                         // Normal player
                VEC_HULL_MIN.CopyToArray(mins);
                VEC_HULL_MAX.CopyToArray(maxs);
                iret = 1;
                break;
        case 1:                         // Crouched player
                VEC_DUCK_HULL_MIN.CopyToArray(mins);
                VEC_DUCK_HULL_MAX.CopyToArray(maxs);
                iret = 1;
                break;
        case 2:                         // Point based hull
                Vector( 0, 0, 0 ).CopyToArray(mins);
                Vector( 0, 0, 0 ).CopyToArray(maxs);
                iret = 1;
                break;
        }

        return iret;
}

/*
================================
CreateInstancedBaselines

Create pseudo-baselines for items that aren't placed in the map at spawn time, but which are likely
to be created during play ( e.g., grenades, ammo packs, projectiles, corpses, etc. )
================================
*/
void CreateInstancedBaselines( void )
{
        /*int iret = 0;
        entity_state_t state;

        memset( &state, 0, sizeof(state) );*/


}

/*
================================
InconsistentFile

One of the ENGINE_FORCE_UNMODIFIED files failed the consistency check for the specified player
 Return 0 to allow the client to continue, 1 to force immediate disconnection ( with an optional disconnect message of up to 256 characters )
================================
*/
int InconsistentFile( const edict_t *player, const char *filename, char *disconnect_message )
{
        // Server doesn't care?
        if( CVAR_GET_FLOAT( "mp_consistency" ) != 1.0f )
                return 0;

        // Default behavior is to kick the player
        sprintf( disconnect_message, "Server is enforcing file consistency for %s\n", filename );

        // Kick now with specified disconnect message.
        return 1;
}

/*
================================
AllowLagCompensation

 The game .dll should return 1 if lag compensation should be allowed ( could also just set
  the sv_unlag cvar.
 Most games right now should return 0, until client-side weapon prediction code is written
  and tested for them ( note you can predict weapons, but not do lag compensation, too, 
  if you want.
================================
*/
int AllowLagCompensation( void )
{
        return 1;
}
