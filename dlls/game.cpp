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
#include "eiface.h"
#include "util.h"
#include "game.h"
#include "vcs_info.h"

static cvar_t build_commit = { "sv_game_build_commit", g_VCSInfo_Commit };
static cvar_t build_branch = { "sv_game_build_branch", g_VCSInfo_Branch };

cvar_t displaysoundlist = {"displaysoundlist","0"};

// multiplayer server rules
cvar_t fragsleft        = { "mp_fragsleft","0", FCVAR_SERVER | FCVAR_UNLOGGED };          // Don't spam console/log files/users with this changing
cvar_t timeleft         = { "mp_timeleft","0" , FCVAR_SERVER | FCVAR_UNLOGGED };          // "      "

// multiplayer server rules
cvar_t teamplay         = { "mp_teamplay","0", FCVAR_SERVER };
cvar_t fraglimit        = { "mp_fraglimit","0", FCVAR_SERVER };
cvar_t timelimit        = { "mp_timelimit","0", FCVAR_SERVER };
cvar_t friendlyfire     = { "mp_friendlyfire","0", FCVAR_SERVER };
cvar_t falldamage       = { "mp_falldamage","0", FCVAR_SERVER };
cvar_t weaponstay       = { "mp_weaponstay","0", FCVAR_SERVER };
cvar_t selfgauss        = { "selfgauss", "1", FCVAR_SERVER };
cvar_t chargerfix       = { "chargerfix", "0", FCVAR_SERVER };
cvar_t satchelfix       = { "satchelfix", "1", FCVAR_SERVER };
cvar_t explosionfix     = { "explosionfix", "0", FCVAR_SERVER };
cvar_t monsteryawspeedfix       = { "monsteryawspeedfix", "1", FCVAR_SERVER };
cvar_t corpsephysics = { "corpsephysics", "0", FCVAR_SERVER };
cvar_t pushablemode = { "pushablemode", "0", FCVAR_SERVER };
cvar_t forcerespawn     = { "mp_forcerespawn","1", FCVAR_SERVER };
cvar_t flashlight       = { "mp_flashlight","0", FCVAR_SERVER };
cvar_t aimcrosshair     = { "mp_autocrosshair","1", FCVAR_SERVER };
cvar_t decalfrequency   = { "decalfrequency","30", FCVAR_SERVER };
cvar_t teamlist         = { "mp_teamlist","hgrunt;scientist", FCVAR_SERVER };
cvar_t teamoverride     = { "mp_teamoverride","1" };
cvar_t defaultteam      = { "mp_defaultteam","0" };
cvar_t allowmonsters    = { "mp_allowmonsters","0", FCVAR_SERVER };
cvar_t bhopcap          = { "mp_bhopcap", "1", FCVAR_SERVER };

cvar_t allow_spectators = { "allow_spectators", "0", FCVAR_SERVER };    // 0 prevents players from being spectators
cvar_t multibyte_only = { "mp_multibyte_only", "0", FCVAR_SERVER };

cvar_t mp_chattime      = { "mp_chattime","10", FCVAR_SERVER };

// Engine Cvars
cvar_t *g_psv_gravity;
cvar_t *g_psv_aim;
cvar_t *g_psv_allow_autoaim;
cvar_t *g_footsteps;
cvar_t *g_enable_cheats;

cvar_t *g_psv_developer;

//CVARS FOR SKILL LEVEL SETTINGS
// Agrunt
cvar_t  sk_agrunt_health1 = {"sk_agrunt_health1","0"};
cvar_t  sk_agrunt_health2 = {"sk_agrunt_health2","0"};
cvar_t  sk_agrunt_health3 = {"sk_agrunt_health3","0"};

cvar_t  sk_agrunt_dmg_punch1 = {"sk_agrunt_dmg_punch1","0"};
cvar_t  sk_agrunt_dmg_punch2 = {"sk_agrunt_dmg_punch2","0"};
cvar_t  sk_agrunt_dmg_punch3 = {"sk_agrunt_dmg_punch3","0"};

// Apache
cvar_t  sk_apache_health1 = {"sk_apache_health1","0"};
cvar_t  sk_apache_health2 = {"sk_apache_health2","0"};
cvar_t  sk_apache_health3 = {"sk_apache_health3","0"};

// Barney
cvar_t  sk_barney_health1 = {"sk_barney_health1","0"};
cvar_t  sk_barney_health2 = {"sk_barney_health2","0"};
cvar_t  sk_barney_health3 = {"sk_barney_health3","0"};

// Bullsquid
cvar_t  sk_bullsquid_health1 = {"sk_bullsquid_health1","0"};
cvar_t  sk_bullsquid_health2 = {"sk_bullsquid_health2","0"};
cvar_t  sk_bullsquid_health3 = {"sk_bullsquid_health3","0"};

cvar_t  sk_bullsquid_dmg_bite1 = {"sk_bullsquid_dmg_bite1","0"};
cvar_t  sk_bullsquid_dmg_bite2 = {"sk_bullsquid_dmg_bite2","0"};
cvar_t  sk_bullsquid_dmg_bite3 = {"sk_bullsquid_dmg_bite3","0"};

cvar_t  sk_bullsquid_dmg_whip1 = {"sk_bullsquid_dmg_whip1","0"};
cvar_t  sk_bullsquid_dmg_whip2 = {"sk_bullsquid_dmg_whip2","0"};
cvar_t  sk_bullsquid_dmg_whip3 = {"sk_bullsquid_dmg_whip3","0"};

cvar_t  sk_bullsquid_dmg_spit1 = {"sk_bullsquid_dmg_spit1","0"};
cvar_t  sk_bullsquid_dmg_spit2 = {"sk_bullsquid_dmg_spit2","0"};
cvar_t  sk_bullsquid_dmg_spit3 = {"sk_bullsquid_dmg_spit3","0"};


// Big Momma
cvar_t  sk_bigmomma_health_factor1 = {"sk_bigmomma_health_factor1","1.0"};
cvar_t  sk_bigmomma_health_factor2 = {"sk_bigmomma_health_factor2","1.0"};
cvar_t  sk_bigmomma_health_factor3 = {"sk_bigmomma_health_factor3","1.0"};

cvar_t  sk_bigmomma_dmg_slash1 = {"sk_bigmomma_dmg_slash1","50"};
cvar_t  sk_bigmomma_dmg_slash2 = {"sk_bigmomma_dmg_slash2","50"};
cvar_t  sk_bigmomma_dmg_slash3 = {"sk_bigmomma_dmg_slash3","50"};

cvar_t  sk_bigmomma_dmg_blast1 = {"sk_bigmomma_dmg_blast1","100"};
cvar_t  sk_bigmomma_dmg_blast2 = {"sk_bigmomma_dmg_blast2","100"};
cvar_t  sk_bigmomma_dmg_blast3 = {"sk_bigmomma_dmg_blast3","100"};

cvar_t  sk_bigmomma_radius_blast1 = {"sk_bigmomma_radius_blast1","250"};
cvar_t  sk_bigmomma_radius_blast2 = {"sk_bigmomma_radius_blast2","250"};
cvar_t  sk_bigmomma_radius_blast3 = {"sk_bigmomma_radius_blast3","250"};

// Gargantua
cvar_t  sk_gargantua_health1 = {"sk_gargantua_health1","0"};
cvar_t  sk_gargantua_health2 = {"sk_gargantua_health2","0"};
cvar_t  sk_gargantua_health3 = {"sk_gargantua_health3","0"};

cvar_t  sk_gargantua_dmg_slash1 = {"sk_gargantua_dmg_slash1","0"};
cvar_t  sk_gargantua_dmg_slash2 = {"sk_gargantua_dmg_slash2","0"};
cvar_t  sk_gargantua_dmg_slash3 = {"sk_gargantua_dmg_slash3","0"};

cvar_t  sk_gargantua_dmg_fire1 = {"sk_gargantua_dmg_fire1","0"};
cvar_t  sk_gargantua_dmg_fire2 = {"sk_gargantua_dmg_fire2","0"};
cvar_t  sk_gargantua_dmg_fire3 = {"sk_gargantua_dmg_fire3","0"};

cvar_t  sk_gargantua_dmg_stomp1 = {"sk_gargantua_dmg_stomp1","0"};
cvar_t  sk_gargantua_dmg_stomp2 = {"sk_gargantua_dmg_stomp2","0"};
cvar_t  sk_gargantua_dmg_stomp3 = {"sk_gargantua_dmg_stomp3","0"};


// Hassassin
cvar_t  sk_hassassin_health1 = {"sk_hassassin_health1","0"};
cvar_t  sk_hassassin_health2 = {"sk_hassassin_health2","0"};
cvar_t  sk_hassassin_health3 = {"sk_hassassin_health3","0"};


// Headcrab
cvar_t  sk_headcrab_health1 = {"sk_headcrab_health1","0"};
cvar_t  sk_headcrab_health2 = {"sk_headcrab_health2","0"};
cvar_t  sk_headcrab_health3 = {"sk_headcrab_health3","0"};

cvar_t  sk_headcrab_dmg_bite1 = {"sk_headcrab_dmg_bite1","0"};
cvar_t  sk_headcrab_dmg_bite2 = {"sk_headcrab_dmg_bite2","0"};
cvar_t  sk_headcrab_dmg_bite3 = {"sk_headcrab_dmg_bite3","0"};


// Hgrunt 
cvar_t  sk_hgrunt_health1 = {"sk_hgrunt_health1","0"};
cvar_t  sk_hgrunt_health2 = {"sk_hgrunt_health2","0"};
cvar_t  sk_hgrunt_health3 = {"sk_hgrunt_health3","0"};

cvar_t  sk_hgrunt_kick1 = {"sk_hgrunt_kick1","0"};
cvar_t  sk_hgrunt_kick2 = {"sk_hgrunt_kick2","0"};
cvar_t  sk_hgrunt_kick3 = {"sk_hgrunt_kick3","0"};

cvar_t  sk_hgrunt_pellets1 = {"sk_hgrunt_pellets1","0"};
cvar_t  sk_hgrunt_pellets2 = {"sk_hgrunt_pellets2","0"};
cvar_t  sk_hgrunt_pellets3 = {"sk_hgrunt_pellets3","0"};

cvar_t  sk_hgrunt_gspeed1 = {"sk_hgrunt_gspeed1","0"};
cvar_t  sk_hgrunt_gspeed2 = {"sk_hgrunt_gspeed2","0"};
cvar_t  sk_hgrunt_gspeed3 = {"sk_hgrunt_gspeed3","0"};

// Houndeye
cvar_t  sk_houndeye_health1 = {"sk_houndeye_health1","0"};
cvar_t  sk_houndeye_health2 = {"sk_houndeye_health2","0"};
cvar_t  sk_houndeye_health3 = {"sk_houndeye_health3","0"};

cvar_t  sk_houndeye_dmg_blast1 = {"sk_houndeye_dmg_blast1","0"};
cvar_t  sk_houndeye_dmg_blast2 = {"sk_houndeye_dmg_blast2","0"};
cvar_t  sk_houndeye_dmg_blast3 = {"sk_houndeye_dmg_blast3","0"};


