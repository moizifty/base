#include "baseMath.h"

vec2f vec2fAdd(vec2f a, vec2f b)
{
    return Vec2f(a.x + b.x, a.y + b.y);
}
vec2f vec2fSub(vec2f a, vec2f b)
{
    return Vec2f(a.x - b.x, a.y - b.y);
}
vec2f vec2fMult(vec2f a, vec2f b)
{
    return Vec2f(a.x * b.x, a.y * b.y);
}
vec2f vec2fDiv(vec2f a, vec2f b)
{
    return Vec2f(a.x / b.x, a.y / b.y);
}
vec2f vec2fMultF32(vec2f a, f32 s)
{
    return Vec2f(a.x * s, a.y * s);
}
vec2f vec2fDivF32(vec2f a, f32 s)
{
    return Vec2f(a.x / s, a.y / s);
}
vec2f vec2fNorm(vec2f a)
{
    f32 mag = vec2fMag(a);
    return Vec2f(a.x / mag, a.y / mag);
}
f32 vec2fDot(vec2f a, vec2f b)
{
    return a.x * b.x + a.y * b.y;
}
f32 vec2fMag(vec2f a)
{
    return BASE_SQRTF32(vec2fMagSqr(a));
}
f32 vec2fMagSqr(vec2f a)
{
    return vec2fDot(a, a);
}

