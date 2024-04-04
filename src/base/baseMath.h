#ifndef BASE_MATH_H
#define BASE_MATH_H

// NOTE: matrices are represented with column vectors
// Which means the columns of the matrices are the basis vectors
// this also means that matrices are right multipled by column vectors
// eg: Mv, where M is a matrix and v is a column vector.
#include <math.h>
#include "baseCore.h"

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

#define Vec2f32(X, Y)    ((vec2f32){.x = (f32)(X), .y = (f32)(Y)})
#define Vec2f32FromVec(V)   (Vec2f32((V).x, (V).y))

#define Vec3f32(X, Y, Z)    ((vec3f32){.x = (f32)(X), .y = (f32)(Y), .z = (f32)(Z)})
#define Vec3f32FromVec(V)   (Vec3f32((V).x, (V).y, (V).z))

#define Vec4f32(X, Y, Z, W)    ((vec4f32){.x = (f32)(X), .y = (f32)(Y), .z = (f32)(Z), .w = (f32)(W)})
#define Vec4f32FromVec(V)   (Vec4f32((V).x, (V).y, (V).z, (V).w))

#define Quatf32(X, Y, Z, W)    ((quatf32){.x = (f32)(X), .y = (f32)(Y), .z = (f32)(Z), .w = (f32)(W)})

#define Mat3f32(DIAG)   ((mat3f32){.m00 = (f32)(DIAG), .m11 = (f32)(DIAG), .m22 = (f32)(DIAG)})
#define Mat3f32FromMat(M)   ((mat3f32){.m00 = (f32)(M).m00, .m01 = (f32)(M).m01, .m02 = (f32)(M).m02, \
                                       .m10 = (f32)(M).m10, .m11 = (f32)(M).m11, .m12 = (f32)(M).m12, \
                                       .m20 = (f32)(M).m20, .m21 = (f32)(M).m21, .m22 = (f32)(M).m22, })
#define MAT3F32_IDENTITY    (Mat3f32(1))

#define Mat4f32(DIAG)   ((mat4f32){.m00 = (f32)(DIAG), .m11 = (f32)(DIAG), .m22 = (f32)(DIAG), .m33 = (f32)(DIAG)})
#define Mat4f32FromMat(M)   ((mat4f32){.m00 = (f32)(M).m00, .m01 = (f32)(M).m01, .m02 = (f32)(M).m02, .m03 = (f32)(M).m03, \
                                       .m10 = (f32)(M).m10, .m11 = (f32)(M).m11, .m12 = (f32)(M).m12, .m13 = (f32)(M).m13, \
                                       .m20 = (f32)(M).m20, .m21 = (f32)(M).m21, .m22 = (f32)(M).m22, .m23 = (f32)(M).m23, \
                                       .m30 = (f32)(M).m30, .m31 = (f32)(M).m31, .m32 = (f32)(M).m32, .m33 = (f32)(M).m33, })

#define Mat4f32FromMat3(M)   ((mat4f32){.m00 = (f32)(M).m00, .m01 = (f32)(M).m01, .m02 = (f32)(M).m02, .m03 = 0, \
                                       .m10 = (f32)(M).m10, .m11 = (f32)(M).m11, .m12 = (f32)(M).m12, .m13 = 0, \
                                       .m20 = (f32)(M).m20, .m21 = (f32)(M).m21, .m22 = (f32)(M).m22, .m23 = 0, \
                                       .m30 = 0, .m31 = 0, .m32 = 0, .m33 = 1, })


#define MAT4F32_IDENTITY    (Mat4f32(1))

typedef struct vec2f32
{
    union
    {
        f32 v[2];
        struct
        {
            f32 x;
            f32 y;
        };
    };   
}vec2f32;

typedef struct vec3f32
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
            vec2f32 xy;
            f32 _z;
        };
    };   
}vec3f32;

typedef struct vec4f32
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
            vec3f32 xyz;
            f32 _w;
        };
    };   
}vec4f32;

typedef struct quatf32
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
            vec3f32 xyz;
            f32 _w;
        };
    };   
}quatf32;

