#pragma once
#if !defined(COF_GL_H)
#define COF_GL_H

#if defined(_WIN32) && !defined(APIENTRY)
#define APIENTRY __stdcall
#define GL_RED_BITS                              0x0D52
#define GL_GREEN_BITS                            0x0D53
#define GL_BLUE_BITS                             0x0D54
#define GL_ALPHA_BITS                            0x0D55
#define GL_DEPTH_BITS                            0x0D56
#define GL_STENCIL_BITS                          0x0D57
#define GL_ACCUM_RED_BITS                        0x0D58
#define GL_ACCUM_GREEN_BITS                      0x0D59
#define GL_ACCUM_BLUE_BITS                       0x0D5A
#define GL_ACCUM_ALPHA_BITS                      0x0D5B
#define GL_ALPHA_TEST_FUNC                       0x0BC1
#define GL_ALPHA_TEST_REF                        0x0BC2
#define GL_BLUE                                  0x1905
#define GL_PERSPECTIVE_CORRECTION_HINT           0x0C50
#define GL_TEXTURE_WIDTH                         0x1000
#define GL_TEXTURE_HEIGHT                        0x1001

#endif
#if !defined(APIENTRY)
#define APIENTRY
#define GL_RED_BITS                              0x0D52
#define GL_GREEN_BITS                            0x0D53
#define GL_BLUE_BITS                             0x0D54
#define GL_ALPHA_BITS                            0x0D55
#define GL_DEPTH_BITS                            0x0D56
#define GL_STENCIL_BITS                          0x0D57
#define GL_ACCUM_RED_BITS                        0x0D58
#define GL_ACCUM_GREEN_BITS                      0x0D59
#define GL_ACCUM_BLUE_BITS                       0x0D5A
#define GL_ACCUM_ALPHA_BITS                      0x0D5B
#define GL_ALPHA_TEST_FUNC                       0x0BC1
#define GL_ALPHA_TEST_REF                        0x0BC2
#define GL_BLUE                                  0x1905
#define GL_PERSPECTIVE_CORRECTION_HINT           0x0C50
#define GL_TEXTURE_WIDTH                         0x1000
#define GL_TEXTURE_HEIGHT                        0x1001

#endif
#if !defined(APIENTRYP)
#define APIENTRYP APIENTRY *
#define GL_RED_BITS                              0x0D52
#define GL_GREEN_BITS                            0x0D53
#define GL_BLUE_BITS                             0x0D54
#define GL_ALPHA_BITS                            0x0D55
#define GL_DEPTH_BITS                            0x0D56
#define GL_STENCIL_BITS                          0x0D57
#define GL_ACCUM_RED_BITS                        0x0D58
#define GL_ACCUM_GREEN_BITS                      0x0D59
#define GL_ACCUM_BLUE_BITS                       0x0D5A
#define GL_ACCUM_ALPHA_BITS                      0x0D5B
#define GL_ALPHA_TEST_FUNC                       0x0BC1
#define GL_ALPHA_TEST_REF                        0x0BC2
#define GL_BLUE                                  0x1905
#define GL_PERSPECTIVE_CORRECTION_HINT           0x0C50
#define GL_TEXTURE_WIDTH                         0x1000
#define GL_TEXTURE_HEIGHT                        0x1001

#endif
#if !defined(WINGDIAPI)
#define WINGDIAPI
#define GL_RED_BITS                              0x0D52
#define GL_GREEN_BITS                            0x0D53
#define GL_BLUE_BITS                             0x0D54
#define GL_ALPHA_BITS                            0x0D55
#define GL_DEPTH_BITS                            0x0D56
#define GL_STENCIL_BITS                          0x0D57
#define GL_ACCUM_RED_BITS                        0x0D58
#define GL_ACCUM_GREEN_BITS                      0x0D59
#define GL_ACCUM_BLUE_BITS                       0x0D5A
#define GL_ACCUM_ALPHA_BITS                      0x0D5B
#define GL_ALPHA_TEST_FUNC                       0x0BC1
#define GL_ALPHA_TEST_REF                        0x0BC2
#define GL_BLUE                                  0x1905
#define GL_PERSPECTIVE_CORRECTION_HINT           0x0C50
#define GL_TEXTURE_WIDTH                         0x1000
#define GL_TEXTURE_HEIGHT                        0x1001

#endif

typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef void GLvoid;
typedef signed char GLbyte;
typedef short GLshort;
typedef int GLint;
typedef unsigned char GLubyte;
typedef unsigned short GLushort;
typedef unsigned int GLuint;
typedef int GLsizei;
typedef float GLfloat;
typedef float GLclampf;
typedef double GLdouble;
typedef double GLclampd;

