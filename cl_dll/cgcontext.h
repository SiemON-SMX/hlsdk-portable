
#ifndef CGCONTEXT_H
#define CGCONTEXT_H

#include "texturemanager.h"

//=========================================================
//=========================================================
typedef struct _CGcontext *CGcontext;
typedef struct _CGprogram *CGprogram;
typedef struct _CGparameter *CGparameter;

typedef enum
{
	CG_PROFILE_START = 6144,
	CG_PROFILE_UNKNOWN,
	CG_PROFILE_VP20,
	CG_PROFILE_FP20,
	CG_PROFILE_VP30,
	CG_PROFILE_FP30,
	CG_PROFILE_ARBVP1,
	CG_PROFILE_FP40,
	CG_PROFILE_ARBFP1 = 7000,
	CG_PROFILE_VP40 = 7001,
	CG_PROFILE_MAX = 7100,
} CGprofile;

typedef enum
{
	CG_UNKNOWN = 4096,
	CG_IN,
	CG_OUT,
	CG_INOUT,
	CG_MIXED,
	CG_VARYING,
	CG_UNIFORM,
	CG_CONSTANT,
	CG_PROGRAM_SOURCE,
	CG_PROGRAM_ENTRY,
	CG_COMPILED_PROGRAM,
	CG_PROGRAM_PROFILE,
	CG_GLOBAL,
	CG_PROGRAM,
	CG_DEFAULT,
	CG_ERROR,
	CG_SOURCE,
	CG_OBJECT,
} CGenum;

typedef int CGerror;

typedef enum
{
	CG_GL_MATRIX_IDENTITY = 0,
	CG_GL_MATRIX_TRANSPOSE = 1,
	CG_GL_MATRIX_INVERSE = 2,
	CG_GL_MATRIX_INVERSE_TRANSPOSE = 3,

	CG_GL_MODELVIEW_MATRIX,
	CG_GL_PROJECTION_MATRIX,
	CG_GL_TEXTURE_MATRIX,
	CG_GL_MODELVIEW_PROJECTION_MATRIX,

	CG_GL_VERTEX,
	CG_GL_FRAGMENT,
} CGGLenum;

bool CgAvailable( void );
CGcontext cgCreateContext( void );
CGprogram cgCreateProgramFromFile( CGcontext ctx, CGenum program_type, const char *program_file, CGprofile profile, const char *entry, const char **args );
CGerror cgGetError( void );
const char *cgGetErrorString( CGerror error );
CGparameter cgGetNamedParameter( CGprogram prog, const char *name );

CGprofile cgGLGetLatestProfile( CGGLenum profile_type );
void cgGLSetOptimalOptions( CGprofile profile );
void cgGLLoadProgram( CGprogram program );
void cgGLEnableProfile( CGprofile profile );
void cgGLDisableProfile( CGprofile profile );
void cgGLBindProgram( CGprogram program );
void cgGLSetParameter1f( CGparameter param, float x );
void cgGLSetParameter4fv( CGparameter param, const float *v );
void cgGLSetStateMatrixParameter( CGparameter param, CGGLenum matrix, CGGLenum transform );

#define CGTYPE_FRAGMENT	1
#define CGTYPE_VERTEX		2

typedef struct cgprogram_s
{
	char		name[32];
	int			type;
	CGprogram	program;
	struct cgprogram_s *next;
	struct cgprogram_s *prev;
} cgprogram_t;

class CCgContext
{
public:
	CCgContext();
	~CCgContext();

	cgprogram_t *AddProgram( const char *name, int type );
	cgprogram_t *FindProgram( const char *name );
	CGprofile GetFragmentProfile( void );
	bool IsInitialized( void );
	bool IsProgramValid( const char *path, int type );
	cgprogram_t *RemoveProgram( cgprogram_t *program );
	void Shutdown( void );
	bool Startup( void );

	void EnableVertexProfile( void )
	{
		if ( g_TextureManager.IsHardware() )
			cgGLEnableProfile( m_vertexProfile );
	}
	void EnableFragmentProfile( void )
	{
		if ( g_TextureManager.IsHardware() )
			cgGLEnableProfile( m_fragmentProfile );
	}
	void DisableVertexProfile( void )
	{
		if ( g_TextureManager.IsHardware() )
			cgGLDisableProfile( m_vertexProfile );
	}
	void DisableFragmentProfile( void )
	{
		if ( g_TextureManager.IsHardware() )
			cgGLDisableProfile( m_fragmentProfile );
	}
	void BindProgram( cgprogram_t *program )
	{
		if ( g_TextureManager.IsHardware() )
			cgGLBindProgram( program->program );
	}

	CGcontext	m_context;
	CGprofile	m_vertexProfile;
	CGprofile	m_fragmentProfile;
	cgprogram_t *m_pPrograms;
	bool		m_bInitialized;
};

extern CCgContext g_CgContext;

#endif