typedef struct mat3f32
{
    union
    {
        vec3f32 v[3];
        f32 m[3 * 3];
        f32 rows[3][3];
        struct
        {
            f32 m00, m01, m02;
            f32 m10, m11, m12;
            f32 m20, m21, m22;
        };
    };
}mat3f32;

typedef struct mat4f32
{
    union
    {
        vec4f32 v[4];
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
}mat4f32;

vec2f32 vec2f32Add(vec2f32 a, vec2f32 b);
vec2f32 vec2f32Sub(vec2f32 a, vec2f32 b);
vec2f32 vec2f32Mult(vec2f32 a, vec2f32 b); // componentwise mult
vec2f32 vec2f32Div(vec2f32 a, vec2f32 b); // componentwise mult
vec2f32 vec2f32MultF32(vec2f32 a, f32 s);
vec2f32 vec2f32DivF32(vec2f32 a, f32 s);
vec2f32 vec2f32Norm(vec2f32 a);
f32 vec2f32Dot(vec2f32 a, vec2f32 b);
f32 vec2f32Mag(vec2f32 a);
f32 vec2f32MagSqr(vec2f32 a);

vec3f32 vec3f32Add(vec3f32 a, vec3f32 b);
vec3f32 vec3f32Sub(vec3f32 a, vec3f32 b);
vec3f32 vec3f32Mult(vec3f32 a, vec3f32 b); // componentwise mult
vec3f32 vec3f32Div(vec3f32 a, vec3f32 b); // componentwise mult
vec3f32 vec3f32MultF32(vec3f32 a, f32 s);
vec3f32 vec3f32DivF32(vec3f32 a, f32 s);
vec3f32 vec3f32Norm(vec3f32 a);
f32 vec3f32Dot(vec3f32 a, vec3f32 b);
f32 vec3f32Mag(vec3f32 a);
f32 vec3f32MagSqr(vec3f32 a);

vec4f32 vec4f32Add(vec4f32 a, vec4f32 b);
vec4f32 vec4f32Sub(vec4f32 a, vec4f32 b);
vec4f32 vec4f32Mult(vec4f32 a, vec4f32 b); // componentwise mult
vec4f32 vec4f32Div(vec4f32 a, vec4f32 b); // componentwise mult
vec4f32 vec4f32MultF32(vec4f32 a, f32 s);
vec4f32 vec4f32DivF32(vec4f32 a, f32 s);
vec4f32 vec4f32Norm(vec4f32 a);
f32 vec4f32Dot(vec4f32 a, vec4f32 b);
f32 vec4f32Mag(vec4f32 a);
f32 vec4f32MagSqr(vec4f32 a);

mat3f32 mat3f32FromColVec3f32(vec3f32 cols[3]);
mat3f32 mat3f32FromRowVec3f32(vec3f32 rows[3]);
mat3f32 mat3f32Transpose(mat3f32 a);
f32 mat3f32Det(mat3f32 a);
mat3f32 mat3f32Inverse(mat3f32 a);
mat3f32 mat3f32Add(mat3f32 a, mat3f32 b);
mat3f32 mat3f32Sub(mat3f32 a, mat3f32 b);
mat3f32 mat3f32Mult(mat3f32 a, mat3f32 b);
mat3f32 mat3f32MultComponentWise(mat3f32 a, mat3f32 b);
mat3f32 mat3f32MultF32(mat3f32 a, f32 s);
vec3f32 mat3f32MultVec3f32(mat3f32 a, vec3f32 v);
mat3f32 mat3f32Div(mat3f32 a, mat3f32 b);
mat3f32 mat3f32DivComponentWise(mat3f32 a, mat3f32 b);
mat3f32 mat3f32DivF32(mat3f32 a, f32 s);
mat3f32 mat3f32NormCols(mat3f32 a);
mat3f32 mat3f32NormRows(mat3f32 a);

mat3f32 mat3f32GiveScale(vec3f32 scale);
mat3f32 mat3f32GiveScaleUniform(f32 scale);
mat3f32 mat3f32GiveScaleAxis(vec3f32 axis, f32 scale);
mat3f32 mat3f32Scale(mat3f32 a, vec3f32 scale);
mat3f32 mat3f32ScaleUniform(mat3f32 a, f32 scale);
mat3f32 mat3f32ScaleAxis(mat3f32 a, vec3f32 axis, f32 scale);

mat3f32 mat3f32GiveRotateX(f32 rad);
mat3f32 mat3f32GiveRotateY(f32 rad);
mat3f32 mat3f32GiveRotateZ(f32 rad);
mat3f32 mat3f32GiveRotateYXZ(f32 radX, f32 radY, f32 radZ);
mat3f32 mat3f32GiveRotateAxis(vec3f32 axis, f32 rad);
mat3f32 mat3f32RotateX(mat3f32 a, f32 rad);
mat3f32 mat3f32RotateY(mat3f32 a, f32 rad);
mat3f32 mat3f32RotateZ(mat3f32 a, f32 rad);
mat3f32 mat3f32RotateYXZ(mat3f32 a, f32 radX, f32 radY, f32 radZ);
mat3f32 mat3f32RotateAxis(mat3f32 a, vec3f32 axis, f32 rad);

// mat4f32
mat4f32 mat4f32FromColVec4f32(vec4f32 cols[4]);
mat4f32 mat4f32FromRowVec4f32(vec4f32 rows[4]);
mat4f32 mat4f32Transpose(mat4f32 a);
f32 mat4f32Det(mat4f32 a);
mat4f32 mat4f32Inverse(mat4f32 a);
mat4f32 mat4f32Add(mat4f32 a, mat4f32 b);
mat4f32 mat4f32Sub(mat4f32 a, mat4f32 b);
mat4f32 mat4f32Mult(mat4f32 a, mat4f32 b);
mat4f32 mat4f32MultComponentWise(mat4f32 a, mat4f32 b);
mat4f32 mat4f32MultF32(mat4f32 a, f32 s);
vec4f32 mat4f32MultVec4f32(mat4f32 a, vec4f32 v);
mat4f32 mat4f32Div(mat4f32 a, mat4f32 b);
mat4f32 mat4f32DivComponentWise(mat4f32 a, mat4f32 b);
mat4f32 mat4f32DivF32(mat4f32 a, f32 s);
mat4f32 mat4f32NormCols(mat4f32 a);
mat4f32 mat4f32NormRows(mat4f32 a);

mat4f32 mat4f32GiveScale(vec4f32 scale);
mat4f32 mat4f32GiveScaleUniform(f32 scale);
mat4f32 mat4f32GiveScaleAxis(vec3f32 axis, f32 scale);
mat4f32 mat4f32Scale(mat4f32 a, vec4f32 scale);
mat4f32 mat4f32ScaleUniform(mat4f32 a, f32 scale);
mat4f32 mat4f32ScaleAxis(mat4f32 a, vec3f32 axis, f32 scale);

mat4f32 mat4f32GiveRotateX(f32 rad);
mat4f32 mat4f32GiveRotateY(f32 rad);
mat4f32 mat4f32GiveRotateZ(f32 rad);
mat4f32 mat4f32GiveRotateYXZ(f32 radX, f32 radY, f32 radZ);
mat4f32 mat4f32GiveRotateAxis(vec3f32 axis, f32 rad);
mat4f32 mat4f32RotateX(mat4f32 a, f32 rad);
mat4f32 mat4f32RotateY(mat4f32 a, f32 rad);
mat4f32 mat4f32RotateZ(mat4f32 a, f32 rad);
mat4f32 mat4f32RotateYXZ(mat4f32 a, f32 radX, f32 radY, f32 radZ);
mat4f32 mat4f32RotateAxis(mat4f32 a, vec3f32 axis, f32 rad);

mat4f32 mat4f32GiveTranslate(vec3f32 t);
mat4f32 mat4f32Translate(mat4f32 a, vec3f32 t);

// scale -> rotate -> translate
mat4f32 mat4f32GiveTransformSRT(vec3f32 translate, vec3f32 rotateRads, vec4f32 scale);
// scale -> rotate -> translate
mat4f32 mat4f32TransformSRT(mat4f32 a, vec3f32 translate, vec3f32 rotateRads, vec4f32 scale);
#endif