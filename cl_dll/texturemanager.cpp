
#include <string>
#include <list>

#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "com_model.h"
#include "triangleapi.h"
#include "studio_util.h"
#include "r_studioint.h"
#include "glmanager.h"
#include "texturemanager.h"
#include "client_port.h"

#include "image.h"

extern engine_studio_api_t IEngineStudio;

char szImageTypes[][5] =
{
	"bmp", "cut", "dds", "gif", "ico", "cur", "jpg", "jpe", "jpeg", "lbm",
	"lif", "lmp", "mdl", "mng", "pcd", "pcx", "pic", "pix", "png", "pbm",
	"pgm", "ppm", "pnm", "psd", "pxr", "sgi", "bw", "rgb", "rgba", "tga",
	"tif", "tiff", "wal", "xpm"
};

CTextureManager g_TextureManager;

CTextureManager::CTextureManager()
{
}

CTextureManager::~CTextureManager()
{
	LevelInit();
}

void CTextureManager::CheckMipmapSupport( void )
{
	const char *extensions = strdup( (const char *)glGetString( GL_EXTENSIONS ) );
	float version = atof( (const char *)glGetString( GL_VERSION ) );

	if ( isExtensionSupported( "GL_SGIS_generate_mipmap", extensions ) || isExtensionSupported( "GL_EXT_generate_mipmap", extensions ) )
	{
		m_iMipmapMode = 2;
	}
	else if ( version >= 1.4 )
	{
		m_iMipmapMode = 1;
	}
	else
	{
		gEngfuncs.Con_Printf( "MipMap extension not found\n" );
		m_iMipmapMode = 0;
	}
}

