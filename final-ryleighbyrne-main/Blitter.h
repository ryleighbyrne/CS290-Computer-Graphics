#ifndef Blitter_DEFINED
#define Blitter_DEFINED

#include "GBitmap.h"
#include "GPaint.h"
#include "Blendmode.h"
#include "GMatrix.h"
#include <stack>


class Blitter {
public:
    Blitter(const GBitmap& bitmap, GMatrix CTM)
        : fBitmap(bitmap),
        fCTM(CTM)
        {}
        

    void fillRow(int y, int xLeft, int xRight, Blendfunc blendFunc, const GPaint& paint);

private:
    const GBitmap fBitmap;
    GMatrix fCTM; 
};


#endif