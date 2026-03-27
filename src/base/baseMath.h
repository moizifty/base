#ifndef BASE_MATH_H
#define BASE_MATH_H

// NOTE: matrices are represented with column vectors
// Which means the columns of the matrices are the basis vectors
// this also means that matrices are right multipled by column vectors
// eg: Mv, where M is a matrix and v is a column vector.
#include <math.h>
#include "baseCore.h"
#include "baseMetagen.h"
#include "baseMath.gen.h"

#define BASE_PI (3.14159265358979323846)

#define BASE_SQRTF32  sqrtf
#define BASE_SINF32  sinf
#define BASE_COSF32  cosf
#define BASE_TANF32  tanf
#define BASE_ASINF32  asinf
#define BASE_ACOSF32  acosf
#define BASE_ATANF32  atanf
#define BASE_ATAN2F32  atan2f

#define baseRadToDegF32(RAD)   ((RAD) * (180.0f) / ((f32)BASE_PI))
#define baseDegToRadF32(DEG)   ((DEG) * ((f32)(BASE_PI)) / 180.0f)
#define baseClamp(V, MN, MX)   (((V) < (MN)) ? (MN) : (((V) > (MX)) ? (MX) : (V)))
#define baseGreater(V, MN)   (((V) < (MN)) ? (MN) : (V))
#define baseLesser(V, MN)   (((V) < (MN)) ? (V) : (MN))

#define Vec2f(X, Y)    ((vec2f){.x = (f32)(X), .y = (f32)(Y)})
#define Vec2fFromVec(V)   (Vec2f((V).x, (V).y))

#define Vec2i(X, Y)    ((vec2i){.x = (i64)(X), .y = (i64)(Y)})
#define Vec2iFromVec(V)   (Vec2i((V).x, (V).y))

#define Vec2i8(X, Y)    ((vec2i8){.x = (i8)(X), .y = (i8)(Y)})
#define Vec2i8FromVec(V)   (Vec2i8((V).x, (V).y))

#define Vec3f(X, Y, Z)    ((vec3f){.x = (f32)(X), .y = (f32)(Y), .z = (f32)(Z)})
#define Vec3fFromVec(V)   (Vec3f((V).x, (V).y, (V).z))

#define Vec3i8(X, Y, Z)    ((vec3i8){.x = (i8)(X), .y = (i8)(Y), .z = (i8)(Z)})
#define Vec3i8FromVec(V)   (Vec3i8((V).x, (V).y, (V).z))

#define Vec3u8(X, Y, Z)    ((vec3u8){.x = (u8)(X), .y = (u8)(Y), .z = (u8)(Z)})
#define Vec3u8FromVec(V)   (Vec3u8((V).x, (V).y, (V).z))

#define Vec4f(X, Y, Z, W)    ((vec4f){.x = (f32)(X), .y = (f32)(Y), .z = (f32)(Z), .w = (f32)(W)})
#define Vec4fFromVec(V)   (Vec4f((V).x, (V).y, (V).z, (V).w))

#define Vec4i8(X, Y, Z, W)    ((vec4i8){.x = (i8)(X), .y = (i8)(Y), .z = (i8)(Z), .w = (i8)(W)})
#define Vec4i8FromVec(V)   (Vec4i8((V).x, (V).y, (V).z, (V).w))

#define Vec4u8(X, Y, Z, W)    ((vec4u8){.x = (u8)(X), .y = (u8)(Y), .z = (u8)(Z), .w = (u8)(W)})
#define Vec4u8FromVec(V)   (Vec4u8((V).x, (V).y, (V).z, (V).w))

#define Quatf(X, Y, Z, W)    ((quatf){.x = (f32)(X), .y = (f32)(Y), .z = (f32)(Z), .w = (f32)(W)})
#define QUATF_IDENTITY  Quatf(0, 0, 0, 1)

#define Mat3f(DIAG)   ((mat3f){.m00 = (f32)(DIAG), .m11 = (f32)(DIAG), .m22 = (f32)(DIAG)})
#define Mat3fFromMat(M)   ((mat3f){.m00 = (f32)(M).m00, .m01 = (f32)(M).m01, .m02 = (f32)(M).m02, \
                                       .m10 = (f32)(M).m10, .m11 = (f32)(M).m11, .m12 = (f32)(M).m12, \
                                       .m20 = (f32)(M).m20, .m21 = (f32)(M).m21, .m22 = (f32)(M).m22, })
