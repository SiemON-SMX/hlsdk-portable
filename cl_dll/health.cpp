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

#include <stdio.h>
#include <stdlib.h>
#include <cmath>

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include <string.h>

#include "mobility_int.h"

DECLARE_MESSAGE( m_Health, Health )
DECLARE_MESSAGE( m_Health, Damage )

#define PAIN_NAME "sprites/%d_pain.spr"
#define DAMAGE_NAME "sprites/%d_dmg.spr"

int giDmgHeight, giDmgWidth;

int giDmgFlags[NUM_DMG_TYPES] =
{
        DMG_POISON,
        DMG_ACID,
        DMG_FREEZE|DMG_SLOWFREEZE,
        DMG_DROWN,
        DMG_BURN|DMG_SLOWBURN,
        DMG_NERVEGAS, 
        DMG_RADIATION,
        DMG_SHOCK,
        DMG_CALTROP,
        DMG_TRANQ,
        DMG_CONCUSS,
        DMG_HALLUC
};

int CHudHealth::Init( void )
{
        HOOK_MESSAGE( Health );
        HOOK_MESSAGE( Damage );
        m_iHealth = 100;
        painkillerEffect = 0;
        m_fFade = 0;
        fadeOut = 0;
        m_iFlags = 0;
        m_bitsDamage = 0;
        m_fAttackFront = m_fAttackRear = m_fAttackRight = m_fAttackLeft = 0;
        giDmgHeight = 0;
        giDmgWidth = 0;

        memset( m_dmg, 0, sizeof(DAMAGE_IMAGE) * NUM_DMG_TYPES );

        gHUD.AddHudElem( this );
        return 1;
}

void CHudHealth::Reset( void )
{
        // make sure the pain compass is cleared when the player respawns
        m_fAttackFront = m_fAttackRear = m_fAttackRight = m_fAttackLeft = 0;

        // force all the flashing damage icons to expire
        m_bitsDamage = 0;
        for( int i = 0; i < NUM_DMG_TYPES; i++ )
        {
                m_dmg[i].fExpire = 0;
        }
}

int CHudHealth::VidInit( void )
{
        m_hSprite = LoadSprite( PAIN_NAME );

        m_HUD_dmg_bio = gHUD.GetSpriteIndex( "dmg_bio" ) + 1;
        m_HUD_cross   = gHUD.GetSpriteIndex( "cross" );
        healthSprite  = gHUD.GetSpriteIndex( "health" );

        giDmgHeight = gHUD.GetSpriteRect( m_HUD_dmg_bio ).right - gHUD.GetSpriteRect( m_HUD_dmg_bio ).left;
        giDmgWidth  = gHUD.GetSpriteRect( m_HUD_dmg_bio ).bottom - gHUD.GetSpriteRect( m_HUD_dmg_bio ).top;

        return 1;
}

int CHudHealth::MsgFunc_Health( const char *pszName, int iSize, void *pbuf )
{
        BEGIN_READ( pbuf, iSize );
        int x = READ_LONG();
        painkillerEffect = READ_SHORT();

        m_iFlags |= HUD_ACTIVE;

        // Only update the fade if we've changed health
        if( x != m_iHealth )
        {
                m_fFade = FADE_TIME;
                m_iHealth = x;
        }

        return 1;
}

int CHudHealth::MsgFunc_Damage( const char *pszName, int iSize, void *pbuf )
{
        BEGIN_READ( pbuf, iSize );

        int armor = READ_BYTE();        // armor
        int damageTaken = READ_BYTE();  // health
        long bitsDamage = READ_LONG(); // damage bits

        vec3_t vecFrom;

        for( int i = 0; i < 3; i++ )
                vecFrom[i] = READ_COORD();

        UpdateTiles( gHUD.m_flTime, bitsDamage );

        // Actually took damage?
        if( damageTaken > 0 || armor > 0 )
        {
                CalcDamageDirection( vecFrom );

                if( gMobileEngfuncs && damageTaken > 0 )
                {
                        float time = damageTaken * 4.0f;

                        if( time > 200.0f )
                                time = 200.0f;
                        gMobileEngfuncs->pfnVibrate( time, 0 );
                }
        }

        return 1;
}

