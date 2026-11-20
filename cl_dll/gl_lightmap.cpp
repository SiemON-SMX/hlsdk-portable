/*******************************************************
*
*  Lightmaps loading.
*
*  written by BUzer for Half-Life: Paranoia modification
*
********************************************************/

#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "cdll_int.h"
#include "com_model.h"
#include "glmanager.h"
#include "entity_types.h"
#include "gl_texloader.h"
#include "gl_renderer.h"
#include "gl_dlight.h"
#include "in_defs.h"

#define	BLOCK_WIDTH		128
#define	BLOCK_HEIGHT	128
#define	MAX_LIGHTMAPS	64
#define BLOCKLIGHTS_SIZE	(18*18)

// BUMP lightmaps base indexes
int g_lightmap_baselight = 0; // contains normal lightmap divided by two if no bump for surface
int g_lightmap_addlight = 0;
int g_lightmap_lightvecs = 0;

char szMapName[256]; // map for which we have lightmaps prepared
int lightmaps_initialized = 0;
int current_bumpstate = 0;
float current_gamma = 0;
float current_brightness = 0;
float current_contrast = 0;

color24	lightmaps[MAX_LIGHTMAPS*BLOCK_WIDTH*BLOCK_HEIGHT];
color24 blocklights[BLOCKLIGHTS_SIZE];

bool lightmapsResetA;
bool lightmapsResetB;

int iUnknown104CC468;

int lightmapstotal;

static model_t *world;

#define VectorMaximum(a) ( Q_max( (a)[0], Q_max( (a)[1], (a)[2] ) ) )

// test
int WriteTGA_24(color24 *pixels, unsigned short width, unsigned short height, const char *filename );

//
// HELPERS
//
int StyleIndex ( msurface_t *surf, int style )
{
	for (int j = 0 ; j < MAXLIGHTMAPS && surf->styles[j] != 255 ; j++)
	{
		if (surf->styles[j] == style)
			return j;
	}
	return -1;
}

float ApplyGamma( float value )
{
	if (current_gamma == 1) return (value * current_contrast + current_brightness);
	return (pow(value, current_gamma) * current_contrast + current_brightness);
}

/*
unsigned char Clamp( float value )
{
	value *= 255;
	if (value > 255) value = 255;
	if (value < 0) value = 0;
	return (unsigned char)value;
}
*/

void Clamp( vec3_t in, color24 *out )
{
	VectorScale(in, 255, in);
	float max = VectorMaximum(in);
	if (max > 255)
	{
		float scale = 255.0 / max;
		VectorScale(in, scale, in);
	}

	if (in[0] < 0) in[0] = 0;
	if (in[1] < 0) in[1] = 0;
	if (in[2] < 0) in[2] = 0;

	out->r = (unsigned char)in[0];
	out->g = (unsigned char)in[1];
	out->b = (unsigned char)in[2];
}

int LightmapSize( msurface_t *surf )
{
	int smax = (surf->extents[0]>>4)+1;
	int tmax = (surf->extents[1]>>4)+1;
	return smax*tmax;
}


//===================
// UploadBlocklights
// copies lightmap data from blocklights to common lightmap texture
//===================
void UploadBlocklights( msurface_t *surf )
{
	color24 *bl = blocklights;
	color24	*dest = lightmaps + surf->lightmaptexturenum*BLOCK_WIDTH*BLOCK_HEIGHT;
	dest += (surf->light_t * BLOCK_WIDTH + surf->light_s);	

	int smax = (surf->extents[0]>>4)+1;
	int tmax = (surf->extents[1]>>4)+1;

	for (int i=0 ; i<tmax ; i++, dest += BLOCK_WIDTH)
	{
		for (int j=0 ; j<smax ; j++)
		{
			dest[j].r = bl->r;
			dest[j].g = bl->g;
			dest[j].b = bl->b;
			bl++;
		}
	}
}


