
#ifndef TEXTUREMANAGER_H
#define TEXTUREMANAGER_H

#include <string>
#include <GL/gl.h>
#include <list>

// texture types
#define TEXTYPE_SPRITE	1
#define TEXTYPE_IMAGE		2

typedef struct managed_texture_s
{
	int			type;
	std::string	name;
	std::string	path;
	int			texture;
	struct model_s *pModel;	// sprite model
	int			iFound;
	int			iFormat;
} managed_texture_t;

class CTextureManager
{
public:
	CTextureManager();
	~CTextureManager();

	void CheckMipmapSupport( void );
	void CreateTexture( int width, int height, GLuint *texture, GLenum target, GLenum format );
	std::string GetPath( const std::string &name, int type, int *found );
	std::string GetTexturePath( const std::string &name, int *found );
	managed_texture_t *GetTexture( const std::string &name );
	std::string ImportFrames( const std::string &name, int *count );
	std::string ImportImageFile( const std::string &path );
	bool IsHardware( void );
	void LevelInit( void );
	void LoadFile( managed_texture_t *tex );
	managed_texture_t *LoadFromName( const char *name );
	managed_texture_t *LoadImage( const std::string &name );
	managed_texture_t *LoadSprite( const std::string &name );
	managed_texture_t *LoadTex( const std::string &name );
	bool LoadTexture( int texture, const char *name, int format, bool check );
	void PrecachePlayerShadow( void );

	int		m_iMipmapMode;
	int		m_iNextTexture;
	std::list<managed_texture_t> m_Textures;
};

extern CTextureManager g_TextureManager;

byte isExtensionSupported( const char *extension, const char *extensions );

#endif
