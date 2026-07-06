/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"monsters.h"
#include	"soundent.h"

LINK_ENTITY_TO_CLASS( soundent, CSoundEnt )

CSoundEnt *pSoundEnt;

void CSound::Clear( void )
{
	m_vecOrigin = g_vecZero;
	m_iType = 0;
	m_iVolume = 0;
	m_flExpireTime = 0;
	m_iNext = SOUNDLIST_EMPTY;
	m_iNextAudible = 0;
}

void CSound::Reset( void )
{
	m_vecOrigin = g_vecZero;
	m_iType = 0;
	m_iVolume = 0;
	m_iNext = SOUNDLIST_EMPTY;
}

BOOL CSound::FIsSound( void )
{
	if( m_iType & ( bits_SOUND_COMBAT | bits_SOUND_WORLD | bits_SOUND_PLAYER | bits_SOUND_DANGER ) )
	{
		return TRUE;
	}

	return FALSE;
}

BOOL CSound::FIsScent( void )
{
	if( m_iType & ( bits_SOUND_CARCASS | bits_SOUND_MEAT | bits_SOUND_GARBAGE ) )
	{
		return TRUE;
	}

	return FALSE;
}

void CSoundEnt::Spawn( void )
{
	pev->solid = SOLID_NOT;
	Initialize();

	pev->nextthink = gpGlobals->time + 1.0f;
}

void CSoundEnt::Think( void )
{
	int iSound;
	int iPreviousSound;

	pev->nextthink = gpGlobals->time + 0.3f;// how often to check the sound list.

	iPreviousSound = SOUNDLIST_EMPTY;
	iSound = m_iActiveSound; 

	while( iSound != SOUNDLIST_EMPTY )
	{
		if( m_SoundPool[ iSound ].m_flExpireTime <= gpGlobals->time && m_SoundPool[ iSound ].m_flExpireTime != SOUND_NEVER_EXPIRE )
		{
			int iNext = m_SoundPool[ iSound ].m_iNext;

			// move this sound back into the free list
			FreeSound( iSound, iPreviousSound );

			iSound = iNext;
		}
		else
		{
			iPreviousSound = iSound;
			iSound = m_SoundPool[ iSound ].m_iNext;
		}
	}

	if( m_fShowReport )
	{
		ALERT( at_aiconsole, "Soundlist: %d / %d  (%d)\n", ISoundsInList( SOUNDLISTTYPE_ACTIVE ),ISoundsInList( SOUNDLISTTYPE_FREE ), ISoundsInList( SOUNDLISTTYPE_ACTIVE ) - m_cLastActiveSounds );
		m_cLastActiveSounds = ISoundsInList( SOUNDLISTTYPE_ACTIVE );
	}
}

void CSoundEnt::Precache( void )
{
}

void CSoundEnt::FreeSound( int iSound, int iPrevious )
{
	if( !pSoundEnt )
	{
		// no sound ent!
		return;
	}

	if( iPrevious != SOUNDLIST_EMPTY )
	{
		pSoundEnt->m_SoundPool[iPrevious].m_iNext = pSoundEnt->m_SoundPool[iSound].m_iNext;
	}
	else 
	{
		// the sound we're freeing IS the head of the active list.
		pSoundEnt->m_iActiveSound = pSoundEnt->m_SoundPool[iSound].m_iNext;
	}

	// make iSound the head of the Free list.
	pSoundEnt->m_SoundPool[iSound].m_iNext = pSoundEnt->m_iFreeSound;
	pSoundEnt->m_iFreeSound = iSound;
}

int CSoundEnt::IAllocSound( void )
{
	int iNewSound;

	if( m_iFreeSound == SOUNDLIST_EMPTY )
	{
		// no free sound!
		ALERT( at_console, "Free Sound List is full!\n" );
		return SOUNDLIST_EMPTY;
	}


	iNewSound = m_iFreeSound;// copy the index of the next free sound

	m_iFreeSound = m_SoundPool[m_iFreeSound].m_iNext;// move the index down into the free list. 

	m_SoundPool[iNewSound].m_iNext = m_iActiveSound;// point the new sound at the top of the active list.

	m_iActiveSound = iNewSound;// now make the new sound the top of the active list. You're done.

	return iNewSound;
}