// ISlave
cvar_t  sk_islave_health1 = {"sk_islave_health1","0"};
cvar_t  sk_islave_health2 = {"sk_islave_health2","0"};
cvar_t  sk_islave_health3 = {"sk_islave_health3","0"};

cvar_t  sk_islave_dmg_claw1 = {"sk_islave_dmg_claw1","0"};
cvar_t  sk_islave_dmg_claw2 = {"sk_islave_dmg_claw2","0"};
cvar_t  sk_islave_dmg_claw3 = {"sk_islave_dmg_claw3","0"};

cvar_t  sk_islave_dmg_clawrake1 = {"sk_islave_dmg_clawrake1","0"};
cvar_t  sk_islave_dmg_clawrake2 = {"sk_islave_dmg_clawrake2","0"};
cvar_t  sk_islave_dmg_clawrake3 = {"sk_islave_dmg_clawrake3","0"};
        
cvar_t  sk_islave_dmg_zap1 = {"sk_islave_dmg_zap1","0"};
cvar_t  sk_islave_dmg_zap2 = {"sk_islave_dmg_zap2","0"};
cvar_t  sk_islave_dmg_zap3 = {"sk_islave_dmg_zap3","0"};


// Icthyosaur
cvar_t  sk_ichthyosaur_health1  = {"sk_ichthyosaur_health1","0"};
cvar_t  sk_ichthyosaur_health2  = {"sk_ichthyosaur_health2","0"};
cvar_t  sk_ichthyosaur_health3  = {"sk_ichthyosaur_health3","0"};

cvar_t  sk_ichthyosaur_shake1   = {"sk_ichthyosaur_shake1","0"};
cvar_t  sk_ichthyosaur_shake2   = {"sk_ichthyosaur_shake2","0"};
cvar_t  sk_ichthyosaur_shake3   = {"sk_ichthyosaur_shake3","0"};


// Leech
cvar_t  sk_leech_health1 = {"sk_leech_health1","0"};
cvar_t  sk_leech_health2 = {"sk_leech_health2","0"};
cvar_t  sk_leech_health3 = {"sk_leech_health3","0"};

cvar_t  sk_leech_dmg_bite1 = {"sk_leech_dmg_bite1","0"};
cvar_t  sk_leech_dmg_bite2 = {"sk_leech_dmg_bite2","0"};
cvar_t  sk_leech_dmg_bite3 = {"sk_leech_dmg_bite3","0"};

// Controller
cvar_t  sk_controller_health1 = {"sk_controller_health1","0"};
cvar_t  sk_controller_health2 = {"sk_controller_health2","0"};
cvar_t  sk_controller_health3 = {"sk_controller_health3","0"};

cvar_t  sk_controller_dmgzap1 = {"sk_controller_dmgzap1","0"};
cvar_t  sk_controller_dmgzap2 = {"sk_controller_dmgzap2","0"};
cvar_t  sk_controller_dmgzap3 = {"sk_controller_dmgzap3","0"};

cvar_t  sk_controller_speedball1 = {"sk_controller_speedball1","0"};
cvar_t  sk_controller_speedball2 = {"sk_controller_speedball2","0"};
cvar_t  sk_controller_speedball3 = {"sk_controller_speedball3","0"};

cvar_t  sk_controller_dmgball1 = {"sk_controller_dmgball1","0"};
cvar_t  sk_controller_dmgball2 = {"sk_controller_dmgball2","0"};
cvar_t  sk_controller_dmgball3 = {"sk_controller_dmgball3","0"};

// Nihilanth
cvar_t  sk_nihilanth_health1 = {"sk_nihilanth_health1","0"};
cvar_t  sk_nihilanth_health2 = {"sk_nihilanth_health2","0"};
cvar_t  sk_nihilanth_health3 = {"sk_nihilanth_health3","0"};

cvar_t  sk_nihilanth_zap1 = {"sk_nihilanth_zap1","0"};
cvar_t  sk_nihilanth_zap2 = {"sk_nihilanth_zap2","0"};
cvar_t  sk_nihilanth_zap3 = {"sk_nihilanth_zap3","0"};

// Scientist
cvar_t  sk_scientist_health1 = {"sk_scientist_health1","0"};
cvar_t  sk_scientist_health2 = {"sk_scientist_health2","0"};
cvar_t  sk_scientist_health3 = {"sk_scientist_health3","0"};

// Snark
cvar_t  sk_snark_health1 = {"sk_snark_health1","0"};
cvar_t  sk_snark_health2 = {"sk_snark_health2","0"};
cvar_t  sk_snark_health3 = {"sk_snark_health3","0"};

cvar_t  sk_snark_dmg_bite1 = {"sk_snark_dmg_bite1","0"};
cvar_t  sk_snark_dmg_bite2 = {"sk_snark_dmg_bite2","0"};
cvar_t  sk_snark_dmg_bite3 = {"sk_snark_dmg_bite3","0"};

cvar_t  sk_snark_dmg_pop1 = {"sk_snark_dmg_pop1","0"};
cvar_t  sk_snark_dmg_pop2 = {"sk_snark_dmg_pop2","0"};
cvar_t  sk_snark_dmg_pop3 = {"sk_snark_dmg_pop3","0"};

cvar_t  sk_scorpion_health1 = {"sk_scorpion_health1","10"};
cvar_t  sk_scorpion_health2 = {"sk_scorpion_health2","15"};
cvar_t  sk_scorpion_health3 = {"sk_scorpion_health3","20"};

cvar_t  sk_scorpion_dmg_sting1 = {"sk_scorpion_dmg_sting1","10"};
cvar_t  sk_scorpion_dmg_sting2 = {"sk_scorpion_dmg_sting2","15"};
cvar_t  sk_scorpion_dmg_sting3 = {"sk_scorpion_dmg_sting3","20"};

