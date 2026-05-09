/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*	Use, distribution, and modification of this source code and/or resulting
*	object code is restricted to non-commercial enhancements to products from
*	Valve LLC.  All other use, distribution, or modification is prohibited
*	without written permission from Valve LLC.
*
****/

#pragma once

// CArrow - bow projectile entity.
// Callers must have already included extdll.h, util.h, cbase.h, etc.

class CArrow : public CBaseEntity
{
public:
	void Spawn( void );
	void Precache( void );
	void EXPORT ArrowTouch( CBaseEntity *pOther );
	void EXPORT ArrowThink( void );

	static CArrow *Shoot( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity );

	int  m_iDamage;
	BOOL m_bFired;
};
