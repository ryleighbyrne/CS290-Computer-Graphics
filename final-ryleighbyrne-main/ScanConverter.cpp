#include <vector>
#include "Blitter.h"
#include "GMath.h"
#include "ScanConverter.h"
#include "Clipper.h"
#include <algorithm>
#include <vector>
#include <iostream>

using namespace std;

bool edgeComparator(Edge e0, Edge e1) {
    if (e0.top < e1.top) {
        return true;
    }
    else if (e1.top < e0.top) {
        return false;
    }
    else if (e0.currX < e1.currX) {
        return true;
    }
    else if (e1.currX < e0.currX) {
        return false;
    }
    return e0.slope < e1.slope;
}

void ScanConverter::scan(Edge* edges, int count, Blitter& blitter, Blendfunc blendFunc, const GPaint& paint) {

    std::sort(edges, edges + count, edgeComparator);
    int lastY = edges[count - 1].bottom;

    Edge left = edges[0];
    Edge right = edges[1];

    int nextEdge = 2;

    float currY = left.top;

    float leftX = left.currX;
    float rightX = right.currX;

    while (currY < lastY) {
        blitter.fillRow(currY, GRoundToInt(leftX), GRoundToInt(rightX), blendFunc, paint);
        currY++;

        if (currY == left.bottom +1 ) {
            left = edges[nextEdge];
            nextEdge++;

            leftX = left.currX;
        } else {
            leftX += left.slope;
        }

        if (currY == right.bottom + 1) {
            right = edges[nextEdge];
            nextEdge++;

            rightX = right.currX;
        } else {
            rightX += right.slope;
        }
    }
}


bool xComparator(Edge* first, Edge* second) {
    return first->currX < second->currX;
}


void resort(Edge* edge, std::vector<Edge*>& edges) {
    int pos = std::find(edges.begin(), edges.end(), edge) - edges.begin();
    std::sort(edges.begin(), edges.begin() + pos, xComparator);
}

void ScanConverter::scanNonConvex(Edge* edges, int count, Blitter& blitter, Blendfunc blendFunc, const GPaint& paint) { 
    
    std::sort(edges, edges + count, edgeComparator);
    std::vector<Edge*> edgeList;
    int maxY = -1000000000;
    for (int i = 0; i < count; ++i) {
        edgeList.push_back(&edges[i]);
        if(edges[i].bottom > maxY){
            maxY = edges[i].bottom; 
        }
    }

    int minY = edgeList.front()->top;
    // make index 

// loop through ys 
    for (int y = minY; y < maxY;) {
        int wind = 0;
        Edge* edge = edgeList.front();
        Edge* nextEdge;
// while edge is still 'active'
        //cout << " begin y: " << y << endl; 
        while (edge != nullptr && edge->top <= y) {
            assert(edge->bottom > y);
            int left, right;
            // if wind is 0 for left, set left
            if (wind == 0) {
                left = GRoundToInt(edge->currX);
            }

            assert(edge->windingValue == -1 || edge->windingValue == 1); 
            wind += edge->windingValue;
            // if wind is 0 again, set right and fill that row from left to right
            if (wind == 0) {
                right = GRoundToInt(edge->currX);
                blitter.fillRow(y, left, right, blendFunc, paint);
            }

            int nextEdgeIndex = std::find(edgeList.begin(), edgeList.end(), edge) - edgeList.begin() + 1;
            if (nextEdgeIndex >= edgeList.size()) {
                nextEdge = nullptr;
            } else {
                nextEdge = edgeList[nextEdgeIndex];
            }
            // if edge is not active anymore, remove it from the edgeList
            // if still active, increment currX by slope
            if (edge->bottom == y + 1) {
                //cout << "y: " << y << " wind: " << edge->windingValue << endl; 
                edgeList.erase(edgeList.begin() + (std::find(edgeList.begin(), edgeList.end(), edge) - edgeList.begin()));
            } else {
                edge->currX += edge->slope;
                //resort(edge, edgeList); // resort edges now based on currentX
            }

            edge = nextEdge; // move to next edge
        }
       // cout << " end y: " << y << endl; 
        assert(wind == 0); 
        y++; // next row 
    // while nextEdge is sitll active 
        while (edge != nullptr && edge->top == y ) {
            int nextEdgeIndex = std::find(edgeList.begin(), edgeList.end(), edge) - edgeList.begin()  + 1;
            if (nextEdgeIndex >= edgeList.size()) {
                nextEdge = nullptr;
            } else {
                nextEdge = edgeList[nextEdgeIndex];
            }
           
            edge = nextEdge;
        }
        resort(edge, edgeList);
    }
}


    
