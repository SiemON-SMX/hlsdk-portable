#include "hud.h"
#include "cl_util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <strings.h>

#define WAVFIX_VERSION 2
#define WAVFIX_MAXRATE 65535
#define WAVFIX_OUTRATE 44100
#define WAVFIX_MARKER "custom/sound/.cof_wavfix"

struct WavFormat
{
	int tag;
	int channels;
	int rate;
	int bits;
	int blockalign;
};

static int g_iCoFWavFixed;
static int g_iCoFWavFailed;

static uint16_t RdU16( const unsigned char *p )
{
	return (uint16_t)( p[0] | ( p[1] << 8 ));
}

static uint32_t RdU32( const unsigned char *p )
{
	return (uint32_t)p[0] | ( (uint32_t)p[1] << 8 ) | ( (uint32_t)p[2] << 16 ) | ( (uint32_t)p[3] << 24 );
}

static void WrU16( unsigned char *p, uint16_t v )
{
	p[0] = v & 0xFF;
	p[1] = ( v >> 8 ) & 0xFF;
}

static void WrU32( unsigned char *p, uint32_t v )
{
	p[0] = v & 0xFF;
	p[1] = ( v >> 8 ) & 0xFF;
	p[2] = ( v >> 16 ) & 0xFF;
	p[3] = ( v >> 24 ) & 0xFF;
}

static bool ParseFmt( const unsigned char *fmt, uint32_t len, WavFormat *out )
{
	if( len < 16 )
		return false;

	out->tag = RdU16( fmt );
	out->channels = RdU16( fmt + 2 );
	out->rate = (int)RdU32( fmt + 4 );
	out->blockalign = RdU16( fmt + 12 );
	out->bits = RdU16( fmt + 14 );

	if( out->tag == 0xFFFE && len >= 26 )
		out->tag = RdU16( fmt + 24 );

	return true;
}

static bool EngineCanLoad( const WavFormat *f )
{
	if( f->tag == 85 )
		return true;

	return f->tag == 1 && ( f->channels == 1 || f->channels == 2 ) && ( f->bits == 8 || f->bits == 16 ) && f->rate > 0 && f->rate <= WAVFIX_MAXRATE;
}

static bool CanConvert( const WavFormat *f )
{
	if( f->channels < 1 || f->channels > 8 || f->rate <= 0 )
		return false;

	if( f->tag == 1 )
		return f->bits == 8 || f->bits == 16 || f->bits == 24 || f->bits == 32;

	if( f->tag == 3 )
		return f->bits == 32 || f->bits == 64;

	return false;
}

static bool ReadHeaderFormat( const char *path, WavFormat *out )
{
	FILE *fp = fopen( path, "rb" );
	unsigned char hdr[12];
	bool found = false;

	if( !fp )
		return false;

	if( fread( hdr, 1, 12, fp ) != 12 || memcmp( hdr, "RIFF", 4 ) || memcmp( hdr + 8, "WAVE", 4 ))
	{
		fclose( fp );
		return false;
	}

	for( int guard = 0; guard < 64; guard++ )
	{
		unsigned char ch[8];

		if( fread( ch, 1, 8, fp ) != 8 )
			break;

		uint32_t len = RdU32( ch + 4 );

		if( !memcmp( ch, "fmt ", 4 ))
		{
			unsigned char fmt[64];
			uint32_t want = len < sizeof( fmt ) ? len : sizeof( fmt );

			if( fread( fmt, 1, want, fp ) == want )
				found = ParseFmt( fmt, want, out );
			break;
		}

		if( fseek( fp, (long)( len + ( len & 1 )), SEEK_CUR ))
			break;
	}

	fclose( fp );
	return found;
}

static bool MakeDirs( const char *path )
{
	char buf[1024];
	size_t n = strlen( path );

	if( n >= sizeof( buf ))
		return false;

	memcpy( buf, path, n + 1 );

	for( size_t i = 1; i < n; i++ )
	{
		if( buf[i] == '/' )
		{
			buf[i] = 0;
			mkdir( buf, 0777 );
			buf[i] = '/';
		}
	}

	return true;
}

