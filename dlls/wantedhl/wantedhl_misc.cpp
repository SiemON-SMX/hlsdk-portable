/***
*
* WantedHL - Miscellaneous entities
*
***/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "player.h"
#include "items.h"
#include "gamerules.h"
#include "skill.h"
#include "animation.h"

class CWantedDeadProp : public CBaseAnimating
{
public:
        int m_iPose;

        void KeyValue( KeyValueData *pkvd )
        {
                if( FStrEq( pkvd->szKeyName, "pose" ) )
                {
                        m_iPose = atoi( pkvd->szValue );
                        pkvd->fHandled = TRUE;
                }
                else
                        CBaseAnimating::KeyValue( pkvd );
        }

        void Spawn( void )
        {
                pev->solid    = SOLID_NOT;
                pev->movetype = MOVETYPE_NONE;
                pev->frame    = 255;

                if( FStringNull(pev->model) )
                {
                        pev->effects |= EF_NODRAW;
                        return;
                }

                SET_MODEL( ENT(pev), STRING(pev->model) );

                // Ordered list of dead-pose sequence names. The pose index from
                // the map selects the first name to try; if the model doesn't
                // have that sequence we walk the whole list and use whatever we find.
                static const char *const s_poses[] =
                {
                        "lying_on_back",    // 0 — scientist/townmex skeleton
                        "lying_on_stomach", // 1
                        "dead_sitting",     // 2
                        "dead_table1",      // 3
                        "dead_table2",      // 4
                        "dead_table3",      // 5
                        "deadstomach",      // 6 — soldier/miner/cowboy skeleton
                        "dead_on_stomach",  // 7
                        "deadside",         // 8
                        "deadsitting",      // 9
                        "diesimple",        // 10 — barney/hoss/annie skeleton
                        "lying_on_side",    // 11
                };
                static const int NUM_POSES = ARRAYSIZE( s_poses );

                // Clamp pose index to valid range
                int iPose = m_iPose;
                if( iPose < 0 || iPose >= NUM_POSES ) iPose = 0;

                // Try the requested pose first, then fall back through the whole list
                int iSeq = LookupSequence( s_poses[iPose] );
                if( iSeq == -1 )
                {
                        for( int i = 0; i < NUM_POSES; i++ )
                        {
                                iSeq = LookupSequence( s_poses[i] );
                                if( iSeq != -1 ) break;
                        }
                }

                // Last resort: sequence 0 (at minimum the model won't be missing)
                pev->sequence = ( iSeq != -1 ) ? iSeq : 0;
        }

        int ObjectCaps( void ) { return FCAP_DONT_SAVE; }
};

