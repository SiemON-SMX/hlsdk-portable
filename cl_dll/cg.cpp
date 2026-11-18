#include "hud.h"
#include "cl_util.h"
#include "cgcontext.h"

#if _WIN32
#define HSPRITE HSPRITE_win32
#include <windows.h>
#undef HSPRITE
#define CG_LIB		"cg.dll"
#define CGGL_LIB	"cgGL.dll"
#define CGCALL		__cdecl
static void *CgOpen( const char *name ) { return (void *)LoadLibraryA( name ); }
static void *CgSym( void *lib, const char *name ) { return (void *)GetProcAddress( (HMODULE)lib, name ); }
#elif defined(__ANDROID__) || defined(__APPLE__)
#define CG_GLSL 1
#else
#include <dlfcn.h>
#define CG_LIB		"libCg.so"
#define CGGL_LIB	"libCgGL.so"
#define CGCALL
static void *CgOpen( const char *name ) { return dlopen( name, RTLD_NOW ); }
static void *CgSym( void *lib, const char *name ) { return dlsym( lib, name ); }
#endif

#if !defined( CG_GLSL )
typedef CGcontext ( CGCALL *pfnCgCreateContext )( void );
typedef CGprogram ( CGCALL *pfnCgCreateProgramFromFile )( CGcontext, CGenum, const char *, CGprofile, const char *, const char ** );
typedef CGerror ( CGCALL *pfnCgGetError )( void );
typedef const char *( CGCALL *pfnCgGetErrorString )( CGerror );
typedef CGparameter ( CGCALL *pfnCgGetNamedParameter )( CGprogram, const char * );
typedef CGprofile ( CGCALL *pfnCgGLGetLatestProfile )( CGGLenum );
typedef void ( CGCALL *pfnCgGLSetOptimalOptions )( CGprofile );
typedef void ( CGCALL *pfnCgGLLoadProgram )( CGprogram );
typedef void ( CGCALL *pfnCgGLEnableProfile )( CGprofile );
typedef void ( CGCALL *pfnCgGLDisableProfile )( CGprofile );
typedef void ( CGCALL *pfnCgGLBindProgram )( CGprogram );
typedef void ( CGCALL *pfnCgGLSetParameter1f )( CGparameter, float );
typedef void ( CGCALL *pfnCgGLSetParameter4fv )( CGparameter, const float * );
typedef void ( CGCALL *pfnCgGLSetStateMatrixParameter )( CGparameter, CGGLenum, CGGLenum );

static struct
{
	int loaded;
	void *cg;
	void *cggl;
	pfnCgCreateContext CreateContext;
	pfnCgCreateProgramFromFile CreateProgramFromFile;
	pfnCgGetError GetError;
	pfnCgGetErrorString GetErrorString;
	pfnCgGetNamedParameter GetNamedParameter;
	pfnCgGLGetLatestProfile GLGetLatestProfile;
	pfnCgGLSetOptimalOptions GLSetOptimalOptions;
	pfnCgGLLoadProgram GLLoadProgram;
	pfnCgGLEnableProfile GLEnableProfile;
	pfnCgGLDisableProfile GLDisableProfile;
	pfnCgGLBindProgram GLBindProgram;
	pfnCgGLSetParameter1f GLSetParameter1f;
	pfnCgGLSetParameter4fv GLSetParameter4fv;
	pfnCgGLSetStateMatrixParameter GLSetStateMatrixParameter;
} g_Cg;