void CHudHealth::GetPainColor( int &r, int &g, int &b )
{
#if 0
        int iHealth = m_iHealth;

        if( iHealth > 25 )
                iHealth -= 25;
        else if( iHealth < 0 )
                iHealth = 0;

        g = iHealth * 255 / 100;
        r = 255 - g;
        b = 0;
#else
        if( m_iHealth > 25 )
        {
                UnpackRGB( r, g, b, RGB_YELLOWISH );
        }
        else
        {
                r = 250;
                g = 0;
                b = 0;
        }
#endif
}

int CHudHealth::Draw( float flTime )
{
        if( !( gHUD.m_iWeaponBits & ( 1 << ( WEAPON_SUIT ) ) )
                || ( gHUD.m_iHideHUDDisplay & HIDEHUD_HEALTH )
                || gEngfuncs.IsSpectateOnly()
                || strstr( gEngfuncs.pfnGetLevelName(), "nightmare" )
        ) {
                return 1;
        }

        wrect_t painRect  = gHUD.GetSpriteRect( healthSprite );
        wrect_t painRect2 = gHUD.GetSpriteRect( healthSprite );
        wrect_t painRect3 = gHUD.GetSpriteRect( healthSprite );

        int maxSpriteH = YRES( HEALTH_SPRITE_HEIGHT );
        int maxSpriteW = XRES( HEALTH_SPRITE_WIDTH );
        if( ( painRect.bottom - painRect.top ) > maxSpriteH )
        {
                painRect.bottom  = painRect.top  + maxSpriteH;
                painRect2.bottom = painRect2.top + maxSpriteH;
                painRect3.bottom = painRect3.top + maxSpriteH;
        }
        if( ( painRect.right - painRect.left ) > maxSpriteW )
        {
                painRect.right  = painRect.left  + maxSpriteW;
                painRect2.right = painRect2.left + maxSpriteW;
                painRect3.right = painRect3.left + maxSpriteW;
        }

        int painRectHeight = painRect.bottom - painRect.top;

        float healthPercent           = ( m_iHealth / 100.0f );
        float painkillerEffectPercent = ( painkillerEffect / 100.0f );
        float damagePercent           = 1.0f - ( healthPercent + painkillerEffectPercent );

        int x = XRES( CORNER_OFFSET );
        int y = ScreenHeight - painRectHeight - YRES( CORNER_OFFSET );

        if( m_iHealth > 0 )
        {
                painRect.bottom = (int)( painRect.bottom * healthPercent );
                SPR_Set( gHUD.GetSprite( healthSprite ), 255, 255, 255 );
                SPR_DrawAdditive( 0, x, y, &painRect );
        }

        if( painkillerEffectPercent > 0.0f )
        {
                int painkillerEffectOffset = (int)( painRectHeight * healthPercent );
                painRect3.top    = painRect.bottom;
                painRect3.bottom = (int)( gHUD.GetSpriteRect( healthSprite ).bottom * ( healthPercent + painkillerEffectPercent ) );
                SPR_Set( gHUD.GetSprite( healthSprite ), 255, 125, 125 );
                SPR_DrawAdditive( 0, x, y + painkillerEffectOffset, &painRect3 );
        }

        if( damagePercent > 0.0f )
        {
                int damageHeight = (int)( painRectHeight * damagePercent );
                int healthOffset = (int)( painRectHeight * ( healthPercent + painkillerEffectPercent ) );
                painRect2.top    = painRect2.bottom - damageHeight;
                SPR_Set( gHUD.GetSprite( healthSprite ), 255, 0, 0 );
                SPR_DrawAdditive( 0, x, y + healthOffset, &painRect2 );
        }

        // Show numerical health when above 100 (overhealed / boosted beyond max)
        if( m_iHealth > 100 )
        {
                int painRectWidth = painRect.right - painRect.left;
                char healthString[16];
                sprintf( healthString, "%d\n", m_iHealth );
                gHUD.DrawHudString( x + painRectWidth / 2 + 3, y - 25, x + painRectWidth / 2 + 53, healthString, 200, 200, 200 );
        }

        DrawDamage( flTime );
        return DrawPain( flTime );
}