#define GL_ADD                                   0x0104
#define GL_ALL_ATTRIB_BITS                       0x000fffff
#define GL_ALPHA                                 0x1906
#define GL_ALPHA8                                0x803C
#define GL_ALPHA_SCALE                           0x0D1C
#define GL_ALPHA_TEST                            0x0BC0
#define GL_ALWAYS                                0x0207
#define GL_BACK                                  0x0405
#define GL_BLEND                                 0x0BE2
#define GL_CLAMP                                 0x2900
#define GL_CLIP_PLANE0                           0x3000
#define GL_CULL_FACE                             0x0B44
#define GL_DECR                                  0x1E03
#define GL_DEPTH_TEST                            0x0B71
#define GL_DST_ALPHA                             0x0304
#define GL_DST_COLOR                             0x0306
#define GL_EQUAL                                 0x0202
#define GL_EXTENSIONS                            0x1F03
#define GL_EYE_LINEAR                            0x2400
#define GL_EYE_PLANE                             0x2502
#define GL_FALSE                                 0x0
#define GL_FLAT                                  0x1D00
#define GL_FLOAT                                 0x1406
#define GL_FOG                                   0x0B60
#define GL_FOG_COLOR                             0x0B66
#define GL_FOG_DENSITY                           0x0B62
#define GL_FOG_END                               0x0B64
#define GL_FOG_HINT                              0x0C54
#define GL_FOG_MODE                              0x0B65
#define GL_FOG_START                             0x0B63
#define GL_FRONT                                 0x0404
#define GL_GREATER                               0x0204
#define GL_INCR                                  0x1E02
#define GL_KEEP                                  0x1E00
#define GL_LEQUAL                                0x0203
#define GL_LINEAR                                0x2601
#define GL_LINEAR_MIPMAP_LINEAR                  0x2703
#define GL_LINEAR_MIPMAP_NEAREST                 0x2701
#define GL_LUMINANCE                             0x1909
#define GL_MAX_TEXTURE_SIZE                      0x0D33
#define GL_VIEWPORT                              0x0BA2
#define GL_MODELVIEW                             0x1700
#define GL_MODULATE                              0x2100
#define GL_NEAREST                               0x2600
#define GL_NICEST                                0x1102
#define GL_NONE                                  0
#define GL_NOTEQUAL                              0x0205
#define GL_NO_ERROR                              0x0
#define GL_ONE                                   0x1
#define GL_ONE_MINUS_SRC_ALPHA                   0x0303
#define GL_OUT_OF_MEMORY                         0x0505
#define GL_POLYGON                               0x0009
#define GL_POLYGON_OFFSET_FILL                   0x8037
#define GL_PROJECTION                            0x1701
#define GL_Q                                     0x2003
#define GL_QUADS                                 0x0007
#define GL_R                                     0x2002
#define GL_RENDERER                              0x1F01
#define GL_REPEAT                                0x2901
#define GL_REPLACE                               0x1E01
#define GL_RGB                                   0x1907
#define GL_RGBA                                  0x1908
#define GL_RGBA8                                 0x8058
#define GL_S                                     0x2000
#define GL_SMOOTH                                0x1D01
#define GL_SRC_ALPHA                             0x0302
#define GL_SRC_COLOR                             0x0300
#define GL_STENCIL_BUFFER_BIT                    0x00000400
#define GL_STENCIL_TEST                          0x0B90
#define GL_T                                     0x2001
#define GL_TEXTURE                               0x1702
#define GL_TEXTURE_1D                            0x0DE0
#define GL_TEXTURE_2D                            0x0DE1
#define GL_TEXTURE_BINDING_2D                    0x8069
#define GL_TEXTURE_COORD_ARRAY                   0x8078
#define GL_TEXTURE_ENV                           0x2300
#define GL_TEXTURE_ENV_COLOR                     0x2201
#define GL_TEXTURE_ENV_MODE                      0x2200
#define GL_TEXTURE_GEN_MODE                      0x2500
#define GL_TEXTURE_GEN_Q                         0x0C63
#define GL_TEXTURE_GEN_R                         0x0C62
#define GL_TEXTURE_GEN_S                         0x0C60
#define GL_TEXTURE_GEN_T                         0x0C61
#define GL_TEXTURE_MAG_FILTER                    0x2800
#define GL_TEXTURE_MIN_FILTER                    0x2801
#define GL_TEXTURE_WRAP_S                        0x2802
#define GL_TEXTURE_WRAP_T                        0x2803
#define GL_TRIANGLES                             0x0004
#define GL_TRIANGLE_FAN                          0x0006
#define GL_TRIANGLE_STRIP                        0x0005
#define GL_TRUE                                  0x1
#define GL_UNPACK_ALIGNMENT                      0x0CF5
#define GL_UNSIGNED_BYTE                         0x1401
#define GL_VENDOR                                0x1F00
#define GL_VERSION                               0x1F02
#define GL_VERTEX_ARRAY                          0x8074
#define GL_ZERO                                  0x0

#define GL_RED_BITS                              0x0D52
#define GL_GREEN_BITS                            0x0D53
#define GL_BLUE_BITS                             0x0D54
#define GL_ALPHA_BITS                            0x0D55
#define GL_DEPTH_BITS                            0x0D56
#define GL_STENCIL_BITS                          0x0D57
#define GL_ACCUM_RED_BITS                        0x0D58
#define GL_ACCUM_GREEN_BITS                      0x0D59
#define GL_ACCUM_BLUE_BITS                       0x0D5A
#define GL_ACCUM_ALPHA_BITS                      0x0D5B
#define GL_ALPHA_TEST_FUNC                       0x0BC1
#define GL_ALPHA_TEST_REF                        0x0BC2
#define GL_BLUE                                  0x1905
#define GL_PERSPECTIVE_CORRECTION_HINT           0x0C50
#define GL_TEXTURE_WIDTH                         0x1000
#define GL_TEXTURE_HEIGHT                        0x1001

#endif
