#include "GBitmap.h"
#include "GMatrix.h"
#include "GShader.h"

class BilerpShader : public GShader {

public:
    BilerpShader(const GBitmap& bitmap, const GMatrix& localMatrix)
        : fBitmap(bitmap)
        , fLM(localMatrix) {}

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
            x_prime = clamp(inv_pt.fX, fBitmap.width());
            y_prime = clamp(inv_pt.fY, fBitmap.height());
            
            row[i] = *fBitmap.getAddr(x_prime, y_prime);
        }
        
    }

    int clamp(float x, int bounds) {
        int x_prime = std::max(GFloorToInt(x), 0);
        x_prime = std::min(GFloorToInt(x_prime), bounds - 1);
        return x_prime;
    }


private:
    GBitmap fBitmap;
    GMatrix fLM;
    GMatrix inverse; 
};