// ========== WantedHL Monsters ==========
// Annie
cvar_t sk_annie_health1 = {"sk_annie_health1","35"};
cvar_t sk_annie_health2 = {"sk_annie_health2","35"};
cvar_t sk_annie_health3 = {"sk_annie_health3","35"};
// Bear
cvar_t sk_bear_health1 = {"sk_bear_health1","400"};
cvar_t sk_bear_health2 = {"sk_bear_health2","420"};
cvar_t sk_bear_health3 = {"sk_bear_health3","440"};
cvar_t sk_bear_dmg_claw1 = {"sk_bear_dmg_claw1","25"};
cvar_t sk_bear_dmg_claw2 = {"sk_bear_dmg_claw2","27"};
cvar_t sk_bear_dmg_claw3 = {"sk_bear_dmg_claw3","30"};
cvar_t sk_bear_dmg_pounce1 = {"sk_bear_dmg_pounce1","30"};
cvar_t sk_bear_dmg_pounce2 = {"sk_bear_dmg_pounce2","32"};
cvar_t sk_bear_dmg_pounce3 = {"sk_bear_dmg_pounce3","35"};
// BigMiner
cvar_t sk_bigminer_health1 = {"sk_bigminer_health1","140"};
cvar_t sk_bigminer_health2 = {"sk_bigminer_health2","150"};
cvar_t sk_bigminer_health3 = {"sk_bigminer_health3","150"};
cvar_t sk_bigminer_punch1 = {"sk_bigminer_punch1","15"};
cvar_t sk_bigminer_punch2 = {"sk_bigminer_punch2","18"};
cvar_t sk_bigminer_punch3 = {"sk_bigminer_punch3","22"};
// Chicken
cvar_t sk_chicken_health1 = {"sk_chicken_health1","5"};
cvar_t sk_chicken_health2 = {"sk_chicken_health2","5"};
cvar_t sk_chicken_health3 = {"sk_chicken_health3","5"};
cvar_t sk_chicken_dmg_peck1 = {"sk_chicken_dmg_peck1","1"};
cvar_t sk_chicken_dmg_peck2 = {"sk_chicken_dmg_peck2","1"};
cvar_t sk_chicken_dmg_peck3 = {"sk_chicken_dmg_peck3","1"};
// Colonel
cvar_t sk_colonel_health1 = {"sk_colonel_health1","100"};
cvar_t sk_colonel_health2 = {"sk_colonel_health2","100"};
cvar_t sk_colonel_health3 = {"sk_colonel_health3","100"};
cvar_t sk_colonel_heal1 = {"sk_colonel_heal1","25"};
cvar_t sk_colonel_heal2 = {"sk_colonel_heal2","25"};
cvar_t sk_colonel_heal3 = {"sk_colonel_heal3","25"};
// Crispen
cvar_t sk_crispen_health1 = {"sk_crispen_health1","75"};
cvar_t sk_crispen_health2 = {"sk_crispen_health2","75"};
cvar_t sk_crispen_health3 = {"sk_crispen_health3","75"};
cvar_t sk_crispen_heal1 = {"sk_crispen_heal1","25"};
cvar_t sk_crispen_heal2 = {"sk_crispen_heal2","25"};
cvar_t sk_crispen_heal3 = {"sk_crispen_heal3","25"};
// Cowboy
cvar_t sk_cowboy_health1 = {"sk_cowboy_health1","100"};
cvar_t sk_cowboy_health2 = {"sk_cowboy_health2","100"};
cvar_t sk_cowboy_health3 = {"sk_cowboy_health3","100"};
cvar_t sk_cowboy_punch1 = {"sk_cowboy_punch1","5"};
cvar_t sk_cowboy_punch2 = {"sk_cowboy_punch2","7"};
cvar_t sk_cowboy_punch3 = {"sk_cowboy_punch3","10"};
cvar_t sk_cowboy_dynamitespeed1 = {"sk_cowboy_dynamitespeed1","400"};
cvar_t sk_cowboy_dynamitespeed2 = {"sk_cowboy_dynamitespeed2","600"};
cvar_t sk_cowboy_dynamitespeed3 = {"sk_cowboy_dynamitespeed3","800"};
// Dave
cvar_t sk_dave_health1 = {"sk_dave_health1","75"};
cvar_t sk_dave_health2 = {"sk_dave_health2","75"};
cvar_t sk_dave_health3 = {"sk_dave_health3","75"};
cvar_t sk_dave_heal1 = {"sk_dave_heal1","25"};
cvar_t sk_dave_heal2 = {"sk_dave_heal2","25"};
cvar_t sk_dave_heal3 = {"sk_dave_heal3","25"};
// Horse
cvar_t sk_horse_health1 = {"sk_horse_health1","110"};
cvar_t sk_horse_health2 = {"sk_horse_health2","110"};
cvar_t sk_horse_health3 = {"sk_horse_health3","110"};
cvar_t sk_horse_dmg_kick1 = {"sk_horse_dmg_kick1","10"};
cvar_t sk_horse_dmg_kick2 = {"sk_horse_dmg_kick2","10"};
cvar_t sk_horse_dmg_kick3 = {"sk_horse_dmg_kick3","10"};
// Hoss
cvar_t sk_hoss_health1 = {"sk_hoss_health1","90"};
cvar_t sk_hoss_health2 = {"sk_hoss_health2","90"};
cvar_t sk_hoss_health3 = {"sk_hoss_health3","90"};
// Kaiewi
cvar_t sk_kaiewi_health1 = {"sk_kaiewi_health1","105"};
cvar_t sk_kaiewi_health2 = {"sk_kaiewi_health2","107"};
cvar_t sk_kaiewi_health3 = {"sk_kaiewi_health3","110"};
cvar_t sk_kaiewi_punch1 = {"sk_kaiewi_punch1","5"};
cvar_t sk_kaiewi_punch2 = {"sk_kaiewi_punch2","7"};
cvar_t sk_kaiewi_punch3 = {"sk_kaiewi_punch3","10"};
// Masala
cvar_t sk_masala_health1 = {"sk_masala_health1","100"};
cvar_t sk_masala_health2 = {"sk_masala_health2","100"};
cvar_t sk_masala_health3 = {"sk_masala_health3","100"};
cvar_t sk_masala_heal1 = {"sk_masala_heal1","25"};
cvar_t sk_masala_heal2 = {"sk_masala_heal2","25"};
cvar_t sk_masala_heal3 = {"sk_masala_heal3","25"};
// MexBandit
cvar_t sk_mexbandit_health1 = {"sk_mexbandit_health1","100"};
cvar_t sk_mexbandit_health2 = {"sk_mexbandit_health2","100"};
cvar_t sk_mexbandit_health3 = {"sk_mexbandit_health3","100"};
cvar_t sk_mexbandit_kick1 = {"sk_mexbandit_kick1","20"};
cvar_t sk_mexbandit_kick2 = {"sk_mexbandit_kick2","25"};
cvar_t sk_mexbandit_kick3 = {"sk_mexbandit_kick3","25"};
cvar_t sk_mexbandit_dynamitespeed1 = {"sk_mexbandit_dynamitespeed1","500"};
cvar_t sk_mexbandit_dynamitespeed2 = {"sk_mexbandit_dynamitespeed2","500"};
cvar_t sk_mexbandit_dynamitespeed3 = {"sk_mexbandit_dynamitespeed3","500"};
// Mexican Townsfolk
cvar_t sk_townmex_health1 = {"sk_townmex_health1","90"};
cvar_t sk_townmex_health2 = {"sk_townmex_health2","90"};
cvar_t sk_townmex_health3 = {"sk_townmex_health3","90"};
cvar_t sk_townmex_heal1 = {"sk_townmex_heal1","25"};
cvar_t sk_townmex_heal2 = {"sk_townmex_heal2","25"};
cvar_t sk_townmex_heal3 = {"sk_townmex_heal3","25"};
// Nagatow
cvar_t sk_nagatow_health1 = {"sk_nagatow_health1","100"};
cvar_t sk_nagatow_health2 = {"sk_nagatow_health2","100"};
cvar_t sk_nagatow_health3 = {"sk_nagatow_health3","100"};
cvar_t sk_nagatow_heal1 = {"sk_nagatow_heal1","25"};
cvar_t sk_nagatow_heal2 = {"sk_nagatow_heal2","25"};
cvar_t sk_nagatow_heal3 = {"sk_nagatow_heal3","25"};
// Puma
cvar_t sk_puma_health1 = {"sk_puma_health1","130"};
cvar_t sk_puma_health2 = {"sk_puma_health2","135"};
cvar_t sk_puma_health3 = {"sk_puma_health3","140"};
cvar_t sk_puma_dmg_claw1 = {"sk_puma_dmg_claw1","22"};
cvar_t sk_puma_dmg_claw2 = {"sk_puma_dmg_claw2","25"};
cvar_t sk_puma_dmg_claw3 = {"sk_puma_dmg_claw3","27"};
cvar_t sk_puma_dmg_pounce1 = {"sk_puma_dmg_pounce1","30"};
cvar_t sk_puma_dmg_pounce2 = {"sk_puma_dmg_pounce2","34"};
cvar_t sk_puma_dmg_pounce3 = {"sk_puma_dmg_pounce3","38"};
// Ramone
cvar_t sk_ramone_health1 = {"sk_ramone_health1","1600"};
cvar_t sk_ramone_health2 = {"sk_ramone_health2","1800"};
cvar_t sk_ramone_health3 = {"sk_ramone_health3","2000"};
cvar_t sk_ramone_punch1 = {"sk_ramone_punch1","20"};
cvar_t sk_ramone_punch2 = {"sk_ramone_punch2","25"};
cvar_t sk_ramone_punch3 = {"sk_ramone_punch3","25"};
cvar_t sk_ramone_dynamitespeed1 = {"sk_ramone_dynamitespeed1","500"};
cvar_t sk_ramone_dynamitespeed2 = {"sk_ramone_dynamitespeed2","700"};
cvar_t sk_ramone_dynamitespeed3 = {"sk_ramone_dynamitespeed3","900"};
// SmallMiner
cvar_t sk_smallminer_health1 = {"sk_smallminer_health1","100"};
cvar_t sk_smallminer_health2 = {"sk_smallminer_health2","100"};
cvar_t sk_smallminer_health3 = {"sk_smallminer_health3","100"};
cvar_t sk_smallminer_pick1 = {"sk_smallminer_pick1","25"};
cvar_t sk_smallminer_pick2 = {"sk_smallminer_pick2","30"};
cvar_t sk_smallminer_pick3 = {"sk_smallminer_pick3","34"};
// Snake
cvar_t sk_snake_health1 = {"sk_snake_health1","15"};
cvar_t sk_snake_health2 = {"sk_snake_health2","15"};
cvar_t sk_snake_health3 = {"sk_snake_health3","15"};
cvar_t sk_snake_dmg_bite1 = {"sk_snake_dmg_bite1","20"};
cvar_t sk_snake_dmg_bite2 = {"sk_snake_dmg_bite2","22"};
cvar_t sk_snake_dmg_bite3 = {"sk_snake_dmg_bite3","25"};
// Western Townie A
cvar_t sk_wtowna_health1 = {"sk_wtowna_health1","80"};
cvar_t sk_wtowna_health2 = {"sk_wtowna_health2","80"};
cvar_t sk_wtowna_health3 = {"sk_wtowna_health3","80"};
cvar_t sk_wtowna_heal1 = {"sk_wtowna_heal1","25"};
cvar_t sk_wtowna_heal2 = {"sk_wtowna_heal2","25"};
cvar_t sk_wtowna_heal3 = {"sk_wtowna_heal3","25"};
// Western Townie B
cvar_t sk_wtownb_health1 = {"sk_wtownb_health1","80"};
cvar_t sk_wtownb_health2 = {"sk_wtownb_health2","80"};
cvar_t sk_wtownb_health3 = {"sk_wtownb_health3","80"};
cvar_t sk_wtownb_heal1 = {"sk_wtownb_heal1","25"};
cvar_t sk_wtownb_heal2 = {"sk_wtownb_heal2","25"};
cvar_t sk_wtownb_heal3 = {"sk_wtownb_heal3","25"};
// WantedHL Monster Weapons
cvar_t sk_pistol_bullet1 = {"sk_pistol_bullet1","5"};
cvar_t sk_pistol_bullet2 = {"sk_pistol_bullet2","5"};
cvar_t sk_pistol_bullet3 = {"sk_pistol_bullet3","5"};
cvar_t sk_shotgun_bullet1 = {"sk_shotgun_bullet1","5"};
cvar_t sk_shotgun_bullet2 = {"sk_shotgun_bullet2","7"};
cvar_t sk_shotgun_bullet3 = {"sk_shotgun_bullet3","7"};
cvar_t sk_gattlinggun_bullet1 = {"sk_gattlinggun_bullet1","14"};
cvar_t sk_gattlinggun_bullet2 = {"sk_gattlinggun_bullet2","16"};
cvar_t sk_gattlinggun_bullet3 = {"sk_gattlinggun_bullet3","16"};
cvar_t sk_winchester_bullet1 = {"sk_winchester_bullet1","70"};
cvar_t sk_winchester_bullet2 = {"sk_winchester_bullet2","75"};
cvar_t sk_winchester_bullet3 = {"sk_winchester_bullet3","80"};
cvar_t sk_bow_arrow1 = {"sk_bow_arrow1","60"};
cvar_t sk_bow_arrow2 = {"sk_bow_arrow2","65"};
cvar_t sk_bow_arrow3 = {"sk_bow_arrow3","70"};
// WantedHL Health items
cvar_t sk_herbs1 = {"sk_herbs1","5"};
cvar_t sk_herbs2 = {"sk_herbs2","5"};
cvar_t sk_herbs3 = {"sk_herbs3","5"};
cvar_t sk_elixer1 = {"sk_elixer1","100"};
cvar_t sk_elixer2 = {"sk_elixer2","100"};
cvar_t sk_elixer3 = {"sk_elixer3","100"};
// WantedHL Player Weapons
cvar_t sk_plr_knife1 = {"sk_plr_knife1","22"};
cvar_t sk_plr_knife2 = {"sk_plr_knife2","20"};
cvar_t sk_plr_knife3 = {"sk_plr_knife3","20"};
cvar_t sk_plr_pick1 = {"sk_plr_pick1","35"};
cvar_t sk_plr_pick2 = {"sk_plr_pick2","32"};
cvar_t sk_plr_pick3 = {"sk_plr_pick3","30"};
cvar_t sk_plr_pistol_bullet1 = {"sk_plr_pistol_bullet1","25"};
cvar_t sk_plr_pistol_bullet2 = {"sk_plr_pistol_bullet2","22"};
cvar_t sk_plr_pistol_bullet3 = {"sk_plr_pistol_bullet3","20"};
cvar_t sk_plr_colts_bullet1 = {"sk_plr_colts_bullet1","36"};
cvar_t sk_plr_colts_bullet2 = {"sk_plr_colts_bullet2","33"};
cvar_t sk_plr_colts_bullet3 = {"sk_plr_colts_bullet3","30"};
cvar_t sk_plr_winchester_bullet1 = {"sk_plr_winchester_bullet1","85"};
cvar_t sk_plr_winchester_bullet2 = {"sk_plr_winchester_bullet2","80"};
cvar_t sk_plr_winchester_bullet3 = {"sk_plr_winchester_bullet3","75"};
cvar_t sk_plr_shotgun1 = {"sk_plr_shotgun1","20"};
cvar_t sk_plr_shotgun2 = {"sk_plr_shotgun2","18"};
cvar_t sk_plr_shotgun3 = {"sk_plr_shotgun3","15"};
cvar_t sk_plr_buffalo1 = {"sk_plr_buffalo1","130"};
cvar_t sk_plr_buffalo2 = {"sk_plr_buffalo2","120"};
cvar_t sk_plr_buffalo3 = {"sk_plr_buffalo3","110"};
cvar_t sk_plr_bow_arrow1 = {"sk_plr_bow_arrow1","80"};
cvar_t sk_plr_bow_arrow2 = {"sk_plr_bow_arrow2","75"};
cvar_t sk_plr_bow_arrow3 = {"sk_plr_bow_arrow3","70"};
cvar_t sk_plr_gattlinggun_bullet1 = {"sk_plr_gattlinggun_bullet1","18"};
cvar_t sk_plr_gattlinggun_bullet2 = {"sk_plr_gattlinggun_bullet2","14"};
cvar_t sk_plr_gattlinggun_bullet3 = {"sk_plr_gattlinggun_bullet3","12"};
cvar_t sk_plr_cannon_ball1 = {"sk_plr_cannon_ball1","70"};
cvar_t sk_plr_cannon_ball2 = {"sk_plr_cannon_ball2","70"};
cvar_t sk_plr_cannon_ball3 = {"sk_plr_cannon_ball3","70"};
cvar_t sk_plr_dynamite1 = {"sk_plr_dynamite1","120"};
cvar_t sk_plr_dynamite2 = {"sk_plr_dynamite2","110"};
cvar_t sk_plr_dynamite3 = {"sk_plr_dynamite3","100"};
cvar_t sk_plr_beartrap1 = {"sk_plr_beartrap1","80"};
cvar_t sk_plr_beartrap2 = {"sk_plr_beartrap2","75"};
cvar_t sk_plr_beartrap3 = {"sk_plr_beartrap3","70"};
// ========== End WantedHL Monsters ==========

