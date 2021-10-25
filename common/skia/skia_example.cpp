#include "skia_example.h"
#include "SkPaint.h"
#include "SkCanvas.h"
#include "SkPath.h"
#include "SkPathBuilder.h"

SkImageInfo* SkiaExample::newImageInfo(int w, int h,SkColorType ct,SkAlphaType at,SkColorSpace* cs)
{
    return new SkImageInfo(SkImageInfo::Make(w, h, ct, at, sk_ref_sp(cs)));
}
SkSurface* SkiaExample::newSurface(const SkImageInfo* info, const SkSurfaceProps* props){
     return SkSurface::MakeRaster(*info, props).release();
}

SkSurface* SkiaExample::make_surface(int32_t w, int32_t h){
    SkImageInfo* info = newImageInfo(w, h, kRGBA_8888_SkColorType, kPremul_SkAlphaType, NULL);
    SkSurface* surface = newSurface(info, NULL);
    delete info;
    return surface;
}

void SkiaExample::emit_png(const char* path, SkSurface* surface) {
    SkImage* image = surface->makeImageSnapshot().release();
    SkData * data = image->encodeToData().release();
    image->unref();
    FILE* f = fopen(path, "wb");
    fwrite(data->data(), data->size(), 1, f);
    fclose(f);
    data->unref();
}

void SkiaExample::draw(SkCanvas* canvas) {
    SkPaint* fill = new SkPaint();
    fill->setColor(SkColorSetARGB(0xFF, 0x00, 0x00, 0xFF));
    canvas->drawPaint(*fill);

    fill->setColor(SkColorSetARGB(0xFF, 0x00, 0xFF, 0xFF));
    SkRect rect;
    rect.fLeft = 100.0f;
    rect.fTop = 100.0f;
    rect.fRight = 540.0f;
    rect.fBottom = 380.0f;
    canvas->drawRect(rect, *fill);

    SkPaint* stroke = new SkPaint();
    stroke->setColor(SkColorSetARGB(0xFF, 0xFF, 0x00, 0x00));
    stroke->setAntiAlias(true);
    stroke->setStroke(true);
    stroke->setStrokeWidth(5.0f);

    SkPathBuilder* path_builder = new SkPathBuilder();
    path_builder->moveTo(50.0f, 50.0f)
            .lineTo(590.0f, 50.0f)
            .cubicTo(-490.0f, 50.0f, 1130.0f, 430.0f, 50.0f, 430.0f)
            .lineTo(590.0f, 430.0f);

    SkPath* path = new SkPath(path_builder->detach());
    canvas->drawPath(*path, *stroke);

    fill->setColor(SkColorSetARGB(0x80, 0x00, 0xFF, 0x00));
    SkRect rect2;
    rect2.fLeft = 120.0f;
    rect2.fTop = 120.0f;
    rect2.fRight = 520.0f;
    rect2.fBottom = 360.0f;
    canvas->drawOval(rect2, *fill);

    delete path_builder;
    delete path;
    delete stroke;
    delete fill;
}

void SkiaExample::test1(){
    SkSurface* surface = SkiaExample::make_surface(640, 480);
    SkCanvas* canvas = surface->getCanvas();
    SkiaExample::draw(canvas);
    SkiaExample::emit_png("skia-c-example.png", surface);
    surface->unref();
}

extern "C" int main(){
    SkiaExample::test1();
    return 0;
}


