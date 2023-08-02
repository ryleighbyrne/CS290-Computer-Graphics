
#include "GBitmap.h"
#include "GMatrix.h"
#include "GShader.h"
#include "Blendmode.h"
#include <vector>

class RadialGradientShader : public GShader {
public:
    RadialGradientShader(GPoint center, float radius, const GColor colors[], const int count, GShader::TileMode mode) {
        m_center = center;
        m_radius = radius;
        fTileMode = mode; 
        fColors = (GColor*) malloc(count * sizeof(GColor));
        memcpy(fColors, colors, count * sizeof(GColor));
        fColorCount = count;
    }

    bool setContext(const GMatrix& ctm) override {
        return ctm.invert(&m_ctm);
    }

    bool isOpaque() override {return false;}

    void shadeRow(int x, int y, int count, GPixel row[]) override {
        for (int i = 0; i < count; ++i) {
            GPoint pts[1]{x + i, y};
            m_ctm.mapPoints(pts,pts, 1);
            GPoint point = pts[0];
            float dx = point.fX - m_center.fX;
            float dy = point.fY - m_center.fY;

            float distance = sqrtf(dx * dx + dy * dy);

            float t = distance / m_radius;

            if (fTileMode == TileMode::kRepeat) {
                t = t - floor(t);
            } else if (fTileMode == TileMode::kMirror) {
                t = t - GFloorToInt(t);
                if (GFloorToInt(x) % 2 == 0) {
                    t = 1 - t;
                }

            }

            t = clamp(t, 0.0f, 1.0f);

            if (t == 0) {
                row[i] = convertColorToGPixel(fColors[0].pinToUnit());
            } else if (t == 1) {
                row[i] = convertColorToGPixel(fColors[fColorCount - 1].pinToUnit());
            } else {
                int index = floor(t * (fColorCount - 1));
                float span = 1.0f / (fColorCount - 1);
                float start = index * span;

                GColor c1 = fColors[index].pinToUnit();
                GColor c2 = fColors[index + 1].pinToUnit();

                t = clamp((t - start) / span, 0.0f, 1.0f);

                GColor color = GColor::RGBA(
                    c1.r * (1 - t) + c2.r * t,
                    c1.g * (1 - t) + c2.g * t,
                    c1.b * (1 - t) + c2.b * t, 
                    c1.a * (1 - t) + c2.a * t);

                row[i] = convertColorToGPixel(color);
            }
        }
    }


    float clamp(float val, float min, float max) {
        return std::max(min, std::min(max, val));
    }


    GColor blendColor(GColor& left, GColor& right, float t) {
        return GColor::RGBA(
                left.r * (1 - t) + right.r * t,
                left.g * (1 - t) + right.g * t,
                left.b * (1 - t) + right.b * t,
                left.a * (1 - t) + right.a * t
        );
    }

    GPoint m_center;
    float m_radius;
    GMatrix m_ctm;
    std::vector<GColor> colorList; 
    TileMode fTileMode;
    GColor* fColors;
    int fColorCount;


};


