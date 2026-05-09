/***
*
* WantedHL - monster_kaiewi
* Kaiewi Warriors. Fires winchester and bow. Hostile to player.
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

class CKaiewi : public CBarney
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




LINK_ENTITY_TO_CLASS( monster_kaiewi, CKaiewi );



int CKaiewi::Classify( void ) { return CLASS_HUMAN_MILITARY; }

void CKaiewi::TalkInit( void )
{
        CTalkMonster::TalkInit();
        m_szGrp[TLK_ANSWER]  = "!KW_ANSWER";
        m_szGrp[TLK_QUESTION] = "!KW_QUESTION";
        m_szGrp[TLK_IDLE]    = "!KW_IDLE";
        m_szGrp[TLK_STARE]   = "!KW_STARE";
        m_szGrp[TLK_USE]     = "!KW_OK";
        m_szGrp[TLK_UNUSE]   = "!KW_WAIT";
        m_szGrp[TLK_STOP]    = "!KW_STOP";
        m_szGrp[TLK_NOSHOOT] = "!KW_SCARED";
        m_szGrp[TLK_HELLO]   = "!KW_HELLO";
        m_szGrp[TLK_PLPUSH]  = "!KW_PUSH";
        m_szGrp[TLK_PLREJECT]= "!KW_REJECT";
        m_szGrp[TLK_SMELL]   = NULL;
        m_szGrp[TLK_WOUND]   = "!KW_WOUND";
        m_szGrp[TLK_MORTAL]  = "!KW_MORTAL";
}

void CKaiewi::AlertSound( void )
{
        EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "kaiewi/kw_alert1.wav", 1.0, ATTN_NORM, 0, 100 );
}
void CKaiewi::PainSound( void )
{
        EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "kaiewi/kw_pain1.wav", 1.0, ATTN_NORM, 0, 100 );
}
void CKaiewi::DeathSound( void )
{
        EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "kaiewi/kw_die1.wav", 1.0, ATTN_NORM, 0, 100 );
}

void CKaiewi::BarneyFirePistol( void )
{
        Vector vecShootOrigin = GetGunPosition();
        Vector vecShootDir    = ShootAtEnemy( vecShootOrigin );

        FireBullets( 1, vecShootOrigin, vecShootDir, VECTOR_CONE_3DEGREES,
                2048, BULLET_MONSTER_12MM, 0, gSkillData.monDmgWinchesterBullet );

        EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "kaiewi/kw_shoot1.wav",
                RANDOM_FLOAT(0.8f,1.0f), ATTN_NORM, 0, RANDOM_LONG(95,110) );

        CSoundEnt::InsertSound( bits_SOUND_COMBAT, pev->origin, 512, 0.3f );
}

void CKaiewi::Precache( void )
{
        PRECACHE_MODEL( "models/kaiewi.mdl" );
        PRECACHE_SOUND( "kaiewi/kw_alert1.wav" );
        PRECACHE_SOUND( "kaiewi/kw_pain1.wav" );
        PRECACHE_SOUND( "kaiewi/kw_die1.wav" );
        PRECACHE_SOUND( "kaiewi/kw_shoot1.wav" );
        TalkInit();
        CTalkMonster::Precache();
}

void CKaiewi::Spawn( void )
{
        Precache();
        SET_MODEL( ENT(pev), "models/kaiewi.mdl" );
        UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );
        pev->solid      = SOLID_SLIDEBOX;
        pev->movetype   = MOVETYPE_STEP;
        m_bloodColor    = BLOOD_COLOR_RED;
        pev->health     = gSkillData.kaiwiHealth;
        pev->view_ofs   = Vector(0,0,50);
        m_flFieldOfView = 0.2f;
        m_MonsterState  = MONSTERSTATE_NONE;
        m_afCapability  = bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP
                        | bits_CAP_RANGE_ATTACK1 | bits_CAP_MELEE_ATTACK1;
        pev->body       = 1; // BARNEY_BODY_GUNDRAWN — gun visible in hand

        TalkInit();
        MonsterInit();
}
