
#include <string>
#include <list>

#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "com_model.h"
#include "studio_util.h"
#include "r_studioint.h"
#include "glmanager.h"
#include "cgcontext.h"

extern engine_studio_api_t IEngineStudio;

CCgContext g_CgContext;

CCgContext::CCgContext()
{
	m_pPrograms = NULL;
	m_context = NULL;
	m_bInitialized = false;
}

CCgContext::~CCgContext()
{
	Shutdown();
	m_context = NULL;
	m_bInitialized = false;
}

cgprogram_t *CCgContext::AddProgram( const char *name, int type )
{
	if ( IEngineStudio.IsHardware() != 1 )
		return NULL;

	cgprogram_t *program = FindProgram( name );

	if ( !m_context )
		Startup();

	if ( !program )
	{
		char path[128];
		sprintf( path, "%s/cgshaders/%s", gEngfuncs.pfnGetGameDirectory(), name );

		if ( IsProgramValid( path, type ) )
		{
			program = new cgprogram_t;
			strcpy( program->name, name );

			if ( type == CGTYPE_FRAGMENT )
				program->program = cgCreateProgramFromFile( m_context, CG_SOURCE, path, m_fragmentProfile, "main", NULL );
			else if ( type == CGTYPE_VERTEX )
				program->program = cgCreateProgramFromFile( m_context, CG_SOURCE, path, m_vertexProfile, "main", NULL );

			program->type = type;
			program->prev = NULL;
			program->next = m_pPrograms;
			if ( m_pPrograms )
				m_pPrograms->prev = program;
			m_pPrograms = program;

			cgGLLoadProgram( program->program );
		}
	}

	return program;
}

cgprogram_t *CCgContext::FindProgram( const char *name )
{
	for ( cgprogram_t *program = m_pPrograms; program; program = program->next )
	{
		if ( !stricmp( name, program->name ) )
			return program;
	}

	return NULL;
}

CGprofile CCgContext::GetFragmentProfile( void )
{
	return m_fragmentProfile;
}

bool CCgContext::IsInitialized( void )
{
	return m_bInitialized;
}

bool CCgContext::IsProgramValid( const char *path, int type )
{
	CGprogram program;

	if ( type == CGTYPE_FRAGMENT )
		program = cgCreateProgramFromFile( m_context, CG_SOURCE, path, m_fragmentProfile, "main", NULL );
	else if ( type == CGTYPE_VERTEX )
		program = cgCreateProgramFromFile( m_context, CG_SOURCE, path, m_vertexProfile, "main", NULL );

	if ( !program )
	{
		gEngfuncs.Con_Printf( "%s\n", cgGetErrorString( cgGetError() ) );
		return false;
	}

	return true;
}

cgprogram_t *CCgContext::RemoveProgram( cgprogram_t *program )
{
	cgprogram_t *next = program->next;

	if ( program == m_pPrograms )
		m_pPrograms = program->next;

	if ( program->next )
		program->next->prev = program->prev;

	if ( program->prev )
		program->prev->next = program->next;

	delete program;

	return next;
}

void CCgContext::Shutdown( void )
{
	cgprogram_t *program = m_pPrograms;

	while ( program )
		program = RemoveProgram( program );

	m_pPrograms = NULL;
	m_bInitialized = false;
}

bool CCgContext::Startup( void )
{
	bool result;

	m_context = cgCreateContext();

	if ( m_context )
	{
		m_vertexProfile = cgGLGetLatestProfile( CG_GL_VERTEX );

		if ( m_vertexProfile != CG_PROFILE_UNKNOWN )
		{
			cgGLSetOptimalOptions( m_vertexProfile );
			m_fragmentProfile = cgGLGetLatestProfile( CG_GL_FRAGMENT );

			if ( m_fragmentProfile != CG_PROFILE_UNKNOWN )
			{
				cgGLSetOptimalOptions( m_fragmentProfile );
				result = true;
			}
			else
				result = false;
		}
		else
			result = false;
	}
	else
		result = false;

	m_bInitialized = result;
	return result;
}