static float ReadSample( const unsigned char *p, const WavFormat *f )
{
	if( f->tag == 3 )
	{
		if( f->bits == 32 )
		{
			uint32_t u = RdU32( p );
			float v;
			memcpy( &v, &u, sizeof( v ));
			return v;
		}

		uint64_t u = (uint64_t)RdU32( p ) | ( (uint64_t)RdU32( p + 4 ) << 32 );
		double v;
		memcpy( &v, &u, sizeof( v ));
		return (float)v;
	}

	switch( f->bits )
	{
	case 8:
		return ( (int)p[0] - 128 ) / 128.0f;
	case 16:
		return (int16_t)RdU16( p ) / 32768.0f;
	case 24:
	{
		int32_t v = (int32_t)( ( (uint32_t)p[0] << 8 ) | ( (uint32_t)p[1] << 16 ) | ( (uint32_t)p[2] << 24 ));
		return ( v >> 8 ) / 8388608.0f;
	}
	case 32:
		return (int32_t)RdU32( p ) / 2147483648.0f;
	}

	return 0.0f;
}

static int16_t ToS16( float v )
{
	int s = (int)( v * 32767.0f + ( v >= 0.0f ? 0.5f : -0.5f ));

	if( s > 32767 )
		s = 32767;
	if( s < -32768 )
		s = -32768;

	return (int16_t)s;
}

