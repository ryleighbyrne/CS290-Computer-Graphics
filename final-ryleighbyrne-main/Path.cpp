
#include "GMatrix.h"
#include "GPath.h"
#include "GPoint.h"
#include "GRect.h"
#include "GMath.h"
#include <math.h>
using namespace std;


void GPath::ChopCubicAt(const GPoint src[4], GPoint dst[7], float t) {
   
    GPoint ab = (1 - t) * src[0] + t * src[1];
    GPoint bc = (1 - t) * src[1] + t * src[2];
    GPoint cd = (1 - t) * src[2] + t * src[3];
    GPoint ab_bc = (1 - t) * ab + t * bc;
    GPoint bc_cd = (1 - t) * bc + t * cd;

    dst[0] = src[0];
    dst[1] = ab;
    dst[2] = ab_bc;
    dst[3] = (1 - t) * (1 - t) * (1 - t) * src[0] + 3 * t * (1 - t) * (1 - t) * src[1] + 3 * t * t * (1 - t) * src[2] + t * t * t * src[3];
    dst[4] = bc_cd;
    dst[5] = cd;
    dst[6] = src[3]; 
}


void GPath::ChopQuadAt(const GPoint src[3], GPoint dst[5], float t) {
    dst[0] = src[0];
    dst[1] = (1 - t) * src[0] + t * src[1];
    dst[2] = (1 - t) * (1 - t) * src[0] + 2 * t * (1 - t) * src[1] + t * t * src[2];
    dst[3] = (1 - t) * src[1] + t * src[2];
    dst[4] = src[2];
}

GPath& GPath::addCircle(GPoint center, float radius, Direction dir) {

    float k_constant = 0.551915f;
    GPoint a = GPoint::Make(center.x(), center.y() - radius); 
    GPoint b = GPoint::Make(a.x() + radius * k_constant, a.y());
    GPoint c =  GPoint::Make(center.x() + radius, center.y() - radius * k_constant);
    GPoint d = GPoint::Make(center.x() + radius, center.y());
    GPoint e = GPoint::Make(center.x() + radius, center.y() + radius * k_constant);
    GPoint f = GPoint::Make(center.x() + radius * k_constant, center.y() + radius);
    GPoint g = GPoint::Make(center.x(), center.y() + radius);
    GPoint h = GPoint::Make(center.x() - radius * k_constant, center.y() + radius);
    GPoint i = GPoint::Make(center.x() - radius, center.y() + radius * k_constant);
    GPoint j = GPoint::Make(center.x() - radius, center.y());
    GPoint k = GPoint::Make(center.x() - radius, center.y() - radius * k_constant);
    GPoint l = GPoint::Make(center.x() - radius * k_constant, center.y() - radius);

    if(dir == kCCW_Direction){
        this->moveTo(a); 
        this->cubicTo(l,k,j);
        this->cubicTo(i,h,g); 
        this->cubicTo(f,e,d); 
        this->cubicTo(c,b,a); 
    }else{
        this->moveTo(a); 
        this->cubicTo(b,c,d);
        this->cubicTo(e,f,g); 
        this->cubicTo(h, i, j); 
        this->cubicTo(k,l, a); 
    }
    return *this; 

}

GPath& GPath::addRect(const GRect& rect, Direction dir){
    this->moveTo(rect.left(), rect.top()); 

    if(dir == GPath::kCW_Direction){
        this->lineTo(GPoint::Make(rect.right(), rect.top())); 
        this->lineTo(GPoint::Make(rect.right(), rect.bottom())); 
        this->lineTo(GPoint::Make(rect.left(), rect.bottom())); 
    }
    else{
        this->lineTo(GPoint::Make(rect.left(), rect.bottom())); 
        this->lineTo(GPoint::Make(rect.right(), rect.bottom())); 
        this->lineTo(GPoint::Make(rect.right(), rect.top())); 
    }
    return *this; 
}


GPath& GPath::addPolygon(const GPoint pts[], int count){
    if(count <= 1){
        return *this; 
    }

    this->moveTo(pts[0]); 
    for(int i = 1; i < count; i++){
        this->lineTo(pts[i]); 
    }

    return *this; 
}

GRect GPath::bounds() const{
    int count = this->fPts.size();

    if (count == 0) {
        return GRect::MakeWH(0, 0);
    }

    float minX = fPts[0].x(), maxX = fPts[0].x();
    float minY = fPts[0].y(), maxY = fPts[0].y();

    for (int i=1; i<fPts.size(); i++){
        minX = std::min(minX, fPts[i].x());
        maxX = std::max(maxX, fPts[i].x());
        minY = std::min(minY, fPts[i].y());
        maxY = std::max(maxY, fPts[i].y());
    }

    return GRect::MakeLTRB(
        minX, minY, maxX, maxY);
}

void GPath::transform(const GMatrix& matrix){
    matrix.mapPoints(this->fPts.data(), this->fPts.data(), this->fPts.size()); 
}
