#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string>
#include <vector>
#include <cstdio>
#include <cstring>

static void DrawStringCentered( int cx, int y, const char *str, int r, int g, int b )
{
        int len = gHUD.DrawHudStringLen( str );
        gHUD.DrawHudString( cx - len / 2, y, cx + len, str, r, g, b );
}

static void DrawStringRight( int rx, int y, const char *str, int r, int g, int b )
{
        gHUD.DrawHudStringReverse( rx, y, 0, str, r, g, b );
}



DECLARE_MESSAGE( m_EndScreen, EndActiv )
DECLARE_MESSAGE( m_EndScreen, EndTitle )
DECLARE_MESSAGE( m_EndScreen, EndTime )
DECLARE_MESSAGE( m_EndScreen, EndScore )
DECLARE_MESSAGE( m_EndScreen, EndStat )

#define MESSAGE_BRIGHTENESS 200

static void SplitPipe( const char *src, std::string &left, std::string &right )
{
        const char *pipe = strchr( src, '|' );
        if( pipe )
        {
                left  = std::string( src, pipe - src );
                right = std::string( pipe + 1 );
        }
        else
        {
                left  = src;
                right = "";
        }
}

static std::string FormatTime( float seconds )
{
        int mins = (int)seconds / 60;
        int secs = (int)seconds % 60;
        char buf[32];
        sprintf( buf, "%d:%02d", mins, secs );
        return buf;
}

int CHudEndScreen::Init( void )
{
        HOOK_MESSAGE( EndActiv );
        HOOK_MESSAGE( EndTitle );
        HOOK_MESSAGE( EndTime );
        HOOK_MESSAGE( EndScore );
        HOOK_MESSAGE( EndStat );

        gHUD.AddHudElem( this );

        return 1;
}

void CHudEndScreen::Reset( void )
{
        m_iFlags = 0;
        cheated = false;
        titleLines.clear();
        animLines.clear();
        statLines.clear();
}

int CHudEndScreen::Draw( float flTime )
{
        if( ( gHUD.m_iHideHUDDisplay & HIDEHUD_ALL ) || gEngfuncs.IsSpectateOnly() )
                return 1;

        const int r = MESSAGE_BRIGHTENESS;
        const int g = MESSAGE_BRIGHTENESS;
        const int b = MESSAGE_BRIGHTENESS;

        const int actualW = 640;
        const int actualH = 480;
        const int xOffset = ( ScreenWidth  / 2 ) - ( actualW / 2 );
        const int yOffset = ( ScreenHeight / 2 ) - ( actualH / 2 );

        int x = ScreenWidth / 2;
        int y = CORNER_OFFSET + yOffset;

        // Black overlay
        gEngfuncs.pfnFillRGBABlend( 0, 0, ScreenWidth, ScreenHeight, 0, 0, 0, 255 );

        // Title lines
        for( const auto &line : titleLines )
        {
                DrawStringCentered( x, y, line.c_str(), r, g, b );
                y += gHUD.m_scrinfo.iCharHeight;
        }
        y += gHUD.m_scrinfo.iCharHeight;

        // Time / score lines
        int lx = xOffset + CORNER_OFFSET + 60;
        int rx = ScreenWidth - xOffset - CORNER_OFFSET - 60;
        for( const auto &al : animLines )
        {
                gHUD.DrawHudString( lx, y, 300, al.label.c_str(), r, g, b );
                DrawStringRight( rx, y, al.value.c_str(), r, g, b );
                y += gHUD.m_scrinfo.iCharHeight * 2;
        }

        // Stat lines
        x = ScreenWidth / 2;
        for( const auto &sl : statLines )
        {
                gHUD.DrawHudString( x - 140, y, 200, sl.key.c_str(), r, g, b );
                DrawStringRight( x + 140, y, sl.value.c_str(), r, g, b );
                y += gHUD.m_scrinfo.iCharHeight - 2;
        }

        // Bottom prompt
        y = ScreenHeight - yOffset - CORNER_OFFSET - gHUD.m_scrinfo.iCharHeight;
        DrawStringCentered( x, y, "PRESS ESC TO QUIT OR START THE NEW GAME", r, g, b );

        if( cheated )
        {
                y -= gHUD.m_scrinfo.iCharHeight;
                DrawStringCentered( x, y, "YOU'VE BEEN CHEATING - RESULTS WON'T BE SAVED", 200, 0, 0 );
        }

        return 1;
}

int CHudEndScreen::MsgFunc_EndActiv( const char *pszName, int iSize, void *pbuf )
{
        BEGIN_READ( pbuf, iSize );
        cheated = READ_BYTE() != 0;
        m_iFlags |= HUD_ACTIVE;
        return 1;
}

int CHudEndScreen::MsgFunc_EndTitle( const char *pszName, int iSize, void *pbuf )
{
        BEGIN_READ( pbuf, iSize );
        titleLines.push_back( READ_STRING() );
        return 1;
}

int CHudEndScreen::MsgFunc_EndTime( const char *pszName, int iSize, void *pbuf )
{
        BEGIN_READ( pbuf, iSize );
        const char *msg = READ_STRING();
        float time   = (float)READ_LONG();
        /*int record =*/ READ_LONG();
        /*int   beaten =*/ READ_BYTE();

        std::string label, right;
        SplitPipe( msg, label, right );

        animLines.push_back( { label, FormatTime( time ) } );
        return 1;
}

int CHudEndScreen::MsgFunc_EndScore( const char *pszName, int iSize, void *pbuf )
{
        BEGIN_READ( pbuf, iSize );
        const char *msg = READ_STRING();
        int score    = READ_LONG();
        /*int record =*/ READ_LONG();
        /*int beaten =*/ READ_BYTE();

        std::string label, right;
        SplitPipe( msg, label, right );

        char buf[32];
        sprintf( buf, "%d", score );
        animLines.push_back( { label, buf } );
        return 1;
}

int CHudEndScreen::MsgFunc_EndStat( const char *pszName, int iSize, void *pbuf )
{
        BEGIN_READ( pbuf, iSize );
        const char *msg = READ_STRING();

        std::string key, value;
        SplitPipe( msg, key, value );
        statLines.push_back( { key, value } );
        return 1;
}