// xash only loads 8/16 bit pcm and its rate field is 16 bit, so 24/32 bit and 96k wavs sound slow
// convert them to 16 bit / 44100 and write the copy to custom/sound
static bool ConvertWav( const char *src, const char *dst )
{
	FILE *fp = fopen( src, "rb" );

	if( !fp )
		return false;

	fseek( fp, 0, SEEK_END );
	long size = ftell( fp );
	fseek( fp, 0, SEEK_SET );

	if( size < 44 )
	{
		fclose( fp );
		return false;
	}

	unsigned char *in = (unsigned char *)malloc( size );

	if( !in || fread( in, 1, size, fp ) != (size_t)size )
	{
		free( in );
		fclose( fp );
		return false;
	}

	fclose( fp );

	WavFormat f;
	bool havefmt = false;
	const unsigned char *data = NULL;
	uint32_t datalen = 0;
	size_t extralen = 0;

	for( long pos = 12; pos + 8 <= size; )
	{
		uint32_t len = RdU32( in + pos + 4 );
		long body = pos + 8;
		uint32_t avail = (uint32_t)( size - body ) < len ? (uint32_t)( size - body ) : len;

		if( !memcmp( in + pos, "fmt ", 4 ))
			havefmt = ParseFmt( in + body, avail, &f );
		else if( !memcmp( in + pos, "data", 4 ))
		{
			data = in + body;
			datalen = avail;
		}
		else if( memcmp( in + pos, "fact", 4 ))
			extralen += 8 + avail + ( avail & 1 );

		pos = body + (long)len + ( len & 1 );
	}

	if( !havefmt || !data || !CanConvert( &f ))
	{
		free( in );
		return false;
	}

	int inbytes = f.bits / 8;
	int frame = f.blockalign >= inbytes * f.channels ? f.blockalign : inbytes * f.channels;
	uint32_t frames = datalen / frame;
	int outch = f.channels > 2 ? 2 : f.channels;
	int outrate = f.rate > WAVFIX_MAXRATE ? WAVFIX_OUTRATE : f.rate;
	double ratio = (double)outrate / (double)f.rate;
	uint32_t outframes = outrate == f.rate ? frames : (uint32_t)( frames * ratio );
	float *mix = (float *)malloc( sizeof( float ) * ( frames ? frames : 1 ) * outch );

	if( !mix || !outframes )
	{
		free( mix );
		free( in );
		return false;
	}

	for( uint32_t i = 0; i < frames; i++ )
	{
		const unsigned char *fr = data + (size_t)i * frame;

		if( f.channels <= 2 )
		{
			for( int c = 0; c < f.channels; c++ )
				mix[i * outch + c] = ReadSample( fr + c * inbytes, &f );
		}
		else
		{
			float l = 0.0f, r = 0.0f;
			int nl = 0, nr = 0;

			for( int c = 0; c < f.channels; c++ )
			{
				float v = ReadSample( fr + c * inbytes, &f );

				if( c & 1 )
				{
					r += v;
					nr++;
				}
				else
				{
					l += v;
					nl++;
				}
			}

			mix[i * 2] = nl ? l / nl : 0.0f;
			mix[i * 2 + 1] = nr ? r / nr : 0.0f;
		}
	}

	uint32_t outdata = outframes * outch * 2;
	size_t total = 12 + 8 + 16 + extralen + 8 + outdata + ( outdata & 1 );
	unsigned char *out = (unsigned char *)malloc( total );

	if( !out )
	{
		free( mix );
		free( in );
		return false;
	}

	unsigned char *w = out;
	memcpy( w, "RIFF", 4 );
	WrU32( w + 4, (uint32_t)( total - 8 ));
	memcpy( w + 8, "WAVE", 4 );
	w += 12;

	memcpy( w, "fmt ", 4 );
	WrU32( w + 4, 16 );
	WrU16( w + 8, 1 );
	WrU16( w + 10, (uint16_t)outch );
	WrU32( w + 12, (uint32_t)outrate );
	WrU32( w + 16, (uint32_t)( outrate * outch * 2 ));
	WrU16( w + 20, (uint16_t)( outch * 2 ));
	WrU16( w + 22, 16 );
	w += 24;

	for( long pos = 12; pos + 8 <= size; )
	{
		uint32_t len = RdU32( in + pos + 4 );
		long body = pos + 8;
		uint32_t avail = (uint32_t)( size - body ) < len ? (uint32_t)( size - body ) : len;

		if( memcmp( in + pos, "fmt ", 4 ) && memcmp( in + pos, "data", 4 ) && memcmp( in + pos, "fact", 4 ))
		{
			unsigned char *chunk = w;

			memcpy( w, in + pos, 4 );
			WrU32( w + 4, avail );
			memcpy( w + 8, in + body, avail );
			w += 8 + avail;
			if( avail & 1 )
				*w++ = 0;

			if( outrate != f.rate )
			{
				unsigned char *cb = chunk + 8;

				if( !memcmp( chunk, "cue ", 4 ) && avail >= 4 )
				{
					uint32_t count = RdU32( cb );

					for( uint32_t k = 0; k < count && 4 + ( k + 1 ) * 24 <= avail; k++ )
					{
						unsigned char *pt = cb + 4 + k * 24;
						WrU32( pt + 4, (uint32_t)( RdU32( pt + 4 ) * ratio ));
						WrU32( pt + 20, (uint32_t)( RdU32( pt + 20 ) * ratio ));
					}
				}
				else if( !memcmp( chunk, "LIST", 4 ) && avail >= 4 && !memcmp( cb, "adtl", 4 ))
				{
					for( uint32_t q = 4; q + 8 <= avail; )
					{
						uint32_t sl = RdU32( cb + q + 4 );

						if( !memcmp( cb + q, "ltxt", 4 ) && sl >= 8 && q + 16 <= avail )
							WrU32( cb + q + 12, (uint32_t)( RdU32( cb + q + 12 ) * ratio ));

						q += 8 + sl + ( sl & 1 );
					}
				}
			}
		}

		pos = body + (long)len + ( len & 1 );
	}

	memcpy( w, "data", 4 );
	WrU32( w + 4, outdata );
	w += 8;

	for( uint32_t i = 0; i < outframes; i++ )
	{
		for( int c = 0; c < outch; c++ )
		{
			float v;

			if( outrate == f.rate )
				v = mix[i * outch + c];
			else
			{
				double start = i / ratio;
				double end = ( i + 1 ) / ratio;
				uint32_t s0 = (uint32_t)start;
				uint32_t s1 = (uint32_t)end;
				double acc = 0.0, wsum = 0.0;

				if( s1 >= frames )
					s1 = frames - 1;

				for( uint32_t k = s0; k <= s1; k++ )
				{
					double lo = k > start ? k : start;
					double hi = ( k + 1 ) < end ? ( k + 1 ) : end;
					double wt = hi - lo;

					if( wt <= 0.0 )
						continue;

					acc += mix[k * outch + c] * wt;
					wsum += wt;
				}

				v = wsum > 0.0 ? (float)( acc / wsum ) : 0.0f;
			}

			WrU16( w, (uint16_t)ToS16( v ));
			w += 2;
		}
	}

	free( mix );

	if( outdata & 1 )
		*w++ = 0;

	free( in );

	char tmp[2112];
	snprintf( tmp, sizeof( tmp ), "%s.tmp", dst );
	MakeDirs( dst );

	FILE *op = fopen( tmp, "wb" );
	bool ok = false;

	if( op )
	{
		ok = fwrite( out, 1, (size_t)( w - out ), op ) == (size_t)( w - out );
		ok = ( fclose( op ) == 0 ) && ok;

		if( ok )
		{
			remove( dst );
			ok = rename( tmp, dst ) == 0;
		}

		if( !ok )
			remove( tmp );
	}

	free( out );
	return ok;
}

