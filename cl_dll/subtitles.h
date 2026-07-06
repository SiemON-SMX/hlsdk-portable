#pragma once

#include "cl_dll.h"
#include <string>
#include <vector>

struct SubtitleColor
{
	float r, g, b;
};

struct Subtitle
{
	float delay;
	float duration;
	std::string colorKey;
	std::string text;
};

struct SubtitleOutput
{
	float startTime;           // absolute client time when to start showing
	float endTime;             // absolute client time when to stop
	int   ignoreLongDistances;
	Vector color;
	Vector pos;                // world origin of the speaker
	std::string text;
};

// Lifecycle
void Subtitles_Init();
void Subtitles_ParseSubtitles( const std::string &filePath, const std::string &language );
void Subtitles_Draw();

// Network message handlers (hooked in Subtitles_Init)
int Subtitles_OnSound  ( const char *pszName, int iSize, void *pbuf );
int Subtitles_SubtClear( const char *pszName, int iSize, void *pbuf );
int Subtitles_SubtRemove( const char *pszName, int iSize, void *pbuf );