//===================
// BuildLightMap_FromOriginal
//
// Builds lightmap from original zero style
//===================
void BuildLightMap_FromOriginal (msurface_t *surf)
{
	int size = LightmapSize(surf);
	color24 *lightmap = surf->samples;

	if (!lightmap || size > BLOCKLIGHTS_SIZE)
		return;

	for (int i=0 ; i<size ; i++)
	{
		vec3_t col;
		col[0] = (float)lightmap[i].r / 255;
		col[1] = (float)lightmap[i].g / 255;
		col[2] = (float)lightmap[i].b / 255;

	//	blocklights[i].r = Clamp( ApplyGamma(col[0])/2 );
	//	blocklights[i].g = Clamp( ApplyGamma(col[1])/2 );
	//	blocklights[i].b = Clamp( ApplyGamma(col[2])/2 );
		col[0] = ApplyGamma(col[0])/2;
		col[1] = ApplyGamma(col[1])/2;
		col[2] = ApplyGamma(col[2])/2;
		Clamp(col, &blocklights[i]);
	}

	UploadBlocklights(surf);
}

//===================
// BuildLightMap_FromBaselight
//
// Builds lightmap from baselight - for faces that doesnt have
// direct light in bump mode, but have it in normal mode
//===================
void BuildLightMap_FromBaselight (msurface_t *surf)
{
	int size = LightmapSize(surf);
	if (!surf->samples || size > BLOCKLIGHTS_SIZE)
		return;

	color24 *lightmap = surf->samples + size * StyleIndex(surf, BUMP_BASELIGHT_STYLE);

	for (int i=0 ; i<size ; i++)
	{
		vec3_t col;
		col[0] = (float)lightmap[i].r / 255;
		col[1] = (float)lightmap[i].g / 255;
		col[2] = (float)lightmap[i].b / 255;

		col[0] = ApplyGamma(col[0]);
		col[1] = ApplyGamma(col[1]);
		col[2] = ApplyGamma(col[2]);
		Clamp(col, &blocklights[i]);
	}

	UploadBlocklights(surf);
}


//===================
// BuildLightMap_Combine
//
// builds lightmap data combining three bump styles
// Used for surfaces, who have bump styles but doesn't have normal map loaded
//
// (should give better results than just taking zero style, because
// bump lightmaps has increased light intensity range)
//===================
void BuildLightMap_Combine (msurface_t *surf)
{
	int size = LightmapSize(surf);
	color24 *lightmap = surf->samples;

	if (!lightmap || size > BLOCKLIGHTS_SIZE)
		return;

	int baselight_index = StyleIndex(surf, BUMP_BASELIGHT_STYLE);
	int addlight_index = StyleIndex(surf, BUMP_ADDLIGHT_STYLE);
	int lightvecs_index = StyleIndex(surf, BUMP_LIGHTVECS_STYLE);

	color24 *baselight_offset = lightmap + size * baselight_index;
	color24 *addlight_offset = lightmap + size * addlight_index;
	color24 *lightvecs_offset = lightmap + size * lightvecs_index;

	for (int i=0 ; i<size ; i++)
	{
		float dot;
		vec3_t baselight, addlight, result;

		baselight[0] = (float)baselight_offset[i].r / 255;
		baselight[1] = (float)baselight_offset[i].g / 255;
		baselight[2] = (float)baselight_offset[i].b / 255;

		addlight[0] = (float)addlight_offset[i].r / 255;
		addlight[1] = (float)addlight_offset[i].g / 255;
		addlight[2] = (float)addlight_offset[i].b / 255;

		dot = (float)lightvecs_offset[i].b;
		dot = (dot / 127) - 1;

		VectorScale(addlight, dot, result);
		VectorAdd(result, baselight, result);

	//	blocklights[i].r = Clamp( ApplyGamma(result[0]*2)/2 );
	//	blocklights[i].g = Clamp( ApplyGamma(result[1]*2)/2 );
	//	blocklights[i].b = Clamp( ApplyGamma(result[2]*2)/2 );
		result[0] = ApplyGamma(result[0]*2)/2;
		result[1] = ApplyGamma(result[1]*2)/2;
		result[2] = ApplyGamma(result[2]*2)/2;
		Clamp(result, &blocklights[i]);
	}

	UploadBlocklights(surf);
}