static int LoadCg( void )
{
	if( g_Cg.loaded )
		return g_Cg.cg && g_Cg.cggl;

	g_Cg.loaded = 1;

	if( !CG_LIB[0] )
		return 0;

	g_Cg.cg = CgOpen( CG_LIB );
	g_Cg.cggl = CgOpen( CGGL_LIB );

	if( !g_Cg.cg || !g_Cg.cggl )
	{
		gEngfuncs.Con_Printf( "Cg runtime not found, Cg water is disabled\n" );
		g_Cg.cg = g_Cg.cggl = NULL;
		return 0;
	}

	g_Cg.CreateContext = (pfnCgCreateContext)CgSym( g_Cg.cg, "cgCreateContext" );
	g_Cg.CreateProgramFromFile = (pfnCgCreateProgramFromFile)CgSym( g_Cg.cg, "cgCreateProgramFromFile" );
	g_Cg.GetError = (pfnCgGetError)CgSym( g_Cg.cg, "cgGetError" );
	g_Cg.GetErrorString = (pfnCgGetErrorString)CgSym( g_Cg.cg, "cgGetErrorString" );
	g_Cg.GetNamedParameter = (pfnCgGetNamedParameter)CgSym( g_Cg.cg, "cgGetNamedParameter" );
	g_Cg.GLGetLatestProfile = (pfnCgGLGetLatestProfile)CgSym( g_Cg.cggl, "cgGLGetLatestProfile" );
	g_Cg.GLSetOptimalOptions = (pfnCgGLSetOptimalOptions)CgSym( g_Cg.cggl, "cgGLSetOptimalOptions" );
	g_Cg.GLLoadProgram = (pfnCgGLLoadProgram)CgSym( g_Cg.cggl, "cgGLLoadProgram" );
	g_Cg.GLEnableProfile = (pfnCgGLEnableProfile)CgSym( g_Cg.cggl, "cgGLEnableProfile" );
	g_Cg.GLDisableProfile = (pfnCgGLDisableProfile)CgSym( g_Cg.cggl, "cgGLDisableProfile" );
	g_Cg.GLBindProgram = (pfnCgGLBindProgram)CgSym( g_Cg.cggl, "cgGLBindProgram" );
	g_Cg.GLSetParameter1f = (pfnCgGLSetParameter1f)CgSym( g_Cg.cggl, "cgGLSetParameter1f" );
	g_Cg.GLSetParameter4fv = (pfnCgGLSetParameter4fv)CgSym( g_Cg.cggl, "cgGLSetParameter4fv" );
	g_Cg.GLSetStateMatrixParameter = (pfnCgGLSetStateMatrixParameter)CgSym( g_Cg.cggl, "cgGLSetStateMatrixParameter" );

	if( !g_Cg.CreateContext || !g_Cg.CreateProgramFromFile || !g_Cg.GetError || !g_Cg.GetErrorString || !g_Cg.GetNamedParameter
		|| !g_Cg.GLGetLatestProfile || !g_Cg.GLSetOptimalOptions || !g_Cg.GLLoadProgram || !g_Cg.GLEnableProfile
		|| !g_Cg.GLDisableProfile || !g_Cg.GLBindProgram || !g_Cg.GLSetParameter1f || !g_Cg.GLSetParameter4fv || !g_Cg.GLSetStateMatrixParameter )
	{
		gEngfuncs.Con_Printf( "Cg runtime is incomplete, Cg water is disabled\n" );
		g_Cg.cg = g_Cg.cggl = NULL;
		return 0;
	}

	return 1;
}

bool CgAvailable( void )
{
	return LoadCg() ? true : false;
}

CGcontext cgCreateContext( void )
{
	if( !LoadCg() )
		return NULL;
	return g_Cg.CreateContext();
}

CGprogram cgCreateProgramFromFile( CGcontext ctx, CGenum program_type, const char *program_file, CGprofile profile, const char *entry, const char **args )
{
	if( !LoadCg() || !ctx )
		return NULL;
	return g_Cg.CreateProgramFromFile( ctx, program_type, program_file, profile, entry, args );
}

CGerror cgGetError( void )
{
	if( !LoadCg() )
		return 1;
	return g_Cg.GetError();
}

const char *cgGetErrorString( CGerror error )
{
	if( !LoadCg() )
		return "Cg runtime is not available";
	return g_Cg.GetErrorString( error );
}

CGparameter cgGetNamedParameter( CGprogram prog, const char *name )
{
	if( !LoadCg() || !prog )
		return NULL;
	return g_Cg.GetNamedParameter( prog, name );
}

