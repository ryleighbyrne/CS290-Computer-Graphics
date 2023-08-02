#include "GCanvas.h"
#include "GFinal.h"
#include "GRect.h"
#include "GColor.h"
#include "GBitmap.h"
#include "GPoint.h"
#include "GBlendMode.h"
#include "Blendmode.h"
#include "GMath.h"
#include "Clipper.h"
#include "GMatrix.h"
#include "GShader.h"
#include "GPath.h"
#include "Blitter.h"
#include "ScanConverter.h"
#include <deque>
#include <iostream>
#include <stack>
#include "RadialGradientShader.cpp"
#include "BilerpShader.cpp"
using namespace std;

class m_Final :public GFinal {
public: 
    m_Final(){}

     /**
     *  Return a radial-gradient shader.
     *
     *  This is a shader defined by a circle with center and a radius.
     *  The array of colors are evenly distributed between the center (color[0]) out to
     *  the radius (color[count-1]). Beyond the radius, it respects the TileMode.
     */
    std::unique_ptr<GShader> createRadialGradient(GPoint center, float radius,
                                                          const GColor colors[], int count,
                                                          GShader::TileMode mode) override{
        auto shader = new RadialGradientShader(center,radius, colors, count, mode);
        return std::unique_ptr<GShader>(shader); 
    }

    /**
     * Return a bitmap shader that performs kClamp tiling with a bilinear filter on each sample, respecting the
     * localMatrix.
     *
     * This is in contrast to the existing GCreateBitmapShader, which performs "nearest neightbor" sampling
     * when it fetches a pixel from the src bitmap.
     */
    std::unique_ptr<GShader> createBilerpShader(const GBitmap& bitmap,
                                                        const GMatrix& localMatrix) override{
        auto shader = new BilerpShader(bitmap, localMatrix);
        return std::unique_ptr<GShader>(shader); 
    }

    /**
     *  Add contour(s) to the specified path that will draw a line from p0 to p1 with the specified
     *  width and CapType. Note that "width" is the distance from one side of the stroke to the
     *  other, ala its thickness.
     */

    void addLine(GPath* path, GPoint p0, GPoint p1, float width, CapType) override {
        float k; 
        if((p1.fX - p0.fX)==0){
            k = (p1.fY - p0.fY) / .001; 
        } else{ 
            k = (p1.fY - p0.fY) / (p1.fX - p0.fX); 
        }   
        float angle = atan(k); 
        float length = hypot((p1.fX - p0.fX), p1.fY - p0.fY);

        float cornerx = cos(angle) * width / 2;
        float cornery = sin(angle) * width / 2;

        GPoint point0{p0.fX + cornerx, p0.fY + cornery}; 
        GPoint point1{p0.fX - cornerx, p0.fY - cornery}; 
        GPoint point2{p1.fX - cornerx, p1.fY - cornery}; 
        GPoint point3{p1.fX + cornerx, p1.fY + cornery};
        
        GPoint points[4] = {{0, -width/2}, {0, width/2}, {1, width/2}, {1, -width/2}};

        GMatrix mat = GMatrix::Scale(length, 1); 
        GMatrix mat_ang = GMatrix::Rotate(angle);
        GMatrix mat_trans = GMatrix::Translate(p0.fX, p0.fY);
        mat = mat_ang * mat; 
        mat = mat_trans * mat; 

        mat.mapPoints(points, points, 4);

        if(CapType::kRound){
            path->moveTo(points[0]);
            path->lineTo(points[1]);
            path->lineTo(points[2]);
            path->lineTo(points[3]);
           
            GPoint avg_point0{((points[0].fX + points[1].fX)/2), ((points[0].fY + points[1].fY)/2)};
            path->addCircle(avg_point0, width/2, GPath::Direction::kCCW_Direction);
            
            GPoint avg_point1{((points[2].fX + points[3].fX)/2), ((points[2].fY + points[3].fY)/2)};
            path->addCircle(avg_point1, width/2, GPath::Direction::kCCW_Direction);
        }
        if(CapType::kButt){
            path->moveTo(point0).lineTo(point1).lineTo(point2).lineTo(point3);
        }
        if(CapType::kSquare){
            path->moveTo(point0).lineTo(point1).lineTo(point2).lineTo(point3);
        }
    }
};

std::unique_ptr<GFinal> GCreateFinal(){
    return std::unique_ptr<GFinal>(new m_Final());
}