//===================
// BuildLightMap_FromBumpStyle
//
// Uses specified bump style to fill lightmap block data.
//===================
void BuildLightMap_FromBumpStyle (msurface_t *surf, int rstyle, int gammacorrection)
{
	int size = LightmapSize(surf);
	color24 *lightmap = surf->samples;

	if (!lightmap || size > BLOCKLIGHTS_SIZE)
		return;

	color24 *target_offset = lightmap + size * StyleIndex(surf, rstyle);

	color24 *baselight_offset, *addlight_offset, *lightvecs_offset;
	if (gammacorrection)
	{
		baselight_offset = lightmap + size * StyleIndex(surf, BUMP_BASELIGHT_STYLE);
		addlight_offset = lightmap + size * StyleIndex(surf, BUMP_ADDLIGHT_STYLE);
		lightvecs_offset = lightmap + size * StyleIndex(surf, BUMP_LIGHTVECS_STYLE);
	}

	for (int i=0 ; i<size ; i++)
	{
		if (gammacorrection)
		{
			// calculate "combine" lightmap and apply gamma to it
			float dot;
			vec3_t baselight, addlight, result;
			vec3_t scale = Vector(0, 0, 0);

			baselight[0] = (float)baselight_offset[i].r / 255;
			baselight[1] = (float)baselight_offset[i].g / 255;
			baselight[2] = (float)baselight_offset[i].b / 255;

			addlight[0] = (float)addlight_offset[i].r / 255;
			addlight[1] = (float)addlight_offset[i].g / 255;
			addlight[2] = (float)addlight_offset[i].b / 255;

			dot = (float)lightvecs_offset[i].b;
			dot = (dot / 127) - 1;

			VectorScale(addlight, dot, result);
			VectorAdd(result, baselight, result);
			VectorScale(result, 2, result);

			if (result[0]) scale[0] = ApplyGamma(result[0]) / result[0];
			if (result[1]) scale[1] = ApplyGamma(result[1]) / result[1];
			if (result[2]) scale[2] = ApplyGamma(result[2]) / result[2];

		//	blocklights[i].r = Clamp( (float)target_offset[i].r / 255 * scale[0] );
		//	blocklights[i].g = Clamp( (float)target_offset[i].g / 255 * scale[1] );
		//	blocklights[i].b = Clamp( (float)target_offset[i].b / 255 * scale[2] );
			result[0] = (float)target_offset[i].r / 255.0 * scale[0];
			result[1] = (float)target_offset[i].g / 255.0 * scale[1];
			result[2] = (float)target_offset[i].b / 255.0 * scale[2];
			Clamp(result, &blocklights[i]);
		}
		else
		{
			blocklights[i].r = target_offset[i].r;
			blocklights[i].g = target_offset[i].g;
			blocklights[i].b = target_offset[i].b;
		}
	}

	UploadBlocklights(surf);
}


//===================
// BumpMarkFaces
//
// makes checks easier
//===================
void BumpMarkFaces()
{
	Log("Marking faces...");
	model_t	*m = gEngfuncs.GetEntityByIndex(0)->model;
	if (!m) return;

	for (int i=0 ; i<m->numsurfaces ; i++)
	{
		msurface_t *surf = &m->surfaces[i];
		surf->flags &= ~SURF_BUMPREADY; // make shure that flag isn't set..
		surf->flags &= ~SURF_SPECULAR; // make shure that flag isn't set..

		if (surf->flags & (SURF_DRAWSKY|SURF_DRAWTURB))
			continue;
		
		if (StyleIndex(surf, BUMP_BASELIGHT_STYLE) == -1) continue;
		if (StyleIndex(surf, BUMP_ADDLIGHT_STYLE) == -1) continue;
		if (StyleIndex(surf, BUMP_LIGHTVECS_STYLE) == -1) continue;
		
		// has normal map for this texture?
		if (surf->texinfo->texture->anim_min != -666) continue;
		ExtTextureData *pExtData = (ExtTextureData*)surf->texinfo->texture->anim_next;
		if (pExtData->gl_normalmap_id)
		{
			// surface ready to be drawn bumped
			surf->flags |= SURF_BUMPREADY;
		}

		if (pExtData->gl_glossmap_id || (pExtData->gl_extra_glossmap_id && pExtData->gl_normalmap_id))
		{
			// surface ready to be drawn with specular
			surf->flags |= SURF_SPECULAR;
		}		
	}
	Log("ok\n");
}

//===================
// UploadLightmaps
//
// uploads all lightmaps that were filled
//===================
void UploadLightmaps( int base )
{
	for (int i=0 ; i<(lightmapstotal+1) ; i++)
	{
		color24 *lightmap = lightmaps + i*BLOCK_WIDTH*BLOCK_HEIGHT;

		gl.glBindTexture(GL_TEXTURE_2D, base + i);
	//	gl.glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		gl.glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		gl.glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//	gl.glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		gl.glTexImage2D (GL_TEXTURE_2D, 0, 3, BLOCK_WIDTH, BLOCK_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, lightmap);
	}
}


