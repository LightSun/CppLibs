/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkLuaCanvas.h"

#include "SkLua.h"
//#include "SkStringUtils.h"
//#include "SkTo.h"

extern "C" {
    #include "lua.h"
    #include "lauxlib.h"
}

class AutoCallLua : public SkLua {
public:
    AutoCallLua(lua_State* L, const char func[], const char verb[]) : INHERITED(L) {
        lua_getglobal(L, func);
        if (!lua_isfunction(L, -1)) {
            int t = lua_type(L, -1);
            SkDebugf("--- expected function %d\n", t);
        }

        lua_newtable(L);
        this->pushString(verb, "verb");
    }

    ~AutoCallLua() {
        lua_State* L = this->get();
        if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
            SkDebugf("lua err: %s\n", lua_tostring(L, -1));
        }
        lua_settop(L, -1);
    }

private:
    typedef SkLua INHERITED;
};

#define AUTO_LUA(verb)  AutoCallLua lua(fL, fFunc.c_str(), verb)
///////////////////////////////////////////////////////////////////////////////

void SkLuaCanvas::pushThis() {
    SkLua(fL).pushCanvas(this);
}

///////////////////////////////////////////////////////////////////////////////

SkLuaCanvas::SkLuaCanvas(int width, int height, lua_State* L, const char func[])
    : INHERITED(width, height)
    , fL(L)
    , fFunc(func) {
}

SkLuaCanvas::~SkLuaCanvas() {}

void SkLuaCanvas::willSave() {
    AUTO_LUA("save");
    this->INHERITED::willSave();
}

SkCanvas::SaveLayerStrategy SkLuaCanvas::getSaveLayerStrategy(const SaveLayerRec& rec) {
    AUTO_LUA("saveLayer");
    if (rec.fBounds) {
        lua.pushRect(*rec.fBounds, "bounds");
    }
    if (rec.fPaint) {
        lua.pushPaint(*rec.fPaint, "paint");
    }

    (void)this->INHERITED::getSaveLayerStrategy(rec);
    // No need for a layer.
    return kNoLayer_SaveLayerStrategy;
}

void SkLuaCanvas::willRestore() {
    AUTO_LUA("restore");
    this->INHERITED::willRestore();
}
void SkLuaCanvas::didConcat44(const SkM44& matrix) {
     AUTO_LUA("concat");
     lua.pushMat44(matrix);
     this->INHERITED::didConcat44(matrix);
}

void SkLuaCanvas::onClipRect(const SkRect& r, SkClipOp op, ClipEdgeStyle edgeStyle) {
    AUTO_LUA("clipRect");
    lua.pushRect(r, "rect");
    lua.pushBool(kSoft_ClipEdgeStyle == edgeStyle, "aa");
    this->INHERITED::onClipRect(r, op, edgeStyle);
}

void SkLuaCanvas::onClipRRect(const SkRRect& rrect, SkClipOp op, ClipEdgeStyle edgeStyle) {
    AUTO_LUA("clipRRect");
    lua.pushRRect(rrect, "rrect");
    lua.pushBool(kSoft_ClipEdgeStyle == edgeStyle, "aa");
    this->INHERITED::onClipRRect(rrect, op, edgeStyle);
}

void SkLuaCanvas::onClipPath(const SkPath& path, SkClipOp op, ClipEdgeStyle edgeStyle) {
    AUTO_LUA("clipPath");
    lua.pushPath(path, "path");
    lua.pushBool(kSoft_ClipEdgeStyle == edgeStyle, "aa");
    this->INHERITED::onClipPath(path, op, edgeStyle);
}

void SkLuaCanvas::onClipRegion(const SkRegion& deviceRgn, SkClipOp op) {
    AUTO_LUA("clipRegion");
    this->INHERITED::onClipRegion(deviceRgn, op);
}

void SkLuaCanvas::onDrawPaint(const SkPaint& paint) {
    AUTO_LUA("drawPaint");
    lua.pushPaint(paint, "paint");
}