#define MAT3F_IDENTITY    (Mat3f(1))

#define Mat4f(DIAG)   ((mat4f){.m00 = (f32)(DIAG), .m11 = (f32)(DIAG), .m22 = (f32)(DIAG), .m33 = (f32)(DIAG)})
#define Mat4fFromMat(M)   ((mat4f){.m00 = (f32)(M).m00, .m01 = (f32)(M).m01, .m02 = (f32)(M).m02, .m03 = (f32)(M).m03, \
                                       .m10 = (f32)(M).m10, .m11 = (f32)(M).m11, .m12 = (f32)(M).m12, .m13 = (f32)(M).m13, \
                                       .m20 = (f32)(M).m20, .m21 = (f32)(M).m21, .m22 = (f32)(M).m22, .m23 = (f32)(M).m23, \
                                       .m30 = (f32)(M).m30, .m31 = (f32)(M).m31, .m32 = (f32)(M).m32, .m33 = (f32)(M).m33, })

#define Mat4fFromMat3(M)   ((mat4f){.m00 = (f32)(M).m00, .m01 = (f32)(M).m01, .m02 = (f32)(M).m02, .m03 = 0, \
                                       .m10 = (f32)(M).m10, .m11 = (f32)(M).m11, .m12 = (f32)(M).m12, .m13 = 0, \
                                       .m20 = (f32)(M).m20, .m21 = (f32)(M).m21, .m22 = (f32)(M).m22, .m23 = 0, \
                                       .m30 = 0, .m31 = 0, .m32 = 0, .m33 = 1, })


#define MAT4F_IDENTITY    (Mat4f(1))

#define Rangef(X, Y)    ((rangef){(X), (Y)})


#define Range2f(X, Y)    ((rangef){(f32)(X), (f32)(Y)})
#define Range2i(X, Y)    ((rangei){(i64)(X), (i64)(Y)})
#define Range2iDim(R)    ((vec2i){.x = (R).x1 - (R).x0, .y = (R).y1 - (R).y0})


#define Range3f(X, Y)    ((rangef){(X), (Y)})

metagen_introspect(only: "x", "y")
typedef struct vec2f
{
    union
    {
        f32 v[2];
        struct
        {
            f32 x;
            f32 y;
        };
        struct
        {
            f32 width;
            f32 height;
        };
        struct
        {
            f32 w;
            f32 h;
        };
    };   
}vec2f;

typedef struct vec2i
{
    union
    {
        i64 v[2];
        struct
        {
            i64 x;
            i64 y;
        };
        struct
        {
            i64 width;
            i64 height;
        };
        struct
        {
            i64 w;
            i64 h;
        };
    };   
}vec2i;

typedef struct vec2i8
{
    union
    {
        i8 v[2];
        struct
        {
            i8 x;
            i8 y;
        };
        struct
        {
            i8 width;
            i8 height;
        };
        struct
        {
            i8 w;
            i8 h;
        };
    };   
}vec2i8;

metagen_introspect(only: "x", "y", "z")
typedef struct vec3f
{
    union
    {
        f32 v[3];

        struct
        {
            f32 x;
            f32 y;
            f32 z;
        };

        struct
        {
            f32 r;
            f32 g;
            f32 b;
        };

        struct
        {
            vec2f xy;
            f32 _z;
        };
    };   
}vec3f;

typedef struct vec3i8
{
    union
    {
        i8 v[3];
        struct
        {
            i8 x;
            i8 y;
            i8 z;
        };
        struct
        {
            i8 r;
            i8 g;
            i8 b;
        };
        struct
        {
            vec2i8 xy;
            i8 _z;
        };
    };   
}vec3i8;

typedef struct vec3u8
{
    union
    {
        u8 v[3];
        struct
        {
            u8 x;
            u8 y;
            u8 z;
        };
        struct
        {
            u8 r;
            u8 g;
            u8 b;
        };
    };   
}vec3u8;

