#include "GCanvas.h"
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
#include "Shader.h"
#include <deque>
#include <iostream>
#include <stack>
using namespace std;
   
class m_canvas : public GCanvas {
public:
    m_canvas(const GBitmap& device) : fDevice(device) { 
        GMatrix mat;
        CTM.push(mat);
    }

    void save() override {
        GMatrix currentMat = CTM.top(); 
        CTM.push(currentMat); 
    }

    void restore() override {
        CTM.pop();  
    }

    void concat(const GMatrix& matrix) override{
        CTM.top().preConcat(matrix); 
    }


    void drawConvexPolygon(const GPoint pts[], int count, const GPaint& color) override{
        int bitmap_height = fDevice.height(); 
        int bitmap_width = fDevice.width(); 
        GPoint points[count];
        CTM.top().mapPoints(points, pts, count);

        std::vector<Edge> storage;

        // no area
        if(abs(points[0].fX*(points[1].fY - points[2].fY)+points[1].fX*(points[2].fY - points[0].fY) + points[2].fX*(points[0].fX-points[1].fY)) /2 == 0){
            return; 
        }
    
        for (int i = 0; i < count; ++i) {
            GPoint p0 = points[i];
            GPoint p1 = points[(i + 1) % count];
            clipEdge(p0, p1, 0, bitmap_height, bitmap_width, 0, storage);
        }

        int edgeCount = storage.size();
        if (edgeCount == 0) {
            return;
        }
        Blendfunc blendFunc = getBlendfunc(color.getBlendMode());
        Blitter blit = Blitter(fDevice, CTM.top()); 
        ScanConverter::scan(storage.data(), edgeCount, blit, blendFunc, color); 
    }

    void drawPath(const GPath& path, const GPaint& paint) override {
        int bitmap_height = fDevice.height(); 
        int bitmap_width = fDevice.width(); 

        GPath transformedPath = path; 
        transformedPath.transform(CTM.top()); 

        GPath::Edger edger = GPath::Edger(transformedPath);
        std::vector<Edge> storage;

        int edgeCount = 0;
        GPath::Verb verb;
// walk through pass of points and run through clipper 
        while(verb != GPath::kDone){
            GPoint nextPts[4];
            verb = edger.next(nextPts);

            if (verb == GPath::kLine) {
                clipEdge(nextPts[0], nextPts[1], 0, bitmap_height, bitmap_width, 0, storage);
            } else if(verb == GPath::kQuad){
                int k = generateNumSegmentsQuad(nextPts);  
                GPoint p0 = nextPts[0], p1; 
                for(float i = 1; i < k; ++i){
                    float delta = i /k;
                    p1 = evalQuad(nextPts, delta);
                    clipEdge(p0, p1, 0, bitmap_height, bitmap_width, 0, storage);
                    p0 = p1;
                }
                clipEdge(p0, nextPts[2], 0, bitmap_height, bitmap_width, 0, storage);
                
            } else if(verb == GPath::kCubic){ 
                int k = generateNumSegmentsCubic(nextPts); 
                GPoint p0 = nextPts[0], p1; 
                for(float i = 1; i < k; ++i){ // run loop on k start it at delta 
                    float delta = i /k;
                    p1 = evalCubic(nextPts, delta);
                    clipEdge(p0, p1, 0, bitmap_height, bitmap_width, 0, storage);
                    p0 = p1; 
                }
                clipEdge(p0, nextPts[3], 0, bitmap_height, bitmap_width, 0, storage);
            }
        }
        
        edgeCount = storage.size(); 
        if (edgeCount == 0) {
            return;
        }
        
        Blendfunc blendFunc = getBlendfunc(paint.getBlendMode());
        Blitter blit = Blitter(fDevice, CTM.top()); 
        ScanConverter::scanNonConvex(storage.data(), edgeCount, blit, blendFunc, paint); 
    }

    int generateNumSegmentsQuad(GPoint points[]){
        GPoint vector = .25f * (points[0] + (-2) * points[1] + points[2]);
        float magnitude = sqrt(vector.x() * vector.x() + vector.y() * vector.y()); 
        return GCeilToInt(sqrt(magnitude * 4)); 
    }

