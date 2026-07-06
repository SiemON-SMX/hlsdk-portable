
#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "subtitles.h"

#include <map>
#include <vector>
#include <string>
#include <fstream>
#include <regex>
#include <algorithm>
#include <cctype>
#include <dirent.h>


static std::string Str_Trim( const std::string &s )
{
	size_t start = s.find_first_not_of( " \t\r\n" );
	if( start == std::string::npos ) return "";
	size_t end = s.find_last_not_of( " \t\r\n" );
	return s.substr( start, end - start + 1 );
}

static std::string Str_Upper( const std::string &s )
{
	std::string r = s;
	for( auto &c : r )
		c = (char)toupper( (unsigned char)c );
	return r;
}

static std::vector<std::string> Str_Split( const std::string &s, char delim )
{
	std::vector<std::string> out;
	std::string tok;
	for( char c : s )
	{
		if( c == delim )
		{
			out.push_back( tok );
			tok.clear();
		}
		else
			tok += c;
	}
	out.push_back( tok );
	return out;
}

static std::vector<std::string> Str_WordWrap( const std::string &text, int maxWidth )
{
	std::vector<std::string> lines;
	std::vector<std::string> words = Str_Split( text, ' ' );
	std::string cur;

	for( auto &word : words )
	{
		std::string candidate = cur.empty() ? word : cur + " " + word;
		int w = 0, h = 0;
		gEngfuncs.pfnDrawConsoleStringLen( (char*)candidate.c_str(), &w, &h );
		if( w > maxWidth && !cur.empty() )
		{
			lines.push_back( cur );
			cur = word;
		}
		else
			cur = candidate;
	}
	if( !cur.empty() )
		lines.push_back( cur );
	return lines;
}


static std::map<std::string, SubtitleOutput>            s_subtitlesToDraw;
static std::map<std::string, SubtitleColor>             s_colors;
static std::map<std::string, std::vector<Subtitle>>     s_subtitleMap;

static cvar_t *s_cvSubtitles         = nullptr; // 0=off, 1=player only, 2=everyone
static cvar_t *s_cvLanguage          = nullptr;
static cvar_t *s_cvFontScale         = nullptr; // registered; visual hint only on xash
static cvar_t *s_cvLogCandidates     = nullptr;


static std::string GetKeyWithLanguage( const std::string &key )
{
	const char *lang = s_cvLanguage ? s_cvLanguage->string : "en";
	std::string langKey = Str_Upper( std::string( lang ) + "_" + key );
	if( s_subtitleMap.count( langKey ) )
		return langKey;
	return Str_Upper( "EN_" + key );
}

static bool IsFarFromPlayer( const SubtitleOutput &sub )
{
	if( sub.ignoreLongDistances )
		return false;
	cl_entity_t *pLocal = gEngfuncs.GetLocalPlayer();
	if( !pLocal ) return false;
	Vector delta = sub.pos - pLocal->origin;
	return delta.Length() >= 768.0f;
}

static const SubtitleColor &GetColor( const std::string &colorKey )
{
	static SubtitleColor white = { 1.0f, 1.0f, 1.0f };
	std::string key = Str_Upper( colorKey );
	auto it = s_colors.find( key );
	return it != s_colors.end() ? it->second : white;
}


void Subtitles_ParseSubtitles( const std::string &filePath, const std::string &language )
{
	std::ifstream inp( filePath );
	if( !inp.is_open() )
	{
		gEngfuncs.Con_DPrintf( "SUBTITLE PARSER: failed to open %s\n", filePath.c_str() );
		return;
	}

	std::string line;
	int lineNum = 0;

	while( std::getline( inp, line ) )
	{
		lineNum++;
		line = Str_Trim( line );
		if( line.empty() ) continue;
		if( line.find( "SUBTITLE" ) != 0 && line.find( "COLOR" ) != 0 )
			continue;

		auto parts = Str_Split( line, '|' );

		if( parts[0] == "SUBTITLE" )
		{
			if( (int)parts.size() < 6 )
			{
				gEngfuncs.Con_DPrintf( "SUBTITLE PARSER line %d in %s: not enough fields\n", lineNum, filePath.c_str() );
				continue;
			}
			std::string key = Str_Upper( language + "_" + parts[1] );
			char *endp = nullptr;
			float delay = strtof( parts[3].c_str(), &endp );
			if( endp == parts[3].c_str() )
			{
				gEngfuncs.Con_DPrintf( "SUBTITLE PARSER line %d in %s: bad delay\n", lineNum, filePath.c_str() );
				continue;
			}
			float duration = strtof( parts[4].c_str(), &endp );
			if( endp == parts[4].c_str() )
			{
				gEngfuncs.Con_DPrintf( "SUBTITLE PARSER line %d in %s: bad duration\n", lineNum, filePath.c_str() );
				continue;
			}
			s_subtitleMap[key].push_back( { delay, duration, parts[2], parts[5] } );
		}
		else if( parts[0] == "COLOR" )
		{
			if( (int)parts.size() < 5 ) continue;
			std::string colorKey = Str_Upper( parts[1] );
			char *ep = nullptr;
			float r = strtof( parts[2].c_str(), &ep );
			if( ep == parts[2].c_str() ) continue;
			float g = strtof( parts[3].c_str(), &ep );
			if( ep == parts[3].c_str() ) continue;
			float b = strtof( parts[4].c_str(), &ep );
			if( ep == parts[4].c_str() ) continue;
			s_colors[colorKey] = { r, g, b };
		}
	}
}