int LightmapsNeedsUpdate()
{
	if (!lightmaps_initialized)
	{
		lightmapsResetA = false;
		lightmapsResetB = false;
		return TRUE;
	}
	if (current_gamma != cv_gamma->value) return TRUE;
	if (current_brightness != cv_brightness->value) return TRUE;
	if (current_contrast != cv_contrast->value) return TRUE;
	if (cv_bump->value && !current_bumpstate) return TRUE;
	if (!cv_bump->value && current_bumpstate) return TRUE;

	const char* curmapname = gEngfuncs.pfnGetLevelName();
	if (stricmp(curmapname, szMapName)) return TRUE;

	return FALSE;
}

//===================
// UpdateLightmaps
// 
// (Must be called after BumpMarkFaces)
// detects map and light settings changes and loads appropriate lightmaps
//===================
void UpdateLightmaps()
{
	int i;
	world = gEngfuncs.GetEntityByIndex(0)->model;
	if (!world)
		return;

	if (!gl.IsGLAllowed())
	{
		ONCE( ConLog("ERROR: UpdateLightmaps called when GLmanager not initialized!\n"); );
		return;
	}

	if (!LightmapsNeedsUpdate())
		return;

	const char* curmapname = gEngfuncs.pfnGetLevelName();
	lightmaps_initialized = 1;
	if (cv_contrast->value < 1) gEngfuncs.Cvar_SetValue("gl_contrast", 1);
	if (cv_contrast->value > 2) gEngfuncs.Cvar_SetValue("gl_contrast", 2);
	if (cv_brightness->value > 0.3) gEngfuncs.Cvar_SetValue("gl_brightness", 0.3);
	current_contrast = cv_contrast->value;
	current_brightness = cv_brightness->value;
	current_gamma = cv_gamma->value;
	if (current_gamma < 0) current_gamma = 1;
	if (cv_bump->value) current_bumpstate = 1;
	else current_bumpstate = 0;
	strcpy(szMapName, curmapname);
	memset(lightmaps, 0, sizeof(lightmaps));
	Log("\n>> Loading additional lightmaps for %s\n", curmapname);
	Log("gamma: %f\nbrightness: %f\ncontrast: %f\n", current_gamma, current_brightness, current_contrast);
	Log("bump: %d\n", current_bumpstate);

	//==================================
	// Load BASELIGHT style lightmaps
	//==================================
	lightmapstotal = -1;
	for (i=0 ; i<world->numsurfaces ; i++)
	{
		msurface_t *surf = &world->surfaces[i];
		if (surf->flags & (SURF_DRAWSKY|SURF_DRAWTURB))
			continue;

		if (surf->lightmaptexturenum >= MAX_LIGHTMAPS)
		{
			ConLog("WARNING! surface %d uses lightmap index %d greater than MAX_LIGHTMAPS!\n", i, surf->lightmaptexturenum);
			continue;
		}

		if (surf->lightmaptexturenum > lightmapstotal)
			lightmapstotal = surf->lightmaptexturenum;

		if (StyleIndex(surf, BUMP_BASELIGHT_STYLE) == -1)
		{
			// face doesn't have bump lightmaps
			BuildLightMap_FromOriginal (surf);
			continue;
		}

		if (surf->flags & SURF_BUMPREADY && cv_bump->value)
		{
			// face has bump lightmaps and normal map, it will be drawn bumped
			BuildLightMap_FromBumpStyle (surf, BUMP_BASELIGHT_STYLE, TRUE);
			continue;
		}

		if (StyleIndex(surf, BUMP_ADDLIGHT_STYLE) == -1 ||
			StyleIndex(surf, BUMP_LIGHTVECS_STYLE) == -1)
		{
			// face has baselight style, but no other bump styles.
			// use this as single lightmap
			BuildLightMap_FromBaselight(surf);
			continue;
		}

		// face has bump lightmaps, but no normalmap (or bump is disabled)
		BuildLightMap_Combine (surf);
	}

	if (lightmapstotal == -1)
	{
		ConLog("ERROR: no lightmaps usage!\n");
		return;
	}

	UploadLightmaps( g_lightmap_baselight );

	//==================================
	// Load ADDLIGHT style lightmaps
	//==================================
	int needs_extra_lightmaps = 0;
	for (i=0 ; i<world->numsurfaces ; i++)
	{
		msurface_t *surf = &world->surfaces[i];
		if (surf->flags & (SURF_DRAWSKY|SURF_DRAWTURB))
			continue;

		if (surf->lightmaptexturenum >= MAX_LIGHTMAPS)
			continue;
		
		if (!(surf->flags & SURF_BUMPREADY) && !(surf->flags & SURF_SPECULAR))
			continue;

		BuildLightMap_FromBumpStyle (surf, BUMP_ADDLIGHT_STYLE, TRUE);
		needs_extra_lightmaps = 1;
	}

	if (needs_extra_lightmaps)
		UploadLightmaps( g_lightmap_addlight );

	//==================================
	// Load LIGHTVECS style lightmaps
	//==================================
	if (needs_extra_lightmaps)
	{
		for (i=0 ; i<world->numsurfaces ; i++)
		{
			msurface_t *surf = &world->surfaces[i];
			if (surf->flags & (SURF_DRAWSKY|SURF_DRAWTURB))
				continue;

			if (surf->lightmaptexturenum >= MAX_LIGHTMAPS)
				continue;
			
			if (!(surf->flags & SURF_BUMPREADY) && !(surf->flags & SURF_SPECULAR))
				continue;

			BuildLightMap_FromBumpStyle (surf, BUMP_LIGHTVECS_STYLE, FALSE);
		}

		UploadLightmaps( g_lightmap_lightvecs );
	}

	if (needs_extra_lightmaps)
		ConLog("Loaded %d x 3 additional lightmaps\n", lightmapstotal+1);
	else
		ConLog("Loaded %d additional lightmaps\n", lightmapstotal+1);
}