/// vec3f
vec3f vec3fAdd(vec3f a, vec3f b)
{
    return Vec3f(a.x + b.x, a.y + b.y, a.z + b.z);
}
vec3f vec3fSub(vec3f a, vec3f b)
{
    return Vec3f(a.x - b.x, a.y - b.y, a.z - b.z);
}
vec3f vec3fMult(vec3f a, vec3f b)
{
    return Vec3f(a.x * b.x, a.y * b.y, a.z * b.z);
}
vec3f vec3fDiv(vec3f a, vec3f b)
{
    return Vec3f(a.x / b.x, a.y / b.y, a.z / b.z);
}
vec3f vec3fMultF32(vec3f a, f32 s)
{
    return Vec3f(a.x * s, a.y * s, a.z * s);
}
vec3f vec3fDivF32(vec3f a, f32 s)
{
    return Vec3f(a.x / s, a.y / s, a.z / s);
}
vec3f vec3fNorm(vec3f a)
{
    f32 mag = vec3fMag(a);
    return Vec3f(a.x / mag, a.y / mag, a.z / mag);
}
vec3f vec3fCross(vec3f a, vec3f b)
{
    return Vec3f(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}
f32 vec3fDot(vec3f a, vec3f b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
f32 vec3fMag(vec3f a)
{
    return BASE_SQRTF32(vec3fMagSqr(a));
}
f32 vec3fMagSqr(vec3f a)
{
    return vec3fDot(a, a);
}

//vec4f
vec4f vec4fAdd(vec4f a, vec4f b)
{
    return Vec4f(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}
vec4f vec4fSub(vec4f a, vec4f b)
{
    return Vec4f(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}
vec4f vec4fMult(vec4f a, vec4f b)
{
    return Vec4f(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}
vec4f vec4fDiv(vec4f a, vec4f b)
{
    return Vec4f(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
}
vec4f vec4fMultF32(vec4f a, f32 s)
{
    return Vec4f(a.x * s, a.y * s, a.z * s, a.w * s);
}
vec4f vec4fDivF32(vec4f a, f32 s)
{
    return Vec4f(a.x / s, a.y / s, a.z / s, a.w / s);
}
vec4f vec4fNorm(vec4f a)
{
    f32 mag = vec4fMag(a);
    return Vec4f(a.x / mag, a.y / mag, a.z / mag, a.w / mag);
}
f32 vec4fDot(vec4f a, vec4f b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}
f32 vec4fMag(vec4f a)
{
    return BASE_SQRTF32(vec4fMagSqr(a));
}
f32 vec4fMagSqr(vec4f a)
{
    return vec4fDot(a, a);
}

//mat3f
mat3f mat3fFromColVec3f(vec3f cols[3])
{
    mat3f m = 
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
mat3f mat3fFromRowVec3f(vec3f rows[3])
{
    mat3f m;
    BASE_MEMCPY(&m, rows, sizeof(m));

    return m;
}
mat3f mat3fTranspose(mat3f a)
{
    mat3f m = 
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
f32 mat3fDet(mat3f a)
{
    f32 det = a.m00 * ((a.m11 * a.m22) - (a.m12 * a.m21)) -
              a.m01 * ((a.m10 * a.m22) - (a.m12 * a.m20)) + 
              a.m02 * ((a.m10 * a.m21) - (a.m11 * a.m20));

    return det;
}
mat3f mat3fInverse(mat3f a)
{
    f32 det = mat3fDet(a);
    mat3f t = mat3fTranspose(a);

    mat3f adjointMatrix = 
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

    return mat3fMultF32(adjointMatrix, 1.0f / det);
}
mat3f mat3fAdd(mat3f a, mat3f b)
{
    mat3f m = 
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
mat3f mat3fSub(mat3f a, mat3f b)
{
    mat3f m = 
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
mat3f mat3fMult(mat3f a, mat3f b)
{
    mat3f m = 
    {
        .m00 = vec3fDot(a.v[0], Vec3f(b.m00, b.m10, b.m20)),
        .m01 = vec3fDot(a.v[0], Vec3f(b.m01, b.m11, b.m21)),
        .m02 = vec3fDot(a.v[0], Vec3f(b.m02, b.m12, b.m22)),

        .m10 = vec3fDot(a.v[1], Vec3f(b.m00, b.m10, b.m20)),
        .m11 = vec3fDot(a.v[1], Vec3f(b.m01, b.m11, b.m21)),
        .m12 = vec3fDot(a.v[1], Vec3f(b.m02, b.m12, b.m22)),

        .m20 = vec3fDot(a.v[2], Vec3f(b.m00, b.m10, b.m20)),
        .m21 = vec3fDot(a.v[2], Vec3f(b.m01, b.m11, b.m21)),
        .m22 = vec3fDot(a.v[2], Vec3f(b.m02, b.m12, b.m22)),
    };

    return m;
}
mat3f mat3fMultComponentWise(mat3f a, mat3f b)
{
    mat3f m = 
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
mat3f mat3fMultF32(mat3f a, f32 s)
{
    mat3f m = 
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
vec3f mat3fMultVec3f(mat3f a, vec3f v)
{
    // treat vec as column vector
    vec3f r = 
    {
        .x = vec3fDot(a.v[0], v),
        .y = vec3fDot(a.v[1], v),
        .z = vec3fDot(a.v[2], v),
    };

    return r;
}
mat3f mat3fDiv(mat3f a, mat3f b)
{
    mat3f m = mat3fMult(a, mat3fInverse(b));

    return m;
}
mat3f mat3fDivComponentWise(mat3f a, mat3f b)
{
    mat3f m = 
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
mat3f mat3fDivF32(mat3f a, f32 s)
{
    mat3f m = 
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
mat3f mat3fNormCols(mat3f a)
{
    a = mat3fTranspose(mat3fNormRows(mat3fTranspose(a)));
    
    return a;
}
mat3f mat3fNormRows(mat3f a)
{
    a.v[0] = vec3fNorm(a.v[0]);
    a.v[1] = vec3fNorm(a.v[1]);
    a.v[2] = vec3fNorm(a.v[2]);

    return a;
}

mat3f mat3fGiveScale(vec3f scale)
{
    mat3f m =
    {
        .m00 = scale.x,
        .m11 = scale.y,
        .m22 = scale.z,
    };

    return m;
}
mat3f mat3fGiveScaleUniform(f32 scale)
{
    return Mat3f(scale);
}
mat3f mat3fGiveScaleAxis(vec3f axis, f32 scale)
{
    axis = vec3fNorm(axis);
    mat3f m = 
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
mat3f mat3fScale(mat3f a, vec3f scale)
{
    return mat3fMult(a, mat3fGiveScale(scale));
}
mat3f mat3fScaleUniform(mat3f a, f32 scale)
{
    return mat3fMult(a, mat3fGiveScaleUniform(scale));
}
mat3f mat3fScaleAxis(mat3f a, vec3f axis, f32 scale)
{
    return mat3fMult(a, mat3fGiveScaleAxis(axis, scale));
}

mat3f mat3fGiveRotateX(f32 rad)
{
    f32 c = BASE_COSF32(rad);
    f32 s = BASE_SINF32(rad);

    mat3f m =
    {
        .m00 = 1,
        .m11 = c,
        .m12 = -s,
        .m21 = s,
        .m22 = c,
    };

    return m;
}
mat3f mat3fGiveRotateY(f32 rad)
{
    f32 c = BASE_COSF32(rad);
    f32 s = BASE_SINF32(rad);

    mat3f m =
    {
        .m00 = c,
        .m11 = 1,
        .m12 = s,
        .m20 = -s,
        .m22 = c,
    };

    return m;
}
mat3f mat3fGiveRotateZ(f32 rad)
{
    f32 c = BASE_COSF32(rad);
    f32 s = BASE_SINF32(rad);

    mat3f m =
    {
        .m00 = c,
        .m01 = -s,
        .m11 = c,
        .m10 = s,
        .m22 = 1,
    };

    return m;
}
mat3f mat3fGiveRotateYXZ(f32 radX, f32 radY, f32 radZ)
{
    // column vectors, which means we right multiplay with vectors
    //hence ZXYv
    return mat3fMult(mat3fGiveRotateZ(radZ), mat3fMult(mat3fGiveRotateX(radX), mat3fGiveRotateY(radY)));
}
mat3f mat3fGiveRotateZXY(f32 radX, f32 radY, f32 radZ)
{
    // column vectors, which means we right multiplay with vectors
    //hence ZXYv
    return mat3fMult(mat3fGiveRotateY(radY), mat3fMult(mat3fGiveRotateX(radX), mat3fGiveRotateZ(radZ)));
}

mat3f mat3fGiveRotateAxis(vec3f axis, f32 rad)
{
    axis = vec3fNorm(axis);
    f32 c = BASE_COSF32(rad);
    f32 s = BASE_SINF32(rad);

    mat3f m = 
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
mat3f mat3fRotateX(mat3f a, f32 rad)
{
    return mat3fMult(a, mat3fGiveRotateX(rad));
}
mat3f mat3fRotateY(mat3f a, f32 rad)
{
    return mat3fMult(a, mat3fGiveRotateY(rad));
}
mat3f mat3fRotateZ(mat3f a, f32 rad)
{
    return mat3fMult(a, mat3fGiveRotateZ(rad));
}
mat3f mat3fRotateYXZ(mat3f a, f32 radX, f32 radY, f32 radZ)
{
    return mat3fMult(a, mat3fGiveRotateYXZ(radX, radY, radZ));
}
mat3f mat3fRotateZXY(mat3f a, f32 radX, f32 radY, f32 radZ)
{
    return mat3fMult(a, mat3fGiveRotateZXY(radX, radY, radZ));
}
mat3f mat3fRotateAxis(mat3f a, vec3f axis, f32 rad)
{
    return mat3fMult(a, mat3fGiveRotateAxis(axis, rad));
}

// mat4f
mat4f mat4fFromColVec4f(vec4f cols[4])
{
    mat4f m = 
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
mat4f mat4fFromRowVec4f(vec4f rows[4])
{
    mat4f m;
    BASE_MEMCPY(&m, rows, sizeof(m));

    return m;
}
mat4f mat4fTranspose(mat4f a)
{
    mat4f m = 
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
f32 mat4fDet(mat4f a)
{
    mat3f _0;
    _0.rows[0][0] = a.m11;
    _0.rows[0][1] = a.m12;
    _0.rows[0][2] = a.m13;

    _0.rows[1][0] = a.m21;
    _0.rows[1][1] = a.m22;
    _0.rows[1][2] = a.m23;

    _0.rows[1][0] = a.m31;
    _0.rows[1][1] = a.m32;
    _0.rows[1][2] = a.m33;

    mat3f _1;
    _1.rows[0][0] = a.m10;
    _1.rows[0][1] = a.m12;
    _1.rows[0][2] = a.m13;

    _1.rows[1][0] = a.m20;
    _1.rows[1][1] = a.m22;
    _1.rows[1][2] = a.m23;

    _1.rows[1][0] = a.m30;
    _1.rows[1][1] = a.m32;
    _1.rows[1][2] = a.m33;

    mat3f _2;
    _2.rows[0][0] = a.m10;
    _2.rows[0][1] = a.m11;
    _2.rows[0][2] = a.m13;

    _2.rows[1][0] = a.m20;
    _2.rows[1][1] = a.m21;
    _2.rows[1][2] = a.m23;

    _2.rows[1][0] = a.m30;
    _2.rows[1][1] = a.m31;
    _2.rows[1][2] = a.m33;

    mat3f _3;
    _3.rows[0][0] = a.m10;
    _3.rows[0][1] = a.m11;
    _3.rows[0][2] = a.m12;

    _3.rows[1][0] = a.m20;
    _3.rows[1][1] = a.m21;
    _3.rows[1][2] = a.m22;

    _3.rows[1][0] = a.m30;
    _3.rows[1][1] = a.m31;
    _3.rows[1][2] = a.m32;

    return a.m00 * mat3fDet(_0)
         - a.m01 * mat3fDet(_1)
         + a.m02 * mat3fDet(_2)
         - a.m03 * mat3fDet(_3);
}
mat4f mat4fInverse(mat4f a)
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

    mat4f b = {0};

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
mat4f mat4fAdd(mat4f a, mat4f b)
{
    mat4f m =
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
mat4f mat4fSub(mat4f a, mat4f b)
{
    mat4f m =
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
mat4f mat4fMult(mat4f a, mat4f b)
{
    mat4f m = 
    {
        .m00 = vec4fDot(a.v[0], Vec4f(b.m00, b.m10, b.m20, b.m30)),
        .m01 = vec4fDot(a.v[0], Vec4f(b.m01, b.m11, b.m21, b.m31)),
        .m02 = vec4fDot(a.v[0], Vec4f(b.m02, b.m12, b.m22, b.m32)),
        .m03 = vec4fDot(a.v[0], Vec4f(b.m03, b.m13, b.m23, b.m33)),

        .m10 = vec4fDot(a.v[1], Vec4f(b.m00, b.m10, b.m20, b.m30)),
        .m11 = vec4fDot(a.v[1], Vec4f(b.m01, b.m11, b.m21, b.m31)),
        .m12 = vec4fDot(a.v[1], Vec4f(b.m02, b.m12, b.m22, b.m32)),
        .m13 = vec4fDot(a.v[1], Vec4f(b.m03, b.m13, b.m23, b.m33)),

        .m20 = vec4fDot(a.v[2], Vec4f(b.m00, b.m10, b.m20, b.m30)),
        .m21 = vec4fDot(a.v[2], Vec4f(b.m01, b.m11, b.m21, b.m31)),
        .m22 = vec4fDot(a.v[2], Vec4f(b.m02, b.m12, b.m22, b.m32)),
        .m23 = vec4fDot(a.v[2], Vec4f(b.m03, b.m13, b.m23, b.m33)),

        .m30 = vec4fDot(a.v[3], Vec4f(b.m00, b.m10, b.m20, b.m30)),
        .m31 = vec4fDot(a.v[3], Vec4f(b.m01, b.m11, b.m21, b.m31)),
        .m32 = vec4fDot(a.v[3], Vec4f(b.m02, b.m12, b.m22, b.m32)),
        .m33 = vec4fDot(a.v[3], Vec4f(b.m03, b.m13, b.m23, b.m33)),
    };

    return m;
}
mat4f mat4fMultComponentWise(mat4f a, mat4f b)
{
    mat4f m =
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
mat4f mat4fMultF32(mat4f a, f32 s)
{
    mat4f m =
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
vec4f mat4fMultVec4f(mat4f a, vec4f v)
{
    // treat vec as column vector
    vec4f r = 
    {
        .x = vec4fDot(a.v[0], v),
        .y = vec4fDot(a.v[1], v),
        .z = vec4fDot(a.v[2], v),
        .w = vec4fDot(a.v[3], v),
    };

    return r;
}
mat4f mat4fDiv(mat4f a, mat4f b)
{
    return mat4fMult(a, mat4fInverse(b));
}
mat4f mat4fDivComponentWise(mat4f a, mat4f b)
{
    mat4f m =
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
mat4f mat4fDivF32(mat4f a, f32 s)
{
    mat4f m =
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
mat4f mat4fNormCols(mat4f a)
{
    a = mat4fTranspose(mat4fNormRows(mat4fTranspose(a)));
    
    return a;
}
mat4f mat4fNormRows(mat4f a)
{
    a.v[0] = vec4fNorm(a.v[0]);
    a.v[1] = vec4fNorm(a.v[1]);
    a.v[2] = vec4fNorm(a.v[2]);
    a.v[3] = vec4fNorm(a.v[3]);

    return a;
}

mat4f mat4fGiveScale(vec4f scale)
{
    mat4f m = 
    {
        .m00 =scale.x,
        .m11 =scale.y,
        .m22 =scale.z,
        .m33 =scale.w,
    };

    return m;
}
mat4f mat4fGiveScaleUniform(f32 scale)
{
    return Mat4f(scale);
}
mat4f mat4fGiveScaleAxis(vec3f axis, f32 scale)
{
    mat4f m = Mat4fFromMat3(mat3fGiveScaleAxis(axis, scale));
    return m;
}
mat4f mat4fScale(mat4f a, vec4f scale)
{
    return mat4fMult(a, mat4fGiveScale(scale));
}
mat4f mat4fScaleUniform(mat4f a, f32 scale)
{
    return mat4fMult(a, mat4fGiveScaleUniform(scale));
}
mat4f mat4fScaleAxis(mat4f a, vec3f axis, f32 scale)
{
    return mat4fMult(a, mat4fGiveScaleAxis(axis, scale));
}

mat4f mat4fGiveRotateX(f32 rad)
{
    return Mat4fFromMat3(mat3fGiveRotateX(rad));
}
mat4f mat4fGiveRotateY(f32 rad)
{
    return Mat4fFromMat3(mat3fGiveRotateY(rad));
}
mat4f mat4fGiveRotateZ(f32 rad)
{
    return Mat4fFromMat3(mat3fGiveRotateZ(rad));
}
mat4f mat4fGiveRotateYXZ(f32 radX, f32 radY, f32 radZ)
{
    return Mat4fFromMat3(mat3fGiveRotateYXZ(radX, radY, radZ));
}
mat4f mat4fGiveRotateZXY(f32 radX, f32 radY, f32 radZ)
{
    return Mat4fFromMat3(mat3fGiveRotateZXY(radX, radY, radZ));
}

mat4f mat4fGiveRotateAxis(vec3f axis, f32 rad)
{
    return Mat4fFromMat3(mat3fGiveRotateAxis(axis, rad));
}
mat4f mat4fRotateX(mat4f a, f32 rad)
{
    return mat4fMult(a, mat4fGiveRotateX(rad));
}
mat4f mat4fRotateY(mat4f a, f32 rad)
{
    return mat4fMult(a, mat4fGiveRotateY(rad));
}
mat4f mat4fRotateZ(mat4f a, f32 rad)
{
    return mat4fMult(a, mat4fGiveRotateZ(rad));
}
mat4f mat4fRotateYXZ(mat4f a, f32 radX, f32 radY, f32 radZ)
{
    return mat4fMult(a, mat4fGiveRotateYXZ(radX, radY, radZ));
}
mat4f mat4fRotateZXY(mat4f a, f32 radX, f32 radY, f32 radZ)
{
    return mat4fMult(a, mat4fGiveRotateZXY(radX, radY, radZ));
}
mat4f mat4fRotateAxis(mat4f a, vec3f axis, f32 rad)
{
    return mat4fMult(a, mat4fGiveRotateAxis(axis, rad));
}

mat4f mat4fGiveTranslate(vec3f t)
{
    mat4f m = 
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
mat4f mat4fTranslate(mat4f a, vec3f t)
{
    return mat4fMult(a, mat4fGiveTranslate(t));
}

mat4f mat4fGiveTransformSRT(vec3f translate, vec3f rotateRads, vec4f scale)
{
    //scale -> rotate -> translate
    // since its right multiply with column vectors
    // you have to do: TRSv

    return mat4fMult(mat4fGiveTranslate(translate), 
                       mat4fMult(mat4fGiveRotateZXY(rotateRads.x, rotateRads.y, rotateRads.z),
                                   mat4fGiveScale(scale)));
}
mat4f mat4fTransformSRT(mat4f a, vec3f translate, vec3f rotateRads, vec4f scale)
{
    return mat4fMult(a, mat4fGiveTransformSRT(translate, rotateRads, scale));
}

// quatf
quatf quatfNegate(quatf q)
{
    return quatfMultF32(q, -1.0f);
}
quatf quatfConjugate(quatf q)
{
    return Quatf(-q.x, -q.y, -q.z, q.w);
}
quatf quatfInverse(quatf q)
{
    return quatfDivF32(quatfConjugate(q), quatfMag(q));
}
quatf quatfNorm(quatf q)
{
    q.v = vec4fNorm(q.v);

    return q;
}
quatf quatfAdd(quatf a, quatf b)
{
    return Quatf(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}
quatf quatfDifference(quatf a, quatf b)
{
    // da = b
    // daa^-1 = ba^-1
    // = d = ba^-1
    return quatfMult(b, quatfConjugate(a));
}
quatf quatfSub(quatf a, quatf b)
{
    return Quatf(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}
quatf quatfMult(quatf a, quatf b)
{
    quatf q = QUATF_IDENTITY;
    q.w = a.w * b.w - vec3fDot(a.xyz, b.xyz);
    q.xyz = vec3fAdd(vec3fMultF32(b.xyz, a.w), vec3fAdd(vec3fMultF32(a.xyz, b.w), vec3fCross(a.xyz, b.xyz)));

    return q;
}
quatf quatfMultF32(quatf a, f32 s)
{
    return Quatf(a.x * s, a.y * s, a.z * s, a.w * s);
}
vec3f quatfMultVec3f(quatf a, vec3f v)
{
    return quatfMult(a, quatfMult(Quatf(v.x, v.y, v.z, 0), quatfConjugate(a))).xyz;
}
quatf quatfMultComponentWise(quatf a, quatf b)
{
    return Quatf(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}
quatf quatfDiv(quatf a, quatf b)
{
    return quatfDifference(a, b);
}
quatf quatfDivF32(quatf a, f32 s)
{
    return Quatf(a.x / s, a.y / s, a.z / s, a.w / s);
}
quatf quatfDivComponentWise(quatf a, quatf b)
{
    return Quatf(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
}
f32 quatfDot(quatf a, quatf b)
{
    return a.w * b.w + a.x * b.x + a.y * b.y + a.z * b.z;
}
f32 quatfMag(quatf q)
{
    return vec4fMag(q.v);
}
f32 quatfMagSqr(quatf q)
{
    return vec4fMagSqr(q.v);

}

quatf quatfFromEulerYXZ(vec3f angleRads)
{
    return quatfGiveRotateYXZ(angleRads.x, angleRads.y, angleRads.z);
}
quatf quatfFromEulerZXY(vec3f angleRads)
{
    return quatfGiveRotateZXY(angleRads.x, angleRads.y, angleRads.z);
}
vec3f quatfToEulerYXZ(quatf q)
{
    //todo simiplify
    mat3f m = quatfToMat3f(q);

    f32 sp = m.m21;

    f32 yaw = 0, pitch = 0, roll = 0;
    if(fabsf(sp) > 0.9999f)
    {
        pitch = ((f32)BASE_PI / 2.0f);
        roll = 0;
        yaw = BASE_ATAN2F32(m.m02, m.m00);
    }
    else
    {
        yaw = BASE_ATAN2F32(-m.m20, m.m22);
        pitch = BASE_ASINF32(sp);
        roll = BASE_ATAN2F32(-m.m01, m.m11);
    }

    return Vec3f(pitch, yaw, roll);
}
vec3f quatfToEulerZXY(quatf q)
{
    //todo simiplify
    mat3f m = quatfToMat3f(q);

    f32 sp = m.m21;

    f32 yaw = 0, pitch = 0, roll = 0;
    if(fabsf(sp) > 0.9999f)
    {
        pitch = ((f32)BASE_PI / 2.0f);
        roll = 0;
        yaw = BASE_ATAN2F32(m.m20, m.m00);
    }
    else
    {
        yaw = BASE_ATAN2F32(m.m02, m.m22);
        pitch = BASE_ASINF32(-sp);
        roll = BASE_ATAN2F32(m.m10, m.m11);
    }

    return Vec3f(pitch, yaw, roll);
}
mat3f quatfToMat3f(quatf q)
{
    float x = q.x, y = q.y, z = q.z, w = q.w;

    mat3f m =
    {
        .m00 = 1 - (2 * y * y) - (2 * z * z), 
        .m01 = (2 * x * y) - (2 * w * z),
        .m02 = (2 * x * z) + (2 * w * y),
        .m10 = (2 * x * y) + (2 * w * z),
        .m11 = 1 - (2 * x * x) - (2 * z * z), 
        .m12 = (2 * y * z) - (2 * w * x),
        .m20 = (2 * x * z) - (2 * w * y),     
        .m21 = (2 * y * z) + (2 * w * x),
        .m22 = 1 - (2 * x * x) - (2 * y * y),
    };

    return m;
}
mat4f quatfToMat4f(quatf q)
{
    return Mat4fFromMat3(quatfToMat3f(q));
}

quatf quatfGiveRotateAxis(vec3f axis, f32 rad)
{
    quatf q;
    q.w = BASE_COSF32(rad / 2);
    q.xyz = vec3fMultF32(axis, BASE_SINF32(rad / 2));

    return q;
}
quatf quatfGiveRotateYXZ(f32 radX, f32 radY, f32 radZ)
{
    quatf xRot = quatfGiveRotateAxis(Vec3f(1, 0, 0), radX);
    quatf yRot = quatfGiveRotateAxis(Vec3f(0, 1, 0), radY);
    quatf zRot = quatfGiveRotateAxis(Vec3f(0, 0, 1), radZ);
    
    // vectors right multiply with quaternions
    // hence you do Z*X*Y
    quatf q = quatfMult(zRot, quatfMult(xRot, yRot));
    return q;
}
quatf quatfGiveRotateZXY(f32 radX, f32 radY, f32 radZ)
{
    quatf xRot = quatfGiveRotateAxis(Vec3f(1, 0, 0), radX);
    quatf yRot = quatfGiveRotateAxis(Vec3f(0, 1, 0), radY);
    quatf zRot = quatfGiveRotateAxis(Vec3f(0, 0, 1), radZ);
    
    // vectors right multiply with quaternions
    // hence you do Y*X*Z
    quatf q = quatfMult(yRot, quatfMult(xRot, zRot));
    return q;
}
quatf quatfRotateAxis(quatf q, vec3f axis, f32 rad)
{
    return quatfMult(q, quatfGiveRotateAxis(axis, rad));
}
quatf quatfRotateYXZ(quatf q, f32 radX, f32 radY, f32 radZ)
{
    return quatfMult(q, quatfGiveRotateYXZ(radX, radY, radZ));
}
quatf quatfRotateZXY(quatf q, f32 radX, f32 radY, f32 radZ)
{
    return quatfMult(q, quatfGiveRotateZXY(radX, radY, radZ));
}
vec3f quatfRotateAxisVec3f(vec3f v, vec3f axis, f32 rad)
{
    return quatfMultVec3f(quatfGiveRotateAxis(axis, rad), v);
}
vec3f quatfRotateYXZVec3f(vec3f v, f32 radX, f32 radY, f32 radZ)
{
    return quatfMultVec3f(quatfGiveRotateYXZ(radX, radY, radZ), v);
}
vec3f quatfRotateZXYVec3f(vec3f v, f32 radX, f32 radY, f32 radZ)
{
    return quatfMultVec3f(quatfGiveRotateZXY(radX, radY, radZ), v);
}