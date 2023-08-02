#include "GPaint.h"
#include "GPixel.h"
#include "Blendmode.h"

GPixel multPixels(GPixel p0, GPixel p1){
    unsigned a = div255(GPixel_GetA(p0) * GPixel_GetA(p1));
    unsigned r = div255(GPixel_GetR(p0) * GPixel_GetR(p1));  
    unsigned g = div255(GPixel_GetG(p0) * GPixel_GetG(p1));
    unsigned b = div255(GPixel_GetB(p0) * GPixel_GetB(p1));
    return GPixel_PackARGB(a,r,g,b);
}
 
GPixel convertColorToGPixel(const GPaint& color){ 
    unsigned a = round(GPinToUnit(color.getColor().a) * 255); 
    unsigned r = round(GPinToUnit(color.getColor().r) * GPinToUnit(color.getColor().a) * 255); 
    unsigned b = round(GPinToUnit(color.getColor().b) * GPinToUnit(color.getColor().a) * 255); 
    unsigned g = round(GPinToUnit(color.getColor().g) * GPinToUnit(color.getColor().a) * 255); 
    return GPixel_PackARGB(a, r, g, b); 
}

GPixel blend_clear(GPixel src, GPixel dst){
    unsigned a = 0;
    unsigned r = 0;
    unsigned g = 0;
    unsigned b = 0;
    return GPixel_PackARGB(a,r,g,b); 
}

// blend function that blends pixel with src over
GPixel blend_srcOver(GPixel src, GPixel dst){
    unsigned sa = GPixel_GetA(src);
    unsigned a = srcOver(GPixel_GetA(src), GPixel_GetA(dst), sa);
    unsigned r = srcOver(GPixel_GetR(src), GPixel_GetR(dst), sa);
    unsigned g = srcOver(GPixel_GetG(src), GPixel_GetG(dst), sa);
    unsigned b = srcOver(GPixel_GetB(src), GPixel_GetB(dst), sa);
    return GPixel_PackARGB(a,r,g,b); 
}

unsigned srcOver(unsigned s, unsigned d, unsigned sa){
    if(sa == 255){
        return s; 
    }
    return s + div255((255-sa) * d);  
    }

// blend funciton that blends pixel when sa = 255
GPixel blend_src(GPixel src, GPixel dst){
    return src; 
    }

// blend function that blends pixel when sa = 0
GPixel blend_dst(GPixel src, GPixel dst){
    return dst; 
}

// blend function that blends dst over
GPixel blend_dstOver(GPixel src, GPixel dst){
    unsigned da = GPixel_GetA(dst);
    unsigned a = dstOver(GPixel_GetA(src), GPixel_GetA(dst), da);
    unsigned r = dstOver(GPixel_GetR(src), GPixel_GetR(dst), da);
    unsigned g = dstOver(GPixel_GetG(src), GPixel_GetG(dst), da);
    unsigned b = dstOver(GPixel_GetB(src), GPixel_GetB(dst), da);
    return GPixel_PackARGB(a,r,g,b); 
    }

unsigned dstOver(unsigned s, unsigned d, unsigned da){
    if(da ==255){
        return d; 
    }
    return d + div255((255-da) * s); 
}

// blend function that blends src in 
GPixel blend_srcIn(GPixel src, GPixel dst){
    unsigned da = GPixel_GetA(dst);
    unsigned a = srcIn(GPixel_GetA(src), da);
    unsigned r = srcIn(GPixel_GetR(src), da);
    unsigned g = srcIn(GPixel_GetG(src), da);
    unsigned b = srcIn(GPixel_GetB(src), da);
    return GPixel_PackARGB(a,r,g,b); 
}

unsigned srcIn(unsigned s, unsigned da){
    if(da == 0){
        return 0; 
    }
    return div255(s*da); // ??? 
}

// blend function that blends dst in 
GPixel blend_dstIn(GPixel src, GPixel dst){
    unsigned sa = GPixel_GetA(src);
    unsigned a = dstIn(GPixel_GetA(dst), sa);
    unsigned r = dstIn(GPixel_GetR(dst), sa);
    unsigned g = dstIn(GPixel_GetG(dst), sa);
    unsigned b = dstIn(GPixel_GetB(dst), sa);
    return GPixel_PackARGB(a,r,g,b); 
}