void CSoundEnt::InsertSound( int iType, const Vector &vecOrigin, int iVolume, float flDuration )
{
	int iThisSound;

	if( !pSoundEnt )
	{
		// no sound ent!
		return;
	}

	iThisSound = pSoundEnt->IAllocSound();

	if( iThisSound == SOUNDLIST_EMPTY )
	{
		ALERT( at_console, "Could not AllocSound() for InsertSound() (DLL)\n" );
		return;
	}

	pSoundEnt->m_SoundPool[iThisSound].m_vecOrigin = vecOrigin;
	pSoundEnt->m_SoundPool[iThisSound].m_iType = iType;
	pSoundEnt->m_SoundPool[iThisSound].m_iVolume = iVolume;
	pSoundEnt->m_SoundPool[iThisSound].m_flExpireTime = gpGlobals->time + flDuration;
}

void CSoundEnt::Initialize( void )
{
  	int i;
	int iSound;

	m_cLastActiveSounds = 0;
	m_iFreeSound = 0;
	m_iActiveSound = SOUNDLIST_EMPTY;

	for( i = 0; i < MAX_WORLD_SOUNDS; i++ )
	{
		// clear all sounds, and link them into the free sound list.
		m_SoundPool[i].Clear();
		m_SoundPool[i].m_iNext = i + 1;
	}

	m_SoundPool[i - 1].m_iNext = SOUNDLIST_EMPTY;// terminate the list here.

	// now reserve enough sounds for each client
	for( i = 0; i < gpGlobals->maxClients; i++ )
	{
		iSound = pSoundEnt->IAllocSound();

		if( iSound == SOUNDLIST_EMPTY )
		{
			ALERT( at_console, "Could not AllocSound() for Client Reserve! (DLL)\n" );
			return;
		}

		pSoundEnt->m_SoundPool[iSound].m_flExpireTime = SOUND_NEVER_EXPIRE;
	}

	if( CVAR_GET_FLOAT( "displaysoundlist" ) == 1 )
	{
		m_fShowReport = TRUE;
	}
	else
	{
		m_fShowReport = FALSE;
	}
}

int CSoundEnt::ISoundsInList( int iListType )
{
	int i;
	int iThisSound = 0;

	if( iListType == SOUNDLISTTYPE_FREE )
	{
		iThisSound = m_iFreeSound;
	}
	else if( iListType == SOUNDLISTTYPE_ACTIVE )
	{
		iThisSound = m_iActiveSound;
	}
	else
	{
		ALERT( at_console, "Unknown Sound List Type!\n" );
	}

	if( iThisSound == SOUNDLIST_EMPTY )
	{
		return 0;
	}

	i = 0;

	while( iThisSound != SOUNDLIST_EMPTY )
	{
		i++;

		iThisSound = m_SoundPool[iThisSound].m_iNext;
	}

	return i;
}

int CSoundEnt::ActiveList( void )
{
	if( !pSoundEnt )
	{
		return SOUNDLIST_EMPTY;
	}

	return pSoundEnt->m_iActiveSound;
}

int CSoundEnt::FreeList( void )
{
	if( !pSoundEnt )
	{
		return SOUNDLIST_EMPTY;
	}

	return pSoundEnt->m_iFreeSound;
}

CSound *CSoundEnt::SoundPointerForIndex( int iIndex )
{
	if( !pSoundEnt )
	{
		return NULL;
	}

	if( iIndex > ( MAX_WORLD_SOUNDS - 1 ) )
	{
		ALERT( at_console, "SoundPointerForIndex() - Index too large!\n" );
		return NULL;
	}

	if( iIndex < 0 )
	{
		ALERT( at_console, "SoundPointerForIndex() - Index < 0!\n" );
		return NULL;
	}

	return &pSoundEnt->m_SoundPool[iIndex];
}

int CSoundEnt::ClientSoundIndex( edict_t *pClient )
{
	int iReturn = ENTINDEX( pClient ) - 1;

#if _DEBUG
	if( iReturn < 0 || iReturn > gpGlobals->maxClients )
	{
		ALERT( at_console, "** ClientSoundIndex returning a bogus value! **\n" );
	}
#endif // _DEBUG

	return iReturn;
}
