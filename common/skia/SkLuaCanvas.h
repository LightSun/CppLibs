/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLuaCanvas_DEFINED
#define SkLuaCanvas_DEFINED

#include "SkShader.h"
#include "SkCanvas.h"
#include "SkString.h"
#include "SkVertices.h"

struct lua_State;

class SkLuaCanvas : public SkCanvas {
public:
    void pushThis();

    SkLuaCanvas(int width, int height, lua_State*, const char function[]);
    ~SkLuaCanvas() override;

protected:
    void willSave() override;
    SaveLayerStrategy getSaveLayerStrategy(const SaveLayerRec&) override;
    void willRestore() override;

    void didSetM44(const SkM44&) override;
    void didConcat44(const SkM44&) override;
    void didTranslate(SkScalar, SkScalar) override;
    void didScale(SkScalar, SkScalar)override;

    void onDrawDRRect(const SkRRect&, const SkRRect&, const SkPaint&) override;
     void onDrawTextBlob(const SkTextBlob* blob, SkScalar x, SkScalar y,
                                const SkPaint& paint) override;

    void onDrawPaint(const SkPaint&) override;
    void onDrawPoints(PointMode, size_t count, const SkPoint pts[], const SkPaint&) override;
    void onDrawRect(const SkRect&, const SkPaint&) override;
    void onDrawOval(const SkRect&, const SkPaint&) override;
    void onDrawArc(const SkRect&, SkScalar, SkScalar, bool, const SkPaint&) override;
    void onDrawRRect(const SkRRect&, const SkPaint&) override;
    void onDrawPath(const SkPath&, const SkPaint&) override;

    void onClipRect(const SkRect&, SkClipOp, ClipEdgeStyle) override;
    void onClipRRect(const SkRRect&, SkClipOp, ClipEdgeStyle) override;
    void onClipPath(const SkPath&, SkClipOp, ClipEdgeStyle) override;
    void onClipRegion(const SkRegion&, SkClipOp) override;

    void onDrawPicture(const SkPicture*, const SkMatrix*, const SkPaint*) override;
    void onDrawDrawable(SkDrawable*, const SkMatrix*) override;

    bool onDoSaveBehind(const SkRect*) override;
    void didRestore()override;
    void onMarkCTM(const char*) override;

    // NOTE: If you are adding a new onDraw  to SkCanvas, PLEASE add an override to
    // SkCanvasEnforcer (in SkCanvasEnforcer.h). This ensures that subclasses using
    // that mechanism  will be required to implement the new function.
     void onDrawBehind(const SkPaint& paint) override;
     void onDrawRegion(const SkRegion& region, const SkPaint& paint) override;
     void onDrawGlyphRunList(const SkGlyphRunList& glyphRunList, const SkPaint& paint) override;

     void onDrawPatch(const SkPoint cubics[12], const SkColor colors[4],
                           const SkPoint texCoords[4], SkBlendMode mode, const SkPaint& paint) override;
     void onDrawImage2(const SkImage*, SkScalar dx, SkScalar dy, const SkSamplingOptions&,
                              const SkPaint*) override;
     void onDrawImageRect2(const SkImage*, const SkRect& src, const SkRect& dst,
                                  const SkSamplingOptions&, const SkPaint*, SrcRectConstraint) override;
     void onDrawImageLattice2(const SkImage*, const Lattice&, const SkRect& dst,
                                     SkFilterMode, const SkPaint*) override;
     void onDrawAtlas2(const SkImage*, const SkRSXform[], const SkRect src[],
                              const SkColor[], int count, SkBlendMode, const SkSamplingOptions&,
                              const SkRect* cull, const SkPaint*) override;
     void onDrawEdgeAAImageSet2(const ImageSetEntry imageSet[], int count,
                                       const SkPoint dstClips[], const SkMatrix preViewMatrices[],
                                       const SkSamplingOptions&, const SkPaint*,
                                       SrcRectConstraint) override;

     void onDrawVerticesObject(const SkVertices* vertices, SkBlendMode mode,
                                      const SkPaint& paint) override;

     void onDrawAnnotation(const SkRect& rect, const char key[], SkData* value) override;
     void onDrawShadowRec(const SkPath&, const SkDrawShadowRec&) override;
     void onDrawEdgeAAQuad(const SkRect& rect, const SkPoint clip[4], QuadAAFlags aaFlags,
                                  const SkColor4f& color, SkBlendMode mode) override;

     void onClipShader(sk_sp<SkShader>, SkClipOp) override;
     void onResetClip() override;

     void onDiscard() override;
private:
    lua_State*  fL;
    SkString    fFunc;

    void sendverb(const char verb[]);

    typedef SkCanvas INHERITED;
};

#endif