    int generateNumSegmentsCubic(GPoint points[]){
        GPoint vector1 =  (points[0] + (-2) * points[1] + points[2]);
        GPoint vector2 =  (points[1] + (-2) * points[2] + points[3]);
        GPoint vector = {std::max(std::abs(vector1.x()), std::abs(vector2.x())), std::max(std::abs(vector1.y()), std::abs(vector2.y()))}; 
        float magnitude = sqrt(vector.x() * vector.x() + vector.y() * vector.y());
        return GCeilToInt(sqrt(3 * magnitude)); 
    }


    GPoint evalQuad(GPoint points[], float t){
        GPoint a = points[0] + (-2) * points[1] + points[2]; 
        GPoint b =  2 * (points[1] + (-1) * points[0]);
        GPoint c = points[0]; 
        GPoint ret = (a * t + b) * t + c;
        return ret; 
    }

    GPoint evalCubic(GPoint points[], float t){
        GPoint a = -1 * points[0] + 3 * points[1] + (-3) * points[2] + points[3];
        GPoint b = 3 * points[0] + (-6) * points[1] + 3 * points[2];
        GPoint c = 3 * (points[1] + (-1) * points[0]);
        GPoint d = points[0];
        GPoint ret = ((a* t + b) * t + c) * t + d;
        return ret;
    }

    void drawRect(const GRect& rect, const GPaint& paint) override {
        GPoint points[4] = { {rect.left(), rect.top()}, {rect.right(), rect.top()},{rect.right(), rect.bottom()},{rect.left(), rect.bottom()}};
        drawConvexPolygon(points, 4, paint);
    } 

    void drawPaint(const GPaint& color) override {
        int bottom = GRoundToInt(fDevice.height()); 
        int right = GRoundToInt(fDevice.width());
        Blendfunc blendFunc = getBlendfunc(color.getBlendMode()); 
        
        for(int y = 0; y < bottom; y++){
            drawRow(y, 0, right, color, blendFunc); 
        }
    }

    void drawRow(int y, int leftX, int rightX, const GPaint& paint, Blendfunc blendFunc) {
        leftX = std::max(0, leftX);
        rightX = std::min(fDevice.width(), rightX);
        GShader* shader = paint.getShader();
        if (shader == nullptr) {
            GPixel src = convertColorToGPixel(paint);
            for (int x = leftX; x < rightX; ++x) {
                GPixel* addr = fDevice.getAddr(x, y);
                *addr = blendFunc(src, *addr);
            }
        } else {
            if (!shader->setContext(CTM.top())) {
                return;
            }

            int count = rightX - leftX;
            GPixel shaded[count];
            shader->shadeRow(leftX, y, count, shaded);
            for (int x = leftX; x < rightX; ++x) {
                GPixel* addr = fDevice.getAddr(x, y);
                *addr = blendFunc(shaded[x - leftX], *addr);
            }
        }
    }

    void drawTriangle(const GPoint points[3], const GColor colors[], const GPoint texs[], const GPaint& paint) {
        if (colors != nullptr && texs != nullptr) {
            GPaint _paint(new CompositeShader(paint.getShader(), new TriShader(points, colors)));
            drawConvexPolygon(points, 3, _paint);
        } else if (colors != nullptr) {
            GPaint _paint(new TriShader(points, colors));
            drawConvexPolygon(points, 3, _paint);
        } else {
            drawConvexPolygon(points, 3, paint);
        }
    }

    void drawTriangleWithTex(const GPoint points[3], const GColor colors[3], const GPoint texs[3], GShader* originalShader) {
        GMatrix P, T, invT;
        P = GMatrix(points[1].x() - points[0].x(), points[2].x() - points[0].x(), points[0].x(),
                    points[1].y() - points[0].y(), points[2].y() - points[0].y(), points[0].y());
        T = GMatrix(texs[1].x() - texs[0].x(), texs[2].x() - texs[0].x(), texs[0].x(),
                    texs[1].y() - texs[0].y(), texs[2].y() - texs[0].y(), texs[0].y());

        if (T.invert(&invT) == false) {
            return;
        }

        ProxyShader proxy = ProxyShader(originalShader, P * invT);
        GPaint p(&proxy);

        drawTriangle(points, colors, texs, p);
    }

