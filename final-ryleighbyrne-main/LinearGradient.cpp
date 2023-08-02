#include "GBitmap.h"
#include "GMatrix.h"
#include "GShader.h"
#include "Blendmode.h"

class LinearGradient : public GShader {

public:
    LinearGradient(GPoint p0, GPoint p1, const GColor colors[], int count, GShader::TileMode tilemode) : fTileMode(tilemode){
        inverse =  GMatrix(); 
        colorList = (GColor*) malloc(count * sizeof(GColor)); 
        memcpy(colorList, colors, count * sizeof(GColor));
        N = count; 
        pt0 = p0; 
        pt1 = p1; 
    }

    ~LinearGradient() {
        free(colorList);
    }

    bool isOpaque() override {
        for(int i = 0; i < N; i++){
            if(colorList[i].a != 1){
                return false; 
            }
        }
        return true; 
    }

 

    bool setContext(const GMatrix& ctm) override {

        if(pt1.fX < pt0.fX){
            std::swap(pt0, pt1); 
        }

        float dx = pt1.x() - pt0.x(); 
        float dy = pt1.y() - pt0.y(); 
        GMatrix mat = GMatrix(dx, -dy, pt0.x(), dy, dx, pt0.y());
        return (ctm*mat).invert(&inverse); 
    }


    void shadeRow(int x, int y, int count, GPixel row[]) override { 

        for (int i = 0; i < count; i++) {
            GPoint pt; 
            pt.set(x + 0.5 + i, y + 0.5);
            GPoint p = inverse * pt;
            GColor c;

            // clamp
            float p_x;
            if(fTileMode == TileMode::kMirror) {
                p_x = mirror(p);
            }
            if(fTileMode == TileMode::kRepeat){
                p_x = repeat(p);
            }
            else{
                p_x = clamp(p);
            }
            float x_prime = p_x * (N-1);
            int index = GFloorToInt(x_prime);
            float w = x_prime - index;
            if (w == 0) {
                c = colorList[index];
            } else {
                c = (1 - w) * colorList[index]+ w * colorList[index + 1];
            }
            
            row[i] = convertColorToGPixel(c);
        }
      }

    float clamp(GPoint p) {
        float x = p.x();
        if (p.x() < 0.0) x = 0;
        if (p.x() > 1.0) x = 1;
        return x;
    }

    float repeat(GPoint p) {
        float x = p.x();
        while (x < 0) {
            x += 1.0;
        }
        while (x > 1.0) {
            x -= 1.0;
        }
        return x;
    }

    float mirror(GPoint p) { 
        float x = p.x(); // 4.5
        float r = x - GFloorToInt(x); // .5
        if (GFloorToInt(x) % 2 == 0) {
            return 1 - r;
        } else {
            return r;
        }
    }
    
    
private:
    GMatrix inverse; 
    int N; 
    GPoint pt0; 
    GPoint pt1; 
    GColor* colorList; 
    TileMode fTileMode;
};


std::unique_ptr<GShader> GCreateLinearGradient(GPoint p0, GPoint p1, const GColor colors[], int count, GShader::TileMode tilemode) {
    return std::unique_ptr<GShader>(new LinearGradient(p0, p1, colors, count, tilemode));
}