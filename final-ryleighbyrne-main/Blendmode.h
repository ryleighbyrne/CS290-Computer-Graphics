
#ifndef Blendmode
#define Blendmode

#include "GPaint.h"
#include "GPixel.h"

typedef GPixel (*Blendfunc)(GPixel, GPixel);

GPixel blend_clear(const GPixel src, const GPixel dst); 
GPixel blend_src(const GPixel src, const GPixel dst); 
GPixel blend_dst(const GPixel src, const GPixel dst); 
GPixel blend_srcOver(const GPixel src, const GPixel dst); 
GPixel blend_dstOver(const GPixel src, const GPixel dst); 
GPixel blend_srcIn(const GPixel src, const GPixel dst); 
GPixel blend_dstIn(const GPixel src, const GPixel dst); 
GPixel blend_srcOut(const GPixel src, const GPixel dst); 
GPixel blend_dstOut(const GPixel src, const GPixel dst); 
GPixel blend_srcATop(const GPixel src, const GPixel dst); 
GPixel blend_dstATop(const GPixel src, const GPixel dst); 
GPixel blend_xOr(const GPixel src, const GPixel dst); 

unsigned srcOver(unsigned s, unsigned d, unsigned sa);
unsigned div255(unsigned x); 
unsigned dstOver(unsigned s, unsigned d, unsigned da);
unsigned srcIn(unsigned s, unsigned da);
unsigned dstIn(unsigned d, unsigned sa);
unsigned srcOut(unsigned s, unsigned da);
GPixel convertColorToGPixel(const GPaint& color); 
GPixel multPixels(GPixel p0, GPixel p1);
unsigned dstOut(unsigned d, unsigned sa);
unsigned srcATop(unsigned s, unsigned d, unsigned sa, unsigned da);
unsigned dstATop(unsigned s, unsigned d, unsigned sa, unsigned da);
unsigned xOr(unsigned s, unsigned d, unsigned sa, unsigned da);


const Blendfunc blend_func[] = {
    blend_clear, 
    blend_src, 
    blend_dst, 
    blend_srcOver, 
    blend_dstOver, 
    blend_srcIn, 
    blend_dstIn, 
    blend_srcOut, 
    blend_dstOut, 
    blend_srcATop, 
    blend_dstATop, 
    blend_xOr,
};

Blendfunc getBlendfunc(const GBlendMode mode); 


#endif