        /**
         *  Draw a mesh of triangles, with optional colors and/or texture-coordinates at each vertex.
         *
         *  The triangles are specified by successive triples of indices.
         *      int n = 0;
         *      for (i = 0; i < count; ++i) {
         *          point0 = vertx[indices[n+0]]
         *          point1 = verts[indices[n+1]]
         *          point2 = verts[indices[n+2]]
         *          ...
         *          n += 3
         *      }
         *
         *  If colors is not null, then each vertex has an associated color, to be interpolated
         *  across the triangle. The colors are referenced in the same way as the verts.
         *          color0 = colors[indices[n+0]]
         *          color1 = colors[indices[n+1]]
         *          color2 = colors[indices[n+2]]
         *
         *  If texs is not null, then each vertex has an associated texture coordinate, to be used
         *  to specify a coordinate in the paint's shader's space. If there is no shader on the
         *  paint, then texs[] should be ignored. It is referenced in the same way as verts and colors.
         *          texs0 = texs[indices[n+0]]
         *          texs1 = texs[indices[n+1]]
         *          texs2 = texs[indices[n+2]]
         *
         *  If both colors and texs[] are specified, then at each pixel their values are multiplied
         *  together, component by component.
         */
    void drawMesh(const GPoint verts[], const GColor colors[], const GPoint texs[], int count, const int indices[], const GPaint& paint) override {

        int n = 0;
        for (int i = 0; i < count; ++i) {

            const GPoint points[3] = {verts[indices[n+0]], verts[indices[n+1]], verts[indices[n+2]]};
            if (colors != nullptr && texs != nullptr) {
                const GColor col[3] = {colors[indices[n+0]], colors[indices[n+1]], colors[indices[n+2]]};
                const GPoint texture[3] = {texs[indices[n+0]], texs[indices[n+1]], texs[indices[n+2]]};   
                GShader* shader = paint.getShader(); 
                drawTriangleWithTex(points, col, texture, shader);
            } else if (colors != nullptr) {
                const GColor col[3] = {colors[indices[n+0]], colors[indices[n+1]], colors[indices[n+2]]};
                GShader* shader = paint.getShader(); 
                drawTriangle(points, col, nullptr, shader);
            } else if (texs != nullptr) {
                const GPoint texture[3] = {texs[indices[n+0]], texs[indices[n+1]], texs[indices[n+2]]};  
                GShader* shader = paint.getShader(); 
                drawTriangleWithTex(points, nullptr, texture, shader);
            } else {
                drawConvexPolygon(points, 3, paint);
            }
            n += 3;
        }
    }
            /**
         *  Draw the quad, with optional color and/or texture coordinate at each corner. Tesselate
         *  the quad based on "level":
         *      level == 0 --> 1 quad  -->  2 triangles
         *      level == 1 --> 4 quads -->  8 triangles
         *      level == 2 --> 9 quads --> 18 triangles
         *      ...
         *  The 4 corners of the quad are specified in this order:
         *      top-left --> top-right --> bottom-right --> bottom-left
         *  Each quad is triangulated on the diagonal top-right --> bottom-left
         *      0---1
         *      |  /|
         *      | / |
         *      |/  |
         *      3---2
         *
         *  colors and/or texs can be null. The resulting triangles should be passed to drawMesh(...).
         */
    void drawQuad(const GPoint verts[4], const GColor colors[4], const GPoint texs[4], int level, const GPaint& paint) override{
        int numberOfQuads = (int) pow(level+1, 2);
        int numberOfTriangles = numberOfQuads * 2;
        int numberOfCorners = (int) pow(level+2, 2);
        GPoint corner[numberOfCorners];
        GColor clrs[numberOfCorners];
        GPoint t[numberOfCorners];
        
        int index = 0;
        for (int y = 0; y <= level + 1; y++) {
            float v = (float) y / (level + 1);
            for (int x = 0; x <= level + 1; x++) {
                float u = (float) x / (level + 1);
                corner[index] =   (1.f - u) * (1.f - v) * verts[0]+ u * (1.f - v) * verts[1] + u * v * verts[2] + v * (1.f - u) * verts[3]; 
                if (colors != nullptr) {
                    clrs[index] = (1.f - u) * (1.f - v) * colors[0]+ u * (1.f - v) * colors[1] + u * v * colors[2] + v * (1.f - u) * colors[3]; 
                }
                if (texs != nullptr) {
                    t[index] = (1.f - u) * (1.f - v) * texs[0]+ u * (1.f - v) * texs[1] + u * v * texs[2] + v * (1.f - u) * texs[3];
                }
                index++;
            }
        }

        int indices[6 * numberOfQuads];
        int count = 0; 
        int k = 0;
        int l = 0;
        while(count <= level) {
            indices[k] = l;
            indices[k+1] = l + 1;
            indices[k+2] = l + level + 2;
            l += 1; 
            k += 3;

            for (int innerCount = 0; innerCount < level; innerCount++) {
                indices[k] = l;
                indices[k+1] = l + level + 2;
                indices[k+2] = l + level + 1;
                indices[k+3] = l;
                indices[k+4] = l + 1;
                indices[k+5] = l + level + 2;
                l += 1; 
                k += 6;
            }

            indices[k] = l;
            indices[k+1] = l + level + 2;
            indices[k+2] = l + level + 1;
            l += 1;
            k += 3;
            ++count; 
        }
        
        if (texs == nullptr && colors == nullptr) {
            drawMesh(corner, nullptr, nullptr, numberOfTriangles, indices, paint);
        } else if (texs == nullptr) {
            drawMesh(corner, clrs, nullptr, numberOfTriangles, indices, paint);
        } else if (colors == nullptr) {
            drawMesh(corner, nullptr, t, numberOfTriangles, indices, paint);
        } else {
            drawMesh(corner, clrs, t, numberOfTriangles, indices, paint);
        }
    }

private:
    const GBitmap fDevice;
    std::stack<GMatrix> CTM; 
};