CGprofile cgGLGetLatestProfile( CGGLenum profile_type )
{
	if( !LoadCg() )
		return CG_PROFILE_UNKNOWN;
	return g_Cg.GLGetLatestProfile( profile_type );
}

void cgGLSetOptimalOptions( CGprofile profile )
{
	if( LoadCg() )
		g_Cg.GLSetOptimalOptions( profile );
}

void cgGLLoadProgram( CGprogram program )
{
	if( LoadCg() && program )
		g_Cg.GLLoadProgram( program );
}

void cgGLEnableProfile( CGprofile profile )
{
	if( LoadCg() )
		g_Cg.GLEnableProfile( profile );
}

void cgGLDisableProfile( CGprofile profile )
{
	if( LoadCg() )
		g_Cg.GLDisableProfile( profile );
}

void cgGLBindProgram( CGprogram program )
{
	if( LoadCg() && program )
		g_Cg.GLBindProgram( program );
}

void cgGLSetParameter1f( CGparameter param, float x )
{
	if( LoadCg() && param )
		g_Cg.GLSetParameter1f( param, x );
}

void cgGLSetParameter4fv( CGparameter param, const float *v )
{
	if( LoadCg() && param )
		g_Cg.GLSetParameter4fv( param, v );
}

void cgGLSetStateMatrixParameter( CGparameter param, CGGLenum matrix, CGGLenum transform )
{
	if( LoadCg() && param )
		g_Cg.GLSetStateMatrixParameter( param, matrix, transform );
}

#else
// no Cg runtime here, so the cgshaders are rewritten as GLSL and run through gl4es

#include <string.h>
#include "glmanager.h"
#include "xash_render.h"

#ifndef GL_MODELVIEW_MATRIX
#define GL_MODELVIEW_MATRIX	0x0BA6
#define GL_PROJECTION_MATRIX	0x0BA7
#endif

#ifndef GL_FRAGMENT_SHADER
#define GL_FRAGMENT_SHADER	0x8B30
#define GL_VERTEX_SHADER	0x8B31
#define GL_COMPILE_STATUS	0x8B81
#define GL_LINK_STATUS		0x8B82
#endif

typedef GLuint ( APIENTRY *pfnGlCreateShader )( GLenum );
typedef void ( APIENTRY *pfnGlShaderSource )( GLuint, GLsizei, const char **, const GLint * );
typedef void ( APIENTRY *pfnGlCompileShader )( GLuint );
typedef void ( APIENTRY *pfnGlGetShaderiv )( GLuint, GLenum, GLint * );
typedef void ( APIENTRY *pfnGlGetInfoLog )( GLuint, GLsizei, GLsizei *, char * );
typedef GLuint ( APIENTRY *pfnGlCreateProgram )( void );
typedef void ( APIENTRY *pfnGlAttachShader )( GLuint, GLuint );
typedef void ( APIENTRY *pfnGlLinkProgram )( GLuint );
typedef void ( APIENTRY *pfnGlUseProgram )( GLuint );
typedef GLint ( APIENTRY *pfnGlGetUniformLocation )( GLuint, const char * );
typedef void ( APIENTRY *pfnGlUniform1i )( GLint, GLint );
typedef void ( APIENTRY *pfnGlUniform1f )( GLint, GLfloat );
typedef void ( APIENTRY *pfnGlUniform4fv )( GLint, GLsizei, const GLfloat * );
typedef void ( APIENTRY *pfnGlUniformMatrix4fv )( GLint, GLsizei, GLboolean, const GLfloat * );

