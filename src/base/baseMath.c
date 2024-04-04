#include "baseMath.h"

vec2f32 vec2f32Add(vec2f32 a, vec2f32 b)
{
    return Vec2f32(a.x + b.x, a.y + b.y);
}
vec2f32 vec2f32Sub(vec2f32 a, vec2f32 b)
{
    return Vec2f32(a.x - b.x, a.y - b.y);
}
vec2f32 vec2f32Mult(vec2f32 a, vec2f32 b)
{
    return Vec2f32(a.x * b.x, a.y * b.y);
}
vec2f32 vec2f32Div(vec2f32 a, vec2f32 b)
{
    return Vec2f32(a.x / b.x, a.y / b.y);
}
vec2f32 vec2f32MultF32(vec2f32 a, f32 s)
{
    return Vec2f32(a.x * s, a.y * s);
}
vec2f32 vec2f32DivF32(vec2f32 a, f32 s)
{
    return Vec2f32(a.x / s, a.y / s);
}
vec2f32 vec2f32Norm(vec2f32 a)
{
    f32 mag = vec2f32Mag(a);
    return Vec2f32(a.x / mag, a.y / mag);
}
f32 vec2f32Dot(vec2f32 a, vec2f32 b)
{
    return a.x * b.x + a.y * b.y;
}
f32 vec2f32Mag(vec2f32 a)
{
    return BASE_SQRTF32(vec2f32MagSqr(a));
}
f32 vec2f32MagSqr(vec2f32 a)
{
    return vec2f32Dot(a, a);
}

