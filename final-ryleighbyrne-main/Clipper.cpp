#include "Clipper.h"


bool Edge::init(GPoint p0, GPoint p1, int wind) {
    this->windingValue = wind; 
    if (p0.y() > p1.y()) {
        std::swap(p0, p1);
        this->windingValue = -this->windingValue; 
    }

    this->top = GRoundToInt(p0.y());
    this->bottom = GRoundToInt(p1.y());

    if (top == bottom) {
        return false;
    }

    this->slope = (p1.x() - p0.x()) / (bottom - top);
    float dx = this->slope * (this->top - p0.y() +.5f);
    this->currX = p0.x() + dx;

    return true;
}


void clipEdge(GPoint p0, GPoint p1, int bm_top, int bm_bottom, int bm_right, int bm_left, std::vector<Edge> &edge) {
    int windingVal = 1; 
    Edge tempEdge; 
    
    if (p0.y() > p1.y()) {
        std::swap(p0, p1);
        windingVal = -windingVal;
    }

    if (p1.y() <= bm_top || p0.y() >= bm_bottom) {
        return;
    }
//clip top boundary
    if (p0.y() < bm_top) {
        float newX = p0.x() + (p1.x() - p0.x()) * (bm_top - p0.y()) / (p1.y() - p0.y());
        p0.set(newX, bm_top);
    }
//clip bottom boundary
    if (p1.y() > bm_bottom) {
        float newX = p1.x() - (p1.x() - p0.x()) * (p1.y() - bm_bottom) / (p1.y() - p0.y());
        p1.set(newX, bm_bottom);
    }
//check points again 
    if (p0.x() > p1.x()) {
        std::swap(p0, p1);
        windingVal = -windingVal;
    }
//entirely outside, add projection of left bound
    if (p1.x() <= bm_left) {
        p0.fX = p1.fX = bm_left;
        if(tempEdge.init(p0,p1,windingVal)){
            edge.push_back(tempEdge);
        }
        return;
        //return edge + edge->init(p0, p1, windingVal);
    }
//entirely outside, add projection of right bound
    if (p0.x() >= bm_right) {
        p0.fX = p1.fX = bm_right;
        if(tempEdge.init(p0,p1,windingVal)){
            edge.push_back(tempEdge);
        }
        return;
        //return edge + edge->init(p0, p1, windingVal);
    }
//clip and project left bound
    if (p0.x() < bm_left) {
        float newY = p0.y() + (bm_left - p0.x()) * (p1.y() - p0.y()) / (p1.x() - p0.x());
        GPoint temp0{bm_left, p0.y()};
        GPoint temp1{bm_left, newY};
        if(tempEdge.init(temp0,temp1,windingVal)){
            edge.push_back(tempEdge);
        }
        p0.set(bm_left, newY);
    }
//clip and project right bound
    if (p1.x() > bm_right) {
        float newY = p0.y() + (bm_right - p0.x()) * (p1.y() - p0.y()) / (p1.x() - p0.x());
        GPoint temp0{bm_right, newY};
        GPoint temp1{bm_right, p1.y()};
        if(tempEdge.init(temp0,temp1,windingVal)){
            edge.push_back(tempEdge);
        }
        p1.set(bm_right, newY);
    }
    if(tempEdge.init(p0,p1,windingVal)){
        edge.push_back(tempEdge);
    }
}