unsigned dstIn(unsigned d, unsigned sa){
    if(sa==0){
        return 0;
    }
    return div255(sa*d); // ????
}

// blend function that blends src out
GPixel blend_srcOut(GPixel src, GPixel dst){
    unsigned da = GPixel_GetA(dst);
    unsigned a = srcOut(GPixel_GetA(src), da);
    unsigned r = srcOut(GPixel_GetR(src), da);
    unsigned g = srcOut(GPixel_GetG(src), da);
    unsigned b = srcOut(GPixel_GetB(src), da);
    return GPixel_PackARGB(a,r,g,b); 
}

unsigned srcOut(unsigned s, unsigned da){
    if(da == 255){
        return 0;
    }
    return div255((255-da) * s);
}

// blend function that blends dst out 
GPixel blend_dstOut(GPixel src, GPixel dst){
    unsigned sa = GPixel_GetA(src);
    unsigned a = dstOut(GPixel_GetA(dst), sa);
    unsigned r = dstOut(GPixel_GetR(dst), sa);
    unsigned g = dstOut(GPixel_GetG(dst), sa);
    unsigned b = dstOut(GPixel_GetB(dst), sa);
    return GPixel_PackARGB(a,r,g,b); 
}

unsigned dstOut(unsigned d, unsigned sa){
    if(sa == 255){
        return 0; 
    }
    return div255((255-sa) * d);
}

// blend function that blends src A top 
GPixel blend_srcATop(GPixel src, GPixel dst){
    unsigned sa = GPixel_GetA(src);
    unsigned da = GPixel_GetA(dst);
    unsigned a = srcATop(GPixel_GetA(src), GPixel_GetA(dst), sa, da);
    unsigned r = srcATop(GPixel_GetR(src), GPixel_GetR(dst), sa, da);
    unsigned g = srcATop(GPixel_GetG(src), GPixel_GetG(dst), sa, da);
    unsigned b = srcATop(GPixel_GetB(src), GPixel_GetB(dst), sa, da);
    return GPixel_PackARGB(a,r,g,b); 
}

unsigned srcATop(unsigned s, unsigned d, unsigned sa, unsigned da){
    if(sa == 255){
        return div255(da*s); 
    }
    return div255(da*s) + div255((255-sa) * d); 
}

// blend function that blends dst A top
GPixel blend_dstATop(GPixel src, GPixel dst){
    unsigned sa = GPixel_GetA(src);
    unsigned da = GPixel_GetA(dst);
    unsigned a = dstATop(GPixel_GetA(src), GPixel_GetA(dst), sa, da);
    unsigned r = dstATop(GPixel_GetR(src), GPixel_GetR(dst), sa, da);
    unsigned g = dstATop(GPixel_GetG(src), GPixel_GetG(dst), sa, da);
    unsigned b = dstATop(GPixel_GetB(src), GPixel_GetB(dst), sa, da);
    return GPixel_PackARGB(a,r,g,b); 
}

unsigned dstATop(unsigned s, unsigned d, unsigned sa, unsigned da){
    if(da == 255){
        return div255(d*sa);
    }
    return div255(d*sa) + div255((255-da) * s); 
}   

// blend function that blends xOr
GPixel blend_xOr(GPixel src, GPixel dst){
    unsigned sa = GPixel_GetA(src);
    unsigned da = GPixel_GetA(dst);
    unsigned a = xOr(GPixel_GetA(src), GPixel_GetA(dst), sa, da);
    unsigned r = xOr(GPixel_GetR(src), GPixel_GetR(dst), sa, da);
    unsigned g = xOr(GPixel_GetG(src), GPixel_GetG(dst), sa, da);
    unsigned b = xOr(GPixel_GetB(src), GPixel_GetB(dst), sa, da);
    return GPixel_PackARGB(a,r,g,b); 
}

unsigned xOr(unsigned s, unsigned d, unsigned sa, unsigned da){
    if(sa == 255){
        return div255((255-da)*s);
    }
    if(da == 255){
        return div255((255-sa)*d);
    }
    return div255((255-sa)*d) + div255((255-da)*s);
}

// division by 255
// TODO: change to implementation discussed in class? 
unsigned div255(unsigned x){
    return (x * 65793 + (1<<23))>>24;
}

Blendfunc getBlendfunc(const GBlendMode mode){
    return blend_func[static_cast<int>(mode)]; 
}