void SetLightmapBaseIndex()
{
	g_lightmap_baselight = current_ext_texture_id;
	current_ext_texture_id += MAX_LIGHTMAPS;

	g_lightmap_addlight = current_ext_texture_id;
	current_ext_texture_id += MAX_LIGHTMAPS;

	g_lightmap_lightvecs = current_ext_texture_id;
	current_ext_texture_id += MAX_LIGHTMAPS;
}




#define MAX_HL_SURFS 8192
#define MAX_LM_LIGHTS 32
#define MAX_PARANOIA_DLIGHTS 32
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

extern DynamicLight cl_dlights[];
cvar_t *gl_dynlight_lightmap;

struct LMLight
{
	vec3_t origin;
	vec3_t forward, right, up;
	float radius;
	float intensity[3];
	float tanhor, tanver;
	int spot;
};

static LMLight g_LMLights[MAX_LM_LIGHTS];
static int g_iNumLMLights;
static msurface_t *g_pHLDirty[MAX_HL_SURFS];
static int g_iNumHLDirty;
static msurface_t *g_pHLCandidates[MAX_HL_SURFS];
static int g_iNumHLCandidates;
static int *g_pHLMarks;
static int g_iHLMarkSize;
static int g_iHLFrame;
static model_t *g_pHLWorld;

int SpotShaderActive( void );

int LightmapDlightsActive( void )
{
	return gl_dynlight_lightmap && gl_dynlight_lightmap->value;
}

// xash frees the world on every map load, even the same map,
// so drop the old surface pointers and reload the lightmaps
void ResetLightmaps( void )
{
	szMapName[0] = 0;
	g_pHLWorld = NULL;
	g_iNumHLDirty = 0;
	g_iNumHLCandidates = 0;
	g_iNumLMLights = 0;
}

static bool HLMarkSurface( msurface_t *surf )
{
	int index = surf - world->surfaces;

	if ( index < 0 || index >= g_iHLMarkSize || g_pHLMarks[index] == g_iHLFrame )
		return true;

	if ( surf->flags & ( SURF_DRAWSKY | SURF_DRAWTURB ))
		return true;

	if ( !surf->samples || surf->lightmaptexturenum >= MAX_LIGHTMAPS || LightmapSize( surf ) > BLOCKLIGHTS_SIZE )
		return true;

	if ( g_iNumHLCandidates >= MAX_HL_SURFS )
		return false;

	g_pHLMarks[index] = g_iHLFrame;
	g_pHLCandidates[g_iNumHLCandidates++] = surf;
	return true;
}

