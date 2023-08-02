#include "Blitter.h"
#include "GPaint.h"
#include "GPixel.h"
#include "GShader.h"
#include "Blendmode.h"
#include "GMatrix.h"


void Blitter::fillRow(int y, int leftX, int rightX, Blendfunc blendFunc, const GPaint& paint) {

    leftX = std::max(0, leftX);
    rightX = std::min(this->fBitmap.width(), rightX);
    GShader* shader = paint.getShader();

    if (shader == nullptr) {
        GPixel src = convertColorToGPixel(paint);
        for (int x = leftX; x < rightX; ++x) {
            GPixel* addr = this->fBitmap.getAddr(x, y);
            *addr = blendFunc(src, *addr);
        }
     } else {
        if (!shader->setContext(this->fCTM)) {
             return;
        }
        int count = rightX - leftX;
        GPixel shaded[count];
        shader->shadeRow(leftX, y, count, shaded);
        for (int x = leftX; x < rightX; ++x) {
            GPixel* addr = this->fBitmap.getAddr(x, y);
            *addr = blendFunc(shaded[x - leftX], *addr);
        }
    }
}