static struct
{
	int loaded;
	int ok;
	pfnGlCreateShader CreateShader;
	pfnGlShaderSource ShaderSource;
	pfnGlCompileShader CompileShader;
	pfnGlGetShaderiv GetShaderiv;
	pfnGlGetInfoLog GetShaderInfoLog;
	pfnGlCreateProgram CreateProgram;
	pfnGlAttachShader AttachShader;
	pfnGlLinkProgram LinkProgram;
	pfnGlGetShaderiv GetProgramiv;
	pfnGlGetInfoLog GetProgramInfoLog;
	pfnGlUseProgram UseProgram;
	pfnGlGetUniformLocation GetUniformLocation;
	pfnGlUniform1i Uniform1i;
	pfnGlUniform1f Uniform1f;
	pfnGlUniform4fv Uniform4fv;
	pfnGlUniformMatrix4fv UniformMatrix4fv;
} g_Sl;

static const char *s_szWaterVP =
	"uniform mat4 ModelViewProj;\n"
	"uniform float flScreen;\n"
	"varying vec4 flTexCoord0;\n"
	"varying vec4 flTexCoord1;\n"
	"varying vec4 flTexCoord2;\n"
	"varying vec4 flTexCoord3;\n"
	"void main()\n"
	"{\n"
	"	flTexCoord0 = gl_MultiTexCoord0;\n"
	"	flTexCoord1 = gl_MultiTexCoord0;\n"
	"	flTexCoord2 = gl_MultiTexCoord0;\n"
	"	flTexCoord2.y = gl_MultiTexCoord0.y + flScreen;\n"
	"	flTexCoord3 = gl_MultiTexCoord0;\n"
	"	gl_Position = ModelViewProj * gl_Vertex;\n"
	"}\n";

static const char *s_szWaterFP =
	"uniform sampler2D OffsetTex;\n"
	"uniform sampler2D Identity;\n"
	"uniform sampler2D DecalTex;\n"
	"uniform sampler2D MaskTex;\n"
	"uniform vec4 TexOffset;\n"
	"varying vec4 flTexCoord0;\n"
	"varying vec4 flTexCoord1;\n"
	"varying vec4 flTexCoord2;\n"
	"varying vec4 flTexCoord3;\n"
	"void main()\n"
	"{\n"
	"	vec4 flColor = texture2D( OffsetTex, flTexCoord0.xy );\n"
	"	vec2 flTex = flTexCoord1.xy + TexOffset.xy * flColor.xx + TexOffset.zw * flColor.yy;\n"
	"	vec4 outColor = texture2D( DecalTex, flTex.xy );\n"
	"	outColor.a = 1.0;\n"
	"	float flMask = texture2D( MaskTex, flTexCoord3.xy ).r;\n"
	"	gl_FragColor = outColor * flMask;\n"
	"}\n";

static const char *s_szBlackVP =
	"uniform mat4 ModelViewProj;\n"
	"varying vec4 flTexCoord0;\n"
	"void main()\n"
	"{\n"
	"	flTexCoord0 = gl_MultiTexCoord0;\n"
	"	gl_Position = ModelViewProj * gl_Vertex;\n"
	"}\n";

static const char *s_szBlackFP =
	"uniform sampler2D DecalTex;\n"
	"varying vec4 flTexCoord0;\n"
	"void main()\n"
	"{\n"
	"	vec4 flColor = texture2D( DecalTex, flTexCoord0.xy );\n"
	"	float flColor1 = ( flColor.r + flColor.g + flColor.b ) / 0.5;\n"
	"	flColor1 = 2.0 * ( flColor1 - 0.5 );\n"
	"	gl_FragColor = vec4( vec3( 0.2 - flColor1 ), 1.0 );\n"
	"}\n";

// the TEXUNITn bindings from the .cg files
static const char *s_szWaterSamplers[] = { "OffsetTex", "Identity", "DecalTex", "MaskTex", NULL };
static const char *s_szBlackSamplers[] = { "DecalTex", NULL };

#define CG_MAX_PARAMS 4

struct _CGparameter
{
	struct _CGprogram *prog;
	char name[32];
	int count;
	float value[16];
};

struct _CGprogram
{
	int vertex;
	GLuint shader;
	const char **samplers;
	int numparams;
	struct _CGparameter params[CG_MAX_PARAMS];
};