std::unique_ptr<GCanvas> GCreateCanvas(const GBitmap& device) {
    return std::unique_ptr<GCanvas>(new m_canvas(device));
}

std::string GDrawSomething(GCanvas* canvas, GISize dim) {

    const float twopi = (float)(2*M_PI);
    auto shader = GCreateLinearGradient({0, 0}, {1,0}, {0,0,0,1}, {1,1,1,1});

    const int TRIS = 40;
    GPoint pts[TRIS + 1];
    GColor colors[TRIS + 1];
    int indices[TRIS * 3];

    const float rad = 125;
    const float center = 128;
    pts[0] = { center, center };
    colors[0] = { 0, 0, 1, 1 };

    float angle = 0;
    float da = twopi / (TRIS - 1);
    int* idx = indices;
    for (int i = 1; i <= TRIS; ++i) {
        float x = cos(angle);
        float y = sin(angle);
        pts[i] = { x * rad + center, y * rad + center };
        colors[i] = { angle / twopi, 0, (twopi - angle) / twopi, 1 };
        idx[0] = 0; idx[1] = i; idx[2] = i < TRIS ? i + 1 : 1;
        idx += 3;
        angle += da;
    }
    canvas->drawMesh(pts, colors, nullptr, TRIS, indices, GPaint(shader.get()));
    /*
    GRect left_eye = GRect::LTRB(80,85,112,118); 
    GRect right_eye = GRect::LTRB(144,85,176,118);
    GColor color = {0, 0, 0, 1};
    canvas->drawRect(left_eye, color);
    canvas->drawRect(right_eye, color);

    for(int x = 80; x<156; x++){
        GRect frown = GRect::LTRB(x, 140, x+20, 150); 
        canvas->drawRect(frown, color);
    }

    GRect dimple1 = GRect::LTRB(80, 130, 90, 140);
    GRect dimple2 = GRect::LTRB(166, 130, 175, 140);

    canvas->drawRect(dimple1, color); 
    canvas->drawRect(dimple2, color); 
    */
    return "smiley circle";
}
