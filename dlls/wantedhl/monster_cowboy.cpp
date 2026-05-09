/***
*
* WantedHL - monster_cowboy
* Armed outlaw. Fires pistol and throws dynamite.
* Based on the CBarney framework like Annie and Hoss.
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

class CCowboy : public CBarney
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

LINK_ENTITY_TO_CLASS( monster_cowboy, CCowboy );

int CCowboy::Classify( void ) { return CLASS_HUMAN_MILITARY; }

void CCowboy::TalkInit( void )
{
	CTalkMonster::TalkInit();
	m_szGrp[TLK_ANSWER]  = "!CB_ANSWER";
	m_szGrp[TLK_QUESTION] = "!CB_QUESTION";
	m_szGrp[TLK_IDLE]     = "!CB_IDLE";
	m_szGrp[TLK_STARE]    = "!CB_STARE";
	m_szGrp[TLK_USE]      = "!CB_OK";
	m_szGrp[TLK_UNUSE]    = "!CB_WAIT";
	m_szGrp[TLK_STOP]     = "!CB_STOP";
	m_szGrp[TLK_NOSHOOT]  = "!CB_SCARED";
	m_szGrp[TLK_HELLO]    = "!CB_HELLO";
	m_szGrp[TLK_PLPUSH]   = "!CB_PUSH";
	m_szGrp[TLK_PLREJECT] = "!CB_REJECT";
	m_szGrp[TLK_SMELL]    = NULL;
	m_szGrp[TLK_WOUND]    = "!CB_WOUND";
	m_szGrp[TLK_MORTAL]   = "!CB_MORTAL";
}

void CCowboy::AlertSound( void )
{
	EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "cowboy/cb_alert1.wav", 1.0, ATTN_NORM, 0, 100 );
}

void CCowboy::PainSound( void )
{
	EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "cowboy/cb_pain1.wav", 1.0, ATTN_NORM, 0, 100 );
}

void CCowboy::DeathSound( void )
{
	EMIT_SOUND_DYN( ENT(pev), CHAN_VOICE, "cowboy/cb_die1.wav", 1.0, ATTN_NORM, 0, 100 );
}

void CCowboy::BarneyFirePistol( void )
{
	Vector vecShootOrigin = GetGunPosition();
	Vector vecShootDir    = ShootAtEnemy( vecShootOrigin );

	UTIL_MakeVectors( pev->angles );

	FireBullets( 1, vecShootOrigin, vecShootDir, VECTOR_CONE_2DEGREES,
		1024, BULLET_MONSTER_9MM, 0, gSkillData.monDmgPistolBullet );

	int pitchShift = RANDOM_LONG(0,1) ? 100 : RANDOM_LONG(95,120);
	EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, "cowboy/cb_shoot1.wav",
		RANDOM_FLOAT(0.8f,1.0f), ATTN_NORM, 0, pitchShift );

	CSoundEnt::InsertSound( bits_SOUND_COMBAT, pev->origin, 384, 0.3f );
}

void CCowboy::Precache( void )
{
	PRECACHE_MODEL( "models/cowboy.mdl" );
	PRECACHE_SOUND( "cowboy/cb_alert1.wav" );
	PRECACHE_SOUND( "cowboy/cb_pain1.wav" );
	PRECACHE_SOUND( "cowboy/cb_die1.wav" );
	PRECACHE_SOUND( "cowboy/cb_shoot1.wav" );
	TalkInit();
	CTalkMonster::Precache();
}

void CCowboy::Spawn( void )
{
	Precache();
	SET_MODEL( ENT(pev), "models/cowboy.mdl" );

	UTIL_SetSize( pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX );
	pev->solid        = SOLID_SLIDEBOX;
	pev->movetype     = MOVETYPE_STEP;
	m_bloodColor      = BLOOD_COLOR_RED;
	pev->health       = gSkillData.cowboyHealth;
	pev->view_ofs     = Vector(0, 0, 50);
	m_flFieldOfView   = 0.2f;
	m_MonsterState    = MONSTERSTATE_NONE;
	m_afCapability    = bits_CAP_HEAR | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP
	                  | bits_CAP_RANGE_ATTACK1 | bits_CAP_MELEE_ATTACK1;
	pev->body         = 1; // gun in hand

	TalkInit();
	MonsterInit();
}
