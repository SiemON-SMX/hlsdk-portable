
#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

DECLARE_MESSAGE( m_SlowMotion, SlowMotion )

int CHudSlowMotion::Init( void )
{
        HOOK_MESSAGE( SlowMotion );

        slowMotionCharge = 100;
        m_iFlags = 0;

        gHUD.AddHudElem( this );

        return 1;
}

int CHudSlowMotion::VidInit( void )
{
        hourglassStrokeSprite = gHUD.GetSpriteIndex( "hourglass_stroke" );
        hourglassFillSprite   = gHUD.GetSpriteIndex( "hourglass_fill" );

        return 1;
}

int CHudSlowMotion::Draw( float flTime )
{
        if( !( gHUD.m_iWeaponBits & ( 1 << ( WEAPON_SUIT ) ) )
         || ( gHUD.m_iHideHUDDisplay & HIDEHUD_HEALTH )
         || gEngfuncs.IsSpectateOnly() )
        {
                return 1;
        }

        if( hourglassStrokeSprite < 0 )
                return 1;

        wrect_t hourglassRect = gHUD.GetSpriteRect( hourglassStrokeSprite );

        int maxSpriteH = YRES( HEALTH_SPRITE_HEIGHT );
        int maxSpriteW = XRES( HOURGLASS_SPRITE_WIDTH );
        if( ( hourglassRect.bottom - hourglassRect.top ) > maxSpriteH )
                hourglassRect.bottom = hourglassRect.top + maxSpriteH;
        if( ( hourglassRect.right - hourglassRect.left ) > maxSpriteW )
                hourglassRect.right = hourglassRect.left + maxSpriteW;

        int hourglassRectHeight = hourglassRect.bottom - hourglassRect.top;

        int x = XRES( CORNER_OFFSET ) + HEALTH_SPRITE_WIDTH + BOTTOM_LEFT_SPACING;
        int y = ScreenHeight - hourglassRectHeight - YRES( CORNER_OFFSET );

        // Dim background (empty portion)
        SPR_Set( gHUD.GetSprite( hourglassStrokeSprite ), 20, 20, 20 );
        SPR_DrawAdditive( 0, x, y, &hourglassRect );

        // Bright fill (remaining charge, drawn from current top downward)
        float slowmotionPercent     = slowMotionCharge / 100.0f;
        float slowmotionLackPercent = 1.0f - slowmotionPercent;

        if( slowmotionPercent > 0.0f )
        {
                if( hourglassFillSprite >= 0 )
                        hourglassRect = gHUD.GetSpriteRect( hourglassFillSprite );

                int hourglassHeight = (int)( hourglassRectHeight * slowmotionLackPercent );
                hourglassRect.top   = hourglassHeight;

                SPR_Set( gHUD.GetSprite( hourglassStrokeSprite ), 180, 180, 180 );
                SPR_DrawAdditive( 0, x, y + hourglassHeight, &hourglassRect );
        }

        return 1;
}

int CHudSlowMotion::MsgFunc_SlowMotion( const char *pszName, int iSize, void *pbuf )
{
        BEGIN_READ( pbuf, iSize );
        slowMotionCharge = READ_BYTE();

        m_iFlags |= HUD_ACTIVE;

        return 1;
}