static void HLMarkNode( mnode_t *node, const LMLight *l )
{
	while ( node && node->contents >= 0 )
	{
		mplane_t *plane = node->plane;
		float dist = DotProduct( l->origin, plane->normal ) - plane->dist;

		if ( dist > l->radius )
		{
			node = node->children[0];
			continue;
		}

		if ( dist < -l->radius )
		{
			node = node->children[1];
			continue;
		}

		msurface_t *surf = world->surfaces + node->firstsurface;

		for ( int i = 0; i < node->numsurfaces; i++, surf++ )
		{
			if ( !HLMarkSurface( surf ))
				return;
		}

		HLMarkNode( node->children[0], l );
		node = node->children[1];
	}
}

// brush entities that never move (func_illusionary, func_wall...) aren't in the world tree,
// so mark their faces too or they never get the dynamic light
int IsEntityMoved( cl_entity_t *e );

static void HLMarkEntities( const LMLight *l )
{
	for ( int k = 0; k < numrenderents; k++ )
	{
		cl_entity_t *e = renderents[k];

		if ( !e || !e->model || e->model->type != mod_brush || IsEntityMoved( e ))
			continue;

		model_t *m = e->model;

		if ( l->origin[0] + l->radius < m->mins[0] || l->origin[0] - l->radius > m->maxs[0] ||
			l->origin[1] + l->radius < m->mins[1] || l->origin[1] - l->radius > m->maxs[1] ||
			l->origin[2] + l->radius < m->mins[2] || l->origin[2] - l->radius > m->maxs[2] )
			continue;

		msurface_t *surf = world->surfaces + m->firstmodelsurface;

		for ( int i = 0; i < m->nummodelsurfaces; i++, surf++ )
		{
			float dist = DotProduct( l->origin, surf->plane->normal ) - surf->plane->dist;

			if ( dist > l->radius || dist < -l->radius )
				continue;

			if ( !HLMarkSurface( surf ))
				return;
		}
	}
}

static void HLBuildBase( msurface_t *surf )
{
	if ( StyleIndex( surf, BUMP_BASELIGHT_STYLE ) == -1 )
		BuildLightMap_FromOriginal( surf );
	else if ( surf->flags & SURF_BUMPREADY && cv_bump->value )
		BuildLightMap_FromBumpStyle( surf, BUMP_BASELIGHT_STYLE, TRUE );
	else if ( StyleIndex( surf, BUMP_ADDLIGHT_STYLE ) == -1 || StyleIndex( surf, BUMP_LIGHTVECS_STYLE ) == -1 )
		BuildLightMap_FromBaselight( surf );
	else
		BuildLightMap_Combine( surf );
}

static bool SurfaceBasis( msurface_t *surf, vec3_t inv[3] )
{
	mtexinfo_t *tex = surf->texinfo;
	const float *a = tex->vecs[0];
	const float *b = tex->vecs[1];
	const float *n = surf->plane->normal;

	float det = a[0] * ( b[1] * n[2] - b[2] * n[1] ) - a[1] * ( b[0] * n[2] - b[2] * n[0] ) + a[2] * ( b[0] * n[1] - b[1] * n[0] );

	if ( fabs( det ) < 1e-8f )
		return false;

	float id = 1.0f / det;

	inv[0][0] = ( b[1] * n[2] - b[2] * n[1] ) * id;
	inv[0][1] = ( a[2] * n[1] - a[1] * n[2] ) * id;
	inv[0][2] = ( a[1] * b[2] - a[2] * b[1] ) * id;
	inv[1][0] = ( b[2] * n[0] - b[0] * n[2] ) * id;
	inv[1][1] = ( a[0] * n[2] - a[2] * n[0] ) * id;
	inv[1][2] = ( a[2] * b[0] - a[0] * b[2] ) * id;
	inv[2][0] = ( b[0] * n[1] - b[1] * n[0] ) * id;
	inv[2][1] = ( a[1] * n[0] - a[0] * n[1] ) * id;
	inv[2][2] = ( a[0] * b[1] - a[1] * b[0] ) * id;
	return true;
}