void Subtitles_Init()
{
	gEngfuncs.pfnHookUserMsg( "OnSound",   Subtitles_OnSound );
	gEngfuncs.pfnHookUserMsg( "SubtClear", Subtitles_SubtClear );
	gEngfuncs.pfnHookUserMsg( "SubtRemove", Subtitles_SubtRemove );

	// Register cvars
	s_cvSubtitles     = gEngfuncs.pfnRegisterVariable( "subtitles",             "2", FCVAR_ARCHIVE );
	s_cvLanguage      = gEngfuncs.pfnRegisterVariable( "subtitles_language",    "en", FCVAR_ARCHIVE );
	s_cvFontScale     = gEngfuncs.pfnRegisterVariable( "subtitles_font_scale",  "1", FCVAR_ARCHIVE );
	s_cvLogCandidates = gEngfuncs.pfnRegisterVariable( "subtitles_log_candidates", "0", 0 );

	std::vector<std::string> searchPaths;
	{
		const char *gameDir = gEngfuncs.pfnGetGameDirectory();
		if( gameDir && *gameDir )
			searchPaths.push_back( std::string( gameDir ) + "/resource" );
		searchPaths.push_back( "resource" );
	}

	std::regex rgx( "subtitles_(\\w+)\\.txt", std::regex_constants::icase );

	for( auto &dirPath : searchPaths )
	{
		DIR *dir = opendir( dirPath.c_str() );
		if( !dir ) continue;

		int loaded = 0;
		struct dirent *ent;
		while( ( ent = readdir( dir ) ) != nullptr )
		{
			std::string name = ent->d_name;
			std::smatch match;
			if( std::regex_match( name, match, rgx ) && match.size() > 1 )
			{
				std::string lang = match.str( 1 );
				Subtitles_ParseSubtitles( dirPath + "/" + name, lang );
				gEngfuncs.Con_DPrintf( "Subtitles: loaded %s (%s)\n", name.c_str(), lang.c_str() );
				loaded++;
			}
		}
		closedir( dir );

		if( loaded > 0 )
			break;
	}
}


static void Subtitles_PushEntry( const std::string &key, const std::string &text,
                                  float duration, const Vector &color,
                                  const Vector &pos, float delay, int ignoreLong )
{
	if( s_subtitlesToDraw.count( key ) ) return;

	int maxWidth = ScreenWidth / 2;
	auto lines   = Str_WordWrap( text, maxWidth );
	float now    = gEngfuncs.GetClientTime();

	for( size_t i = 0; i < lines.size(); i++ )
	{
		std::string actualKey = key + "_ln" + std::to_string( i );
		float startTime = now + delay + (float)i * 0.0f; // same start; each line is a sibling
		s_subtitlesToDraw[actualKey] = {
			startTime,
			startTime + duration,
			ignoreLong,
			color,
			pos,
			lines[i]
		};
	}
}

static void Subtitles_Push( const std::string &key, int ignoreLong, const Vector &pos )
{
	if( !s_cvSubtitles || s_cvSubtitles->value <= 0.0f )
		return;

	std::string actualKey = GetKeyWithLanguage( key );
	auto it = s_subtitleMap.find( actualKey );
	if( it == s_subtitleMap.end() ) return;

	const auto &subs = it->second;
	for( size_t i = 0; i < subs.size(); i++ )
	{
		const auto &sub = subs[i];

		bool isPlayerLine = ( Str_Upper( sub.colorKey ) == "PAYNE" );
		if( !isPlayerLine && s_cvSubtitles->value < 2.0f )
			continue;

		const auto &col = GetColor( sub.colorKey );
		Subtitles_PushEntry(
			actualKey + "_" + std::to_string( i ),
			sub.text,
			sub.duration,
			Vector( col.r, col.g, col.b ),
			pos,
			sub.delay,
			ignoreLong
		);
	}
}