void CHudHealth::CalcDamageDirection( vec3_t vecFrom )
{
        vec3_t forward, right, up;
        float side, front;
        vec3_t vecOrigin, vecAngles;

        if( !vecFrom[0] && !vecFrom[1] && !vecFrom[2] )
        {
                m_fAttackFront = m_fAttackRear = m_fAttackRight = m_fAttackLeft = 0;
                return;
        }

        memcpy( vecOrigin, gHUD.m_vecOrigin, sizeof(vec3_t) );
        memcpy( vecAngles, gHUD.m_vecAngles, sizeof(vec3_t) );

        VectorSubtract( vecFrom, vecOrigin, vecFrom );

        float flDistToTarget = vecFrom.Length();

        vecFrom = vecFrom.Normalize();
        AngleVectors( vecAngles, forward, right, up );

        front = DotProduct( vecFrom, right );
        side = DotProduct( vecFrom, forward );

        if( flDistToTarget <= 50 )
        {
                m_fAttackFront = m_fAttackRear = m_fAttackRight = m_fAttackLeft = 1;
        }
        else 
        {
                if( side > 0.0f )
                {
                        if( side > 0.3f )
                                m_fAttackFront = Q_max( m_fAttackFront, side );
                }
                else
                {
                        float f = fabs( side );
                        if( f > 0.3f )
                                m_fAttackRear = Q_max( m_fAttackRear, f );
                }

                if( front > 0.0f )
                {
                        if( front > 0.3f )
                                m_fAttackRight = Q_max( m_fAttackRight, front );
                }
                else
                {
                        float f = fabs( front );
                        if( f > 0.3f )
                                m_fAttackLeft = Q_max( m_fAttackLeft, f );
                }
        }
}

int CHudHealth::DrawPain( float flTime )
{
        if( !( m_fAttackFront || m_fAttackRear || m_fAttackLeft || m_fAttackRight) )
                return 1;

        int r, g, b;
        int x, y, a, shade;

        // TODO:  get the shift value of the health
        a = 255;        // max brightness until then

        float fFade = gHUD.m_flTimeDelta * 2;

        // SPR_Draw top
        if( m_fAttackFront > 0.4f )
        {
                GetPainColor( r, g, b );
                shade = a * Q_max( m_fAttackFront, 0.5f );
                ScaleColors( r, g, b, shade );
                SPR_Set( m_hSprite, r, g, b );

                x = ScreenWidth / 2 - SPR_Width( m_hSprite, 0 ) / 2;
                y = ScreenHeight / 2 - SPR_Height( m_hSprite, 0 ) * 3;
                SPR_DrawAdditive( 0, x, y, NULL );
                m_fAttackFront = Q_max( 0, m_fAttackFront - fFade );
        } else
                m_fAttackFront = 0;

        if( m_fAttackRight > 0.4f )
        {
                GetPainColor( r, g, b );
                shade = a * Q_max( m_fAttackRight, 0.5f );
                ScaleColors( r, g, b, shade );
                SPR_Set( m_hSprite, r, g, b );

                x = ScreenWidth / 2 + SPR_Width( m_hSprite, 1 ) * 2;
                y = ScreenHeight / 2 - SPR_Height( m_hSprite,1 ) / 2;
                SPR_DrawAdditive( 1, x, y, NULL );
                m_fAttackRight = Q_max( 0, m_fAttackRight - fFade );
        }
        else
                m_fAttackRight = 0;

        if( m_fAttackRear > 0.4f )
        {
                GetPainColor( r, g, b );
                shade = a * Q_max( m_fAttackRear, 0.5f );
                ScaleColors( r, g, b, shade );
                SPR_Set( m_hSprite, r, g, b );

                x = ScreenWidth / 2 - SPR_Width( m_hSprite, 2 ) / 2;
                y = ScreenHeight / 2 + SPR_Height( m_hSprite, 2 ) * 2;
                SPR_DrawAdditive( 2, x, y, NULL );
                m_fAttackRear = Q_max( 0, m_fAttackRear - fFade );
        }
        else
                m_fAttackRear = 0;

        if( m_fAttackLeft > 0.4f )
        {
                GetPainColor( r, g, b );
                shade = a * Q_max( m_fAttackLeft, 0.5f );
                ScaleColors( r, g, b, shade );
                SPR_Set( m_hSprite, r, g, b );

                x = ScreenWidth / 2 - SPR_Width( m_hSprite, 3 ) * 3;
                y = ScreenHeight / 2 - SPR_Height( m_hSprite,3 ) / 2;
                SPR_DrawAdditive( 3, x, y, NULL );

                m_fAttackLeft = Q_max( 0, m_fAttackLeft - fFade );
        } else
                m_fAttackLeft = 0;

        return 1;
}