static bool HLAddLights( msurface_t *surf )
{
	int smax = ( surf->extents[0] >> 4 ) + 1;
	int tmax = ( surf->extents[1] >> 4 ) + 1;
	mtexinfo_t *tex = surf->texinfo;
	vec3_t inv[3];
	vec3_t facenormal;
	bool lit = false;

	if ( !SurfaceBasis( surf, inv ))
		return false;

	VectorCopy( surf->plane->normal, facenormal );
	if ( surf->flags & SURF_PLANEBACK )
		VectorScale( facenormal, -1.0f, facenormal );

	for ( int l = 0; l < g_iNumLMLights; l++ )
	{
		LMLight *light = &g_LMLights[l];
		float pdist = DotProduct( light->origin, facenormal ) - ( surf->flags & SURF_PLANEBACK ? -surf->plane->dist : surf->plane->dist );

		if ( pdist <= 0.0f || pdist >= light->radius )
			continue;

		color24 *bl = blocklights;

		for ( int t = 0; t < tmax; t++ )
		{
			float tw = surf->texturemins[1] + t * 16 - tex->vecs[1][3];

			for ( int s = 0; s < smax; s++, bl++ )
			{
				float sw = surf->texturemins[0] + s * 16 - tex->vecs[0][3];
				float d = surf->plane->dist;
				vec3_t pos, delta;

				pos[0] = inv[0][0] * sw + inv[0][1] * tw + inv[0][2] * d;
				pos[1] = inv[1][0] * sw + inv[1][1] * tw + inv[1][2] * d;
				pos[2] = inv[2][0] * sw + inv[2][1] * tw + inv[2][2] * d;

				VectorSubtract( light->origin, pos, delta );
				float len = sqrtf( DotProduct( delta, delta ));

				if ( len >= light->radius || len < 0.001f )
					continue;

				float ndotl = DotProduct( delta, facenormal ) / len;
				if ( ndotl <= 0.0f )
					continue;

				float atten;

				if ( light->spot )
				{
					float fwd = -DotProduct( delta, light->forward );
					if ( fwd <= 1.0f )
						continue;

					float ux = -DotProduct( delta, light->right ) / ( fwd * light->tanhor );
					float uy = -DotProduct( delta, light->up ) / ( fwd * light->tanver );
					float spot = 1.0f - ( ux * ux + uy * uy );
					if ( spot <= 0.0f )
						continue;

					float f = fwd / light->radius;
					if ( f >= 1.0f )
						continue;

					atten = spot * ( 1.0f - f * f );
				}
				else
				{
					float f = len / light->radius;
					atten = 1.0f - f * f;
				}

				float k = ndotl * atten;
				int addr = Q_max( (int)( light->intensity[0] * k ), 0 );
				int addg = Q_max( (int)( light->intensity[1] * k ), 0 );
				int addb = Q_max( (int)( light->intensity[2] * k ), 0 );
				if ( addr <= 0 && addg <= 0 && addb <= 0 )
					continue;

				int r = bl->r + addr, g = bl->g + addg, b = bl->b + addb;
				bl->r = r > 255 ? 255 : r;
				bl->g = g > 255 ? 255 : g;
				bl->b = b > 255 ? 255 : b;
				lit = true;
			}
		}
	}

	return lit;
}

static void HLUpload( msurface_t *surf )
{
	int smax = ( surf->extents[0] >> 4 ) + 1;
	int tmax = ( surf->extents[1] >> 4 ) + 1;

	gl.glBindTexture( GL_TEXTURE_2D, g_lightmap_baselight + surf->lightmaptexturenum );
	gl.glTexSubImage2D( GL_TEXTURE_2D, 0, surf->light_s, surf->light_t, smax, tmax, GL_RGB, GL_UNSIGNED_BYTE, blocklights );
}

static void CollectLights( void )
{
	float time = gEngfuncs.GetClientTime();

	g_iNumLMLights = 0;

	// flashlights go first so the map lights can't use up the surface list
	for ( int pass = 0; pass < 2; pass++ )
	for ( int i = 0; i < MAX_PARANOIA_DLIGHTS; i++ )
	{
		DynamicLight *dl = &cl_dlights[i];

		if ( dl->die < time || dl->radius <= 0.0f )
			continue;

		if (( dl->spot_texture[0] != 0 ) != ( pass == 0 ))
			continue;

		// drawn by the spotlight shader instead
		if ( dl->spot_texture[0] && SpotShaderActive() )
			continue;

		if ( g_iNumLMLights >= MAX_LM_LIGHTS )
			break;

		LMLight *l = &g_LMLights[g_iNumLMLights++];
		VectorCopy( dl->origin, l->origin );
		l->radius = dl->radius;

		if ( dl->spot_texture[0] )
		{
			vec3_t angles = dl->angles;
			if ( angles[PITCH] == -90 ) angles[PITCH] = -89.99;
			else if ( angles[PITCH] == 90 ) angles[PITCH] = 89.99;
			AngleVectors( angles, l->forward, l->right, l->up );
			l->spot = 1;
			l->tanhor = tan(( M_PI / 360.0 ) * dl->cone_hor );
			l->tanver = tan(( M_PI / 360.0 ) * dl->cone_ver );
			if ( l->tanhor < 0.01f ) l->tanhor = 0.01f;
			if ( l->tanver < 0.01f ) l->tanver = 0.01f;

			// 0.46 is the normal flashlight colour, the reduced light is dimmer
			for ( int k = 0; k < 3; k++ )
			{
				float scale = dl->color[k] / 0.46f;
				if ( scale > 1.5f ) scale = 1.5f;
				if ( scale < 0.0f ) scale = 0.0f;
				l->intensity[k] = 255.0f * scale;
			}
		}
		else
		{
			l->spot = 0;
			for ( int k = 0; k < 3; k++ )
			{
				float v = ApplyGamma( dl->color[k] ) * 127.5f;
				l->intensity[k] = v > 510.0f ? 510.0f : ( v > 0.0f ? v : 0.0f );
			}
		}
	}
}

