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
	float startTime;
	float endTime;
	int   ignoreLongDistances;
	Vector color;
	Vector pos;
	std::string text;
};

void Subtitles_Init();
void Subtitles_ParseSubtitles( const std::string &filePath, const std::string &language );
void Subtitles_Draw();

int Subtitles_OnSound  ( const char *pszName, int iSize, void *pbuf );
int Subtitles_SubtClear( const char *pszName, int iSize, void *pbuf );
int Subtitles_SubtRemove( const char *pszName, int iSize, void *pbuf );