struct cglinked_t
{
	struct _CGprogram *vp;
	struct _CGprogram *fp;
	GLuint program;
	cglinked_t *next;
};

static struct _CGcontext { int dummy; } s_CgContext;
static cglinked_t *s_pLinked;
static cglinked_t *s_pCurrent;
static struct _CGprogram *s_pBoundVP;
static struct _CGprogram *s_pBoundFP;
static bool s_bVertexOn;
static bool s_bFragmentOn;
static CGerror s_iError;
static struct _CGprogram *s_pCompiled[4];

template <typename T>
static void SlProc( T &pfn, const char *name, const char *alt )
{
	pfn = (T)GL_GetEngineProcAddress( name );
	if( !pfn )
		pfn = (T)GL_GetEngineProcAddress( alt );
	if( !pfn )
		g_Sl.ok = 0;
}

static int LoadSl( void )
{
	if( g_Sl.loaded )
		return g_Sl.ok;

	g_Sl.loaded = 1;
	g_Sl.ok = 1;

	SlProc( g_Sl.CreateShader, "glCreateShader", "glCreateShaderEXT" );
	SlProc( g_Sl.ShaderSource, "glShaderSource", "glShaderSourceEXT" );
	SlProc( g_Sl.CompileShader, "glCompileShader", "glCompileShaderEXT" );
	SlProc( g_Sl.GetShaderiv, "glGetShaderiv", "glGetShaderivEXT" );
	SlProc( g_Sl.GetShaderInfoLog, "glGetShaderInfoLog", "glGetShaderInfoLogEXT" );
	SlProc( g_Sl.CreateProgram, "glCreateProgram", "glCreateProgramEXT" );
	SlProc( g_Sl.AttachShader, "glAttachShader", "glAttachShaderEXT" );
	SlProc( g_Sl.LinkProgram, "glLinkProgram", "glLinkProgramEXT" );
	SlProc( g_Sl.GetProgramiv, "glGetProgramiv", "glGetProgramivEXT" );
	SlProc( g_Sl.GetProgramInfoLog, "glGetProgramInfoLog", "glGetProgramInfoLogEXT" );
	SlProc( g_Sl.UseProgram, "glUseProgram", "glUseProgramEXT" );
	SlProc( g_Sl.GetUniformLocation, "glGetUniformLocation", "glGetUniformLocationEXT" );
	SlProc( g_Sl.Uniform1i, "glUniform1i", "glUniform1iEXT" );
	SlProc( g_Sl.Uniform1f, "glUniform1f", "glUniform1fEXT" );
	SlProc( g_Sl.Uniform4fv, "glUniform4fv", "glUniform4fvEXT" );
	SlProc( g_Sl.UniformMatrix4fv, "glUniformMatrix4fv", "glUniformMatrix4fvEXT" );

	if( !g_Sl.ok )
		gEngfuncs.Con_Printf( "GLSL is not available, Cg water is disabled\n" );

	return g_Sl.ok;
}

static void ApplyParams( GLuint program, struct _CGprogram *prog )
{
	for( int i = 0; i < prog->numparams; i++ )
	{
		struct _CGparameter *p = &prog->params[i];
		GLint loc;

		if( !p->count )
			continue;

		loc = g_Sl.GetUniformLocation( program, p->name );
		if( loc < 0 )
			continue;

		if( p->count == 1 )
			g_Sl.Uniform1f( loc, p->value[0] );
		else if( p->count == 4 )
			g_Sl.Uniform4fv( loc, 1, p->value );
		else
			g_Sl.UniformMatrix4fv( loc, 1, GL_FALSE, p->value );
	}
}