// adds the dynamic lights into the lightmaps every frame
// walls and bump surfaces light up this way
void UpdateHLLightmaps( void )
{
	if ( !gl_dynlight_lightmap )
		gl_dynlight_lightmap = gEngfuncs.pfnRegisterVariable( "gl_dynlight_lightmap", "1", FCVAR_ARCHIVE );

	model_t *wm = gEngfuncs.GetEntityByIndex( 0 )->model;

	if ( !wm || !lightmaps_initialized || !gl.glTexSubImage2D )
		return;

	world = wm;

	if ( g_pHLWorld != wm || g_iHLMarkSize != wm->numsurfaces )
	{
		delete[] g_pHLMarks;
		g_iHLMarkSize = wm->numsurfaces;
		g_pHLMarks = new int[g_iHLMarkSize];
		memset( g_pHLMarks, 0, sizeof( int ) * g_iHLMarkSize );
		g_pHLWorld = wm;
		g_iNumHLDirty = 0;
		g_iHLFrame = 0;
	}

	if ( LightmapDlightsActive() && gEngfuncs.pfnGetCvarFloat( "gl_dynlight" ) != 0.0f )
		CollectLights();
	else
		g_iNumLMLights = 0;

	if ( !g_iNumLMLights && !g_iNumHLDirty )
		return;

	g_iHLFrame++;
	g_iNumHLCandidates = 0;

	for ( int i = 0; i < g_iNumLMLights; i++ )
	{
		HLMarkNode( wm->nodes, &g_LMLights[i] );
		HLMarkEntities( &g_LMLights[i] );
	}

	GLint oldtex = 0, oldunit = GL_TEXTURE0_ARB, oldalign = 4;
	gl.glGetIntegerv( GL_ACTIVE_TEXTURE_ARB, &oldunit );
	gl.glActiveTextureARB( GL_TEXTURE0_ARB );
	gl.glGetIntegerv( GL_TEXTURE_BINDING_2D, &oldtex );
	gl.glGetIntegerv( GL_UNPACK_ALIGNMENT, &oldalign );
	gl.glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

	int numlit = 0;
	static msurface_t *s_pLit[MAX_HL_SURFS];

	for ( int i = 0; i < g_iNumHLCandidates; i++ )
	{
		msurface_t *surf = g_pHLCandidates[i];

		HLBuildBase( surf );

		if ( HLAddLights( surf ))
		{
			HLUpload( surf );
			s_pLit[numlit++] = surf;
		}
		else
			g_pHLMarks[surf - wm->surfaces] = g_iHLFrame - 1;
	}

	for ( int i = 0; i < g_iNumHLDirty; i++ )
	{
		msurface_t *surf = g_pHLDirty[i];
		int index = surf - wm->surfaces;

		if ( index >= 0 && index < g_iHLMarkSize && g_pHLMarks[index] == g_iHLFrame )
			continue;

		HLBuildBase( surf );
		HLUpload( surf );
	}

	memcpy( g_pHLDirty, s_pLit, sizeof( msurface_t * ) * numlit );
	g_iNumHLDirty = numlit;

	gl.glPixelStorei( GL_UNPACK_ALIGNMENT, oldalign );
	gl.glBindTexture( GL_TEXTURE_2D, oldtex );
	gl.glActiveTextureARB( oldunit );
}
