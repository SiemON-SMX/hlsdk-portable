/***
*
* WantedHL - monster_mexbandit
* Mexican Bandit enemy. Fires gattling gun and throws dynamite.
*
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "talkmonster.h"
#include "schedule.h"
#include "defaultai.h"
#include "scripted.h"
#include "animation.h"
#include "soundent.h"
#include "weapons.h"
#include "barney.h"
#include "skill.h"

class CMexBandit : public CBarney
{
public:
        void Spawn( void );
        void Precache( void );
        void BarneyFirePistol( void );
        void AlertSound( void );
        void PainSound( void );
        void DeathSound( void );
        void TalkInit( void );
        int  Classify( void );
};




LINK_ENTITY_TO_CLASS( monster_mexbandit, CMexBandit );



int CMexBandit::Classify( void ) { return CLASS_HUMAN_MILITARY; }

void CMexBandit::TalkInit( void )
{
        CTalkMonster::TalkInit();
        m_szGrp[TLK_ANSWER]  = "!MX_ANSWER";
        m_szGrp[TLK_QUESTION] = "!MX_QUESTION";
        m_szGrp[TLK_IDLE]    = "!MX_IDLE";
        m_szGrp[TLK_STARE]   = "!MX_STARE";
        m_szGrp[TLK_USE]     = "!MX_OK";
        m_szGrp[TLK_UNUSE]   = "!MX_WAIT";
        m_szGrp[TLK_STOP]    = "!MX_STOP";
        m_szGrp[TLK_NOSHOOT] = "!MX_SCARED";
        m_szGrp[TLK_HELLO]   = "!MX_HELLO";
        m_szGrp[TLK_PLPUSH]  = "!MX_PUSH";
        m_szGrp[TLK_PLREJECT]= "!MX_REJECT";
        m_szGrp[TLK_SMELL]   = NULL;
        m_szGrp[TLK_WOUND]   = "!MX_WOUND";
        m_szGrp[TLK_MORTAL]  = "!MX_MORTAL";
}

void CMexBandit::AlertSound( void )
{
        EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "mexbandit/mb_alert1.wav", 1.0, ATTN_NORM, 0, 100 );
}
void CMexBandit::PainSound( void )
{
        EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "mexbandit/mb_pain1.wav", 1.0, ATTN_NORM, 0, 100 );
}
void CMexBandit::DeathSound( void )
{
        EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "mexbandit/mb_die1.wav", 1.0, ATTN_NORM, 0, 100 );
}

void CMexBandit::BarneyFirePistol( void )
{
        Vector vecShootOrigin = GetGunPosition();
        Vector vecShootDir    = ShootAtEnemy( vecShootOrigin );

        FireBullets( 1, vecShootOrigin, vecShootDir, VECTOR_CONE_5DEGREES,
                1024, BULLET_MONSTER_9MM, 0, gSkillData.monDmgGattlingGunBullet );

        EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "mexbandit/mb_shoot1.wav",
                RANDOM_FLOAT(0.8f,1.0f), ATTN_NORM, 0, RANDOM_LONG(95,110) );

        CSoundEnt::InsertSound( bits_SOUND_COMBAT, pev->origin, 512, 0.3f );
}

void CMexBandit::Precache( void )
{
        PRECACHE_MODEL( "models/bandit_mex.mdl" );
        PRECACHE_SOUND( "mexbandit/mb_alert1.wav" );
        PRECACHE_SOUND( "mexbandit/mb_pain1.wav" );
        PRECACHE_SOUND( "mexbandit/mb_die1.wav" );
        PRECACHE_SOUND( "mexbandit/mb_shoot1.wav" );
        TalkInit();
        CTalkMonster::Precache();
}

void CMexBandit::Spawn( void )
{
        Precache();
        SET_MODEL( ENT(pev), "models/bandit_mex.mdl" );
        UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );
        pev->solid      = SOLID_SLIDEBOX;
        pev->movetype   = MOVETYPE_STEP;
        m_bloodColor    = BLOOD_COLOR_RED;
        pev->health     = gSkillData.mexbanditHealth;
        pev->view_ofs   = Vector(0,0,50);
        m_flFieldOfView = 0.2f;
        m_MonsterState  = MONSTERSTATE_NONE;
        m_afCapability  = bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP
                        | bits_CAP_RANGE_ATTACK1 | bits_CAP_MELEE_ATTACK1;
        pev->body       = 1; // BARNEY_BODY_GUNDRAWN — gun visible in hand

        TalkInit();
        MonsterInit();
}
