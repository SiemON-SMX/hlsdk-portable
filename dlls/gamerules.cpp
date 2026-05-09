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
//=========================================================
// GameRules.cpp
//=========================================================

#include        "extdll.h"
#include        "util.h"
#include        "cbase.h"
#include        "player.h"
#include        "weapons.h"
#include        "gamerules.h"
#include        "teamplay_gamerules.h"
#include        "skill.h"
#include        "game.h"

extern edict_t *EntSelectSpawnPoint( CBaseEntity *pPlayer );

DLL_GLOBAL CGameRules *g_pGameRules = NULL;
extern DLL_GLOBAL BOOL g_fGameOver;
extern int gmsgDeathMsg;        // client dll messages
extern int gmsgMOTD;

int g_teamplay = 0;

//=========================================================
//=========================================================
BOOL CGameRules::CanHaveAmmo( CBasePlayer *pPlayer, const char *pszAmmoName, int iMaxCarry )
{
        int iAmmoIndex;

        if( pszAmmoName )
        {
                iAmmoIndex = pPlayer->GetAmmoIndex( pszAmmoName );

                if( iAmmoIndex > -1 )
                {
                        if( pPlayer->AmmoInventory( iAmmoIndex ) < iMaxCarry )
                        {
                                // player has room for more of this type of ammo
                                return TRUE;
                        }
                }
        }

        return FALSE;
}

//=========================================================
//=========================================================
edict_t *CGameRules::GetPlayerSpawnSpot( CBasePlayer *pPlayer )
{
        edict_t *pentSpawnSpot = EntSelectSpawnPoint( pPlayer );

        pPlayer->pev->origin = VARS( pentSpawnSpot )->origin + Vector( 0, 0, 1 );
        pPlayer->pev->v_angle  = g_vecZero;
        pPlayer->pev->velocity = g_vecZero;
        pPlayer->pev->angles = VARS( pentSpawnSpot )->angles;
        pPlayer->pev->punchangle = g_vecZero;
        pPlayer->pev->fixangle = TRUE;

        return pentSpawnSpot;
}

//=========================================================
//=========================================================
BOOL CGameRules::CanHavePlayerItem( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon )
{
        // only living players can have items
        if( pPlayer->pev->deadflag != DEAD_NO )
                return FALSE;

        if( pWeapon->pszAmmo1() )
        {
                if( !CanHaveAmmo( pPlayer, pWeapon->pszAmmo1(), pWeapon->iMaxAmmo1() ) )
                {
                        // we can't carry anymore ammo for this gun. We can only 
                        // have the gun if we aren't already carrying one of this type
                        if( pPlayer->HasPlayerItem( pWeapon ) )
                        {
                                return FALSE;
                        }
                }
        }
        else
        {
                // weapon doesn't use ammo, don't take another if you already have it.
                if( pPlayer->HasPlayerItem( pWeapon ) )
                {
                        return FALSE;
                }
        }

        // note: will fall through to here if GetItemInfo doesn't fill the struct!
        return TRUE;
}