void Subtitles_Draw()
{
	if( s_subtitlesToDraw.empty() ) return;

	float time = gEngfuncs.GetClientTime();

	// 1. Expire old entries and collect visible ones
	struct DrawEntry { std::string text; Vector color; };
	std::vector<DrawEntry> visible;

	auto it = s_subtitlesToDraw.begin();
	while( it != s_subtitlesToDraw.end() )
	{
		auto &sub = it->second;
		if( time >= sub.endTime )
		{
			it = s_subtitlesToDraw.erase( it );
			continue;
		}
		if( time >= sub.startTime && !IsFarFromPlayer( sub ) )
			visible.push_back( { sub.text, sub.color } );
		++it;
	}

	if( visible.empty() ) return;

	// 2. Measure – find widest line and per-line height
	int charW = 0, charH = 0;
	gEngfuncs.pfnDrawConsoleStringLen( "A", &charW, &charH );
	if( charH <= 0 ) charH = 12;

	int maxTextW = 0;
	for( auto &e : visible )
	{
		int w = 0, h = 0;
		gEngfuncs.pfnDrawConsoleStringLen( (char*)e.text.c_str(), &w, &h );
		if( w > maxTextW ) maxTextW = w;
	}

	int padding  = 6;
	int lineGap  = 2;
	int totalH   = (int)visible.size() * ( charH + lineGap ) - lineGap;
	int boxW     = maxTextW + padding * 2;
	int boxH     = totalH + padding * 2;

	// Position: horizontally centred, ~77 % down the screen
	int boxX = ( ScreenWidth  - boxW ) / 2;
	int boxY = (int)( ScreenHeight / 1.3f ) - boxH / 2;

	// 3. Semi-transparent dark background
	gEngfuncs.pfnFillRGBABlend( boxX, boxY, boxW, boxH, 0, 0, 0, 160 );

	// 4. Draw each subtitle line with its colour
	int drawY = boxY + padding;
	for( auto &e : visible )
	{
		int w = 0, h = 0;
		gEngfuncs.pfnDrawConsoleStringLen( (char*)e.text.c_str(), &w, &h );
		int drawX = ( ScreenWidth - w ) / 2;

		// Clamp colour channels to [0,1] and convert to [0,255]
		int r = (int)( e.color[0] * 255.0f );
		int g = (int)( e.color[1] * 255.0f );
		int b = (int)( e.color[2] * 255.0f );
		r = r < 0 ? 0 : ( r > 255 ? 255 : r );
		g = g < 0 ? 0 : ( g > 255 ? 255 : g );
		b = b < 0 ? 0 : ( b > 255 ? 255 : b );

		DrawSetTextColor( r / 255.0f, g / 255.0f, b / 255.0f );
		DrawConsoleString( drawX, drawY, e.text.c_str() );

		drawY += charH + lineGap;
	}
}


int Subtitles_OnSound( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	std::string key        = READ_STRING();
	int         ignoreLong = READ_BYTE();
	float       x          = READ_COORD();
	float       y          = READ_COORD();
	float       z          = READ_COORD();

	if( s_cvLogCandidates && s_cvLogCandidates->value >= 1.0f )
		gEngfuncs.Con_DPrintf( "SUBTITLE CANDIDATE: %s\n", key.c_str() );

	// Special case: Grunt cutscene player is far from the sound origin
	if( key.find( "!HG_DRAG" ) == 0 )
		ignoreLong = 1;

	Subtitles_Push( key, ignoreLong, Vector( x, y, z ) );
	return 1;
}

int Subtitles_SubtClear( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	s_subtitlesToDraw.clear();
	return 1;
}

int Subtitles_SubtRemove( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	std::string key     = READ_STRING();
	std::string langKey = GetKeyWithLanguage( key );

	auto it = s_subtitlesToDraw.begin();
	while( it != s_subtitlesToDraw.end() )
	{
		if( it->first.find( langKey ) == 0 )
			it = s_subtitlesToDraw.erase( it );
		else
			++it;
	}
	return 1;
}