// Zombie
cvar_t  sk_zombie_health1 = {"sk_zombie_health1","0"};
cvar_t  sk_zombie_health2 = {"sk_zombie_health2","0"};
cvar_t  sk_zombie_health3 = {"sk_zombie_health3","0"};

cvar_t  sk_zombie_dmg_one_slash1 = {"sk_zombie_dmg_one_slash1","0"};
cvar_t  sk_zombie_dmg_one_slash2 = {"sk_zombie_dmg_one_slash2","0"};
cvar_t  sk_zombie_dmg_one_slash3 = {"sk_zombie_dmg_one_slash3","0"};

cvar_t  sk_zombie_dmg_both_slash1 = {"sk_zombie_dmg_both_slash1","0"};
cvar_t  sk_zombie_dmg_both_slash2 = {"sk_zombie_dmg_both_slash2","0"};
cvar_t  sk_zombie_dmg_both_slash3 = {"sk_zombie_dmg_both_slash3","0"};

//Turret
cvar_t  sk_turret_health1 = {"sk_turret_health1","0"};
cvar_t  sk_turret_health2 = {"sk_turret_health2","0"};
cvar_t  sk_turret_health3 = {"sk_turret_health3","0"};

// MiniTurret
cvar_t  sk_miniturret_health1 = {"sk_miniturret_health1","0"};
cvar_t  sk_miniturret_health2 = {"sk_miniturret_health2","0"};
cvar_t  sk_miniturret_health3 = {"sk_miniturret_health3","0"};

// Sentry Turret
cvar_t  sk_sentry_health1 = {"sk_sentry_health1","0"};
cvar_t  sk_sentry_health2 = {"sk_sentry_health2","0"};
cvar_t  sk_sentry_health3 = {"sk_sentry_health3","0"};

// PLAYER WEAPONS

// Crowbar whack
cvar_t  sk_plr_crowbar1 = {"sk_plr_crowbar1","0"};
cvar_t  sk_plr_crowbar2 = {"sk_plr_crowbar2","0"};
cvar_t  sk_plr_crowbar3 = {"sk_plr_crowbar3","0"};

// Glock Round
cvar_t  sk_plr_9mm_bullet1 = {"sk_plr_9mm_bullet1","0"};
cvar_t  sk_plr_9mm_bullet2 = {"sk_plr_9mm_bullet2","0"};
cvar_t  sk_plr_9mm_bullet3 = {"sk_plr_9mm_bullet3","0"};

// 357 Round
cvar_t  sk_plr_357_bullet1 = {"sk_plr_357_bullet1","0"};
cvar_t  sk_plr_357_bullet2 = {"sk_plr_357_bullet2","0"};
cvar_t  sk_plr_357_bullet3 = {"sk_plr_357_bullet3","0"};

// MP5 Round
cvar_t  sk_plr_9mmAR_bullet1 = {"sk_plr_9mmAR_bullet1","0"};
cvar_t  sk_plr_9mmAR_bullet2 = {"sk_plr_9mmAR_bullet2","0"};
cvar_t  sk_plr_9mmAR_bullet3 = {"sk_plr_9mmAR_bullet3","0"};


// M203 grenade
cvar_t  sk_plr_9mmAR_grenade1 = {"sk_plr_9mmAR_grenade1","0"};
cvar_t  sk_plr_9mmAR_grenade2 = {"sk_plr_9mmAR_grenade2","0"};
cvar_t  sk_plr_9mmAR_grenade3 = {"sk_plr_9mmAR_grenade3","0"};


// Shotgun buckshot
cvar_t  sk_plr_buckshot1 = {"sk_plr_buckshot1","0"};
cvar_t  sk_plr_buckshot2 = {"sk_plr_buckshot2","0"};
cvar_t  sk_plr_buckshot3 = {"sk_plr_buckshot3","0"};


// Crossbow
cvar_t  sk_plr_xbow_bolt_client1 = {"sk_plr_xbow_bolt_client1","0"};
cvar_t  sk_plr_xbow_bolt_client2 = {"sk_plr_xbow_bolt_client2","0"};
cvar_t  sk_plr_xbow_bolt_client3 = {"sk_plr_xbow_bolt_client3","0"};

cvar_t  sk_plr_xbow_bolt_monster1 = {"sk_plr_xbow_bolt_monster1","0"};
cvar_t  sk_plr_xbow_bolt_monster2 = {"sk_plr_xbow_bolt_monster2","0"};
cvar_t  sk_plr_xbow_bolt_monster3 = {"sk_plr_xbow_bolt_monster3","0"};


// RPG
cvar_t  sk_plr_rpg1 = {"sk_plr_rpg1","0"};
cvar_t  sk_plr_rpg2 = {"sk_plr_rpg2","0"};
cvar_t  sk_plr_rpg3 = {"sk_plr_rpg3","0"};


// Zero Point Generator
cvar_t  sk_plr_gauss1 = {"sk_plr_gauss1","0"};
cvar_t  sk_plr_gauss2 = {"sk_plr_gauss2","0"};
cvar_t  sk_plr_gauss3 = {"sk_plr_gauss3","0"};


// Tau Cannon
cvar_t  sk_plr_egon_narrow1 = {"sk_plr_egon_narrow1","0"};
cvar_t  sk_plr_egon_narrow2 = {"sk_plr_egon_narrow2","0"};
cvar_t  sk_plr_egon_narrow3 = {"sk_plr_egon_narrow3","0"};

cvar_t  sk_plr_egon_wide1 = {"sk_plr_egon_wide1","0"};
cvar_t  sk_plr_egon_wide2 = {"sk_plr_egon_wide2","0"};
cvar_t  sk_plr_egon_wide3 = {"sk_plr_egon_wide3","0"};


// Hand Grendade
cvar_t  sk_plr_hand_grenade1 = {"sk_plr_hand_grenade1","0"};
cvar_t  sk_plr_hand_grenade2 = {"sk_plr_hand_grenade2","0"};
cvar_t  sk_plr_hand_grenade3 = {"sk_plr_hand_grenade3","0"};


// Satchel Charge
cvar_t  sk_plr_satchel1 = {"sk_plr_satchel1","0"};
cvar_t  sk_plr_satchel2 = {"sk_plr_satchel2","0"};
cvar_t  sk_plr_satchel3 = {"sk_plr_satchel3","0"};


// Tripmine
cvar_t  sk_plr_tripmine1 = {"sk_plr_tripmine1","0"};
cvar_t  sk_plr_tripmine2 = {"sk_plr_tripmine2","0"};
cvar_t  sk_plr_tripmine3 = {"sk_plr_tripmine3","0"};


// WORLD WEAPONS
cvar_t  sk_12mm_bullet1 = {"sk_12mm_bullet1","0"};
cvar_t  sk_12mm_bullet2 = {"sk_12mm_bullet2","0"};
cvar_t  sk_12mm_bullet3 = {"sk_12mm_bullet3","0"};

cvar_t  sk_9mmAR_bullet1 = {"sk_9mmAR_bullet1","0"};
cvar_t  sk_9mmAR_bullet2 = {"sk_9mmAR_bullet2","0"};
cvar_t  sk_9mmAR_bullet3 = {"sk_9mmAR_bullet3","0"};

cvar_t  sk_9mm_bullet1 = {"sk_9mm_bullet1","0"};
cvar_t  sk_9mm_bullet2 = {"sk_9mm_bullet2","0"};
cvar_t  sk_9mm_bullet3 = {"sk_9mm_bullet3","0"};


// HORNET
cvar_t  sk_hornet_dmg1 = {"sk_hornet_dmg1","0"};
cvar_t  sk_hornet_dmg2 = {"sk_hornet_dmg2","0"};
cvar_t  sk_hornet_dmg3 = {"sk_hornet_dmg3","0"};

// HEALTH/CHARGE
cvar_t  sk_suitcharger1 = { "sk_suitcharger1","0" };
cvar_t  sk_suitcharger2 = { "sk_suitcharger2","0" };            
cvar_t  sk_suitcharger3 = { "sk_suitcharger3","0" };            

cvar_t  sk_battery1     = { "sk_battery1","0" };                        
cvar_t  sk_battery2     = { "sk_battery2","0" };                        
cvar_t  sk_battery3     = { "sk_battery3","0" };                        

cvar_t  sk_healthcharger1       = { "sk_healthcharger1","0" };          
cvar_t  sk_healthcharger2       = { "sk_healthcharger2","0" };          
cvar_t  sk_healthcharger3       = { "sk_healthcharger3","0" };          

cvar_t  sk_healthkit1   = { "sk_healthkit1","0" };              
cvar_t  sk_healthkit2   = { "sk_healthkit2","0" };              
cvar_t  sk_healthkit3   = { "sk_healthkit3","0" };              

cvar_t  sk_scientist_heal1      = { "sk_scientist_heal1","0" }; 
cvar_t  sk_scientist_heal2      = { "sk_scientist_heal2","0" }; 
cvar_t  sk_scientist_heal3      = { "sk_scientist_heal3","0" }; 


// monster damage adjusters
cvar_t  sk_monster_head1        = { "sk_monster_head1","2" };
cvar_t  sk_monster_head2        = { "sk_monster_head2","2" };
cvar_t  sk_monster_head3        = { "sk_monster_head3","2" };

cvar_t  sk_monster_chest1       = { "sk_monster_chest1","1" };
cvar_t  sk_monster_chest2       = { "sk_monster_chest2","1" };
cvar_t  sk_monster_chest3       = { "sk_monster_chest3","1" };

cvar_t  sk_monster_stomach1     = { "sk_monster_stomach1","1" };
cvar_t  sk_monster_stomach2     = { "sk_monster_stomach2","1" };
cvar_t  sk_monster_stomach3     = { "sk_monster_stomach3","1" };

cvar_t  sk_monster_arm1 = { "sk_monster_arm1","1" };
cvar_t  sk_monster_arm2 = { "sk_monster_arm2","1" };
cvar_t  sk_monster_arm3 = { "sk_monster_arm3","1" };