//=========================================================
// load the SkillData struct with the proper values based on the skill level.
//=========================================================
void CGameRules::RefreshSkillData ( void )
{
        int iSkill;

        iSkill = (int)CVAR_GET_FLOAT( "skill" );
        g_iSkillLevel = iSkill;

        if( iSkill < 1 )
        {
                iSkill = 1;
        }
        else if( iSkill > 3 )
        {
                iSkill = 3; 
        }

        gSkillData.iSkillLevel = iSkill;

        ALERT( at_console, "\nGAME SKILL LEVEL:%d\n",iSkill );

        //Agrunt                
        gSkillData.agruntHealth = GetSkillCvar( "sk_agrunt_health" );
        gSkillData.agruntDmgPunch = GetSkillCvar( "sk_agrunt_dmg_punch" );

        // Apache 
        gSkillData.apacheHealth = GetSkillCvar( "sk_apache_health" );

        // Barney
        gSkillData.barneyHealth = GetSkillCvar( "sk_barney_health" );

        // Big Momma
        gSkillData.bigmommaHealthFactor = GetSkillCvar( "sk_bigmomma_health_factor" );
        gSkillData.bigmommaDmgSlash = GetSkillCvar( "sk_bigmomma_dmg_slash" );
        gSkillData.bigmommaDmgBlast = GetSkillCvar( "sk_bigmomma_dmg_blast" );
        gSkillData.bigmommaRadiusBlast = GetSkillCvar( "sk_bigmomma_radius_blast" );

        // Bullsquid
        gSkillData.bullsquidHealth = GetSkillCvar( "sk_bullsquid_health" );
        gSkillData.bullsquidDmgBite = GetSkillCvar( "sk_bullsquid_dmg_bite" );
        gSkillData.bullsquidDmgWhip = GetSkillCvar( "sk_bullsquid_dmg_whip" );
        gSkillData.bullsquidDmgSpit = GetSkillCvar( "sk_bullsquid_dmg_spit" );

        // Gargantua
        gSkillData.gargantuaHealth = GetSkillCvar( "sk_gargantua_health" );
        gSkillData.gargantuaDmgSlash = GetSkillCvar( "sk_gargantua_dmg_slash" );
        gSkillData.gargantuaDmgFire = GetSkillCvar( "sk_gargantua_dmg_fire" );
        gSkillData.gargantuaDmgStomp = GetSkillCvar( "sk_gargantua_dmg_stomp" );

        // Hassassin
        gSkillData.hassassinHealth = GetSkillCvar( "sk_hassassin_health" );

        // Headcrab
        gSkillData.headcrabHealth = GetSkillCvar( "sk_headcrab_health" );
        gSkillData.headcrabDmgBite = GetSkillCvar( "sk_headcrab_dmg_bite" );

        // Hgrunt 
        gSkillData.hgruntHealth = GetSkillCvar( "sk_hgrunt_health" );
        gSkillData.hgruntDmgKick = GetSkillCvar( "sk_hgrunt_kick" );
        gSkillData.hgruntShotgunPellets = GetSkillCvar( "sk_hgrunt_pellets" );
        gSkillData.hgruntGrenadeSpeed = GetSkillCvar( "sk_hgrunt_gspeed" );

        // Houndeye
        gSkillData.houndeyeHealth = GetSkillCvar( "sk_houndeye_health" );
        gSkillData.houndeyeDmgBlast = GetSkillCvar( "sk_houndeye_dmg_blast" );

        // ISlave
        gSkillData.slaveHealth = GetSkillCvar( "sk_islave_health" );
        gSkillData.slaveDmgClaw = GetSkillCvar( "sk_islave_dmg_claw" );
        gSkillData.slaveDmgClawrake = GetSkillCvar( "sk_islave_dmg_clawrake" );
        gSkillData.slaveDmgZap = GetSkillCvar( "sk_islave_dmg_zap" );

        // Icthyosaur
        gSkillData.ichthyosaurHealth = GetSkillCvar( "sk_ichthyosaur_health" );
        gSkillData.ichthyosaurDmgShake = GetSkillCvar( "sk_ichthyosaur_shake" );

        // Leech
        gSkillData.leechHealth = GetSkillCvar( "sk_leech_health" );

        gSkillData.leechDmgBite = GetSkillCvar( "sk_leech_dmg_bite" );

        // Controller
        gSkillData.controllerHealth = GetSkillCvar( "sk_controller_health" );
        gSkillData.controllerDmgZap = GetSkillCvar( "sk_controller_dmgzap" );
        gSkillData.controllerSpeedBall = GetSkillCvar( "sk_controller_speedball" );
        gSkillData.controllerDmgBall = GetSkillCvar( "sk_controller_dmgball" );

        // Nihilanth
        gSkillData.nihilanthHealth = GetSkillCvar( "sk_nihilanth_health" );
        gSkillData.nihilanthZap = GetSkillCvar( "sk_nihilanth_zap" );

        // Scientist
        gSkillData.scientistHealth = GetSkillCvar( "sk_scientist_health" );

        // Snark
        gSkillData.snarkHealth = GetSkillCvar( "sk_snark_health" );
        gSkillData.snarkDmgBite = GetSkillCvar( "sk_snark_dmg_bite" );
        gSkillData.snarkDmgPop = GetSkillCvar( "sk_snark_dmg_pop" );

        gSkillData.scorpionHealth   = GetSkillCvar( "sk_scorpion_health" );
        gSkillData.scorpionDmgSting = GetSkillCvar( "sk_scorpion_dmg_sting" );

        // Zombie
        gSkillData.zombieHealth = GetSkillCvar( "sk_zombie_health" );
        gSkillData.zombieDmgOneSlash = GetSkillCvar( "sk_zombie_dmg_one_slash" );
        gSkillData.zombieDmgBothSlash = GetSkillCvar( "sk_zombie_dmg_both_slash" );

        //Turret
        gSkillData.turretHealth = GetSkillCvar( "sk_turret_health" );

        // MiniTurret
        gSkillData.miniturretHealth = GetSkillCvar( "sk_miniturret_health" );

        // Sentry Turret
        gSkillData.sentryHealth = GetSkillCvar( "sk_sentry_health" );

        // PLAYER WEAPONS

        // Crowbar whack
        gSkillData.plrDmgCrowbar = GetSkillCvar( "sk_plr_crowbar" );

        // Glock Round
        gSkillData.plrDmg9MM = GetSkillCvar( "sk_plr_9mm_bullet" );

        // 357 Round
        gSkillData.plrDmg357 = GetSkillCvar( "sk_plr_357_bullet" );

        // MP5 Round
        gSkillData.plrDmgMP5 = GetSkillCvar( "sk_plr_9mmAR_bullet" );

        // M203 grenade
        gSkillData.plrDmgM203Grenade = GetSkillCvar( "sk_plr_9mmAR_grenade" );

        // Shotgun buckshot
        gSkillData.plrDmgBuckshot = GetSkillCvar( "sk_plr_buckshot" );

        // Crossbow
        gSkillData.plrDmgCrossbowClient = GetSkillCvar( "sk_plr_xbow_bolt_client" );
        gSkillData.plrDmgCrossbowMonster = GetSkillCvar( "sk_plr_xbow_bolt_monster" );

        // RPG
        gSkillData.plrDmgRPG = GetSkillCvar( "sk_plr_rpg" );

        // Gauss gun
        gSkillData.plrDmgGauss = GetSkillCvar( "sk_plr_gauss" );

        // Egon Gun
        gSkillData.plrDmgEgonNarrow = GetSkillCvar( "sk_plr_egon_narrow" );
        gSkillData.plrDmgEgonWide = GetSkillCvar( "sk_plr_egon_wide" );

        // Hand Grendade
        gSkillData.plrDmgHandGrenade = GetSkillCvar( "sk_plr_hand_grenade" );

        // Satchel Charge
        gSkillData.plrDmgSatchel = GetSkillCvar( "sk_plr_satchel" );

        // Tripmine
        gSkillData.plrDmgTripmine = GetSkillCvar( "sk_plr_tripmine" );

        // MONSTER WEAPONS
        gSkillData.monDmg12MM = GetSkillCvar( "sk_12mm_bullet" );
        gSkillData.monDmgMP5 = GetSkillCvar ("sk_9mmAR_bullet" );
        gSkillData.monDmg9MM = GetSkillCvar( "sk_9mm_bullet" );

        // MONSTER HORNET
        gSkillData.monDmgHornet = GetSkillCvar( "sk_hornet_dmg" );

        // PLAYER HORNET
// Up to this point, player hornet damage and monster hornet damage were both using
// monDmgHornet to determine how much damage to do. In tuning the hivehand, we now need
// to separate player damage and monster hivehand damage. Since it's so late in the project, we've
// added plrDmgHornet to the SKILLDATA struct, but not to the engine CVar list, so it's inaccesible
// via SKILLS.CFG. Any player hivehand tuning must take place in the code. (sjb)
        gSkillData.plrDmgHornet = 7;

        // ========== WantedHL Monsters ==========
        // WHL_SKILL: use CVar value if > 0, else fall back to hardcoded default
        // so damage works even when skill.cfg is absent or sets values to 0.
#define WHL_SKILL( dst, cvar, fb ) \
        { float _v = GetSkillCvar( cvar ); (dst) = ( _v > 0.0f ) ? _v : (fb); }

        WHL_SKILL( gSkillData.annieHealth,             "sk_annie_health",            35.0f  );
        WHL_SKILL( gSkillData.bearHealth,              "sk_bear_health",            400.0f  );
        WHL_SKILL( gSkillData.bearDmgClaw,             "sk_bear_dmg_claw",           25.0f  );
        WHL_SKILL( gSkillData.bearDmgPounce,           "sk_bear_dmg_pounce",         30.0f  );
        WHL_SKILL( gSkillData.bigminerHealth,          "sk_bigminer_health",        140.0f  );
        WHL_SKILL( gSkillData.bigminerPunch,           "sk_bigminer_punch",          15.0f  );
        WHL_SKILL( gSkillData.chickenHealth,           "sk_chicken_health",           5.0f  );
        WHL_SKILL( gSkillData.chickenDmgPeck,          "sk_chicken_dmg_peck",         1.0f  );
        WHL_SKILL( gSkillData.colonelHealth,           "sk_colonel_health",         100.0f  );
        WHL_SKILL( gSkillData.colonelHeal,             "sk_colonel_heal",            25.0f  );
        WHL_SKILL( gSkillData.crispenHealth,           "sk_crispen_health",          75.0f  );
        WHL_SKILL( gSkillData.crispenHeal,             "sk_crispen_heal",            25.0f  );
        WHL_SKILL( gSkillData.cowboyHealth,            "sk_cowboy_health",          100.0f  );
        WHL_SKILL( gSkillData.cowboyPunch,             "sk_cowboy_punch",             5.0f  );
        WHL_SKILL( gSkillData.cowboyDynamiteSpeed,     "sk_cowboy_dynamitespeed",   400.0f  );
        WHL_SKILL( gSkillData.daveHealth,              "sk_dave_health",             75.0f  );
        WHL_SKILL( gSkillData.daveHeal,                "sk_dave_heal",               25.0f  );
        WHL_SKILL( gSkillData.horseHealth,             "sk_horse_health",           110.0f  );
        WHL_SKILL( gSkillData.horseDmgKick,            "sk_horse_dmg_kick",          10.0f  );
        WHL_SKILL( gSkillData.hossHealth,              "sk_hoss_health",             90.0f  );
        WHL_SKILL( gSkillData.kaiwiHealth,             "sk_kaiewi_health",          105.0f  );
        WHL_SKILL( gSkillData.kaiwiPunch,              "sk_kaiewi_punch",             5.0f  );
        WHL_SKILL( gSkillData.masalaHealth,            "sk_masala_health",          100.0f  );
        WHL_SKILL( gSkillData.masalaHeal,              "sk_masala_heal",             25.0f  );
        WHL_SKILL( gSkillData.mexbanditHealth,         "sk_mexbandit_health",       100.0f  );
        WHL_SKILL( gSkillData.mexbanditKick,           "sk_mexbandit_kick",          20.0f  );
        WHL_SKILL( gSkillData.mexbanditDynamiteSpeed,  "sk_mexbandit_dynamitespeed",500.0f  );
        WHL_SKILL( gSkillData.nagatowHealth,           "sk_nagatow_health",         100.0f  );
        WHL_SKILL( gSkillData.nagatowHeal,             "sk_nagatow_heal",            25.0f  );
        WHL_SKILL( gSkillData.pumaHealth,              "sk_puma_health",            130.0f  );
        WHL_SKILL( gSkillData.pumaDmgClaw,             "sk_puma_dmg_claw",           22.0f  );
        WHL_SKILL( gSkillData.pumaDmgPounce,           "sk_puma_dmg_pounce",         30.0f  );
        WHL_SKILL( gSkillData.ramoneHealth,            "sk_ramone_health",         1600.0f  );
        WHL_SKILL( gSkillData.ramonePunch,             "sk_ramone_punch",            20.0f  );
        WHL_SKILL( gSkillData.ramoneDynamiteSpeed,     "sk_ramone_dynamitespeed",   500.0f  );
        WHL_SKILL( gSkillData.smallminerHealth,        "sk_smallminer_health",       80.0f  );
        WHL_SKILL( gSkillData.smallminerPick,          "sk_smallminer_pick",         12.0f  );
        WHL_SKILL( gSkillData.snakeHealth,             "sk_snake_health",            15.0f  );
        WHL_SKILL( gSkillData.snakeDmgBite,            "sk_snake_dmg_bite",          20.0f  );
        WHL_SKILL( gSkillData.townmexHealth,           "sk_townmex_health",          90.0f  );
        WHL_SKILL( gSkillData.townmexHeal,             "sk_townmex_heal",            25.0f  );
        WHL_SKILL( gSkillData.wtownaHealth,            "sk_wtowna_health",           80.0f  );
        WHL_SKILL( gSkillData.wtownaHeal,              "sk_wtowna_heal",             25.0f  );
        WHL_SKILL( gSkillData.wtownbHealth,            "sk_wtownb_health",           80.0f  );
        WHL_SKILL( gSkillData.wtownbHeal,              "sk_wtownb_heal",             25.0f  );
        // WantedHL monster weapons
        WHL_SKILL( gSkillData.monDmgPistolBullet,      "sk_pistol_bullet",           5.0f  );
        WHL_SKILL( gSkillData.monDmgShotgunBullet,     "sk_shotgun_bullet",          5.0f  );
        WHL_SKILL( gSkillData.monDmgGattlingGunBullet, "sk_gattlinggun_bullet",     14.0f  );
        WHL_SKILL( gSkillData.monDmgWinchesterBullet,  "sk_winchester_bullet",      70.0f  );
        WHL_SKILL( gSkillData.monDmgBowArrow,          "sk_bow_arrow",              60.0f  );
        // WantedHL health items
        WHL_SKILL( gSkillData.herbsCapacity,           "sk_herbs",                   5.0f  );
        WHL_SKILL( gSkillData.elixerCapacity,          "sk_elixer",                100.0f  );
        // WantedHL player weapons
        WHL_SKILL( gSkillData.plrDmgKnife,             "sk_plr_knife",              22.0f  );
        WHL_SKILL( gSkillData.plrDmgPick,              "sk_plr_pick",               35.0f  );
        WHL_SKILL( gSkillData.plrDmgPistolBullet,      "sk_plr_pistol_bullet",      25.0f  );
        WHL_SKILL( gSkillData.plrDmgColtsBullet,       "sk_plr_colts_bullet",       36.0f  );
        WHL_SKILL( gSkillData.plrDmgWinchesterBullet,  "sk_plr_winchester_bullet",  85.0f  );
        WHL_SKILL( gSkillData.plrDmgWantedShotgun,     "sk_plr_shotgun",            20.0f  );
        WHL_SKILL( gSkillData.plrDmgBuffalo,           "sk_plr_buffalo",           130.0f  );
        WHL_SKILL( gSkillData.plrDmgBowArrow,          "sk_plr_bow_arrow",          80.0f  );
        WHL_SKILL( gSkillData.plrDmgGattlingGunBullet, "sk_plr_gattlinggun_bullet", 18.0f  );
        WHL_SKILL( gSkillData.plrDmgCannonBall,        "sk_plr_cannon_ball",        70.0f  );
        WHL_SKILL( gSkillData.plrDmgDynamite,          "sk_plr_dynamite",          120.0f  );
        WHL_SKILL( gSkillData.plrDmgBeartrap,          "sk_plr_beartrap",           80.0f  );
#undef WHL_SKILL
        // ========== End WantedHL ==========

        // HEALTH/CHARGE
        gSkillData.suitchargerCapacity = GetSkillCvar( "sk_suitcharger" );
        gSkillData.batteryCapacity = GetSkillCvar( "sk_battery" );
        gSkillData.healthchargerCapacity = GetSkillCvar ( "sk_healthcharger" );
        gSkillData.healthkitCapacity = GetSkillCvar ( "sk_healthkit" );
        gSkillData.scientistHeal = GetSkillCvar ( "sk_scientist_heal" );

        // monster damage adj
        gSkillData.monHead = GetSkillCvar( "sk_monster_head" );
        gSkillData.monChest = GetSkillCvar( "sk_monster_chest" );
        gSkillData.monStomach = GetSkillCvar( "sk_monster_stomach" );
        gSkillData.monLeg = GetSkillCvar( "sk_monster_leg" );
        gSkillData.monArm = GetSkillCvar( "sk_monster_arm" );

        // player damage adj
        gSkillData.plrHead = GetSkillCvar( "sk_player_head" );
        gSkillData.plrChest = GetSkillCvar( "sk_player_chest" );
        gSkillData.plrStomach = GetSkillCvar( "sk_player_stomach" );
        gSkillData.plrLeg = GetSkillCvar( "sk_player_leg" );
        gSkillData.plrArm = GetSkillCvar( "sk_player_arm" );
}

void CGameRules::ClientUserInfoChanged( CBasePlayer *pPlayer, char *infobuffer )
{
        pPlayer->SetPrefsFromUserinfo( infobuffer );
}

//=========================================================
// instantiate the proper game rules object
//=========================================================

CGameRules *InstallGameRules( void )
{
        SERVER_COMMAND( "exec game.cfg\n" );
        SERVER_EXECUTE();

        if( !gpGlobals->deathmatch )
        {
                // generic half-life
                g_teamplay = 0;
                return new CHalfLifeRules;
        }
        else
        {
                if( teamplay.value > 0 )
                {
                        // teamplay
                        g_teamplay = 1;
                        return new CHalfLifeTeamplay;
                }
                if( sv_busters.value > 0 )
                {
                        g_teamplay = 0;
                        return new CMultiplayBusters;
                }
                if( (int)gpGlobals->deathmatch == 1 )
                {
                        // vanilla deathmatch
                        g_teamplay = 0;
                        return new CHalfLifeMultiplay;
                }
                else
                {
                        // vanilla deathmatch??
                        g_teamplay = 0;
                        return new CHalfLifeMultiplay;
                }
        }
}