typedef struct vec4f
{
    union
    {
        f32 v[4];
        struct
        {
            f32 x;
            f32 y;
            f32 z;
            f32 w;
        };
        struct
        {
            f32 r;
            f32 g;
            f32 b;
            f32 a;
        };
        struct
        {
            vec3f xyz;
            f32 _w;
        };
    };   
}vec4f;

typedef struct vec4i8
{
    union
    {
        i8 v[4];
        struct
        {
            i8 x;
            i8 y;
            i8 z;
            i8 w;
        };
        struct
        {
            i8 r;
            i8 g;
            i8 b;
            i8 a;
        };
        struct
        {
            vec3i8 xyz;
            i8 _w;
        };
    };   
}vec4i8;

typedef struct vec4u8
{
    union
    {
        u8 v[4];
        u32 asU32;
        struct
        {
            u8 x;
            u8 y;
            u8 z;
            u8 w;
        };
        struct
        {
            u8 r;
            u8 g;
            u8 b;
            u8 a;
        };
        struct
        {
            vec3u8 xyz;
            u8 _w;
        };
    };   
}vec4u8;

typedef struct quatf
{
    union
    {
        f32 elems[4];
        vec4f v;
        struct
        {
            f32 x;
            f32 y;
            f32 z;
            f32 w;
        };
        struct
        {
            vec3f xyz;
            f32 _w;
        };
    };   
}quatf;

typedef struct mat3f
{
    union
    {
        vec3f v[3];
        f32 m[3 * 3];
        f32 rows[3][3];
        struct
        {
            f32 m00, m01, m02;
            f32 m10, m11, m12;
            f32 m20, m21, m22;
        };
    };
}mat3f;

typedef struct mat4f
{
    union
    {
        vec4f v[4];
        f32 m[4 * 4];
        f32 rows[4][4];
        struct
        {
            f32 m00, m01, m02, m03;
            f32 m10, m11, m12, m13;
            f32 m20, m21, m22, m23;
            f32 m30, m31, m32, m33;
        };
    };
}mat4f;

// 1-dimension range
typedef struct rangef
{
    union
    {
        struct
        {
            f32 start;
            f32 end;
        };

        struct
        {
            f32 min;
            f32 max;
        };

        struct
        {
            f32 width;
            f32 height;
        };
    };
}rangef;

// 2-dimension range
typedef struct range2f
{
    union
    {
        struct
        {
            vec2f start;
            vec2f end;
        };

        struct
        {
            vec2f min;
            vec2f max;
        };

        struct
        {
            vec2f topleft;
            vec2f bottomright;
        };

        struct
        {
            f32 x0;
            f32 y0;
            f32 x1;
            f32 y1;
        };
    };
}range2f;

typedef struct range2i
{
    union
    {
        struct
        {
            vec2i start;
            vec2i end;
        };

        struct
        {
            vec2i min;
            vec2i max;
        };

        struct
        {
            vec2i topleft;
            vec2i bottomright;
        };

        struct
        {
            i64 x0;
            i64 y0;
            i64 x1;
            i64 y1;
        };
    };
}range2i;

// 3-dimension range

metagen_introspect()
typedef struct range3f
{
    union
    {
        struct
        {
            vec3f start;
            vec3f end;
        };

        struct
        {
            vec3f min;
            vec3f max;
        };

        struct
        {
            f32 x0;
            f32 y0;
            f32 z0;
            f32 x1;
            f32 y1;
            f32 z1;
        };
    };
}range3f;

vec2f vec2fAdd(vec2f a, vec2f b);
vec2f vec2fSub(vec2f a, vec2f b);
vec2f vec2fMult(vec2f a, vec2f b); // componentwise mult
vec2f vec2fDiv(vec2f a, vec2f b); // componentwise mult
vec2f vec2fMultF32(vec2f a, f32 s);
vec2f vec2fDivF32(vec2f a, f32 s);
vec2f vec2fNorm(vec2f a);
f32 vec2fDot(vec2f a, vec2f b);
f32 vec2fMag(vec2f a);
f32 vec2fMagSqr(vec2f a);