cvar_t  sk_monster_leg1 = { "sk_monster_leg1","1" };
cvar_t  sk_monster_leg2 = { "sk_monster_leg2","1" };
cvar_t  sk_monster_leg3 = { "sk_monster_leg3","1" };

// player damage adjusters
cvar_t  sk_player_head1 = { "sk_player_head1","2" };
cvar_t  sk_player_head2 = { "sk_player_head2","2" };
cvar_t  sk_player_head3 = { "sk_player_head3","2" };

cvar_t  sk_player_chest1 = { "sk_player_chest1","1" };
cvar_t  sk_player_chest2 = { "sk_player_chest2","1" };
cvar_t  sk_player_chest3 = { "sk_player_chest3","1" };

cvar_t  sk_player_stomach1 = { "sk_player_stomach1","1" };
cvar_t  sk_player_stomach2 = { "sk_player_stomach2","1" };
cvar_t  sk_player_stomach3 = { "sk_player_stomach3","1" };

cvar_t  sk_player_arm1  = { "sk_player_arm1","1" };
cvar_t  sk_player_arm2  = { "sk_player_arm2","1" };
cvar_t  sk_player_arm3  = { "sk_player_arm3","1" };

cvar_t  sk_player_leg1  = { "sk_player_leg1","1" };
cvar_t  sk_player_leg2  = { "sk_player_leg2","1" };
cvar_t  sk_player_leg3  = { "sk_player_leg3","1" };

// END Cvars for Skill Level settings

cvar_t sv_pushable_fixed_tick_fudge = { "sv_pushable_fixed_tick_fudge", "15" };
cvar_t sv_busters = { "sv_busters", "0" };

// WantedHL bot/multiplayer CVars — maps and configs reference these;
// registering them here silences "Cvar_Set: variable not found" spam.
cvar_t sv_botskill     = { "sv_botskill",     "1",   FCVAR_SERVER };
cvar_t sv_adjbotskill  = { "sv_adjbotskill",  "0",   FCVAR_SERVER };
cvar_t sv_sendvelocity = { "sv_sendvelocity", "1",   FCVAR_SERVER };

