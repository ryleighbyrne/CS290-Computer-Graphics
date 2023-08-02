#include "GBitmap.h"
#include "GMatrix.h"
#include "GShader.h"

class BitmapShader : public GShader {

public:
    BitmapShader(const GBitmap& bitmap, const GMatrix& localMatrix, GShader::TileMode tilemode)
        : fBitmap(bitmap)
        , fLM(localMatrix)
        , fTileMode(tilemode) {}

    bool isOpaque() override {
        return false;
    }

    bool setContext(const GMatrix& ctm) override {
        if (!ctm.invert(&inverse)) {
            return false;
        } 

        GMatrix temp; 
        temp = ctm * fLM; 
        temp.invert(&temp); 
        inverse = temp; 

        return true;
    }

    void shadeRow(int x, int y, int count, GPixel row[]) override {
        for (int i = 0; i < count; i++) {
            GPoint canvas_pt; 
            canvas_pt.set(x + 0.5 + i, y + 0.5);
            GPoint inv_pt = inverse * canvas_pt;

            // clamp
            int x_prime, y_prime;
            if(fTileMode == TileMode::kRepeat){
                x_prime = repeat(inv_pt.fX, fBitmap.width());
                y_prime = repeat(inv_pt.fY, fBitmap.height());
            }
            else if(fTileMode == TileMode::kMirror) {
                x_prime = clamp((float) mirror(inv_pt.fX, fBitmap.width()), fBitmap.width());
                y_prime = clamp((float) mirror(inv_pt.fY, fBitmap.height()), fBitmap.height());
            }   
            else{
                x_prime = clamp(inv_pt.fX, fBitmap.width());
                y_prime = clamp(inv_pt.fY, fBitmap.height());
            }

            row[i] = *fBitmap.getAddr(x_prime, y_prime);
        }
        
    }

        int clamp(float x, int bounds) {
        int x_prime = std::max(GFloorToInt(x), 0);
        x_prime = std::min(GFloorToInt(x_prime), bounds - 1);
        return x_prime;
    }

    int repeat(float x, int bounds) {
        while (x < 0) {
            x += bounds;
        }
        while (x >= bounds) {
            x -= bounds;
        }
        return GFloorToInt(x);
    }

    int mirror(float x, int bounds) {
        if (x < 0) x *= -1;
        float r = x / bounds;
        int floor = GFloorToInt(r);
        int x_prime = GFloorToInt(x);
        if (floor % 2 == 0) {
            return x_prime % bounds;
        } else {
            return bounds - (x_prime % bounds);
        }
    }

private:
    GBitmap fBitmap;
    GMatrix fLM;
    GMatrix inverse;
    TileMode fTileMode; 
};


std::unique_ptr<GShader> GCreateBitmapShader(const GBitmap& bitmap, const GMatrix& localMatrix, GShader::TileMode tilemode) {
    return std::unique_ptr<GShader>(new BitmapShader(bitmap, localMatrix, tilemode));
}