static cglinked_t *GetLinked( struct _CGprogram *vp, struct _CGprogram *fp )
{
	cglinked_t *l;
	GLint status = 0;
	char log[512];

	for( l = s_pLinked; l; l = l->next )
	{
		if( l->vp == vp && l->fp == fp )
			return l->program ? l : NULL;
	}

	l = new cglinked_t;
	l->vp = vp;
	l->fp = fp;
	l->program = g_Sl.CreateProgram();
	l->next = s_pLinked;
	s_pLinked = l;

	g_Sl.AttachShader( l->program, vp->shader );
	g_Sl.AttachShader( l->program, fp->shader );
	g_Sl.LinkProgram( l->program );
	g_Sl.GetProgramiv( l->program, GL_LINK_STATUS, &status );

	if( !status )
	{
		log[0] = 0;
		g_Sl.GetProgramInfoLog( l->program, sizeof( log ), NULL, log );
		gEngfuncs.Con_Printf( "Cg water link failed: %s\n", log );
		l->program = 0;
		return NULL;
	}

	g_Sl.UseProgram( l->program );
	for( int i = 0; fp->samplers[i]; i++ )
	{
		GLint loc = g_Sl.GetUniformLocation( l->program, fp->samplers[i] );
		if( loc >= 0 )
			g_Sl.Uniform1i( loc, i );
	}

	return l;
}

static void UpdateProgram( void )
{
	cglinked_t *l = NULL;

	if( s_bVertexOn && s_bFragmentOn && s_pBoundVP && s_pBoundFP )
		l = GetLinked( s_pBoundVP, s_pBoundFP );

	if( !l )
	{
		if( s_pCurrent )
			g_Sl.UseProgram( 0 );
		s_pCurrent = NULL;
		return;
	}

	g_Sl.UseProgram( l->program );
	s_pCurrent = l;
	ApplyParams( l->program, l->vp );
	ApplyParams( l->program, l->fp );
}

static void SetParam( CGparameter param, const float *v, int count )
{
	param->count = count;
	memcpy( param->value, v, count * sizeof( float ) );

	if( !s_pCurrent || ( s_pCurrent->vp != param->prog && s_pCurrent->fp != param->prog ) )
		return;

	GLint loc = g_Sl.GetUniformLocation( s_pCurrent->program, param->name );
	if( loc < 0 )
		return;

	if( count == 1 )
		g_Sl.Uniform1f( loc, v[0] );
	else if( count == 4 )
		g_Sl.Uniform4fv( loc, 1, v );
	else
		g_Sl.UniformMatrix4fv( loc, 1, GL_FALSE, v );
}

bool CgAvailable( void )
{
	return LoadSl() ? true : false;
}

CGcontext cgCreateContext( void )
{
	if( !LoadSl() )
		return NULL;
	return &s_CgContext;
}

CGprogram cgCreateProgramFromFile( CGcontext ctx, CGenum program_type, const char *program_file, CGprofile profile, const char *entry, const char **args )
{
	const char *name, *src = NULL;
	const char **samplers = NULL;
	int vertex = 0;
	GLint status = 0;
	char log[512];

	if( !LoadSl() || !ctx || !program_file )
	{
		s_iError = 1;
		return NULL;
	}

	name = strrchr( program_file, '/' );
	name = name ? name + 1 : program_file;

	if( !stricmp( name, "water_vp.cg" ) )
		src = s_szWaterVP, vertex = 1;
	else if( !stricmp( name, "water_fp.cg" ) )
		src = s_szWaterFP, samplers = s_szWaterSamplers;
	else if( !stricmp( name, "black_vp.cg" ) )
		src = s_szBlackVP, vertex = 1;
	else if( !stricmp( name, "black_fp.cg" ) )
		src = s_szBlackFP, samplers = s_szBlackSamplers;

	if( !src )
	{
		s_iError = 2;
		return NULL;
	}

	int slot = src == s_szWaterVP ? 0 : src == s_szWaterFP ? 1 : src == s_szBlackVP ? 2 : 3;
	if( s_pCompiled[slot] )
		return s_pCompiled[slot];

	struct _CGprogram *prog = new struct _CGprogram;
	memset( prog, 0, sizeof( *prog ) );
	prog->vertex = vertex;
	prog->samplers = samplers;
	prog->shader = g_Sl.CreateShader( vertex ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER );
	g_Sl.ShaderSource( prog->shader, 1, &src, NULL );
	g_Sl.CompileShader( prog->shader );
	g_Sl.GetShaderiv( prog->shader, GL_COMPILE_STATUS, &status );

	if( !status )
	{
		log[0] = 0;
		g_Sl.GetShaderInfoLog( prog->shader, sizeof( log ), NULL, log );
		gEngfuncs.Con_Printf( "%s: %s\n", name, log );
		delete prog;
		s_iError = 3;
		return NULL;
	}

	s_pCompiled[slot] = prog;
	return prog;
}

