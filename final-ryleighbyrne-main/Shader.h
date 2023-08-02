#ifndef Shader
#define Shader
#include "GBitmap.h"
#include "GMatrix.h"
#include "GShader.h"
#include "Blendmode.h"

class ProxyShader : public GShader {
public:

    ProxyShader(GShader* shader, const GMatrix& transform) 
    : actualShader(shader),
    fTransform(transform) {}

    bool isOpaque() override {
        return actualShader->isOpaque();
    }

    bool setContext(const GMatrix& ctm) override {
        return actualShader->setContext(ctm * fTransform);
    }

    void shadeRow(int x, int y, int count, GPixel row[]) override {
        actualShader->shadeRow(x, y, count, row);
    }

private:
    GShader* actualShader;
    GMatrix fTransform;
};


std::unique_ptr<GShader> GCreateProxyShader(GShader* shader, const GMatrix& transform) {
    return std::unique_ptr<GShader>(new ProxyShader(shader, transform));
}

class CompositeShader : public GShader {
public:

    CompositeShader(GShader* shader0, GShader* shader1) : 
        s0(shader0), s1(shader1) {}

    bool isOpaque() override {
        return s0->isOpaque() && s1->isOpaque();
    }

    bool setContext(const GMatrix& ctm) override {
        return s0->setContext(ctm) && s1->setContext(ctm);
    }

    void shadeRow(int x, int y, int count, GPixel row[]) override {
        GPixel c0[count], c1[count];
        s0->shadeRow(x, y, count, c0);
        s1->shadeRow(x, y, count, c1);
        for (int i = 0; i < count; i++) {
            row[i] = multPixels(c0[i], c1[i]);
        }
    }

private:
    GShader* s0;
    GShader* s1;
};

std::unique_ptr<GShader> GCreateCompositeShader(GShader* shader0, GShader* shader1) {
    return std::unique_ptr<GShader>(new CompositeShader(shader0, shader1));
}

class TriShader : public GShader {
public:

    TriShader(const GPoint points[3], const GColor colors[3]) {
        C0 = colors[0]; 
        C1 = colors[1]; 
        C2 = colors[2];
        P0 = points[0]; 
        P1 = points[1]; 
        P2 = points[2];

        fInverse = GMatrix();

        GPoint u = P1 - P0;
        GPoint v = P2 - P0;

        fMatrix = GMatrix(u.x(), v.x(), P0.x(), u.y(), v.y(), P0.y());                
    }

    bool isOpaque() override {
        return (C0.a == 1.0 && C1.a == 1.0 && C2.a == 1.0);
    }

    bool setContext(const GMatrix& ctm) override {
        return (ctm * fMatrix).invert(&fInverse);
    }

    void shadeRow(int x, int y, int count, GPixel row[]) override {

        GColor DC1 = C1 - C0;
        GColor DC2 = C2 - C0;
        
        GPoint pt; 
        pt.set(x + 0.5, y + 0.5);
        GPoint p_prime = fInverse * pt;

        GColor C = p_prime.x() * DC1 + p_prime.y() * DC2 + C0;
        GColor aDC1 = fInverse[0] * DC1;
        GColor dDC2 = fInverse[3] * DC2;
        GColor DC = aDC1 + dDC2;

        for (int i = 0; i < count; ++i) {
            row[i] = convertColorToGPixel(C);
            C += DC;
        }

    }

private:
    GColor C0,C1,C2;
    GPoint P0,P1,P2;
    GMatrix fInverse;
    GMatrix fMatrix;
};


#endif