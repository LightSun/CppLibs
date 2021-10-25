#ifndef SKIAEXAMPLE_H
#define SKIAEXAMPLE_H

#include "include/core/SkTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceProps.h"

class SkiaExample
{
public:
    static SkImageInfo* newImageInfo(int w, int h,SkColorType ct,SkAlphaType at,SkColorSpace* cs);
    static SkSurface* newSurface(const SkImageInfo* info, const SkSurfaceProps* props = NULL);

    static SkSurface* make_surface(int32_t w, int32_t h);
    static void emit_png(const char* path, SkSurface* surface);
    static void draw(SkCanvas* canvas);
    static void test1();
};

#endif // SKIAEXAMPLE_H
