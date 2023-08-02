#ifndef ScanConverter_DEFINED
#define ScanConverter_DEFINED

#include "Clipper.h"
#include "Blitter.h"


class ScanConverter {
public:

    static void scan(Edge* edges, int count, Blitter& blitter, Blendfunc blendFunc, const GPaint& paint);

    static void scanNonConvex(Edge* edges, int count, Blitter& blitter, Blendfunc blendFunc, const GPaint& paint);
};


#endif