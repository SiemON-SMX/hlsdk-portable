#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include <algorithm>

DECLARE_MESSAGE( m_endCredits, EndCredits )

int CHudEndCredits::Init( void )
{
        HOOK_MESSAGE( EndCredits );

        gHUD.AddHudElem( this );

        return 1;
}

int CHudEndCredits::VidInit( void )
{
        creditSprites[0] = gHUD.GetSpriteIndex( "credits1" );
        creditSprites[1] = gHUD.GetSpriteIndex( "credits2" );
        creditSprites[2] = gHUD.GetSpriteIndex( "credits3" );
        creditSprites[3] = gHUD.GetSpriteIndex( "credits4" );
        creditSprites[4] = gHUD.GetSpriteIndex( "credits5" );
        creditSprites[5] = gHUD.GetSpriteIndex( "credits5a" );
        creditSprites[6] = gHUD.GetSpriteIndex( "credits6" );
        creditSprites[7] = gHUD.GetSpriteIndex( "credits10" );
        creditSprites[8] = gHUD.GetSpriteIndex( "credits11" );
        creditSprites[9] = gHUD.GetSpriteIndex( "credits12" );

        return 1;
}

void CHudEndCredits::Reset( void )
{
        m_iFlags = 0;
        ended = false;
}

int CHudEndCredits::XPosition( float x, int width, int totalWidth )
{
        int xPos;

        if( x == -1 )
        {
                xPos = ( ScreenWidth - width ) / 2;
        }
        else
        {
                if( x < 0 )
                        xPos = ( 1.0 + x ) * ScreenWidth - totalWidth;
                else
                        xPos = x * ScreenWidth;
        }

        if( xPos + width > ScreenWidth )
                xPos = ScreenWidth - width;
        else if( xPos < 0 )
                xPos = 0;

        return xPos;
}

int CHudEndCredits::YPosition( float y, int height )
{
        int yPos;

        if( y == -1 )
                yPos = ( ScreenHeight - height ) * 0.5;
        else
        {
                if( y < 0 )
                        yPos = ( 1.0 + y ) * ScreenHeight - height;
                else
                        yPos = y * ScreenHeight;
        }

        if( yPos + height > ScreenHeight )
                yPos = ScreenHeight - height;
        else if( yPos < 0 )
                yPos = 0;

        return yPos;
}

int CHudEndCredits::Draw( float flTime )
{
        if( timeStart < 0.0f ) timeStart = flTime;
        float time = flTime - timeStart;

        int spriteIndex = -1;
        int alpha = 255;

        gEngfuncs.pfnFillRGBABlend( 0, 0, ScreenWidth, ScreenHeight, 0, 0, 0, 255 );

        if( time >= 0 && time <= 5.8 ) {
                spriteIndex = 0;
        } else if( time >= 6.0 && time <= 11.8 ) {
                spriteIndex = 1;
        } else if( time >= 12.0 && time <= 17.8 ) {
                spriteIndex = 2;
        } else if( time >= 18.0 && time <= 23.8 ) {
                spriteIndex = 3;
        } else if( time >= 24.0 && time <= 29.8 ) {
                spriteIndex = 4;
        } else if( time >= 30.0 && time <= 35.8 ) {
                spriteIndex = 5;
        } else if( time >= 36.0 && time <= 41.8 ) {
                spriteIndex = 6;
        } else if( time >= 42.0 && time <= 47.5 ) {
                spriteIndex = 7;
        } else if( time >= 47.7 && time <= 52.2 ) {
                spriteIndex = 8;
        } else if( time >= 52.4 && time <= 56.4 ) {
                spriteIndex = 9;
        } else if( time >= 56.681 && time < 96.4f ) {
                spriteIndex = 0;
                float difference = 64.0f - std::min( time, 64.0f );
                float maxDiff = 7.3f;
                alpha = ( ( maxDiff - difference ) / maxDiff ) * 255;
        } else if( time >= 96.4f ) {
                gEngfuncs.pfnClientCmd( "disconnect" );
        }

        if( spriteIndex >= 0 && creditSprites[spriteIndex] >= 0 )
        {
                int width  = gHUD.GetSpriteRect( creditSprites[spriteIndex] ).right  - gHUD.GetSpriteRect( creditSprites[spriteIndex] ).left;
                int height = gHUD.GetSpriteRect( creditSprites[spriteIndex] ).bottom - gHUD.GetSpriteRect( creditSprites[spriteIndex] ).top;

                int x = XPosition( -1, width, width );
                int y = YPosition( -1, height );

                SPR_Set( gHUD.GetSprite( creditSprites[spriteIndex] ), alpha, alpha, alpha );
                SPR_DrawAdditive( 0, x, y, &gHUD.GetSpriteRect( creditSprites[spriteIndex] ) );
        }

        return 1;
}

int CHudEndCredits::MsgFunc_EndCredits( const char *pszName, int iSize, void *pbuf )
{
        if( ended )
                return 1;

        BEGIN_READ( pbuf, iSize );

        ended = true;
        timeStart = -1.0f;  // set on first Draw call
        m_iFlags |= HUD_ACTIVE;

        return 1;
}