/// vec3f32
vec3f32 vec3f32Add(vec3f32 a, vec3f32 b)
{
    return Vec3f32(a.x + b.x, a.y + b.y, a.z + b.z);
}
vec3f32 vec3f32Sub(vec3f32 a, vec3f32 b)
{
    return Vec3f32(a.x - b.x, a.y - b.y, a.z - b.z);
}
vec3f32 vec3f32Mult(vec3f32 a, vec3f32 b)
{
    return Vec3f32(a.x * b.x, a.y * b.y, a.z * b.z);
}
vec3f32 vec3f32Div(vec3f32 a, vec3f32 b)
{
    return Vec3f32(a.x / b.x, a.y / b.y, a.z / b.z);
}
vec3f32 vec3f32MultF32(vec3f32 a, f32 s)
{
    return Vec3f32(a.x * s, a.y * s, a.z * s);
}
vec3f32 vec3f32DivF32(vec3f32 a, f32 s)
{
    return Vec3f32(a.x / s, a.y / s, a.z / s);
}
vec3f32 vec3f32Norm(vec3f32 a)
{
    f32 mag = vec3f32Mag(a);
    return Vec3f32(a.x / mag, a.y / mag, a.z / mag);
}
vec3f32 vec3f32Cross(vec3f32 a, vec3f32 b)
{
    return Vec3f32(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}
f32 vec3f32Dot(vec3f32 a, vec3f32 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
f32 vec3f32Mag(vec3f32 a)
{
    return BASE_SQRTF32(vec3f32MagSqr(a));
}
f32 vec3f32MagSqr(vec3f32 a)
{
    return vec3f32Dot(a, a);
}

//vec4f32
vec4f32 vec4f32Add(vec4f32 a, vec4f32 b)
{
    return Vec4f32(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}
vec4f32 vec4f32Sub(vec4f32 a, vec4f32 b)
{
    return Vec4f32(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}
vec4f32 vec4f32Mult(vec4f32 a, vec4f32 b)
{
    return Vec4f32(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}
vec4f32 vec4f32Div(vec4f32 a, vec4f32 b)
{
    return Vec4f32(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
}
vec4f32 vec4f32MultF32(vec4f32 a, f32 s)
{
    return Vec4f32(a.x * s, a.y * s, a.z * s, a.w * s);
}
vec4f32 vec4f32DivF32(vec4f32 a, f32 s)
{
    return Vec4f32(a.x / s, a.y / s, a.z / s, a.w / s);
}
vec4f32 vec4f32Norm(vec4f32 a)
{
    f32 mag = vec4f32Mag(a);
    return Vec4f32(a.x / mag, a.y / mag, a.z / mag, a.w / mag);
}
f32 vec4f32Dot(vec4f32 a, vec4f32 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}
f32 vec4f32Mag(vec4f32 a)
{
    return BASE_SQRTF32(vec4f32MagSqr(a));
}
f32 vec4f32MagSqr(vec4f32 a)
{
    return vec4f32Dot(a, a);
}

//mat3f32
mat3f32 mat3f32FromColVec3f32(vec3f32 cols[3])
{
    mat3f32 m = 
    {
        .m00 = cols[0].x,
        .m01 = cols[1].x,
        .m02 = cols[2].x,

        .m10 = cols[0].y,
        .m11 = cols[1].y,
        .m12 = cols[2].y,

        .m20 = cols[0].z,
        .m21 = cols[1].z,
        .m22 = cols[2].z,
    };

    return m;
}
mat3f32 mat3f32FromRowVec3f32(vec3f32 rows[3])
{
    mat3f32 m;
    BASE_MEMCPY(&m, rows, sizeof(m));

    return m;
}
mat3f32 mat3f32Transpose(mat3f32 a)
{
    mat3f32 m = 
    {
        .m00 = a.m00,
        .m01 = a.m10,
        .m02 = a.m20,

        .m10 = a.m01,
        .m11 = a.m11,
        .m12 = a.m21,

        .m20 = a.m02,
        .m21 = a.m12,
        .m22 = a.m22,
    };

    return m;
}
f32 mat3f32Det(mat3f32 a)
{
    f32 det = a.m00 * ((a.m11 * a.m22) - (a.m12 * a.m21)) -
              a.m01 * ((a.m10 * a.m22) - (a.m12 * a.m20)) + 
              a.m02 * ((a.m10 * a.m21) - (a.m11 * a.m20));

    return det;
}
mat3f32 mat3f32Inverse(mat3f32 a)
{
    f32 det = mat3f32Det(a);
    mat3f32 t = mat3f32Transpose(a);

    mat3f32 adjointMatrix = 
    {
        .m00 = + ((t.m11 * t.m22) - (t.m12 * t.m21)),
        .m01 = - ((t.m10 * t.m22) - (t.m12 * t.m20)),
        .m02 = + ((t.m10 * t.m21) - (t.m11 * t.m20)),

        .m10 = - ((t.m01 * t.m22) - (t.m02 * t.m21)),
        .m11 = + ((t.m00 * t.m22) - (t.m02 * t.m20)),
        .m12 = - ((t.m00 * t.m21) - (t.m01 * t.m20)),

        .m20 = + ((t.m01 * t.m12) - (t.m02 * t.m11)),
        .m21 = - ((t.m00 * t.m12) - (t.m02 * t.m10)),
        .m22 = + ((t.m00 * t.m11) - (t.m01 * t.m10)),
    };

    return mat3f32MultF32(adjointMatrix, 1.0f / det);
}
mat3f32 mat3f32Add(mat3f32 a, mat3f32 b)
{
    mat3f32 m = 
    {
        .m00 = a.m00 + b.m00,
        .m01 = a.m01 + b.m01,
        .m02 = a.m02 + b.m02,

        .m10 = a.m10 + b.m10,
        .m11 = a.m11 + b.m11,
        .m12 = a.m12 + b.m12,

        .m20 = a.m20 + b.m20,
        .m21 = a.m21 + b.m21,
        .m22 = a.m22 + b.m22,

    };

    return m;
}
mat3f32 mat3f32Sub(mat3f32 a, mat3f32 b)
{
    mat3f32 m = 
    {
        .m00 = a.m00 - b.m00,
        .m01 = a.m01 - b.m01,
        .m02 = a.m02 - b.m02,

        .m10 = a.m10 - b.m10,
        .m11 = a.m11 - b.m11,
        .m12 = a.m12 - b.m12,

        .m20 = a.m20 - b.m20,
        .m21 = a.m21 - b.m21,
        .m22 = a.m22 - b.m22,

    };

    return m;
}
mat3f32 mat3f32Mult(mat3f32 a, mat3f32 b)
{
    mat3f32 m = 
    {
        .m00 = vec3f32Dot(a.v[0], Vec3f32(b.m00, b.m10, b.m20)),
        .m01 = vec3f32Dot(a.v[0], Vec3f32(b.m01, b.m11, b.m21)),
        .m02 = vec3f32Dot(a.v[0], Vec3f32(b.m02, b.m12, b.m22)),

        .m10 = vec3f32Dot(a.v[1], Vec3f32(b.m00, b.m10, b.m20)),
        .m11 = vec3f32Dot(a.v[1], Vec3f32(b.m01, b.m11, b.m21)),
        .m12 = vec3f32Dot(a.v[1], Vec3f32(b.m02, b.m12, b.m22)),

        .m20 = vec3f32Dot(a.v[2], Vec3f32(b.m00, b.m10, b.m20)),
        .m21 = vec3f32Dot(a.v[2], Vec3f32(b.m01, b.m11, b.m21)),
        .m22 = vec3f32Dot(a.v[2], Vec3f32(b.m02, b.m12, b.m22)),
    };

    return m;
}
mat3f32 mat3f32MultComponentWise(mat3f32 a, mat3f32 b)
{
    mat3f32 m = 
    {
        .m00 = a.m00 * b.m00,
        .m01 = a.m01 * b.m01,
        .m02 = a.m02 * b.m02,

        .m10 = a.m10 * b.m10,
        .m11 = a.m11 * b.m11,
        .m12 = a.m12 * b.m12,

        .m20 = a.m20 * b.m20,
        .m21 = a.m21 * b.m21,
        .m22 = a.m22 * b.m22,

    };

    return m;
}
mat3f32 mat3f32MultF32(mat3f32 a, f32 s)
{
    mat3f32 m = 
    {
        .m00 = a.m00 * s,
        .m01 = a.m01 * s,
        .m02 = a.m02 * s,

        .m10 = a.m10 * s,
        .m11 = a.m11 * s,
        .m12 = a.m12 * s,

        .m20 = a.m20 * s,
        .m21 = a.m21 * s,
        .m22 = a.m22 * s,

    };

    return m;
}
vec3f32 mat3f32MultVec3f32(mat3f32 a, vec3f32 v)
{
    // treat vec as column vector
    vec3f32 r = 
    {
        .x = vec3f32Dot(a.v[0], v),
        .y = vec3f32Dot(a.v[1], v),
        .z = vec3f32Dot(a.v[2], v),
    };

    return r;
}
mat3f32 mat3f32Div(mat3f32 a, mat3f32 b)
{
    mat3f32 m = mat3f32Mult(a, mat3f32Inverse(b));

    return m;
}
mat3f32 mat3f32DivComponentWise(mat3f32 a, mat3f32 b)
{
    mat3f32 m = 
    {
        .m00 = a.m00 / b.m00,
        .m01 = a.m01 / b.m01,
        .m02 = a.m02 / b.m02,

        .m10 = a.m10 / b.m10,
        .m11 = a.m11 / b.m11,
        .m12 = a.m12 / b.m12,

        .m20 = a.m20 / b.m20,
        .m21 = a.m21 / b.m21,
        .m22 = a.m22 / b.m22,

    };

    return m;
}
mat3f32 mat3f32DivF32(mat3f32 a, f32 s)
{
    mat3f32 m = 
    {
        .m00 = a.m00 / s,
        .m01 = a.m01 / s,
        .m02 = a.m02 / s,

        .m10 = a.m10 / s,
        .m11 = a.m11 / s,
        .m12 = a.m12 / s,

        .m20 = a.m20 / s,
        .m21 = a.m21 / s,
        .m22 = a.m22 / s,

    };

    return m;
}
mat3f32 mat3f32NormCols(mat3f32 a)
{
    a = mat3f32Transpose(mat3f32NormRows(mat3f32Transpose(a)));
    
    return a;
}
mat3f32 mat3f32NormRows(mat3f32 a)
{
    a.v[0] = vec3f32Norm(a.v[0]);
    a.v[1] = vec3f32Norm(a.v[1]);
    a.v[2] = vec3f32Norm(a.v[2]);

    return a;
}

mat3f32 mat3f32GiveScale(vec3f32 scale)
{
    mat3f32 m =
    {
        .m00 = scale.x,
        .m11 = scale.y,
        .m22 = scale.z,
    };

    return m;
}
mat3f32 mat3f32GiveScaleUniform(f32 scale)
{
    return Mat3f32(scale);
}
mat3f32 mat3f32GiveScaleAxis(vec3f32 axis, f32 scale)
{
    axis = vec3f32Norm(axis);
    mat3f32 m = 
    {
        .m00 = 1.0f + (scale - 1) * axis.x * axis.x,
        .m11 = 1.0f + (scale - 1) * axis.y * axis.y,
        .m22 = 1.0f + (scale - 1) * axis.z * axis.z,

        .m01 = (scale - 1) * axis.x * axis.y,
        .m02 = (scale - 1) * axis.x * axis.z,

        .m10 = (scale - 1) * axis.x * axis.y,
        .m12 = (scale - 1) * axis.y * axis.z,

        .m20 = (scale - 1) * axis.x * axis.z,
        .m21 = (scale - 1) * axis.y * axis.z,
    };

    return m;
}
mat3f32 mat3f32Scale(mat3f32 a, vec3f32 scale)
{
    return mat3f32Mult(a, mat3f32GiveScale(scale));
}
mat3f32 mat3f32ScaleUniform(mat3f32 a, f32 scale)
{
    return mat3f32Mult(a, mat3f32GiveScaleUniform(scale));
}
mat3f32 mat3f32ScaleAxis(mat3f32 a, vec3f32 axis, f32 scale)
{
    return mat3f32Mult(a, mat3f32GiveScaleAxis(axis, scale));
}

mat3f32 mat3f32GiveRotateX(f32 rad)
{
    f32 c = BASE_COSF32(rad);
    f32 s = BASE_SINF32(rad);

    mat3f32 m =
    {
        .m00 = 1,
        .m11 = c,
        .m12 = -s,
        .m21 = s,
        .m22 = c,
    };

    return m;
}
mat3f32 mat3f32GiveRotateY(f32 rad)
{
    f32 c = BASE_COSF32(rad);
    f32 s = BASE_SINF32(rad);

    mat3f32 m =
    {
        .m00 = c,
        .m11 = 1,
        .m12 = s,
        .m20 = -s,
        .m22 = c,
    };

    return m;
}
mat3f32 mat3f32GiveRotateZ(f32 rad)
{
    f32 c = BASE_COSF32(rad);
    f32 s = BASE_SINF32(rad);

    mat3f32 m =
    {
        .m00 = c,
        .m01 = -s,
        .m11 = c,
        .m10 = s,
        .m22 = 1,
    };

    return m;
}
mat3f32 mat3f32GiveRotateYXZ(f32 radX, f32 radY, f32 radZ)
{
    // column vectors, which means we right multiplay with vectors
    //hence ZXYv
    return mat3f32Mult(mat3f32GiveRotateZ(radZ), mat3f32Mult(mat3f32GiveRotateX(radX), mat3f32GiveRotateY(radY)));
}
mat3f32 mat3f32GiveRotateZXY(f32 radX, f32 radY, f32 radZ)
{
    // column vectors, which means we right multiplay with vectors
    //hence ZXYv
    return mat3f32Mult(mat3f32GiveRotateY(radY), mat3f32Mult(mat3f32GiveRotateX(radX), mat3f32GiveRotateZ(radZ)));
}

mat3f32 mat3f32GiveRotateAxis(vec3f32 axis, f32 rad)
{
    axis = vec3f32Norm(axis);
    f32 c = BASE_COSF32(rad);
    f32 s = BASE_SINF32(rad);

    mat3f32 m = 
    {
        .m00 = axis.x * axis.x  * (1 - c) + c,
        .m01 = axis.x * axis.y * (1 - c) - axis.z * s,
        .m02 = axis.x * axis.z * (1 - c) + axis.y * s,

        .m10 = axis.x * axis.y  * (1 - c) + axis.z * s,
        .m11 = axis.y * axis.y * (1 - c) + c,
        .m12 = axis.y * axis.z * (1 - c) - axis.x * s,

        .m20 = axis.x * axis.z  * (1 - c) - axis.y * s,
        .m21 = axis.y * axis.x * (1 - c) - axis.z * s,
        .m22 = axis.z * axis.z * (1 - c) + c,
    };

    return m;
}
mat3f32 mat3f32RotateX(mat3f32 a, f32 rad)
{
    return mat3f32Mult(a, mat3f32GiveRotateX(rad));
}
mat3f32 mat3f32RotateY(mat3f32 a, f32 rad)
{
    return mat3f32Mult(a, mat3f32GiveRotateY(rad));
}
mat3f32 mat3f32RotateZ(mat3f32 a, f32 rad)
{
    return mat3f32Mult(a, mat3f32GiveRotateZ(rad));
}
mat3f32 mat3f32RotateYXZ(mat3f32 a, f32 radX, f32 radY, f32 radZ)
{
    return mat3f32Mult(a, mat3f32GiveRotateYXZ(radX, radY, radZ));
}
mat3f32 mat3f32RotateZXY(mat3f32 a, f32 radX, f32 radY, f32 radZ)
{
    return mat3f32Mult(a, mat3f32GiveRotateZXY(radX, radY, radZ));
}
mat3f32 mat3f32RotateAxis(mat3f32 a, vec3f32 axis, f32 rad)
{
    return mat3f32Mult(a, mat3f32GiveRotateAxis(axis, rad));
}

// mat4f32
mat4f32 mat4f32FromColVec4f32(vec4f32 cols[4])
{
    mat4f32 m = 
    {
        .m00 = cols[0].x,
        .m01 = cols[1].x,
        .m02 = cols[2].x,
        .m03 = cols[3].x,

        .m10 = cols[0].y,
        .m11 = cols[1].y,
        .m12 = cols[2].y,
        .m13 = cols[3].y,

        .m20 = cols[0].z,
        .m21 = cols[1].z,
        .m22 = cols[2].z,
        .m23 = cols[3].z,

        .m30 = cols[0].w,
        .m31 = cols[1].w,
        .m32 = cols[2].w,
        .m33 = cols[3].w,
    };

    return m;
}
mat4f32 mat4f32FromRowVec4f32(vec4f32 rows[4])
{
    mat4f32 m;
    BASE_MEMCPY(&m, rows, sizeof(m));

    return m;
}
mat4f32 mat4f32Transpose(mat4f32 a)
{
    mat4f32 m = 
    {
        .m00 = a.m00,
        .m01 = a.m10,
        .m02 = a.m20,
        .m03 = a.m30,

        .m10 = a.m01,
        .m11 = a.m11,
        .m12 = a.m21,
        .m13 = a.m31,

        .m20 = a.m02,
        .m21 = a.m12,
        .m22 = a.m22,
        .m23 = a.m32,

        .m30 = a.m03,
        .m31 = a.m13,
        .m32 = a.m23,
        .m33 = a.m33,
    };

    return m;
}
f32 mat4f32Det(mat4f32 a)
{
    mat3f32 _0;
    _0.rows[0][0] = a.m11;
    _0.rows[0][1] = a.m12;
    _0.rows[0][2] = a.m13;

    _0.rows[1][0] = a.m21;
    _0.rows[1][1] = a.m22;
    _0.rows[1][2] = a.m23;

    _0.rows[1][0] = a.m31;
    _0.rows[1][1] = a.m32;
    _0.rows[1][2] = a.m33;

    mat3f32 _1;
    _1.rows[0][0] = a.m10;
    _1.rows[0][1] = a.m12;
    _1.rows[0][2] = a.m13;

    _1.rows[1][0] = a.m20;
    _1.rows[1][1] = a.m22;
    _1.rows[1][2] = a.m23;

    _1.rows[1][0] = a.m30;
    _1.rows[1][1] = a.m32;
    _1.rows[1][2] = a.m33;

    mat3f32 _2;
    _2.rows[0][0] = a.m10;
    _2.rows[0][1] = a.m11;
    _2.rows[0][2] = a.m13;

    _2.rows[1][0] = a.m20;
    _2.rows[1][1] = a.m21;
    _2.rows[1][2] = a.m23;

    _2.rows[1][0] = a.m30;
    _2.rows[1][1] = a.m31;
    _2.rows[1][2] = a.m33;

    mat3f32 _3;
    _3.rows[0][0] = a.m10;
    _3.rows[0][1] = a.m11;
    _3.rows[0][2] = a.m12;

    _3.rows[1][0] = a.m20;
    _3.rows[1][1] = a.m21;
    _3.rows[1][2] = a.m22;

    _3.rows[1][0] = a.m30;
    _3.rows[1][1] = a.m31;
    _3.rows[1][2] = a.m32;

    return a.m00 * mat3f32Det(_0)
         - a.m01 * mat3f32Det(_1)
         + a.m02 * mat3f32Det(_2)
         - a.m03 * mat3f32Det(_3);
}
mat4f32 mat4f32Inverse(mat4f32 a)
{
    f32 s0 = a.rows[0][0] * a.rows[1][1] - a.rows[1][0] * a.rows[0][1];
    f32 s1 = a.rows[0][0] * a.rows[1][2] - a.rows[1][0] * a.rows[0][2];
    f32 s2 = a.rows[0][0] * a.rows[1][3] - a.rows[1][0] * a.rows[0][3];
    f32 s3 = a.rows[0][1] * a.rows[1][2] - a.rows[1][1] * a.rows[0][2];
    f32 s4 = a.rows[0][1] * a.rows[1][3] - a.rows[1][1] * a.rows[0][3];
    f32 s5 = a.rows[0][2] * a.rows[1][3] - a.rows[1][2] * a.rows[0][3];

    f32 c5 = a.rows[2][2] * a.rows[3][3] - a.rows[3][2] * a.rows[2][3];
    f32 c4 = a.rows[2][1] * a.rows[3][3] - a.rows[3][1] * a.rows[2][3];
    f32 c3 = a.rows[2][1] * a.rows[3][2] - a.rows[3][1] * a.rows[2][2];
    f32 c2 = a.rows[2][0] * a.rows[3][3] - a.rows[3][0] * a.rows[2][3];
    f32 c1 = a.rows[2][0] * a.rows[3][2] - a.rows[3][0] * a.rows[2][2];
    f32 c0 = a.rows[2][0] * a.rows[3][1] - a.rows[3][0] * a.rows[2][1];

    // Should check for 0 determinant
    f32 invdet = 1.0f / (s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0);

    mat4f32 b = {0};

    b.rows[0][0] = ( a.rows[1][1] * c5 - a.rows[1][2] * c4 + a.rows[1][3] * c3) * invdet;
    b.rows[0][1] = (-a.rows[0][1] * c5 + a.rows[0][2] * c4 - a.rows[0][3] * c3) * invdet;
    b.rows[0][2] = ( a.rows[3][1] * s5 - a.rows[3][2] * s4 + a.rows[3][3] * s3) * invdet;
    b.rows[0][3] = (-a.rows[2][1] * s5 + a.rows[2][2] * s4 - a.rows[2][3] * s3) * invdet;

    b.rows[1][0] = (-a.rows[1][0] * c5 + a.rows[1][2] * c2 - a.rows[1][3] * c1) * invdet;
    b.rows[1][1] = ( a.rows[0][0] * c5 - a.rows[0][2] * c2 + a.rows[0][3] * c1) * invdet;
    b.rows[1][2] = (-a.rows[3][0] * s5 + a.rows[3][2] * s2 - a.rows[3][3] * s1) * invdet;
    b.rows[1][3] = ( a.rows[2][0] * s5 - a.rows[2][2] * s2 + a.rows[2][3] * s1) * invdet;

    b.rows[2][0] = ( a.rows[1][0] * c4 - a.rows[1][1] * c2 + a.rows[1][3] * c0) * invdet;
    b.rows[2][1] = (-a.rows[0][0] * c4 + a.rows[0][1] * c2 - a.rows[0][3] * c0) * invdet;
    b.rows[2][2] = ( a.rows[3][0] * s4 - a.rows[3][1] * s2 + a.rows[3][3] * s0) * invdet;
    b.rows[2][3] = (-a.rows[2][0] * s4 + a.rows[2][1] * s2 - a.rows[2][3] * s0) * invdet;

    b.rows[3][0] = (-a.rows[1][0] * c3 + a.rows[1][1] * c1 - a.rows[1][2] * c0) * invdet;
    b.rows[3][1] = ( a.rows[0][0] * c3 - a.rows[0][1] * c1 + a.rows[0][2] * c0) * invdet;
    b.rows[3][2] = (-a.rows[3][0] * s3 + a.rows[3][1] * s1 - a.rows[3][2] * s0) * invdet;
    b.rows[3][3] = ( a.rows[2][0] * s3 - a.rows[2][1] * s1 + a.rows[2][2] * s0) * invdet;

    return b;
}
mat4f32 mat4f32Add(mat4f32 a, mat4f32 b)
{
    mat4f32 m =
    {
        .m00 = a.m00 + b.m00,
        .m01 = a.m01 + b.m01,
        .m02 = a.m02 + b.m02,
        .m03 = a.m03 + b.m03,

        .m10 = a.m10 + b.m10,
        .m11 = a.m11 + b.m11,
        .m12 = a.m12 + b.m12,
        .m13 = a.m13 + b.m13,

        .m20 = a.m20 + b.m20,
        .m21 = a.m21 + b.m21,
        .m22 = a.m22 + b.m22,
        .m23 = a.m23 + b.m23,

        .m30 = a.m30 + b.m30,
        .m31 = a.m31 + b.m31,
        .m32 = a.m32 + b.m32,
        .m33 = a.m33 + b.m33,
    };
    
    return m;
}
mat4f32 mat4f32Sub(mat4f32 a, mat4f32 b)
{
    mat4f32 m =
    {
        .m00 = a.m00 - b.m00,
        .m01 = a.m01 - b.m01,
        .m02 = a.m02 - b.m02,
        .m03 = a.m03 - b.m03,

        .m10 = a.m10 - b.m10,
        .m11 = a.m11 - b.m11,
        .m12 = a.m12 - b.m12,
        .m13 = a.m13 - b.m13,

        .m20 = a.m20 - b.m20,
        .m21 = a.m21 - b.m21,
        .m22 = a.m22 - b.m22,
        .m23 = a.m23 - b.m23,

        .m30 = a.m30 - b.m30,
        .m31 = a.m31 - b.m31,
        .m32 = a.m32 - b.m32,
        .m33 = a.m33 - b.m33,
    };
    
    return m;
}
mat4f32 mat4f32Mult(mat4f32 a, mat4f32 b)
{
    mat4f32 m = 
    {
        .m00 = vec4f32Dot(a.v[0], Vec4f32(b.m00, b.m10, b.m20, b.m30)),
        .m01 = vec4f32Dot(a.v[0], Vec4f32(b.m01, b.m11, b.m21, b.m31)),
        .m02 = vec4f32Dot(a.v[0], Vec4f32(b.m02, b.m12, b.m22, b.m32)),
        .m03 = vec4f32Dot(a.v[0], Vec4f32(b.m03, b.m13, b.m23, b.m33)),

        .m10 = vec4f32Dot(a.v[1], Vec4f32(b.m00, b.m10, b.m20, b.m30)),
        .m11 = vec4f32Dot(a.v[1], Vec4f32(b.m01, b.m11, b.m21, b.m31)),
        .m12 = vec4f32Dot(a.v[1], Vec4f32(b.m02, b.m12, b.m22, b.m32)),
        .m13 = vec4f32Dot(a.v[1], Vec4f32(b.m03, b.m13, b.m23, b.m33)),

        .m20 = vec4f32Dot(a.v[2], Vec4f32(b.m00, b.m10, b.m20, b.m30)),
        .m21 = vec4f32Dot(a.v[2], Vec4f32(b.m01, b.m11, b.m21, b.m31)),
        .m22 = vec4f32Dot(a.v[2], Vec4f32(b.m02, b.m12, b.m22, b.m32)),
        .m23 = vec4f32Dot(a.v[2], Vec4f32(b.m03, b.m13, b.m23, b.m33)),

        .m30 = vec4f32Dot(a.v[3], Vec4f32(b.m00, b.m10, b.m20, b.m30)),
        .m31 = vec4f32Dot(a.v[3], Vec4f32(b.m01, b.m11, b.m21, b.m31)),
        .m32 = vec4f32Dot(a.v[3], Vec4f32(b.m02, b.m12, b.m22, b.m32)),
        .m33 = vec4f32Dot(a.v[3], Vec4f32(b.m03, b.m13, b.m23, b.m33)),
    };

    return m;
}
mat4f32 mat4f32MultComponentWise(mat4f32 a, mat4f32 b)
{
    mat4f32 m =
    {
        .m00 = a.m00 * b.m00,
        .m01 = a.m01 * b.m01,
        .m02 = a.m02 * b.m02,
        .m03 = a.m03 * b.m03,

        .m10 = a.m10 * b.m10,
        .m11 = a.m11 * b.m11,
        .m12 = a.m12 * b.m12,
        .m13 = a.m13 * b.m13,

        .m20 = a.m20 * b.m20,
        .m21 = a.m21 * b.m21,
        .m22 = a.m22 * b.m22,
        .m23 = a.m23 * b.m23,

        .m30 = a.m30 * b.m30,
        .m31 = a.m31 * b.m31,
        .m32 = a.m32 * b.m32,
        .m33 = a.m33 * b.m33,
    };
    
    return m;
}
mat4f32 mat4f32MultF32(mat4f32 a, f32 s)
{
    mat4f32 m =
    {
        .m00 = a.m00 * s,
        .m01 = a.m01 * s,
        .m02 = a.m02 * s,
        .m03 = a.m03 * s,

        .m10 = a.m10 * s,
        .m11 = a.m11 * s,
        .m12 = a.m12 * s,
        .m13 = a.m13 * s,

        .m20 = a.m20 * s,
        .m21 = a.m21 * s,
        .m22 = a.m22 * s,
        .m23 = a.m23 * s,

        .m30 = a.m30 * s,
        .m31 = a.m31 * s,
        .m32 = a.m32 * s,
        .m33 = a.m33 * s,
    };
    
    return m;
}
vec4f32 mat4f32MultVec4f32(mat4f32 a, vec4f32 v)
{
    // treat vec as column vector
    vec4f32 r = 
    {
        .x = vec4f32Dot(a.v[0], v),
        .y = vec4f32Dot(a.v[1], v),
        .z = vec4f32Dot(a.v[2], v),
        .w = vec4f32Dot(a.v[3], v),
    };

    return r;
}
mat4f32 mat4f32Div(mat4f32 a, mat4f32 b)
{
    return mat4f32Mult(a, mat4f32Inverse(b));
}
mat4f32 mat4f32DivComponentWise(mat4f32 a, mat4f32 b)
{
    mat4f32 m =
    {
        .m00 = a.m00 / b.m00,
        .m01 = a.m01 / b.m01,
        .m02 = a.m02 / b.m02,
        .m03 = a.m03 / b.m03,

        .m10 = a.m10 / b.m10,
        .m11 = a.m11 / b.m11,
        .m12 = a.m12 / b.m12,
        .m13 = a.m13 / b.m13,

        .m20 = a.m20 / b.m20,
        .m21 = a.m21 / b.m21,
        .m22 = a.m22 / b.m22,
        .m23 = a.m23 / b.m23,

        .m30 = a.m30 / b.m30,
        .m31 = a.m31 / b.m31,
        .m32 = a.m32 / b.m32,
        .m33 = a.m33 / b.m33,
    };
    
    return m;
}
mat4f32 mat4f32DivF32(mat4f32 a, f32 s)
{
    mat4f32 m =
    {
        .m00 = a.m00 / s,
        .m01 = a.m01 / s,
        .m02 = a.m02 / s,
        .m03 = a.m03 / s,

        .m10 = a.m10 / s,
        .m11 = a.m11 / s,
        .m12 = a.m12 / s,
        .m13 = a.m13 / s,

        .m20 = a.m20 / s,
        .m21 = a.m21 / s,
        .m22 = a.m22 / s,
        .m23 = a.m23 / s,

        .m30 = a.m30 / s,
        .m31 = a.m31 / s,
        .m32 = a.m32 / s,
        .m33 = a.m33 / s,
    };
    
    return m;
}
mat4f32 mat4f32NormCols(mat4f32 a)
{
    a = mat4f32Transpose(mat4f32NormRows(mat4f32Transpose(a)));
    
    return a;
}
mat4f32 mat4f32NormRows(mat4f32 a)
{
    a.v[0] = vec4f32Norm(a.v[0]);
    a.v[1] = vec4f32Norm(a.v[1]);
    a.v[2] = vec4f32Norm(a.v[2]);
    a.v[3] = vec4f32Norm(a.v[3]);

    return a;
}

mat4f32 mat4f32GiveScale(vec4f32 scale)
{
    mat4f32 m = 
    {
        .m00 =scale.x,
        .m11 =scale.y,
        .m22 =scale.z,
        .m33 =scale.w,
    };

    return m;
}
mat4f32 mat4f32GiveScaleUniform(f32 scale)
{
    return Mat4f32(scale);
}
mat4f32 mat4f32GiveScaleAxis(vec3f32 axis, f32 scale)
{
    mat4f32 m = Mat4f32FromMat3(mat3f32GiveScaleAxis(axis, scale));
    return m;
}
mat4f32 mat4f32Scale(mat4f32 a, vec4f32 scale)
{
    return mat4f32Mult(a, mat4f32GiveScale(scale));
}
mat4f32 mat4f32ScaleUniform(mat4f32 a, f32 scale)
{
    return mat4f32Mult(a, mat4f32GiveScaleUniform(scale));
}
mat4f32 mat4f32ScaleAxis(mat4f32 a, vec3f32 axis, f32 scale)
{
    return mat4f32Mult(a, mat4f32GiveScaleAxis(axis, scale));
}

mat4f32 mat4f32GiveRotateX(f32 rad)
{
    return Mat4f32FromMat3(mat3f32GiveRotateX(rad));
}
mat4f32 mat4f32GiveRotateY(f32 rad)
{
    return Mat4f32FromMat3(mat3f32GiveRotateY(rad));
}
mat4f32 mat4f32GiveRotateZ(f32 rad)
{
    return Mat4f32FromMat3(mat3f32GiveRotateZ(rad));
}
mat4f32 mat4f32GiveRotateYXZ(f32 radX, f32 radY, f32 radZ)
{
    return Mat4f32FromMat3(mat3f32GiveRotateYXZ(radX, radY, radZ));
}
mat4f32 mat4f32GiveRotateZXY(f32 radX, f32 radY, f32 radZ)
{
    return Mat4f32FromMat3(mat3f32GiveRotateZXY(radX, radY, radZ));
}

mat4f32 mat4f32GiveRotateAxis(vec3f32 axis, f32 rad)
{
    return Mat4f32FromMat3(mat3f32GiveRotateAxis(axis, rad));
}
mat4f32 mat4f32RotateX(mat4f32 a, f32 rad)
{
    return mat4f32Mult(a, mat4f32GiveRotateX(rad));
}
mat4f32 mat4f32RotateY(mat4f32 a, f32 rad)
{
    return mat4f32Mult(a, mat4f32GiveRotateY(rad));
}
mat4f32 mat4f32RotateZ(mat4f32 a, f32 rad)
{
    return mat4f32Mult(a, mat4f32GiveRotateZ(rad));
}
mat4f32 mat4f32RotateYXZ(mat4f32 a, f32 radX, f32 radY, f32 radZ)
{
    return mat4f32Mult(a, mat4f32GiveRotateYXZ(radX, radY, radZ));
}
mat4f32 mat4f32RotateZXY(mat4f32 a, f32 radX, f32 radY, f32 radZ)
{
    return mat4f32Mult(a, mat4f32GiveRotateZXY(radX, radY, radZ));
}
mat4f32 mat4f32RotateAxis(mat4f32 a, vec3f32 axis, f32 rad)
{
    return mat4f32Mult(a, mat4f32GiveRotateAxis(axis, rad));
}

mat4f32 mat4f32GiveTranslate(vec3f32 t)
{
    mat4f32 m = 
    {
        .m03 = t.x,
        .m13 = t.y,
        .m23 = t.z,
        .m00 = 1,
        .m11 = 1,
        .m22 = 1,
        .m33 = 1,
    };

    return m;
}
mat4f32 mat4f32Translate(mat4f32 a, vec3f32 t)
{
    return mat4f32Mult(a, mat4f32GiveTranslate(t));
}

mat4f32 mat4f32GiveTransformSRT(vec3f32 translate, vec3f32 rotateRads, vec4f32 scale)
{
    //scale -> rotate -> translate
    // since its right multiply with column vectors
    // you have to do: TRSv

    return mat4f32Mult(mat4f32GiveTranslate(translate), 
                       mat4f32Mult(mat4f32GiveRotateZXY(rotateRads.x, rotateRads.y, rotateRads.z),
                                   mat4f32GiveScale(scale)));
}
mat4f32 mat4f32TransformSRT(mat4f32 a, vec3f32 translate, vec3f32 rotateRads, vec4f32 scale)
{
    return mat4f32Mult(a, mat4f32GiveTransformSRT(translate, rotateRads, scale));
}