void SkLuaCanvas::onDrawPoints(PointMode mode, size_t count,
                               const SkPoint pts[], const SkPaint& paint) {
    AUTO_LUA("drawPoints");
    lua.pushArrayPoint(pts, SkToInt(count), "points");
    lua.pushPaint(paint, "paint");
}

void SkLuaCanvas::onDrawOval(const SkRect& rect, const SkPaint& paint) {
    AUTO_LUA("drawOval");
    lua.pushRect(rect, "rect");
    lua.pushPaint(paint, "paint");
}

void SkLuaCanvas::onDrawArc(const SkRect& rect, SkScalar startAngle, SkScalar sweepAngle,
                            bool useCenter, const SkPaint& paint) {
    AUTO_LUA("drawArc");
    lua.pushRect(rect, "rect");
    lua.pushScalar(startAngle, "startAngle");
    lua.pushScalar(sweepAngle, "sweepAngle");
    lua.pushBool(useCenter, "useCenter");
    lua.pushPaint(paint, "paint");
}

void SkLuaCanvas::onDrawRect(const SkRect& rect, const SkPaint& paint) {
    AUTO_LUA("drawRect");
    lua.pushRect(rect, "rect");
    lua.pushPaint(paint, "paint");
}

void SkLuaCanvas::onDrawRRect(const SkRRect& rrect, const SkPaint& paint) {
    AUTO_LUA("drawRRect");
    lua.pushRRect(rrect, "rrect");
    lua.pushPaint(paint, "paint");
}

void SkLuaCanvas::onDrawDRRect(const SkRRect& outer, const SkRRect& inner,
                               const SkPaint& paint) {
    AUTO_LUA("drawDRRect");
    lua.pushRRect(outer, "outer");
    lua.pushRRect(inner, "inner");
    lua.pushPaint(paint, "paint");
}

void SkLuaCanvas::onDrawPath(const SkPath& path, const SkPaint& paint) {
    AUTO_LUA("drawPath");
    lua.pushPath(path, "path");
    lua.pushPaint(paint, "paint");
}

void SkLuaCanvas::onDrawTextBlob(const SkTextBlob *blob, SkScalar x, SkScalar y,
                                 const SkPaint &paint) {
    AUTO_LUA("drawTextBlob");
    lua.pushTextBlob(blob, "blob");
    lua.pushScalar(x, "x");
    lua.pushScalar(y, "y");
    lua.pushPaint(paint, "paint");
}

void SkLuaCanvas::onDrawPicture(const SkPicture* picture, const SkMatrix* matrix,
                                const SkPaint* paint) {
    AUTO_LUA("drawPicture");
    // call through so we can see the nested picture ops
    this->INHERITED::onDrawPicture(picture, matrix, paint);
}

void SkLuaCanvas::onDrawDrawable(SkDrawable* drawable, const SkMatrix* matrix) {
    AUTO_LUA("drawDrawable");
    // call through so we can see the nested ops
    this->INHERITED::onDrawDrawable(drawable, matrix);
}
//---------------------------------------------

bool SkLuaCanvas::onDoSaveBehind(const SkRect* rect){
    AUTO_LUA("onDoSaveBehind");
    return this->INHERITED::onDoSaveBehind(rect);
}
void SkLuaCanvas::didRestore(){
    AUTO_LUA("didRestore");
    return this->INHERITED::didRestore();
}
void SkLuaCanvas::onMarkCTM(const char* name){
    AUTO_LUA("onMarkCTM");
    lua.pushString(name);
    return this->INHERITED::onMarkCTM(name);
}

void SkLuaCanvas::onDrawBehind(const SkPaint& paint){
    AUTO_LUA("onDrawBehind");
    lua.pushPaint(paint, "paint");
    return this->INHERITED::onDrawBehind(paint);
}
void SkLuaCanvas::onDrawRegion(const SkRegion& region, const SkPaint& paint){
    AUTO_LUA("onDrawRegion");
    lua.pushPaint(paint, "paint");
    lua.pushRegion(region, "region");
    return this->INHERITED::onDrawRegion(region, paint);
}
void SkLuaCanvas::onDrawGlyphRunList(const SkGlyphRunList& glyphRunList, const SkPaint& paint){
    AUTO_LUA("onDrawGlyphRunList");
    lua.pushPaint(paint, "paint");
    return this->INHERITED::onDrawGlyphRunList(glyphRunList, paint);
}

