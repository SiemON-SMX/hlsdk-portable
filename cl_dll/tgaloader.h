
#ifndef TGALOADER_H
#define TGALOADER_H

typedef struct
{
	GLubyte	*imageData;
	GLuint	bpp;
	GLuint	width;
	GLuint	height;
	GLuint	texID;
	GLuint	type;
} Texture;

typedef struct
{
	GLubyte	Header[12];
} TGAHeader;

typedef struct
{
	GLubyte	header[6];
	GLuint	bytesPerPixel;
	GLuint	imageSize;
	GLuint	temp;
	GLuint	type;
	GLuint	Height;
	GLuint	Width;
	GLuint	Bpp;
} TGA;

class CTGALoader
{
public:
	bool LoadTGA( Texture *texture, char *filename );
	bool LoadUncompressedTGA( Texture *texture, char *filename, FILE *fTGA );
	bool LoadCompressedTGA( Texture *texture, char *filename, FILE *fTGA );

	TGAHeader	tgaheader;
	TGA		tga;
	GLubyte			uTGAcompare[12];
	GLubyte			cTGAcompare[12];
};

#endif