void CTextureManager::CreateTexture( int width, int height, GLuint *texture, GLenum target, GLenum format )
{
	unsigned char *data = new unsigned char[width * height * 4];

	*texture = m_iNextTexture++;

	glBindTexture( target, *texture );
	glTexParameteri( target, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( target, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( target, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( target, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
#ifdef __ANDROID__
	// gl4es draws these white when made with glCopyTexImage2D bigger than the screen
	glTexImage2D( target, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, NULL );
#else
	glCopyTexImage2D( target, 0, format, 0, 0, width, height, 0 );
#endif

	managed_texture_t tex;
	tex.type = TEXTYPE_IMAGE;
	tex.name = m_iNextTexture;
	tex.iFound = 0;
	tex.iFormat = GL_RGBA;
	tex.texture = *texture;
	m_Textures.push_back( tex );

	delete[] data;
}

std::string CTextureManager::GetPath( const std::string &name, int type, int *found )
{
	std::string path;

	if ( type == TEXTYPE_SPRITE )
	{
		path = "sprites/";
		path += name;
		path += ".spr";
	}
	else if ( IsHardware() )
	{
		path = GetTexturePath( name, found );
	}

	return path;
}

std::string CTextureManager::GetTexturePath( const std::string &name, int *found )
{
	std::string dir;
	std::string mapname;
	std::string result;
	std::string file;

	dir = gEngfuncs.pfnGetGameDirectory();
	dir += "/textures";

	mapname = gEngfuncs.pfnGetLevelName();
	mapname.erase( 0, 5 );
	mapname.erase( mapname.size() - 4, mapname.size() );

	result = ImportFrames( name, found );

	if ( *found )
		return result;

	std::string base = name;
	base.erase( base.size() - 1, base.size() );

	file = dir + "/" + base + "/" + name;
	result = ImportImageFile( file );

	if ( result.size() )
	{
		(*found)++;
		m_iNextTexture++;
		return result;
	}

	for ( int i = 3; i < 6; i++ )
	{
		file = dir + "/" + mapname + "/";

		switch ( i )
		{
		case 0:
			file += "highest/";
			break;
		case 1:
			file += "high/";
			break;
		case 2:
			file += "medium/";
			break;
		case 3:
			file += "low/";
			break;
		case 5:
			file = dir + "/";
			break;
		}

		file += name;
		result = ImportImageFile( file );

		if ( result.size() )
		{
			(*found)++;
			m_iNextTexture++;
			return result;
		}
	}

	return "";
}

managed_texture_t *CTextureManager::GetTexture( const std::string &name )
{
	for ( std::list<managed_texture_t>::iterator it = m_Textures.begin(); it != m_Textures.end(); ++it )
	{
		if ( it->name == name )
			return &*it;
	}

	return LoadTex( name );
}

std::string CTextureManager::ImportFrames( const std::string &name, int *count )
{
	std::string path;
	std::string base;
	std::string result;
	char number[8];

	*count = 0;

	path = gEngfuncs.pfnGetGameDirectory();
	path += "/textures/";
	path += name;
	base = path;
	path += "/";

	if ( !_access( path.c_str(), 0 ) )
	{
		path += name;
		base = path;
	}

	path = base;
	sprintf( number, "%i", *count );
	path += number;
	path = ImportImageFile( path );

	while ( path.size() )
	{
		managed_texture_t tex;

		(*count)++;
		tex.texture = m_iNextTexture++;
		tex.path = path;
		result = path;
		LoadFile( &tex );

		sprintf( number, "%i", *count );
		path = base;
		path += number;
		path = ImportImageFile( path );
	}

	return result;
}

std::string CTextureManager::ImportImageFile( const std::string &path )
{
	ilBindImage( m_iNextTexture );

	for ( int i = 0; i < 34; i++ )
	{
		std::string file = path;
		file = file + "." + szImageTypes[i];

		if ( ilLoadImage( file.c_str() ) )
			return file;
	}

	return "";
}

bool CTextureManager::IsHardware( void )
{
	return IEngineStudio.IsHardware() == 1;
}

void CTextureManager::LevelInit( void )
{
	if ( IsHardware() )
	{
		ilInit();
		CheckMipmapSupport();
	}

	m_iNextTexture = ( 1 << 24 );
	m_Textures.clear();
}

void CTextureManager::LoadFile( managed_texture_t *tex )
{
	ilBindImage( tex->texture );
	glBindTexture( GL_TEXTURE_2D, tex->texture );

	if ( m_iMipmapMode == 2 || m_iMipmapMode == 1 )
	{
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE );
	}
	else
	{
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	}

	ilLoadImage( tex->path.c_str() );
	glTexImage2D( GL_TEXTURE_2D, 0, ilGetInteger( IL_IMAGE_BPP ), ilGetInteger( IL_IMAGE_WIDTH ), ilGetInteger( IL_IMAGE_HEIGHT ), 0, ilGetInteger( IL_IMAGE_FORMAT ), GL_UNSIGNED_BYTE, ilGetData() );
}

managed_texture_t *CTextureManager::LoadFromName( const char *name )
{
	return GetTexture( std::string( name ) );
}

managed_texture_t *CTextureManager::LoadImage( const std::string &name )
{
	std::string path;
	int found = 0;

	path = GetPath( name, TEXTYPE_IMAGE, &found );

	if ( path.size() )
	{
		managed_texture_t tex;
		tex.type = TEXTYPE_IMAGE;
		tex.name = name;
		tex.path = path;
		tex.iFound = found;
		tex.iFormat = 0;
		tex.texture = m_iNextTexture - found;
		tex.pModel = NULL;

		if ( found == 1 )
			LoadFile( &tex );

		m_Textures.push_back( tex );
		return &*--m_Textures.end();
	}

	return NULL;
}

managed_texture_t *CTextureManager::LoadSprite( const std::string &name )
{
	std::string path = "sprites/";
	path += name;
	path += ".spr";

	managed_texture_t tex;
	tex.name = name;
	tex.path = path;
	tex.type = TEXTYPE_SPRITE;
	tex.iFormat = 0;
	tex.pModel = (struct model_s *)gEngfuncs.GetSpritePointer( gEngfuncs.pfnSPR_Load( path.c_str() ) );

	if ( tex.pModel )
	{
		if ( IsHardware() )
		{
			GLint texture;
			gEngfuncs.pTriAPI->SpriteTexture( tex.pModel, 0 );
			glGetIntegerv( GL_TEXTURE_BINDING_2D, &texture );
			tex.texture = texture;
		}

		m_Textures.push_back( tex );
		return &*--m_Textures.end();
	}

	return NULL;
}

managed_texture_t *CTextureManager::LoadTex( const std::string &name )
{
	managed_texture_t *tex;

	if ( IsHardware() && ( tex = LoadImage( name ) ) != NULL )
		return tex;

	return LoadSprite( name );
}

bool CTextureManager::LoadTexture( int texture, const char *name, int format, bool check )
{
	std::list<managed_texture_t>::iterator it = m_Textures.begin();

	if ( check )
	{
		while ( it != m_Textures.end() )
		{
			if ( (*it).texture == texture )
				return true;
			it++;
		}
	}

	if ( IsHardware() )
	{
		std::string path = name;
		int found = 0;

		path = GetPath( path, TEXTYPE_IMAGE, &found );

		if ( path.size() )
		{
			managed_texture_t tex;
			tex.type = TEXTYPE_IMAGE;
			tex.name = name;
			tex.path = path;
			tex.iFound = found;
			tex.texture = texture;
			tex.iFormat = format;
			LoadFile( &tex );
			m_Textures.push_back( tex );

			return true;
		}
	}

	return false;
}

void CTextureManager::PrecachePlayerShadow( void )
{
	LoadFromName( "playershadow" );
}

byte isExtensionSupported( const char *extension, const char *extensions )
{
	int len = strlen( extensions ) - 1;
	int extlen = strlen( extension );

	for ( int i = 0; i < len; i++ )
	{
		int j = 0;

		while ( 1 )
		{
			int a = tolower( extensions[i] );
			int b = tolower( extension[j] );

			if ( b != a )
				break;

			if ( j == extlen - 1 )
				return true;

			i++;
			j++;
		}
	}

	return false;
}
