/***
*
* WantedHL - monster_ramone
* Boss enemy. Heavy gattling fire, dynamite, massive health.
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

class CRamone : public CBarney
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




LINK_ENTITY_TO_CLASS( monster_ramone, CRamone );



int CRamone::Classify( void ) { return CLASS_HUMAN_MILITARY; }

void CRamone::TalkInit( void )
{
        CTalkMonster::TalkInit();
        m_szGrp[TLK_ANSWER]  = "!RM_ANSWER";
        m_szGrp[TLK_QUESTION] = "!RM_QUESTION";
        m_szGrp[TLK_IDLE]    = "!RM_IDLE";
        m_szGrp[TLK_STARE]   = "!RM_STARE";
        m_szGrp[TLK_USE]     = "!RM_OK";
        m_szGrp[TLK_UNUSE]   = "!RM_WAIT";
        m_szGrp[TLK_STOP]    = "!RM_STOP";
        m_szGrp[TLK_NOSHOOT] = "!RM_SCARED";
        m_szGrp[TLK_HELLO]   = "!RM_HELLO";
        m_szGrp[TLK_PLPUSH]  = "!RM_PUSH";
        m_szGrp[TLK_PLREJECT]= "!RM_REJECT";
        m_szGrp[TLK_SMELL]   = NULL;
        m_szGrp[TLK_WOUND]   = "!RM_WOUND";
        m_szGrp[TLK_MORTAL]  = "!RM_MORTAL";
}

void CRamone::AlertSound( void )
{
        EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "ramone/rm_alert1.wav", 1.0, ATTN_NORM, 0, 100 );
}
void CRamone::PainSound( void )
{
        EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "ramone/rm_pain1.wav", 1.0, ATTN_NORM, 0, 100 );
}
void CRamone::DeathSound( void )
{
        EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "ramone/rm_die1.wav", 1.0, ATTN_NORM, 0, 100 );
}

void CRamone::BarneyFirePistol( void )
{
        Vector vecShootOrigin = GetGunPosition();
        Vector vecShootDir    = ShootAtEnemy( vecShootOrigin );

        // Burst fire - boss fires 3 rounds
        for( int i = 0; i < 3; i++ )
        {
                FireBullets( 1, vecShootOrigin, vecShootDir, VECTOR_CONE_5DEGREES,
                        1024, BULLET_MONSTER_9MM, 0, gSkillData.monDmgGattlingGunBullet );
        }

        EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "ramone/rm_shoot1.wav",
                1.0, ATTN_NORM, 0, 100 );

        CSoundEnt::InsertSound( bits_SOUND_COMBAT, pev->origin, 768, 0.3f );
}

void CRamone::Precache( void )
{
        PRECACHE_MODEL( "models/ramone.mdl" );
        PRECACHE_SOUND( "ramone/rm_alert1.wav" );
        PRECACHE_SOUND( "ramone/rm_pain1.wav" );
        PRECACHE_SOUND( "ramone/rm_die1.wav" );
        PRECACHE_SOUND( "ramone/rm_shoot1.wav" );
        TalkInit();
        CTalkMonster::Precache();
}

void CRamone::Spawn( void )
{
        Precache();
        SET_MODEL( ENT(pev), "models/ramone.mdl" );
        UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );
        pev->solid      = SOLID_SLIDEBOX;
        pev->movetype   = MOVETYPE_STEP;
        m_bloodColor    = BLOOD_COLOR_RED;
        pev->health     = gSkillData.ramoneHealth;
        pev->view_ofs   = Vector(0,0,50);
        m_flFieldOfView = 0.2f;
        m_MonsterState  = MONSTERSTATE_NONE;
        m_afCapability  = bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP
                        | bits_CAP_RANGE_ATTACK1 | bits_CAP_MELEE_ATTACK1;
        pev->body       = 1; // BARNEY_BODY_GUNDRAWN — gun visible in hand

        TalkInit();
        MonsterInit();
}