void SkLuaCanvas::onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
                const SkPoint texCoords[4], SkBlendMode mode, const SkPaint& paint){
    AUTO_LUA("onDrawPatch");
    return this->INHERITED::onDrawPatch(cubics, colors, texCoords, mode, paint);
}
void SkLuaCanvas::onDrawImage2(const SkImage* img, SkScalar dx, SkScalar dy, const SkSamplingOptions& ops,
                  const SkPaint* paint){
    AUTO_LUA("onDrawImage2");
    return this->INHERITED::onDrawImage2(img, dx, dy, ops, paint);
}
void SkLuaCanvas::onDrawImageRect2(const SkImage*img, const SkRect& src, const SkRect& dst,
                      const SkSamplingOptions& ops, const SkPaint* paint, SrcRectConstraint cons){
    AUTO_LUA("onDrawImageRect2");
    return this->INHERITED::onDrawImageRect2(img, src, dst, ops, paint, cons);
}
void SkLuaCanvas::onDrawImageLattice2(const SkImage* img, const Lattice& lat, const SkRect& dst,
                         SkFilterMode mode, const SkPaint* paint){
    AUTO_LUA("onDrawImageLattice2");
    return this->INHERITED::onDrawImageLattice2(img, lat, dst, mode, paint);
}
void SkLuaCanvas::onDrawAtlas2(const SkImage* img, const SkRSXform rxform[], const SkRect rects[],
                         const SkColor colors[], int count, SkBlendMode mode, const SkSamplingOptions& ops,
                  const SkRect* cull, const SkPaint* paint){
    AUTO_LUA("onDrawAtlas2");
    return this->INHERITED::onDrawAtlas2(img, rxform, rects, colors, count, mode, ops, cull, paint);

}
void SkLuaCanvas::onDrawEdgeAAImageSet2(const ImageSetEntry imageSet[], int count,
                                  const SkPoint dstClips[], const SkMatrix preViewMatrices[],
                                  const SkSamplingOptions& ops, const SkPaint* paint,
                           SrcRectConstraint srCons){
    AUTO_LUA("onDrawEdgeAAImageSet2");
    return this->INHERITED::onDrawEdgeAAImageSet2(imageSet, count, dstClips, preViewMatrices, ops, paint, srCons);
}

void SkLuaCanvas::onDrawVerticesObject(const SkVertices* vertices, SkBlendMode mode,
                                       const SkPaint& paint){
    AUTO_LUA("onDrawVerticesObject");
    return this->INHERITED::onDrawVerticesObject(vertices, mode, paint);
}

void SkLuaCanvas::onDrawAnnotation(const SkRect& rect, const char key[], SkData* value){
    AUTO_LUA("onDrawAnnotation");
    return this->INHERITED::onDrawAnnotation(rect, key, value);
}
void SkLuaCanvas::onDrawShadowRec(const SkPath& path, const SkDrawShadowRec& dsr){
    AUTO_LUA("onDrawShadowRec");
    return this->INHERITED::onDrawShadowRec(path, dsr);
}
void SkLuaCanvas::onDrawEdgeAAQuad(const SkRect& rect, const SkPoint clip[4], QuadAAFlags aaFlags,
                                    const SkColor4f& color, SkBlendMode mode){
    AUTO_LUA("onDrawEdgeAAQuad");
    return this->INHERITED::onDrawEdgeAAQuad(rect, clip, aaFlags, color, mode);
}

void SkLuaCanvas::onClipShader(sk_sp<SkShader> sp_shader, SkClipOp clipOp){
    AUTO_LUA("onClipShader");
    return this->INHERITED::onClipShader(sp_shader, clipOp);
}
void SkLuaCanvas::onResetClip(){
    AUTO_LUA("onResetClip");
    return this->INHERITED::onResetClip();
}

void SkLuaCanvas::onDiscard(){
    AUTO_LUA("onDiscard");
    return this->INHERITED::onDiscard();
}

