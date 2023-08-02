#ifndef Clipper_DEFINED
#define Clipper_DEFINED

#include <deque>
#include "GPoint.h"
#include "GRect.h"


struct Edge {
    int top;
    int bottom;
    float currX;
    float slope;
    bool init(GPoint p0, GPoint p1, int wind);
    int windingValue; 
};


void clipEdge(GPoint p0, GPoint p1, int bm_top, int bm_bottom, int bm_right, int bm_left, std::vector<Edge> &edge);


#endif