#include <math.h>
#include "GMatrix.h"
#include "GPoint.h"
#include <iostream>
using namespace std;


GMatrix::GMatrix() { 
    fMat[0] = 1;    fMat[1] = 0;    fMat[2] = 0;
    fMat[3] = 0;    fMat[4] = 1;    fMat[5] = 0;
}


GMatrix GMatrix::Translate(float tx, float ty) {
    GMatrix matrix; 
    matrix.fMat[GMatrix::TX] = tx; 
    matrix.fMat[GMatrix::TY] = ty;
    return matrix; 
}


GMatrix GMatrix::Scale(float sx, float sy) {
    GMatrix matrix; 
    matrix.fMat[GMatrix::SX] = sx;
    matrix.fMat[GMatrix::SY] = sy;
    return matrix; 
}


GMatrix GMatrix::Rotate(float radians) {
    return GMatrix(
        cos(radians), -sin(radians), 0,
        sin(radians), cos(radians), 0);
} 


GMatrix GMatrix::Concat(const GMatrix& a, const GMatrix& b) {
    return GMatrix(
        a[0] * b[0] + a[1] * b[3], 
        a[0] * b[1] + a[1] * b[4], 
        a[0] * b[2] + a[1] * b[5] + a[2], 
        a[3] * b[0] + a[4] * b[3], 
        a[3] * b[1] + a[4] * b[4], 
        a[3] * b[2] + a[4] * b[5] + a[5]
    ); 
}


bool GMatrix::invert(GMatrix* inverse) const {
    float a = this->fMat[0];
    float b = this->fMat[1];
    float c = this->fMat[2];
    float d = this->fMat[3];
    float e = this->fMat[4];
    float f = this->fMat[5];

    float det = a * e - b * d;
    if (det == 0) {
        return false;
    }

    float divisor = 1 / det;
    inverse->fMat[0] = e * divisor;
    inverse->fMat[1] =  -b * divisor;
    inverse->fMat[2] = (b * f - c * e) * divisor;
    inverse->fMat[3] =  -d * divisor;
    inverse->fMat[4] = a * divisor;
    inverse->fMat[5] = -(a * f - c * d) * divisor;

    return true;
}


void GMatrix::mapPoints(GPoint dst[], const GPoint src[], int count) const {
    for (int i = 0; i < count; ++i) {
        GPoint point = src[i];
        float x0 = point.x();
        float y0 = point.y();
        float x = this->fMat[GMatrix::SX] * x0 + this->fMat[GMatrix::KX] * y0 + this->fMat[GMatrix::TX];
        float y = this->fMat[GMatrix::SY] * y0 + this->fMat[GMatrix::KY] * x0 + this->fMat[GMatrix::TY];
        GPoint dstPoint{x,y}; 
        dst[i] = dstPoint;
    }
}