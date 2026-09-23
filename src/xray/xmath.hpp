#ifndef XMATH_HPP
#define XMATH_HPP

#include "SDL3/SDL_stdinc.h"

namespace XRay {

    struct Vector2 {
        float x = 0.0f;
        float y = 0.0f;

        Vector2() = default;
        Vector2(float x, float y) : x(x), y(y) {}

        inline Vector2 operator+(const Vector2& v) const { return {x + v.x, y + v.y}; }
        inline Vector2 operator-(const Vector2& v) const { return {x - v.x, y - v.y}; }
        inline Vector2 operator*(float scalar) const { return {x * scalar, y * scalar}; }
        inline Vector2& operator+=(const Vector2& v) { x += v.x; y += v.y; return *this; }
    };

    struct Vector3 {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
    };

    struct Vector4 {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        float w = 1.0f;
    };

    // Column-major 4x4 matrix
    // Memory indices:
    // [ m0  m4  m8  m12 ]  Row 1
    // [ m1  m5  m9  m13 ]  Row 2
    // [ m2  m6  m10 m14 ]  Row 3
    // [ m3  m7  m11 m15 ]  Row 4
    struct Matrix {
        // Row major 
        // float m0 = 1.0f, m4 = 0.0f, m8  = 0.0f, m12 = 0.0f;
        // float m1 = 0.0f, m5 = 1.0f, m9  = 0.0f, m13 = 0.0f;
        // float m2 = 0.0f, m6 = 0.0f, m10 = 1.0f, m14 = 0.0f;
        // float m3 = 0.0f, m7 = 0.0f, m11 = 0.0f, m15 = 1.0f;
        
        // Column-major 4x4 matrix (Sequential in memory column-by-column)
        // Column 0 (Memory offsets 0, 4, 8, 12)
        float m0 = 1.0f, m1 = 0.0f, m2 = 0.0f, m3 = 0.0f;
        // Column 1 (Memory offsets 16, 20, 24, 28)
        float m4 = 0.0f, m5 = 1.0f, m6 = 0.0f, m7 = 0.0f;
        // Column 2 (Memory offsets 32, 36, 40, 44)
        float m8 = 0.0f, m9 = 0.0f, m10 = 1.0f, m11 = 0.0f;
        // Column 3 / Translation (Memory offsets 48, 52, 56, 60)
        float m12 = 0.0f, m13 = 0.0f, m14 = 0.0f, m15 = 1.0f;
    };

    // Matrix Identity
    inline Matrix MatrixIdentity() {
        return Matrix{};
    }

    // Column-Major Matrix Multiplication (out = A * B)
    inline Matrix MatrixMultiply(const Matrix& A, const Matrix& B) {
        Matrix result;
        
        // Row 0
        result.m0  = A.m0*B.m0  + A.m4*B.m1  + A.m8*B.m2   + A.m12*B.m3;
        result.m4  = A.m0*B.m4  + A.m4*B.m5  + A.m8*B.m6   + A.m12*B.m7;
        result.m8  = A.m0*B.m8  + A.m4*B.m9  + A.m8*B.m10  + A.m12*B.m11;
        result.m12 = A.m0*B.m12 + A.m4*B.m13 + A.m8*B.m14  + A.m12*B.m15;

        // Row 1
        result.m1  = A.m1*B.m0  + A.m5*B.m1  + A.m9*B.m2   + A.m13*B.m3;
        result.m5  = A.m1*B.m4  + A.m5*B.m5  + A.m9*B.m6   + A.m13*B.m7;
        result.m9  = A.m1*B.m8  + A.m5*B.m9  + A.m9*B.m10  + A.m13*B.m11;
        result.m13 = A.m1*B.m12 + A.m5*B.m13 + A.m9*B.m14  + A.m13*B.m15;

        // Row 2
        result.m2  = A.m2*B.m0  + A.m6*B.m1  + A.m10*B.m2  + A.m14*B.m3;
        result.m6  = A.m2*B.m4  + A.m6*B.m5  + A.m10*B.m6  + A.m14*B.m7;
        result.m10 = A.m2*B.m8  + A.m6*B.m9  + A.m10*B.m10 + A.m14*B.m11;
        result.m14 = A.m2*B.m12 + A.m6*B.m13 + A.m10*B.m14 + A.m14*B.m15;

        // Row 3
        result.m3  = A.m3*B.m0  + A.m7*B.m1  + A.m11*B.m2  + A.m15*B.m3;
        result.m7  = A.m3*B.m4  + A.m7*B.m5  + A.m11*B.m6  + A.m15*B.m7;
        result.m11 = A.m3*B.m8  + A.m7*B.m9  + A.m11*B.m10 + A.m15*B.m11;
        result.m15 = A.m3*B.m12 + A.m7*B.m13 + A.m11*B.m14 + A.m15*B.m15;

        return result;
    }

    // Fast 2D Transform Matrix (Translation * Rotation * Scale)
    inline Matrix MatrixTransform2D(Vector2 position, float rotationRadians, Vector2 scale) {
        float sinRot = SDL_sinf(rotationRadians);
        float cosRot = SDL_cosf(rotationRadians);

        Matrix mat = MatrixIdentity();

        // Column 0
        mat.m0 = scale.x * cosRot;
        mat.m1 = scale.x * sinRot;

        // Column 1
        mat.m4 = -scale.y * sinRot;
        mat.m5 =  scale.y * cosRot;

        // Column 3 (Translation)
        mat.m12 = position.x;
        mat.m13 = position.y;

        return mat;
    }
    
    // 2D Top-Left Origin Orthographic Projection Matrix
    inline Matrix MatrixOrtho(float width, float height) {
        Matrix mat = MatrixIdentity();
        mat.m0  =  2.0f / width;
        mat.m5  = -2.0f / height; // Y inverted for top-left (0,0)
        mat.m10 =  1.0f;
        mat.m12 = -1.0f;
        mat.m13 =  1.0f;
        return mat;
    }

    inline Matrix MatrixPerspective(float fieldOfView, float aspectRatio, float nearPlaneDistance, float farPlaneDistance)
    {
        float num = 1.0f / ((float) SDL_tanf(fieldOfView * 0.5f));
        Matrix mat = MatrixIdentity();
        return mat = {
		num / aspectRatio, 0, 0, 0,
		0, num, 0, 0,
		0, 0, farPlaneDistance / (nearPlaneDistance - farPlaneDistance), -1,
		0, 0, (nearPlaneDistance * farPlaneDistance) / (nearPlaneDistance - farPlaneDistance), 0
	    };
    }

    // Transform Vector2 by Matrix
    inline Vector2 Vector2Transform(Vector2 v, const Matrix& mat) {
        return Vector2{
                v.x * mat.m0 + v.y * mat.m4 + mat.m12,
                v.x * mat.m1 + v.y * mat.m5 + mat.m13
            };
    }
    
};


#endif 