// Register your console variables here
// This gets called one time when the game is initialied
void GameDLLInit( void )
{
        g_psv_gravity = CVAR_GET_POINTER( "sv_gravity" );
        g_psv_aim = CVAR_GET_POINTER( "sv_aim" );
        g_psv_allow_autoaim = CVAR_GET_POINTER( "sv_allow_autoaim" );
        g_footsteps = CVAR_GET_POINTER( "mp_footsteps" );

        g_psv_developer = CVAR_GET_POINTER( "developer" );

        g_enable_cheats = CVAR_GET_POINTER( "sv_cheats" );

        CVAR_REGISTER( &build_commit );
        CVAR_REGISTER( &build_branch );

        CVAR_REGISTER( &displaysoundlist );
        CVAR_REGISTER( &allow_spectators );

        CVAR_REGISTER( &teamplay );
        CVAR_REGISTER( &fraglimit );
        CVAR_REGISTER( &timelimit );

        CVAR_REGISTER( &fragsleft );
        CVAR_REGISTER( &timeleft );

        CVAR_REGISTER( &friendlyfire );
        CVAR_REGISTER( &falldamage );
        CVAR_REGISTER( &weaponstay );
        CVAR_REGISTER( &selfgauss );
        CVAR_REGISTER( &chargerfix );
        CVAR_REGISTER( &satchelfix );
        CVAR_REGISTER( &explosionfix );
        CVAR_REGISTER( &monsteryawspeedfix );
        CVAR_REGISTER( &corpsephysics );
        CVAR_REGISTER( &pushablemode );
        CVAR_REGISTER( &forcerespawn );
        CVAR_REGISTER( &flashlight );
        CVAR_REGISTER( &aimcrosshair );
        CVAR_REGISTER( &decalfrequency );
        CVAR_REGISTER( &teamlist );
        CVAR_REGISTER( &teamoverride );
        CVAR_REGISTER( &defaultteam );
        CVAR_REGISTER( &allowmonsters );
        CVAR_REGISTER( &bhopcap );
        CVAR_REGISTER( &multibyte_only );

        CVAR_REGISTER( &mp_chattime );
        CVAR_REGISTER( &sv_busters );

        // WantedHL-specific CVars
        CVAR_REGISTER( &sv_botskill );
        CVAR_REGISTER( &sv_adjbotskill );
        CVAR_REGISTER( &sv_sendvelocity );


// REGISTER CVARS FOR SKILL LEVEL STUFF
        // Agrunt
        CVAR_REGISTER( &sk_agrunt_health1 );// {"sk_agrunt_health1","0"};
        CVAR_REGISTER( &sk_agrunt_health2 );// {"sk_agrunt_health2","0"};
        CVAR_REGISTER( &sk_agrunt_health3 );// {"sk_agrunt_health3","0"};

        CVAR_REGISTER( &sk_agrunt_dmg_punch1 );// {"sk_agrunt_dmg_punch1","0"};
        CVAR_REGISTER( &sk_agrunt_dmg_punch2 );// {"sk_agrunt_dmg_punch2","0"};
        CVAR_REGISTER( &sk_agrunt_dmg_punch3 );// {"sk_agrunt_dmg_punch3","0"};

        // Apache
        CVAR_REGISTER( &sk_apache_health1 );// {"sk_apache_health1","0"};
        CVAR_REGISTER( &sk_apache_health2 );// {"sk_apache_health2","0"};
        CVAR_REGISTER( &sk_apache_health3 );// {"sk_apache_health3","0"};

        // Barney
        CVAR_REGISTER( &sk_barney_health1 );// {"sk_barney_health1","0"};
        CVAR_REGISTER( &sk_barney_health2 );// {"sk_barney_health2","0"};
        CVAR_REGISTER( &sk_barney_health3 );// {"sk_barney_health3","0"};

        // Bullsquid
        CVAR_REGISTER( &sk_bullsquid_health1 );// {"sk_bullsquid_health1","0"};
        CVAR_REGISTER( &sk_bullsquid_health2 );// {"sk_bullsquid_health2","0"};
        CVAR_REGISTER( &sk_bullsquid_health3 );// {"sk_bullsquid_health3","0"};

        CVAR_REGISTER( &sk_bullsquid_dmg_bite1 );// {"sk_bullsquid_dmg_bite1","0"};
        CVAR_REGISTER( &sk_bullsquid_dmg_bite2 );// {"sk_bullsquid_dmg_bite2","0"};
        CVAR_REGISTER( &sk_bullsquid_dmg_bite3 );// {"sk_bullsquid_dmg_bite3","0"};

        CVAR_REGISTER( &sk_bullsquid_dmg_whip1 );// {"sk_bullsquid_dmg_whip1","0"};
        CVAR_REGISTER( &sk_bullsquid_dmg_whip2 );// {"sk_bullsquid_dmg_whip2","0"};
        CVAR_REGISTER( &sk_bullsquid_dmg_whip3 );// {"sk_bullsquid_dmg_whip3","0"};

        CVAR_REGISTER( &sk_bullsquid_dmg_spit1 );// {"sk_bullsquid_dmg_spit1","0"};
        CVAR_REGISTER( &sk_bullsquid_dmg_spit2 );// {"sk_bullsquid_dmg_spit2","0"};
        CVAR_REGISTER( &sk_bullsquid_dmg_spit3 );// {"sk_bullsquid_dmg_spit3","0"};

        CVAR_REGISTER( &sk_bigmomma_health_factor1 );// {"sk_bigmomma_health_factor1","1.0"};
        CVAR_REGISTER( &sk_bigmomma_health_factor2 );// {"sk_bigmomma_health_factor2","1.0"};
        CVAR_REGISTER( &sk_bigmomma_health_factor3 );// {"sk_bigmomma_health_factor3","1.0"};

        CVAR_REGISTER( &sk_bigmomma_dmg_slash1 );// {"sk_bigmomma_dmg_slash1","50"};
        CVAR_REGISTER( &sk_bigmomma_dmg_slash2 );// {"sk_bigmomma_dmg_slash2","50"};
        CVAR_REGISTER( &sk_bigmomma_dmg_slash3 );// {"sk_bigmomma_dmg_slash3","50"};

        CVAR_REGISTER( &sk_bigmomma_dmg_blast1 );// {"sk_bigmomma_dmg_blast1","100"};
        CVAR_REGISTER( &sk_bigmomma_dmg_blast2 );// {"sk_bigmomma_dmg_blast2","100"};
        CVAR_REGISTER( &sk_bigmomma_dmg_blast3 );// {"sk_bigmomma_dmg_blast3","100"};

        CVAR_REGISTER( &sk_bigmomma_radius_blast1 );// {"sk_bigmomma_radius_blast1","250"};
        CVAR_REGISTER( &sk_bigmomma_radius_blast2 );// {"sk_bigmomma_radius_blast2","250"};
        CVAR_REGISTER( &sk_bigmomma_radius_blast3 );// {"sk_bigmomma_radius_blast3","250"};

        // Gargantua
        CVAR_REGISTER( &sk_gargantua_health1 );// {"sk_gargantua_health1","0"};
        CVAR_REGISTER( &sk_gargantua_health2 );// {"sk_gargantua_health2","0"};
        CVAR_REGISTER( &sk_gargantua_health3 );// {"sk_gargantua_health3","0"};

        CVAR_REGISTER( &sk_gargantua_dmg_slash1 );// {"sk_gargantua_dmg_slash1","0"};
        CVAR_REGISTER( &sk_gargantua_dmg_slash2 );// {"sk_gargantua_dmg_slash2","0"};
        CVAR_REGISTER( &sk_gargantua_dmg_slash3 );// {"sk_gargantua_dmg_slash3","0"};

        CVAR_REGISTER( &sk_gargantua_dmg_fire1 );// {"sk_gargantua_dmg_fire1","0"};
        CVAR_REGISTER( &sk_gargantua_dmg_fire2 );// {"sk_gargantua_dmg_fire2","0"};
        CVAR_REGISTER( &sk_gargantua_dmg_fire3 );// {"sk_gargantua_dmg_fire3","0"};

        CVAR_REGISTER( &sk_gargantua_dmg_stomp1 );// {"sk_gargantua_dmg_stomp1","0"};
        CVAR_REGISTER( &sk_gargantua_dmg_stomp2 );// {"sk_gargantua_dmg_stomp2","0"};
        CVAR_REGISTER( &sk_gargantua_dmg_stomp3 );// {"sk_gargantua_dmg_stomp3","0"};

        // Hassassin
        CVAR_REGISTER( &sk_hassassin_health1 );// {"sk_hassassin_health1","0"};
        CVAR_REGISTER( &sk_hassassin_health2 );// {"sk_hassassin_health2","0"};
        CVAR_REGISTER( &sk_hassassin_health3 );// {"sk_hassassin_health3","0"};

        // Headcrab
        CVAR_REGISTER( &sk_headcrab_health1 );// {"sk_headcrab_health1","0"};
        CVAR_REGISTER( &sk_headcrab_health2 );// {"sk_headcrab_health2","0"};
        CVAR_REGISTER( &sk_headcrab_health3 );// {"sk_headcrab_health3","0"};

        CVAR_REGISTER( &sk_headcrab_dmg_bite1 );// {"sk_headcrab_dmg_bite1","0"};
        CVAR_REGISTER( &sk_headcrab_dmg_bite2 );// {"sk_headcrab_dmg_bite2","0"};
        CVAR_REGISTER( &sk_headcrab_dmg_bite3 );// {"sk_headcrab_dmg_bite3","0"};

        // Hgrunt
        CVAR_REGISTER( &sk_hgrunt_health1 );// {"sk_hgrunt_health1","0"};
        CVAR_REGISTER( &sk_hgrunt_health2 );// {"sk_hgrunt_health2","0"};
        CVAR_REGISTER( &sk_hgrunt_health3 );// {"sk_hgrunt_health3","0"};

        CVAR_REGISTER( &sk_hgrunt_kick1 );// {"sk_hgrunt_kick1","0"};
        CVAR_REGISTER( &sk_hgrunt_kick2 );// {"sk_hgrunt_kick2","0"};
        CVAR_REGISTER( &sk_hgrunt_kick3 );// {"sk_hgrunt_kick3","0"};

        CVAR_REGISTER( &sk_hgrunt_pellets1 );
        CVAR_REGISTER( &sk_hgrunt_pellets2 );
        CVAR_REGISTER( &sk_hgrunt_pellets3 );

        CVAR_REGISTER( &sk_hgrunt_gspeed1 );
        CVAR_REGISTER( &sk_hgrunt_gspeed2 );
        CVAR_REGISTER( &sk_hgrunt_gspeed3 );

        // Houndeye
        CVAR_REGISTER( &sk_houndeye_health1 );// {"sk_houndeye_health1","0"};
        CVAR_REGISTER( &sk_houndeye_health2 );// {"sk_houndeye_health2","0"};
        CVAR_REGISTER( &sk_houndeye_health3 );// {"sk_houndeye_health3","0"};

        CVAR_REGISTER( &sk_houndeye_dmg_blast1 );// {"sk_houndeye_dmg_blast1","0"};
        CVAR_REGISTER( &sk_houndeye_dmg_blast2 );// {"sk_houndeye_dmg_blast2","0"};
        CVAR_REGISTER( &sk_houndeye_dmg_blast3 );// {"sk_houndeye_dmg_blast3","0"};

        // ISlave
        CVAR_REGISTER( &sk_islave_health1 );// {"sk_islave_health1","0"};
        CVAR_REGISTER( &sk_islave_health2 );// {"sk_islave_health2","0"};
        CVAR_REGISTER( &sk_islave_health3 );// {"sk_islave_health3","0"};

        CVAR_REGISTER( &sk_islave_dmg_claw1 );// {"sk_islave_dmg_claw1","0"};
        CVAR_REGISTER( &sk_islave_dmg_claw2 );// {"sk_islave_dmg_claw2","0"};
        CVAR_REGISTER( &sk_islave_dmg_claw3 );// {"sk_islave_dmg_claw3","0"};

        CVAR_REGISTER( &sk_islave_dmg_clawrake1 );// {"sk_islave_dmg_clawrake1","0"};
        CVAR_REGISTER( &sk_islave_dmg_clawrake2 );// {"sk_islave_dmg_clawrake2","0"};
        CVAR_REGISTER( &sk_islave_dmg_clawrake3 );// {"sk_islave_dmg_clawrake3","0"};

        CVAR_REGISTER( &sk_islave_dmg_zap1 );// {"sk_islave_dmg_zap1","0"};
        CVAR_REGISTER( &sk_islave_dmg_zap2 );// {"sk_islave_dmg_zap2","0"};
        CVAR_REGISTER( &sk_islave_dmg_zap3 );// {"sk_islave_dmg_zap3","0"};

        // Icthyosaur
        CVAR_REGISTER( &sk_ichthyosaur_health1 );// {"sk_ichthyosaur_health1","0"};
        CVAR_REGISTER( &sk_ichthyosaur_health2 );// {"sk_ichthyosaur_health2","0"};
        CVAR_REGISTER( &sk_ichthyosaur_health3 );// {"sk_ichthyosaur_health3","0"};

        CVAR_REGISTER( &sk_ichthyosaur_shake1 );// {"sk_ichthyosaur_health3","0"};
        CVAR_REGISTER( &sk_ichthyosaur_shake2 );// {"sk_ichthyosaur_health3","0"};
        CVAR_REGISTER( &sk_ichthyosaur_shake3 );// {"sk_ichthyosaur_health3","0"};

        // Leech
        CVAR_REGISTER( &sk_leech_health1 );// {"sk_leech_health1","0"};
        CVAR_REGISTER( &sk_leech_health2 );// {"sk_leech_health2","0"};
        CVAR_REGISTER( &sk_leech_health3 );// {"sk_leech_health3","0"};

        CVAR_REGISTER( &sk_leech_dmg_bite1 );// {"sk_leech_dmg_bite1","0"};
        CVAR_REGISTER( &sk_leech_dmg_bite2 );// {"sk_leech_dmg_bite2","0"};
        CVAR_REGISTER( &sk_leech_dmg_bite3 );// {"sk_leech_dmg_bite3","0"};

        // Controller
        CVAR_REGISTER( &sk_controller_health1 );
        CVAR_REGISTER( &sk_controller_health2 );
        CVAR_REGISTER( &sk_controller_health3 );

        CVAR_REGISTER( &sk_controller_dmgzap1 );
        CVAR_REGISTER( &sk_controller_dmgzap2 );
        CVAR_REGISTER( &sk_controller_dmgzap3 );

        CVAR_REGISTER( &sk_controller_speedball1 );
        CVAR_REGISTER( &sk_controller_speedball2 );
        CVAR_REGISTER( &sk_controller_speedball3 );

        CVAR_REGISTER( &sk_controller_dmgball1 );
        CVAR_REGISTER( &sk_controller_dmgball2 );
        CVAR_REGISTER( &sk_controller_dmgball3 );

        // Nihilanth
        CVAR_REGISTER( &sk_nihilanth_health1 );// {"sk_nihilanth_health1","0"};
        CVAR_REGISTER( &sk_nihilanth_health2 );// {"sk_nihilanth_health2","0"};
        CVAR_REGISTER( &sk_nihilanth_health3 );// {"sk_nihilanth_health3","0"};

        CVAR_REGISTER( &sk_nihilanth_zap1 );
        CVAR_REGISTER( &sk_nihilanth_zap2 );
        CVAR_REGISTER( &sk_nihilanth_zap3 );

        // Scientist
        CVAR_REGISTER( &sk_scientist_health1 );// {"sk_scientist_health1","0"};
        CVAR_REGISTER( &sk_scientist_health2 );// {"sk_scientist_health2","0"};
        CVAR_REGISTER( &sk_scientist_health3 );// {"sk_scientist_health3","0"};

        // Snark
        CVAR_REGISTER( &sk_snark_health1 );// {"sk_snark_health1","0"};
        CVAR_REGISTER( &sk_snark_health2 );// {"sk_snark_health2","0"};
        CVAR_REGISTER( &sk_snark_health3 );// {"sk_snark_health3","0"};

        CVAR_REGISTER( &sk_snark_dmg_bite1 );// {"sk_snark_dmg_bite1","0"};
        CVAR_REGISTER( &sk_snark_dmg_bite2 );// {"sk_snark_dmg_bite2","0"};
        CVAR_REGISTER( &sk_snark_dmg_bite3 );// {"sk_snark_dmg_bite3","0"};

        CVAR_REGISTER( &sk_snark_dmg_pop1 );// {"sk_snark_dmg_pop1","0"};
        CVAR_REGISTER( &sk_snark_dmg_pop2 );// {"sk_snark_dmg_pop2","0"};
        CVAR_REGISTER( &sk_snark_dmg_pop3 );// {"sk_snark_dmg_pop3","0"};

        CVAR_REGISTER( &sk_scorpion_health1 );
        CVAR_REGISTER( &sk_scorpion_health2 );
        CVAR_REGISTER( &sk_scorpion_health3 );

        CVAR_REGISTER( &sk_scorpion_dmg_sting1 );
        CVAR_REGISTER( &sk_scorpion_dmg_sting2 );
        CVAR_REGISTER( &sk_scorpion_dmg_sting3 );

        // Zombie
        CVAR_REGISTER( &sk_zombie_health1 );// {"sk_zombie_health1","0"};
        CVAR_REGISTER( &sk_zombie_health2 );// {"sk_zombie_health3","0"};
        CVAR_REGISTER( &sk_zombie_health3 );// {"sk_zombie_health3","0"};

        CVAR_REGISTER( &sk_zombie_dmg_one_slash1 );// {"sk_zombie_dmg_one_slash1","0"};
        CVAR_REGISTER( &sk_zombie_dmg_one_slash2 );// {"sk_zombie_dmg_one_slash2","0"};
        CVAR_REGISTER( &sk_zombie_dmg_one_slash3 );// {"sk_zombie_dmg_one_slash3","0"};

        CVAR_REGISTER( &sk_zombie_dmg_both_slash1 );// {"sk_zombie_dmg_both_slash1","0"};
        CVAR_REGISTER( &sk_zombie_dmg_both_slash2 );// {"sk_zombie_dmg_both_slash2","0"};
        CVAR_REGISTER( &sk_zombie_dmg_both_slash3 );// {"sk_zombie_dmg_both_slash3","0"};

        //Turret
        CVAR_REGISTER( &sk_turret_health1 );// {"sk_turret_health1","0"};
        CVAR_REGISTER( &sk_turret_health2 );// {"sk_turret_health2","0"};
        CVAR_REGISTER( &sk_turret_health3 );// {"sk_turret_health3","0"};

        // MiniTurret
        CVAR_REGISTER( &sk_miniturret_health1 );// {"sk_miniturret_health1","0"};
        CVAR_REGISTER( &sk_miniturret_health2 );// {"sk_miniturret_health2","0"};
        CVAR_REGISTER( &sk_miniturret_health3 );// {"sk_miniturret_health3","0"};

        // Sentry Turret
        CVAR_REGISTER( &sk_sentry_health1 );// {"sk_sentry_health1","0"};
        CVAR_REGISTER( &sk_sentry_health2 );// {"sk_sentry_health2","0"};
        CVAR_REGISTER( &sk_sentry_health3 );// {"sk_sentry_health3","0"};


        // PLAYER WEAPONS

        // Crowbar whack
        CVAR_REGISTER( &sk_plr_crowbar1 );// {"sk_plr_crowbar1","0"};
        CVAR_REGISTER( &sk_plr_crowbar2 );// {"sk_plr_crowbar2","0"};
        CVAR_REGISTER( &sk_plr_crowbar3 );// {"sk_plr_crowbar3","0"};

        // Glock Round
        CVAR_REGISTER( &sk_plr_9mm_bullet1 );// {"sk_plr_9mm_bullet1","0"};
        CVAR_REGISTER( &sk_plr_9mm_bullet2 );// {"sk_plr_9mm_bullet2","0"};
        CVAR_REGISTER( &sk_plr_9mm_bullet3 );// {"sk_plr_9mm_bullet3","0"};

        // 357 Round
        CVAR_REGISTER( &sk_plr_357_bullet1 );// {"sk_plr_357_bullet1","0"};
        CVAR_REGISTER( &sk_plr_357_bullet2 );// {"sk_plr_357_bullet2","0"};
        CVAR_REGISTER( &sk_plr_357_bullet3 );// {"sk_plr_357_bullet3","0"};

        // MP5 Round
        CVAR_REGISTER( &sk_plr_9mmAR_bullet1 );// {"sk_plr_9mmAR_bullet1","0"};
        CVAR_REGISTER( &sk_plr_9mmAR_bullet2 );// {"sk_plr_9mmAR_bullet2","0"};
        CVAR_REGISTER( &sk_plr_9mmAR_bullet3 );// {"sk_plr_9mmAR_bullet3","0"};

        // M203 grenade
        CVAR_REGISTER( &sk_plr_9mmAR_grenade1 );// {"sk_plr_9mmAR_grenade1","0"};
        CVAR_REGISTER( &sk_plr_9mmAR_grenade2 );// {"sk_plr_9mmAR_grenade2","0"};
        CVAR_REGISTER( &sk_plr_9mmAR_grenade3 );// {"sk_plr_9mmAR_grenade3","0"};

        // Shotgun buckshot
        CVAR_REGISTER( &sk_plr_buckshot1 );// {"sk_plr_buckshot1","0"};
        CVAR_REGISTER( &sk_plr_buckshot2 );// {"sk_plr_buckshot2","0"};
        CVAR_REGISTER( &sk_plr_buckshot3 );// {"sk_plr_buckshot3","0"};

        // Crossbow
        CVAR_REGISTER( &sk_plr_xbow_bolt_monster1 );// {"sk_plr_xbow_bolt1","0"};
        CVAR_REGISTER( &sk_plr_xbow_bolt_monster2 );// {"sk_plr_xbow_bolt2","0"};
        CVAR_REGISTER( &sk_plr_xbow_bolt_monster3 );// {"sk_plr_xbow_bolt3","0"};

        CVAR_REGISTER( &sk_plr_xbow_bolt_client1 );// {"sk_plr_xbow_bolt1","0"};
        CVAR_REGISTER( &sk_plr_xbow_bolt_client2 );// {"sk_plr_xbow_bolt2","0"};
        CVAR_REGISTER( &sk_plr_xbow_bolt_client3 );// {"sk_plr_xbow_bolt3","0"};

        // RPG
        CVAR_REGISTER( &sk_plr_rpg1 );// {"sk_plr_rpg1","0"};
        CVAR_REGISTER( &sk_plr_rpg2 );// {"sk_plr_rpg2","0"};
        CVAR_REGISTER( &sk_plr_rpg3 );// {"sk_plr_rpg3","0"};

        // Gauss Gun
        CVAR_REGISTER( &sk_plr_gauss1 );// {"sk_plr_gauss1","0"};
        CVAR_REGISTER( &sk_plr_gauss2 );// {"sk_plr_gauss2","0"};
        CVAR_REGISTER( &sk_plr_gauss3 );// {"sk_plr_gauss3","0"};

        // Egon Gun
        CVAR_REGISTER( &sk_plr_egon_narrow1 );// {"sk_plr_egon_narrow1","0"};
        CVAR_REGISTER( &sk_plr_egon_narrow2 );// {"sk_plr_egon_narrow2","0"};
        CVAR_REGISTER( &sk_plr_egon_narrow3 );// {"sk_plr_egon_narrow3","0"};

        CVAR_REGISTER( &sk_plr_egon_wide1 );// {"sk_plr_egon_wide1","0"};
        CVAR_REGISTER( &sk_plr_egon_wide2 );// {"sk_plr_egon_wide2","0"};
        CVAR_REGISTER( &sk_plr_egon_wide3 );// {"sk_plr_egon_wide3","0"};

        // Hand Grendade
        CVAR_REGISTER( &sk_plr_hand_grenade1 );// {"sk_plr_hand_grenade1","0"};
        CVAR_REGISTER( &sk_plr_hand_grenade2 );// {"sk_plr_hand_grenade2","0"};
        CVAR_REGISTER( &sk_plr_hand_grenade3 );// {"sk_plr_hand_grenade3","0"};

        // Satchel Charge
        CVAR_REGISTER( &sk_plr_satchel1 );// {"sk_plr_satchel1","0"};
        CVAR_REGISTER( &sk_plr_satchel2 );// {"sk_plr_satchel2","0"};
        CVAR_REGISTER( &sk_plr_satchel3 );// {"sk_plr_satchel3","0"};

        // Tripmine
        CVAR_REGISTER( &sk_plr_tripmine1 );// {"sk_plr_tripmine1","0"};
        CVAR_REGISTER( &sk_plr_tripmine2 );// {"sk_plr_tripmine2","0"};
        CVAR_REGISTER( &sk_plr_tripmine3 );// {"sk_plr_tripmine3","0"};

        // WORLD WEAPONS
        CVAR_REGISTER( &sk_12mm_bullet1 );// {"sk_12mm_bullet1","0"};
        CVAR_REGISTER( &sk_12mm_bullet2 );// {"sk_12mm_bullet2","0"};
        CVAR_REGISTER( &sk_12mm_bullet3 );// {"sk_12mm_bullet3","0"};

        CVAR_REGISTER( &sk_9mmAR_bullet1 );// {"sk_9mm_bullet1","0"};
        CVAR_REGISTER( &sk_9mmAR_bullet2 );// {"sk_9mm_bullet1","0"};
        CVAR_REGISTER( &sk_9mmAR_bullet3 );// {"sk_9mm_bullet1","0"};

        CVAR_REGISTER( &sk_9mm_bullet1 );// {"sk_9mm_bullet1","0"};
        CVAR_REGISTER( &sk_9mm_bullet2 );// {"sk_9mm_bullet2","0"};
        CVAR_REGISTER( &sk_9mm_bullet3 );// {"sk_9mm_bullet3","0"};

        // HORNET
        CVAR_REGISTER( &sk_hornet_dmg1 );// {"sk_hornet_dmg1","0"};
        CVAR_REGISTER( &sk_hornet_dmg2 );// {"sk_hornet_dmg2","0"};
        CVAR_REGISTER( &sk_hornet_dmg3 );// {"sk_hornet_dmg3","0"};

        // HEALTH/SUIT CHARGE DISTRIBUTION
        CVAR_REGISTER( &sk_suitcharger1 );
        CVAR_REGISTER( &sk_suitcharger2 );
        CVAR_REGISTER( &sk_suitcharger3 );

        CVAR_REGISTER( &sk_battery1 );
        CVAR_REGISTER( &sk_battery2 );
        CVAR_REGISTER( &sk_battery3 );

        CVAR_REGISTER( &sk_healthcharger1 );
        CVAR_REGISTER( &sk_healthcharger2 );
        CVAR_REGISTER( &sk_healthcharger3 );

        CVAR_REGISTER( &sk_healthkit1 );
        CVAR_REGISTER( &sk_healthkit2 );
        CVAR_REGISTER( &sk_healthkit3 );

        CVAR_REGISTER( &sk_scientist_heal1 );
        CVAR_REGISTER( &sk_scientist_heal2 );
        CVAR_REGISTER( &sk_scientist_heal3 );

        // ========== WantedHL Skill CVars ==========
        CVAR_REGISTER( &sk_annie_health1 ); CVAR_REGISTER( &sk_annie_health2 ); CVAR_REGISTER( &sk_annie_health3 );
        CVAR_REGISTER( &sk_bear_health1 ); CVAR_REGISTER( &sk_bear_health2 ); CVAR_REGISTER( &sk_bear_health3 );
        CVAR_REGISTER( &sk_bear_dmg_claw1 ); CVAR_REGISTER( &sk_bear_dmg_claw2 ); CVAR_REGISTER( &sk_bear_dmg_claw3 );
        CVAR_REGISTER( &sk_bear_dmg_pounce1 ); CVAR_REGISTER( &sk_bear_dmg_pounce2 ); CVAR_REGISTER( &sk_bear_dmg_pounce3 );
        CVAR_REGISTER( &sk_bigminer_health1 ); CVAR_REGISTER( &sk_bigminer_health2 ); CVAR_REGISTER( &sk_bigminer_health3 );
        CVAR_REGISTER( &sk_bigminer_punch1 ); CVAR_REGISTER( &sk_bigminer_punch2 ); CVAR_REGISTER( &sk_bigminer_punch3 );
        CVAR_REGISTER( &sk_chicken_health1 ); CVAR_REGISTER( &sk_chicken_health2 ); CVAR_REGISTER( &sk_chicken_health3 );
        CVAR_REGISTER( &sk_chicken_dmg_peck1 ); CVAR_REGISTER( &sk_chicken_dmg_peck2 ); CVAR_REGISTER( &sk_chicken_dmg_peck3 );
        CVAR_REGISTER( &sk_colonel_health1 ); CVAR_REGISTER( &sk_colonel_health2 ); CVAR_REGISTER( &sk_colonel_health3 );
        CVAR_REGISTER( &sk_colonel_heal1 ); CVAR_REGISTER( &sk_colonel_heal2 ); CVAR_REGISTER( &sk_colonel_heal3 );
        CVAR_REGISTER( &sk_crispen_health1 ); CVAR_REGISTER( &sk_crispen_health2 ); CVAR_REGISTER( &sk_crispen_health3 );
        CVAR_REGISTER( &sk_crispen_heal1 ); CVAR_REGISTER( &sk_crispen_heal2 ); CVAR_REGISTER( &sk_crispen_heal3 );
        CVAR_REGISTER( &sk_cowboy_health1 ); CVAR_REGISTER( &sk_cowboy_health2 ); CVAR_REGISTER( &sk_cowboy_health3 );
        CVAR_REGISTER( &sk_cowboy_punch1 ); CVAR_REGISTER( &sk_cowboy_punch2 ); CVAR_REGISTER( &sk_cowboy_punch3 );
        CVAR_REGISTER( &sk_cowboy_dynamitespeed1 ); CVAR_REGISTER( &sk_cowboy_dynamitespeed2 ); CVAR_REGISTER( &sk_cowboy_dynamitespeed3 );
        CVAR_REGISTER( &sk_dave_health1 ); CVAR_REGISTER( &sk_dave_health2 ); CVAR_REGISTER( &sk_dave_health3 );
        CVAR_REGISTER( &sk_dave_heal1 ); CVAR_REGISTER( &sk_dave_heal2 ); CVAR_REGISTER( &sk_dave_heal3 );
        CVAR_REGISTER( &sk_horse_health1 ); CVAR_REGISTER( &sk_horse_health2 ); CVAR_REGISTER( &sk_horse_health3 );
        CVAR_REGISTER( &sk_horse_dmg_kick1 ); CVAR_REGISTER( &sk_horse_dmg_kick2 ); CVAR_REGISTER( &sk_horse_dmg_kick3 );
        CVAR_REGISTER( &sk_hoss_health1 ); CVAR_REGISTER( &sk_hoss_health2 ); CVAR_REGISTER( &sk_hoss_health3 );
        CVAR_REGISTER( &sk_kaiewi_health1 ); CVAR_REGISTER( &sk_kaiewi_health2 ); CVAR_REGISTER( &sk_kaiewi_health3 );
        CVAR_REGISTER( &sk_kaiewi_punch1 ); CVAR_REGISTER( &sk_kaiewi_punch2 ); CVAR_REGISTER( &sk_kaiewi_punch3 );
        CVAR_REGISTER( &sk_masala_health1 ); CVAR_REGISTER( &sk_masala_health2 ); CVAR_REGISTER( &sk_masala_health3 );
        CVAR_REGISTER( &sk_masala_heal1 ); CVAR_REGISTER( &sk_masala_heal2 ); CVAR_REGISTER( &sk_masala_heal3 );
        CVAR_REGISTER( &sk_mexbandit_health1 ); CVAR_REGISTER( &sk_mexbandit_health2 ); CVAR_REGISTER( &sk_mexbandit_health3 );
        CVAR_REGISTER( &sk_mexbandit_kick1 ); CVAR_REGISTER( &sk_mexbandit_kick2 ); CVAR_REGISTER( &sk_mexbandit_kick3 );
        CVAR_REGISTER( &sk_mexbandit_dynamitespeed1 ); CVAR_REGISTER( &sk_mexbandit_dynamitespeed2 ); CVAR_REGISTER( &sk_mexbandit_dynamitespeed3 );
        CVAR_REGISTER( &sk_townmex_health1 ); CVAR_REGISTER( &sk_townmex_health2 ); CVAR_REGISTER( &sk_townmex_health3 );
        CVAR_REGISTER( &sk_townmex_heal1 ); CVAR_REGISTER( &sk_townmex_heal2 ); CVAR_REGISTER( &sk_townmex_heal3 );
        CVAR_REGISTER( &sk_nagatow_health1 ); CVAR_REGISTER( &sk_nagatow_health2 ); CVAR_REGISTER( &sk_nagatow_health3 );
        CVAR_REGISTER( &sk_nagatow_heal1 ); CVAR_REGISTER( &sk_nagatow_heal2 ); CVAR_REGISTER( &sk_nagatow_heal3 );
        CVAR_REGISTER( &sk_puma_health1 ); CVAR_REGISTER( &sk_puma_health2 ); CVAR_REGISTER( &sk_puma_health3 );
        CVAR_REGISTER( &sk_puma_dmg_claw1 ); CVAR_REGISTER( &sk_puma_dmg_claw2 ); CVAR_REGISTER( &sk_puma_dmg_claw3 );
        CVAR_REGISTER( &sk_puma_dmg_pounce1 ); CVAR_REGISTER( &sk_puma_dmg_pounce2 ); CVAR_REGISTER( &sk_puma_dmg_pounce3 );
        CVAR_REGISTER( &sk_ramone_health1 ); CVAR_REGISTER( &sk_ramone_health2 ); CVAR_REGISTER( &sk_ramone_health3 );
        CVAR_REGISTER( &sk_ramone_punch1 ); CVAR_REGISTER( &sk_ramone_punch2 ); CVAR_REGISTER( &sk_ramone_punch3 );
        CVAR_REGISTER( &sk_ramone_dynamitespeed1 ); CVAR_REGISTER( &sk_ramone_dynamitespeed2 ); CVAR_REGISTER( &sk_ramone_dynamitespeed3 );
        CVAR_REGISTER( &sk_smallminer_health1 ); CVAR_REGISTER( &sk_smallminer_health2 ); CVAR_REGISTER( &sk_smallminer_health3 );
        CVAR_REGISTER( &sk_smallminer_pick1 ); CVAR_REGISTER( &sk_smallminer_pick2 ); CVAR_REGISTER( &sk_smallminer_pick3 );
        CVAR_REGISTER( &sk_snake_health1 ); CVAR_REGISTER( &sk_snake_health2 ); CVAR_REGISTER( &sk_snake_health3 );
        CVAR_REGISTER( &sk_snake_dmg_bite1 ); CVAR_REGISTER( &sk_snake_dmg_bite2 ); CVAR_REGISTER( &sk_snake_dmg_bite3 );
        CVAR_REGISTER( &sk_wtowna_health1 ); CVAR_REGISTER( &sk_wtowna_health2 ); CVAR_REGISTER( &sk_wtowna_health3 );
        CVAR_REGISTER( &sk_wtowna_heal1 ); CVAR_REGISTER( &sk_wtowna_heal2 ); CVAR_REGISTER( &sk_wtowna_heal3 );
        CVAR_REGISTER( &sk_wtownb_health1 ); CVAR_REGISTER( &sk_wtownb_health2 ); CVAR_REGISTER( &sk_wtownb_health3 );
        CVAR_REGISTER( &sk_wtownb_heal1 ); CVAR_REGISTER( &sk_wtownb_heal2 ); CVAR_REGISTER( &sk_wtownb_heal3 );
        CVAR_REGISTER( &sk_pistol_bullet1 ); CVAR_REGISTER( &sk_pistol_bullet2 ); CVAR_REGISTER( &sk_pistol_bullet3 );
        CVAR_REGISTER( &sk_shotgun_bullet1 ); CVAR_REGISTER( &sk_shotgun_bullet2 ); CVAR_REGISTER( &sk_shotgun_bullet3 );
        CVAR_REGISTER( &sk_gattlinggun_bullet1 ); CVAR_REGISTER( &sk_gattlinggun_bullet2 ); CVAR_REGISTER( &sk_gattlinggun_bullet3 );
        CVAR_REGISTER( &sk_winchester_bullet1 ); CVAR_REGISTER( &sk_winchester_bullet2 ); CVAR_REGISTER( &sk_winchester_bullet3 );
        CVAR_REGISTER( &sk_bow_arrow1 ); CVAR_REGISTER( &sk_bow_arrow2 ); CVAR_REGISTER( &sk_bow_arrow3 );
        CVAR_REGISTER( &sk_herbs1 ); CVAR_REGISTER( &sk_herbs2 ); CVAR_REGISTER( &sk_herbs3 );
        CVAR_REGISTER( &sk_elixer1 ); CVAR_REGISTER( &sk_elixer2 ); CVAR_REGISTER( &sk_elixer3 );
        CVAR_REGISTER( &sk_plr_knife1 ); CVAR_REGISTER( &sk_plr_knife2 ); CVAR_REGISTER( &sk_plr_knife3 );
        CVAR_REGISTER( &sk_plr_pick1 ); CVAR_REGISTER( &sk_plr_pick2 ); CVAR_REGISTER( &sk_plr_pick3 );
        CVAR_REGISTER( &sk_plr_pistol_bullet1 ); CVAR_REGISTER( &sk_plr_pistol_bullet2 ); CVAR_REGISTER( &sk_plr_pistol_bullet3 );
        CVAR_REGISTER( &sk_plr_colts_bullet1 ); CVAR_REGISTER( &sk_plr_colts_bullet2 ); CVAR_REGISTER( &sk_plr_colts_bullet3 );
        CVAR_REGISTER( &sk_plr_winchester_bullet1 ); CVAR_REGISTER( &sk_plr_winchester_bullet2 ); CVAR_REGISTER( &sk_plr_winchester_bullet3 );
        CVAR_REGISTER( &sk_plr_shotgun1 ); CVAR_REGISTER( &sk_plr_shotgun2 ); CVAR_REGISTER( &sk_plr_shotgun3 );
        CVAR_REGISTER( &sk_plr_buffalo1 ); CVAR_REGISTER( &sk_plr_buffalo2 ); CVAR_REGISTER( &sk_plr_buffalo3 );
        CVAR_REGISTER( &sk_plr_bow_arrow1 ); CVAR_REGISTER( &sk_plr_bow_arrow2 ); CVAR_REGISTER( &sk_plr_bow_arrow3 );
        CVAR_REGISTER( &sk_plr_gattlinggun_bullet1 ); CVAR_REGISTER( &sk_plr_gattlinggun_bullet2 ); CVAR_REGISTER( &sk_plr_gattlinggun_bullet3 );
        CVAR_REGISTER( &sk_plr_cannon_ball1 ); CVAR_REGISTER( &sk_plr_cannon_ball2 ); CVAR_REGISTER( &sk_plr_cannon_ball3 );
        CVAR_REGISTER( &sk_plr_dynamite1 ); CVAR_REGISTER( &sk_plr_dynamite2 ); CVAR_REGISTER( &sk_plr_dynamite3 );
        CVAR_REGISTER( &sk_plr_beartrap1 ); CVAR_REGISTER( &sk_plr_beartrap2 ); CVAR_REGISTER( &sk_plr_beartrap3 );
        // ========== End WantedHL Skill CVars ==========

        // monster damage adjusters
        CVAR_REGISTER( &sk_monster_head1 );
        CVAR_REGISTER( &sk_monster_head2 );
        CVAR_REGISTER( &sk_monster_head3 );

        CVAR_REGISTER( &sk_monster_chest1 );
        CVAR_REGISTER( &sk_monster_chest2 );
        CVAR_REGISTER( &sk_monster_chest3 );

        CVAR_REGISTER( &sk_monster_stomach1 );
        CVAR_REGISTER( &sk_monster_stomach2 );
        CVAR_REGISTER( &sk_monster_stomach3 );

        CVAR_REGISTER( &sk_monster_arm1 );
        CVAR_REGISTER( &sk_monster_arm2 );
        CVAR_REGISTER( &sk_monster_arm3 );

        CVAR_REGISTER( &sk_monster_leg1 );
        CVAR_REGISTER( &sk_monster_leg2 );
        CVAR_REGISTER( &sk_monster_leg3 );

        // player damage adjusters
        CVAR_REGISTER( &sk_player_head1 );
        CVAR_REGISTER( &sk_player_head2 );
        CVAR_REGISTER( &sk_player_head3 );

        CVAR_REGISTER( &sk_player_chest1 );
        CVAR_REGISTER( &sk_player_chest2 );
        CVAR_REGISTER( &sk_player_chest3 );

        CVAR_REGISTER( &sk_player_stomach1 );
        CVAR_REGISTER( &sk_player_stomach2 );
        CVAR_REGISTER( &sk_player_stomach3 );

        CVAR_REGISTER( &sk_player_arm1 );
        CVAR_REGISTER( &sk_player_arm2 );
        CVAR_REGISTER( &sk_player_arm3 );

        CVAR_REGISTER( &sk_player_leg1 );
        CVAR_REGISTER( &sk_player_leg2 );
        CVAR_REGISTER( &sk_player_leg3 );
// END REGISTER CVARS FOR SKILL LEVEL STUFF

        CVAR_REGISTER( &sv_pushable_fixed_tick_fudge );

        SERVER_COMMAND( "exec skill.cfg\n" );
}