vec3f vec3fAdd(vec3f a, vec3f b);
vec3f vec3fSub(vec3f a, vec3f b);
vec3f vec3fMult(vec3f a, vec3f b); // componentwise mult
vec3f vec3fDiv(vec3f a, vec3f b); // componentwise mult
vec3f vec3fMultF32(vec3f a, f32 s);
vec3f vec3fDivF32(vec3f a, f32 s);
vec3f vec3fNorm(vec3f a);
vec3f vec3fCross(vec3f a, vec3f b);
f32 vec3fDot(vec3f a, vec3f b);
f32 vec3fMag(vec3f a);
f32 vec3fMagSqr(vec3f a);

vec4f vec4fAdd(vec4f a, vec4f b);
vec4f vec4fSub(vec4f a, vec4f b);
vec4f vec4fMult(vec4f a, vec4f b); // componentwise mult
vec4f vec4fDiv(vec4f a, vec4f b); // componentwise mult
vec4f vec4fMultF32(vec4f a, f32 s);
vec4f vec4fDivF32(vec4f a, f32 s);
vec4f vec4fNorm(vec4f a);
f32 vec4fDot(vec4f a, vec4f b);
f32 vec4fMag(vec4f a);
f32 vec4fMagSqr(vec4f a);

//mat3f
mat3f mat3fFromColVec3f(vec3f cols[3]);
mat3f mat3fFromRowVec3f(vec3f rows[3]);
mat3f mat3fTranspose(mat3f a);
f32 mat3fDet(mat3f a);
mat3f mat3fInverse(mat3f a);
mat3f mat3fAdd(mat3f a, mat3f b);
mat3f mat3fSub(mat3f a, mat3f b);
mat3f mat3fMult(mat3f a, mat3f b);
mat3f mat3fMultComponentWise(mat3f a, mat3f b);
mat3f mat3fMultF32(mat3f a, f32 s);
vec3f mat3fMultVec3f(mat3f a, vec3f v);
mat3f mat3fDiv(mat3f a, mat3f b);
mat3f mat3fDivComponentWise(mat3f a, mat3f b);
mat3f mat3fDivF32(mat3f a, f32 s);
mat3f mat3fNormCols(mat3f a);
mat3f mat3fNormRows(mat3f a);

mat3f mat3fGiveScale(vec3f scale);
mat3f mat3fGiveScaleUniform(f32 scale);
mat3f mat3fGiveScaleAxis(vec3f axis, f32 scale);
mat3f mat3fScale(mat3f a, vec3f scale);
mat3f mat3fScaleUniform(mat3f a, f32 scale);
mat3f mat3fScaleAxis(mat3f a, vec3f axis, f32 scale);

mat3f mat3fGiveRotateX(f32 rad);
mat3f mat3fGiveRotateY(f32 rad);
mat3f mat3fGiveRotateZ(f32 rad);
mat3f mat3fGiveRotateYXZ(f32 radX, f32 radY, f32 radZ);
mat3f mat3fGiveRotateZXY(f32 radX, f32 radY, f32 radZ);
mat3f mat3fGiveRotateAxis(vec3f axis, f32 rad);
mat3f mat3fRotateX(mat3f a, f32 rad);
mat3f mat3fRotateY(mat3f a, f32 rad);
mat3f mat3fRotateZ(mat3f a, f32 rad);
mat3f mat3fRotateYXZ(mat3f a, f32 radX, f32 radY, f32 radZ);
mat3f mat3fRotateZXY(mat3f a, f32 radX, f32 radY, f32 radZ);
mat3f mat3fRotateAxis(mat3f a, vec3f axis, f32 rad);

// mat4f
mat4f mat4fFromColVec4f(vec4f cols[4]);
mat4f mat4fFromRowVec4f(vec4f rows[4]);
mat4f mat4fTranspose(mat4f a);
f32 mat4fDet(mat4f a);
mat4f mat4fInverse(mat4f a);
mat4f mat4fAdd(mat4f a, mat4f b);
mat4f mat4fSub(mat4f a, mat4f b);
mat4f mat4fMult(mat4f a, mat4f b);
mat4f mat4fMultComponentWise(mat4f a, mat4f b);
mat4f mat4fMultF32(mat4f a, f32 s);
vec4f mat4fMultVec4f(mat4f a, vec4f v);
mat4f mat4fDiv(mat4f a, mat4f b);
mat4f mat4fDivComponentWise(mat4f a, mat4f b);
mat4f mat4fDivF32(mat4f a, f32 s);
mat4f mat4fNormCols(mat4f a);
mat4f mat4fNormRows(mat4f a);