CGerror cgGetError( void )
{
	CGerror err = s_iError;
	s_iError = 0;
	return err;
}

const char *cgGetErrorString( CGerror error )
{
	switch( error )
	{
	case 0: return "No error";
	case 2: return "No GLSL version of this cg shader";
	case 3: return "GLSL compile failed";
	}
	return "GLSL is not available";
}

CGparameter cgGetNamedParameter( CGprogram prog, const char *name )
{
	if( !prog || !name )
		return NULL;

	for( int i = 0; i < prog->numparams; i++ )
	{
		if( !strcmp( prog->params[i].name, name ) )
			return &prog->params[i];
	}

	if( prog->numparams >= CG_MAX_PARAMS )
		return NULL;

	CGparameter p = &prog->params[prog->numparams++];
	memset( p, 0, sizeof( *p ) );
	p->prog = prog;
	strncpy( p->name, name, sizeof( p->name ) - 1 );
	return p;
}

CGprofile cgGLGetLatestProfile( CGGLenum profile_type )
{
	if( !LoadSl() )
		return CG_PROFILE_UNKNOWN;
	return profile_type == CG_GL_VERTEX ? CG_PROFILE_ARBVP1 : CG_PROFILE_ARBFP1;
}

void cgGLSetOptimalOptions( CGprofile profile )
{
}

void cgGLLoadProgram( CGprogram program )
{
}

void cgGLEnableProfile( CGprofile profile )
{
	if( !LoadSl() )
		return;

	if( profile == CG_PROFILE_ARBVP1 )
		s_bVertexOn = true;
	else
		s_bFragmentOn = true;

	UpdateProgram();
}

void cgGLDisableProfile( CGprofile profile )
{
	if( !LoadSl() )
		return;

	if( profile == CG_PROFILE_ARBVP1 )
		s_bVertexOn = false;
	else
		s_bFragmentOn = false;

	UpdateProgram();
}

void cgGLBindProgram( CGprogram program )
{
	if( !LoadSl() || !program )
		return;

	if( program->vertex )
		s_pBoundVP = program;
	else
		s_pBoundFP = program;

	UpdateProgram();
}

void cgGLSetParameter1f( CGparameter param, float x )
{
	if( LoadSl() && param )
		SetParam( param, &x, 1 );
}

void cgGLSetParameter4fv( CGparameter param, const float *v )
{
	if( LoadSl() && param && v )
		SetParam( param, v, 4 );
}

void cgGLSetStateMatrixParameter( CGparameter param, CGGLenum matrix, CGGLenum transform )
{
	float mv[16], proj[16], m[16];

	if( !LoadSl() || !param )
		return;

	// Cg takes a snapshot of the matrix when this is called
	gl.glGetFloatv( GL_MODELVIEW_MATRIX, mv );
	gl.glGetFloatv( GL_PROJECTION_MATRIX, proj );

	for( int c = 0; c < 4; c++ )
	{
		for( int r = 0; r < 4; r++ )
		{
			m[c * 4 + r] = proj[0 * 4 + r] * mv[c * 4 + 0] + proj[1 * 4 + r] * mv[c * 4 + 1]
				+ proj[2 * 4 + r] * mv[c * 4 + 2] + proj[3 * 4 + r] * mv[c * 4 + 3];
		}
	}

	SetParam( param, m, 16 );
}
#endif