static bool IsWav( const char *name )
{
	size_t n = strlen( name );
	return n > 4 && !strcasecmp( name + n - 4, ".wav" );
}

static void ScanSoundDir( const char *gamedir, const char *rel, int depth )
{
	char path[2048];
	snprintf( path, sizeof( path ), "%s/%s", gamedir, rel );

	DIR *d = opendir( path );

	if( !d )
		return;

	struct dirent *e;

	while(( e = readdir( d )) != NULL )
	{
		if( e->d_name[0] == '.' )
			continue;

		char childrel[1536];
		char child[2048];
		snprintf( childrel, sizeof( childrel ), "%s/%s", rel, e->d_name );
		snprintf( child, sizeof( child ), "%s/%s", gamedir, childrel );

		struct stat st;

		if( stat( child, &st ))
			continue;

		if( S_ISDIR( st.st_mode ))
		{
			if( depth < 16 )
				ScanSoundDir( gamedir, childrel, depth + 1 );
			continue;
		}

		if( !IsWav( e->d_name ))
			continue;

		WavFormat f;

		if( !ReadHeaderFormat( child, &f ) || EngineCanLoad( &f ) || !CanConvert( &f ))
			continue;

		char dst[2048];
		snprintf( dst, sizeof( dst ), "%s/custom/%s", gamedir, childrel );

		struct stat dst_st;
		WavFormat df;

		if( !stat( dst, &dst_st ) && dst_st.st_size > 0 && dst_st.st_mtime >= st.st_mtime
			&& ReadHeaderFormat( dst, &df ) && EngineCanLoad( &df ))
			continue;

		if( ConvertWav( child, dst ))
			g_iCoFWavFixed++;
		else
			g_iCoFWavFailed++;
	}

	closedir( d );
}

static void RunWavFix( void )
{
	const char *gamedir = gEngfuncs.pfnGetGameDirectory();

	if( !gamedir || !*gamedir )
		return;

	g_iCoFWavFixed = g_iCoFWavFailed = 0;
	ScanSoundDir( gamedir, "sound", 0 );

	char marker[1024];
	snprintf( marker, sizeof( marker ), "%s/" WAVFIX_MARKER, gamedir );
	MakeDirs( marker );

	FILE *fp = fopen( marker, "wb" );

	if( fp )
	{
		fprintf( fp, "%d\n", WAVFIX_VERSION );
		fclose( fp );
	}

	gEngfuncs.Con_Printf( "Converted %d WAV files to 16 bit PCM (%d failed)\n", g_iCoFWavFixed, g_iCoFWavFailed );
}

static void FixSounds_f( void )
{
	RunWavFix();
}

// runs once, the marker file in custom/sound stops it from running again
void FixUnsupportedWavs( void )
{
	gEngfuncs.pfnAddCommand( "cof_fixsounds", FixSounds_f );

	const char *gamedir = gEngfuncs.pfnGetGameDirectory();

	if( !gamedir || !*gamedir )
		return;

	char marker[1024];
	snprintf( marker, sizeof( marker ), "%s/" WAVFIX_MARKER, gamedir );

	FILE *fp = fopen( marker, "rb" );

	if( fp )
	{
		int version = 0;
		bool current = fscanf( fp, "%d", &version ) == 1 && version == WAVFIX_VERSION;
		fclose( fp );

		if( current )
			return;
	}

	RunWavFix();
}