mat4f mat4fGiveScale(vec4f scale);
mat4f mat4fGiveScaleUniform(f32 scale);
mat4f mat4fGiveScaleAxis(vec3f axis, f32 scale);
mat4f mat4fScale(mat4f a, vec4f scale);
mat4f mat4fScaleUniform(mat4f a, f32 scale);
mat4f mat4fScaleAxis(mat4f a, vec3f axis, f32 scale);

mat4f mat4fGiveRotateX(f32 rad);
mat4f mat4fGiveRotateY(f32 rad);
mat4f mat4fGiveRotateZ(f32 rad);
mat4f mat4fGiveRotateYXZ(f32 radX, f32 radY, f32 radZ);
mat4f mat4fGiveRotateZXY(f32 radX, f32 radY, f32 radZ);
mat4f mat4fGiveRotateAxis(vec3f axis, f32 rad);
mat4f mat4fRotateX(mat4f a, f32 rad);
mat4f mat4fRotateY(mat4f a, f32 rad);
mat4f mat4fRotateZ(mat4f a, f32 rad);
mat4f mat4fRotateYXZ(mat4f a, f32 radX, f32 radY, f32 radZ);
mat4f mat4fRotateZXY(mat4f a, f32 radX, f32 radY, f32 radZ);
mat4f mat4fRotateAxis(mat4f a, vec3f axis, f32 rad);

mat4f mat4fGiveTranslate(vec3f t);
mat4f mat4fTranslate(mat4f a, vec3f t);

// scale -> rotate -> translate
mat4f mat4fGiveTransformSRT(vec3f translate, vec3f rotateRads, vec4f scale);
// scale -> rotate -> translate
mat4f mat4fTransformSRT(mat4f a, vec3f translate, vec3f rotateRads, vec4f scale);

//quatf
quatf quatfNegate(quatf q);
quatf quatfConjugate(quatf q);
quatf quatfInverse(quatf q);
quatf quatfNorm(quatf q);
quatf quatfAdd(quatf a, quatf b);
quatf quatfSub(quatf a, quatf b);
//returns a quaternion that gives a rotation from 'a' to 'b' essentionlly b - a in vector terms
quatf quatfDifference(quatf a, quatf b);
quatf quatfMult(quatf a, quatf b);
quatf quatfMultF32(quatf a, f32 s);
vec3f quatfMultVec3f(quatf a, vec3f v);
quatf quatfMultComponentWise(quatf a, quatf b);
quatf quatfDiv(quatf a, quatf b);
quatf quatfDivF32(quatf a, f32 s);
quatf quatfDivComponentWise(quatf a, quatf b);
f32 quatfDot(quatf a, quatf b);
f32 quatfMag(quatf q);
f32 quatfMagSqr(quatf q);

quatf quatfFromEulerYXZ(vec3f angleRads);
vec3f quatfToEulerYXZ(quatf q);
vec3f quatfToEulerZXY(quatf q);
mat3f quatfToMat3f(quatf q);
mat4f quatfToMat4f(quatf q);

quatf quatfGiveRotateAxis(vec3f axis, f32 rad);
quatf quatfGiveRotateYXZ(f32 radX, f32 radY, f32 radZ);
quatf quatfGiveRotateZXY(f32 radX, f32 radY, f32 radZ);
quatf quatfRotateAxis(quatf q, vec3f axis, f32 rad);
quatf quatfRotateYXZ(quatf q, f32 radX, f32 radY, f32 radZ);
quatf quatfRotateZXY(quatf q, f32 radX, f32 radY, f32 radZ);
vec3f quatfRotateAxisVec3f(vec3f v, vec3f axis, f32 rad);
vec3f quatfRotateYXZVec3f(vec3f v, f32 radX, f32 radY, f32 radZ);
vec3f quatfRotateZXYVec3f(vec3f v, f32 radX, f32 radY, f32 radZ);

#include "baseMath.gen.h"

#endif