int CHudHealth::DrawDamage( float flTime )
{
        int i, r, g, b, a;
        DAMAGE_IMAGE *pdmg;

        if( !m_bitsDamage )
                return 1;

        UnpackRGB( r, g, b, RGB_YELLOWISH );

        a = (int)( fabs( sin( flTime * 2.0f ) ) * 256.0f );

        ScaleColors( r, g, b, a );

        // check for bits that should be expired
        for( i = 0; i < NUM_DMG_TYPES; i++ )
        {
                if( m_bitsDamage & giDmgFlags[i] )
                {
                        pdmg = &m_dmg[i];

                        // Draw all the items — offset to sit beside the health bar
                        SPR_Set( gHUD.GetSprite( m_HUD_dmg_bio + i ), r, g, b );
                        SPR_DrawAdditive( 0, pdmg->x + CORNER_OFFSET / 2, pdmg->y - (int)( HEALTH_SPRITE_HEIGHT / 1.5f ), &gHUD.GetSpriteRect( m_HUD_dmg_bio + i ) );

                        pdmg->fExpire = Q_min( flTime + DMG_IMAGE_LIFE, pdmg->fExpire );

                        if( pdmg->fExpire <= flTime             // when the time has expired
                                && a < 40 )                                             // and the flash is at the low point of the cycle
                        {
                                pdmg->fExpire = 0;

                                int y = pdmg->y;
                                pdmg->x = pdmg->y = 0;

                                // move everyone above down
                                for( int j = 0; j < NUM_DMG_TYPES; j++ )
                                {
                                        pdmg = &m_dmg[j];
                                        if( ( pdmg->y ) && ( pdmg->y < y ) )
                                                pdmg->y += giDmgHeight;

                                }

                                m_bitsDamage &= ~giDmgFlags[i];  // clear the bits
                        }
                }
        }

        return 1;
}

void CHudHealth::UpdateTiles( float flTime, long bitsDamage )
{       
        DAMAGE_IMAGE *pdmg;

        // Which types are new?
        long bitsOn = ~m_bitsDamage & bitsDamage;

        for( int i = 0; i < NUM_DMG_TYPES; i++ )
        {
                pdmg = &m_dmg[i];

                // Is this one already on?
                if( m_bitsDamage & giDmgFlags[i] )
                {
                        pdmg->fExpire = flTime + DMG_IMAGE_LIFE; // extend the duration
                        if( !pdmg->fBaseline )
                                pdmg->fBaseline = flTime;
                }

                // Are we just turning it on?
                if( bitsOn & giDmgFlags[i] )
                {
                        // put this one at the bottom
                        pdmg->x = giDmgWidth / 8;
                        pdmg->y = ScreenHeight - giDmgHeight * 2;
                        pdmg->fExpire=flTime + DMG_IMAGE_LIFE;

                        // move everyone else up
                        for( int j = 0; j < NUM_DMG_TYPES; j++ )
                        {
                                if( j == i )
                                        continue;

                                pdmg = &m_dmg[j];
                                if( pdmg->y )
                                        pdmg->y -= giDmgHeight;
                        }
                        // pdmg = &m_dmg[i];
                }
        }

        // damage bits are only turned on here;  they are turned off when the draw time has expired (in DrawDamage())
        m_bitsDamage |= bitsDamage;
}