// --- Dead props ---
// Note: monster_annie_dead -> CDeadAnnie in monster_annie.cpp
//       monster_hoss_dead  -> CDeadHoss  in monster_hoss.cpp
//       monster_crispen_dead -> CDeadCrispen in monster_crispen.cpp
#define DEAD_PROP(classname, mdl) \
class C##classname : public CWantedDeadProp { \
public: \
        void Spawn(void) { \
                if(FStringNull(pev->model)) pev->model = MAKE_STRING(mdl); \
                PRECACHE_MODEL(mdl); \
                CWantedDeadProp::Spawn(); \
        } \
}; \
LINK_ENTITY_TO_CLASS(classname, C##classname);

DEAD_PROP( monster_bigminer_dead,   "models/bigminer.mdl"   )
DEAD_PROP( monster_cowboy_dead,     "models/cowboy.mdl"     )
DEAD_PROP( monster_kaiewi_dead,     "models/kaiewi.mdl"     )
DEAD_PROP( monster_mexbandit_dead,  "models/bandit_mex.mdl"  )
DEAD_PROP( monster_nagatow_dead,    "models/nagatow.mdl"    )
DEAD_PROP( monster_smallminer_dead, "models/smallminer.mdl" )
DEAD_PROP( monster_townmex_dead,    "models/townmex.mdl"    )
DEAD_PROP( monster_twnwesta_dead,   "models/twnwesta.mdl"   )
DEAD_PROP( monster_twnwestb_dead,   "models/twnwesta.mdl"   ) // twnwestb.mdl missing; reuse twnwesta

// NOTE: All ammo_* LINK_ENTITY_TO_CLASS registrations live in their
// corresponding weapon_*.cpp files and in shotgun.cpp (ammo_buckshot).
// Do NOT register them here — that causes duplicate symbol link errors.

class CItemElixer : public CItem
{
public:
        void Spawn( void )
        {
                Precache();
                SET_MODEL( ENT(pev), "models/medicine.mdl" );
                CItem::Spawn();
        }
        void Precache( void )
        {
                PRECACHE_MODEL( "models/medicine.mdl" );
                PRECACHE_SOUND( "items/smallmedkit1.wav" );
        }
        BOOL MyTouch( CBasePlayer *pPlayer )
        {
                if( pPlayer->pev->health >= pPlayer->pev->max_health )
                        return FALSE;
                pPlayer->TakeHealth( gSkillData.elixerCapacity, DMG_GENERIC );
                EMIT_SOUND( ENT(pev), CHAN_ITEM, "items/smallmedkit1.wav", 1, ATTN_NORM );
                return TRUE;
        }
};
LINK_ENTITY_TO_CLASS( item_elixer, CItemElixer );

class CItemHerbs : public CItem
{
public:
        void Spawn( void )
        {
                Precache();
                SET_MODEL( ENT(pev), "models/herbs.mdl" );
                CItem::Spawn();
        }
        void Precache( void )
        {
                PRECACHE_MODEL( "models/herbs.mdl" );
                PRECACHE_SOUND( "items/smallmedkit1.wav" );
        }
        BOOL MyTouch( CBasePlayer *pPlayer )
        {
                if( pPlayer->pev->health >= pPlayer->pev->max_health )
                        return FALSE;
                pPlayer->TakeHealth( gSkillData.herbsCapacity, DMG_GENERIC );
                EMIT_SOUND( ENT(pev), CHAN_ITEM, "items/smallmedkit1.wav", 1, ATTN_NORM );
                return TRUE;
        }
};
LINK_ENTITY_TO_CLASS( item_herbs, CItemHerbs );

// When the player presses +use on one of these items the entity:
//   1. Plays a paper-rustle sound (if the model is a document/telegram).
//   2. Shows any text stored in pev->message (set as "message" in the map
//      editor) centred on the player's HUD via gmsgHudText — this is the
//      "read the paper" display the original WHL showed as a sprite overlay.
//   3. Fires the entity's target (pev->target) so map logic can react
//      (e.g. mark a quest objective complete).
//   4. Hides itself after use so it can't be read twice (like a pick-up).
//
// SOLID_BBOX is required so the player's forward-trace USE ray can hit the
// prop.  MOVETYPE_NONE keeps it pinned in place on the floor.
//=============================================================================
class CWantedQuestItem : public CBaseEntity
{
public:
        void Precache( void )
        {
                PRECACHE_SOUND( "items/ammopickup.wav" );
        }
        void Spawn( void )
        {
                Precache();
                // SOLID_BBOX lets the USE trace hit us.  MOVETYPE_NONE keeps
                // the prop pinned — it will never slide or fall after spawn.
                pev->solid    = SOLID_BBOX;
                pev->movetype = MOVETYPE_NONE;
                if( !FStringNull(pev->model) )
                {
                        PRECACHE_MODEL( (char *)STRING(pev->model) );
                        SET_MODEL( ENT(pev), STRING(pev->model) );
                        DROP_TO_FLOOR( ENT(pev) );
                }
                else
                {
                        pev->effects |= EF_NODRAW;
                        // Invisible items still need a touch bbox so +use
                        // can reach them from close range.
                        UTIL_SetSize( pev, Vector(-8,-8,0), Vector(8,8,16) );
                }
        }
        int ObjectCaps( void ) { return FCAP_DONT_SAVE | FCAP_IMPULSE_USE; }

        void Use( CBaseEntity *pActivator, CBaseEntity *, USE_TYPE, float )
        {
                // Only the player can read/pick-up documents.
                if( !pActivator || !pActivator->IsPlayer() )
                        return;

                // Play a subtle sound so the player gets tactile feedback.
                EMIT_SOUND( ENT(pev), CHAN_ITEM, "items/ammopickup.wav", 0.6f, ATTN_NORM );

                // Show the document text centred on screen (gmsgHudText /
                // UTIL_ShowMessage).  Map makers set this via the "message"
                // key in the entity properties.  If no message is set we
                // still let the sound + target fire work.
                if( !FStringNull(pev->message) )
                        UTIL_ShowMessage( STRING(pev->message), pActivator );

                // Fire the item's target so map logic can react
                // (quest flags, doors, triggers, etc.).
                if( !FStringNull(pev->target) )
                        SUB_UseTargets( pActivator, USE_TOGGLE, 0 );

                // Hide the item after it has been used once so the player
                // cannot pick it up again — matches original WHL behaviour.
                pev->effects |= EF_NODRAW;
                pev->solid    = SOLID_NOT;
        }
};

#define QUEST_ITEM(cls) \
class C##cls : public CWantedQuestItem {}; \
LINK_ENTITY_TO_CLASS(cls, C##cls);

QUEST_ITEM( item_bag1 )
QUEST_ITEM( item_bag2 )
QUEST_ITEM( item_dish )
QUEST_ITEM( item_pickaxe )
QUEST_ITEM( item_shovel )
QUEST_ITEM( item_steerribs )
QUEST_ITEM( item_steerskull )
QUEST_ITEM( item_telegram )
QUEST_ITEM( item_telegraphkey )

class CCTCCapturePoint : public CBaseEntity
{
public:
        void Spawn( void )
        {
                pev->solid    = SOLID_TRIGGER;
                pev->movetype = MOVETYPE_NONE;
                pev->effects |= EF_NODRAW;
                UTIL_SetSize( pev, Vector(-32,-32,0), Vector(32,32,64) );
        }
        int ObjectCaps( void ) { return FCAP_DONT_SAVE; }
};
LINK_ENTITY_TO_CLASS( ctc_capturepoint, CCTCCapturePoint );

class CCTCChicken : public CBaseMonster
{
public:
        int  ISoundMask( void );
        Schedule_t *GetSchedule( void );
        void Spawn( void );
        void Precache( void );
        int  Classify( void ) { return CLASS_NONE; }
};
LINK_ENTITY_TO_CLASS( ctc_chicken, CCTCChicken );

void CCTCChicken::Precache( void )
{
        PRECACHE_MODEL( "models/chicken.mdl" );
}
void CCTCChicken::Spawn( void )
{
        Precache();
        SET_MODEL( ENT(pev), "models/chicken.mdl" );
        UTIL_SetSize( pev, Vector(-6,-6,0), Vector(6,6,12) );
        pev->solid      = SOLID_SLIDEBOX;
        pev->movetype   = MOVETYPE_STEP;
        m_bloodColor    = BLOOD_COLOR_RED;
        pev->health     = 30;
        m_MonsterState  = MONSTERSTATE_NONE;
        MonsterInit();
}

class CWantedFlag : public CBaseEntity
{
public:
        void Spawn( void )
        {
                pev->solid    = SOLID_TRIGGER;
                pev->movetype = MOVETYPE_NONE;
                pev->effects |= EF_NODRAW;
                UTIL_SetSize( pev, Vector(-16,-16,0), Vector(16,16,64) );
        }
        int ObjectCaps( void ) { return FCAP_DONT_SAVE; }
};
LINK_ENTITY_TO_CLASS( monster_flag1, CWantedFlag );
LINK_ENTITY_TO_CLASS( monster_flag2, CWantedFlag );
LINK_ENTITY_TO_CLASS( monster_flag3, CWantedFlag );
LINK_ENTITY_TO_CLASS( monster_flag4, CWantedFlag );

class CWantedDetect : public CPointEntity
{
public:
        void Spawn( void )
        {
                pev->solid    = SOLID_NOT;
                pev->movetype = MOVETYPE_NONE;
        }
};
LINK_ENTITY_TO_CLASS( info_wanteddetect, CWantedDetect );

class CTriggerKillMonster : public CBaseDelay
{
public:
        void Spawn( void )
        {
                pev->solid    = SOLID_NOT;
                pev->movetype = MOVETYPE_NONE;
        }
        void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
        {
                if( FStringNull(pev->target) ) return;
                CBaseEntity *pEnt = NULL;
                while( (pEnt = UTIL_FindEntityByTargetname(pEnt, STRING(pev->target))) != NULL )
                {
                        if( pEnt->IsAlive() )
                                pEnt->TakeDamage( pev, pev, 9999, DMG_GENERIC );
                }
        }
};
LINK_ENTITY_TO_CLASS( trigger_killmonster, CTriggerKillMonster );

class CTriggerShutup : public CBaseDelay
{
public:
        void Spawn( void )
        {
                pev->solid    = SOLID_NOT;
                pev->movetype = MOVETYPE_NONE;
        }
        void Use( CBaseEntity*, CBaseEntity*, USE_TYPE, float ) {}
};
LINK_ENTITY_TO_CLASS( trigger_shutup, CTriggerShutup );

class CPlayerBearTrapStrip : public CBaseEntity
{
public:
        void Spawn( void )
        {
                pev->solid    = SOLID_TRIGGER;
                pev->movetype = MOVETYPE_NONE;
                UTIL_SetSize( pev, Vector(-16,-16,0), Vector(16,16,72) );
                SetTouch( &CPlayerBearTrapStrip::StripTouch );
        }
        void EXPORT StripTouch( CBaseEntity *pOther )
        {
                if( !pOther->IsPlayer() ) return;
                // Strip bear traps: zero out beartrap ammo count
                int idx = pOther->GiveAmmo( 0, "beartrap", 100 );
                if( idx != -1 )
                {
                        CBasePlayer *pPlayer = (CBasePlayer *)pOther;
                        pPlayer->m_rgAmmo[idx] = 0;
                }
        }
        int ObjectCaps( void ) { return FCAP_DONT_SAVE | (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION); }
};
LINK_ENTITY_TO_CLASS( player_beartrapstrip, CPlayerBearTrapStrip );

class CWantedNullEnt : public CBaseEntity
{
public:
        void Spawn( void ) { UTIL_Remove( this ); }
        int  ObjectCaps( void ) { return FCAP_DONT_SAVE; }
};

// Bot / navigation waypoints (HL / CS / TFC editors embed these).
// These entities appear in WantedHL maps but have no gameplay role.
// NOT already registered anywhere in the SDK — safe to define here.
LINK_ENTITY_TO_CLASS( info_botnode,            CWantedNullEnt )
LINK_ENTITY_TO_CLASS( info_botlandmark,        CWantedNullEnt )
LINK_ENTITY_TO_CLASS( info_bot_start,          CWantedNullEnt )
LINK_ENTITY_TO_CLASS( info_bot_spawn,          CWantedNullEnt )
LINK_ENTITY_TO_CLASS( info_hint,               CWantedNullEnt )
LINK_ENTITY_TO_CLASS( info_no_dynamic_shadow,  CWantedNullEnt )
LINK_ENTITY_TO_CLASS( info_compile_parameters, CWantedNullEnt )
LINK_ENTITY_TO_CLASS( info_overlay,            CWantedNullEnt )
LINK_ENTITY_TO_CLASS( info_ladder,             CWantedNullEnt )
LINK_ENTITY_TO_CLASS( info_map_parameters,     CWantedNullEnt )

// Environment / visual extras — not in SDK, safe to stub
LINK_ENTITY_TO_CLASS( env_fog,               CWantedNullEnt )
LINK_ENTITY_TO_CLASS( env_sky,               CWantedNullEnt )
LINK_ENTITY_TO_CLASS( sky_camera,            CWantedNullEnt )
LINK_ENTITY_TO_CLASS( env_sun,               CWantedNullEnt )
LINK_ENTITY_TO_CLASS( env_wind,              CWantedNullEnt )
LINK_ENTITY_TO_CLASS( env_rain,              CWantedNullEnt )
LINK_ENTITY_TO_CLASS( env_snow,              CWantedNullEnt )
LINK_ENTITY_TO_CLASS( env_skypaint,          CWantedNullEnt )
LINK_ENTITY_TO_CLASS( env_cubemap,           CWantedNullEnt )
LINK_ENTITY_TO_CLASS( env_detail_controller, CWantedNullEnt )
LINK_ENTITY_TO_CLASS( env_entity_maker,      CWantedNullEnt )
// light_spot is in lights.cpp — do NOT register here
LINK_ENTITY_TO_CLASS( light_dynamic,         CWantedNullEnt )

// Multiplayer game-rule entities — not in SDK, safe to stub
// NOTE: game_player_equip/game_zone_player/game_score/game_end/
//       game_player_hurt/game_counter/game_counter_set/game_player_team/
//       game_team_master/game_team_set/game_text are all in maprules.cpp
LINK_ENTITY_TO_CLASS( game_ui,             CWantedNullEnt )
LINK_ENTITY_TO_CLASS( game_weapon_manager, CWantedNullEnt )

// func_ extras — not in SDK, safe to stub
// NOTE: func_wall/func_wall_toggle in bmodels.cpp; func_water in doors.cpp
LINK_ENTITY_TO_CLASS( func_fog,            CWantedNullEnt )
LINK_ENTITY_TO_CLASS( func_areaportal,     CWantedNullEnt )
LINK_ENTITY_TO_CLASS( func_occluder,       CWantedNullEnt )
LINK_ENTITY_TO_CLASS( func_buyzone,        CWantedNullEnt )
LINK_ENTITY_TO_CLASS( func_bomb_target,    CWantedNullEnt )
LINK_ENTITY_TO_CLASS( func_hostage_rescue, CWantedNullEnt )
LINK_ENTITY_TO_CLASS( func_vip_safetyzone, CWantedNullEnt )
LINK_ENTITY_TO_CLASS( func_escapezone,     CWantedNullEnt )

// Trigger extras — not in SDK, safe to stub
LINK_ENTITY_TO_CLASS( trigger_look,         CWantedNullEnt )
LINK_ENTITY_TO_CLASS( trigger_wind,         CWantedNullEnt )
LINK_ENTITY_TO_CLASS( trigger_fog,          CWantedNullEnt )
LINK_ENTITY_TO_CLASS( trigger_playerfreeze, CWantedNullEnt )

// Misc editor leftovers — not in SDK, safe to stub
// NOTE: grenade→ggrenade.cpp, hornet→hornet.cpp, laser_spot→rpg.cpp,
//       monster_satchel→satchel.cpp, monster_snark→squeakgrenade.cpp,
//       monster_tripmine→tripmine.cpp, hvr_rocket→apache.cpp — all excluded.
LINK_ENTITY_TO_CLASS( hostage_entity, CWantedNullEnt )
LINK_ENTITY_TO_CLASS( armoury_entity, CWantedNullEnt )
LINK_ENTITY_TO_CLASS( c4bomb,         CWantedNullEnt )

int CCTCChicken::ISoundMask( void )
{
        return 0;
}

Schedule_t *CCTCChicken::GetSchedule( void )
{
        return GetScheduleOfType( SCHED_IDLE_STAND );
}

LINK_ENTITY_TO_CLASS( item_thighpack, CWantedNullEnt )

//=============================================================================
// monster_rogan — dedicated rider / attachment prop
//
// Used by wantintro and other maps as an invisible "rider" sitting on top of
// a horse or other vehicle entity.  The original maps used an invisible
// monster_generic (rendermode 4, black rendercolor) but that entity prints
// "stuck in wall" and "no sequence for act:1" errors because monster_generic
// demands standard HL activities that custom rider models don't have.
//
// monster_rogan:
//   - Renders the model assigned in the map editor (pev->model)
//   - Is non-solid by default so it never clips into geometry
//   - Silently skips missing activities (uses sequence 0 as fallback)
//   - Can be made solid via spawnflag 1 if needed
//=============================================================================
class CMonsterRogan : public CBaseMonster
{
public:
        void Spawn( void );
        void Precache( void );
        int  Classify( void )  { return CLASS_NONE; }
        void SetYawSpeed( void ) { pev->yaw_speed = 0; }
        void SetActivity( Activity NewActivity );
        int  ISoundMask( void ) { return 0; }
};

LINK_ENTITY_TO_CLASS( monster_rogan, CMonsterRogan );

void CMonsterRogan::Precache( void )
{
        if( !FStringNull(pev->model) )
                PRECACHE_MODEL( (char *)STRING(pev->model) );
}

void CMonsterRogan::Spawn( void )
{
        Precache();

        if( !FStringNull(pev->model) )
                SET_MODEL( ENT(pev), STRING(pev->model) );
        else
                pev->effects |= EF_NODRAW;

        // Non-solid by default — rider props must never block movement.
        // Set spawnflag 1 to make the entity solid (SOLID_SLIDEBOX).
        if( pev->spawnflags & 1 )
        {
                pev->solid    = SOLID_SLIDEBOX;
                pev->movetype = MOVETYPE_STEP;
                UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );
        }
        else
        {
                pev->solid    = SOLID_NOT;
                pev->movetype = MOVETYPE_NONE;
        }

        pev->takedamage = DAMAGE_NO;
        pev->health     = 1;
        m_MonsterState  = MONSTERSTATE_NONE;
        m_afCapability  = 0;
}

// Silently fall back to sequence 0 when the model lacks the requested activity.
void CMonsterRogan::SetActivity( Activity NewActivity )
{
        int iSequence = LookupActivity( NewActivity );
        if( iSequence == ACTIVITY_NOT_AVAILABLE )
        {
                pev->sequence  = 0;
                pev->frame     = 0;
                ResetSequenceInfo();
                m_Activity      = NewActivity;
                m_IdealActivity = NewActivity;
                return;
        }
        CBaseMonster::SetActivity( NewActivity );
}
