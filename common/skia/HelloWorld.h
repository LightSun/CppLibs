/*
* Copyright 2017 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef HelloWorld_DEFINED
#define HelloWorld_DEFINED

#include "include/core/SkGraphics.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "tools/sk_app/Application.h"
#include "tools/sk_app/Window.h"

class SkCanvas;

//tools/flags/CommandLineFlags.cpp
/**
 "tools/gpu/BackendSurfaceFactory.cpp",
      "tools/gpu/BackendSurfaceFactory.h",
      "tools/gpu/BackendTextureImageFactory.cpp",
      "tools/gpu/BackendTextureImageFactory.h",
      "tools/gpu/FlushFinishTracker.cpp",
      "tools/gpu/FlushFinishTracker.h",
      "tools/gpu/GrContextFactory.cpp",
      "tools/gpu/GrTest.cpp",
      "tools/gpu/ManagedBackendTexture.cpp",
      "tools/gpu/ManagedBackendTexture.h",
      "tools/gpu/MemoryCache.cpp",
      "tools/gpu/MemoryCache.h",
      "tools/gpu/ProxyUtils.cpp",
      "tools/gpu/TestContext.cpp",
      "tools/gpu/TestOps.cpp",
      "tools/gpu/TestOps.h",
      "tools/gpu/YUVUtils.cpp",
      "tools/gpu/YUVUtils.h",
      "tools/gpu/gl/GLTestContext.cpp",  # See comment below about
                                         # GrContextFactory workaround.
      "tools/gpu/mock/MockTestContext.cpp",
 */
class HelloWorld : public sk_app::Application, sk_app::Window::Layer {
public:
    HelloWorld(int argc, char** argv, void* platformData);
    ~HelloWorld() override;

    void onIdle() override;

    void onBackendCreated() override;
    void onPaint(SkSurface*) override;
    bool onChar(SkUnichar c, skui::ModifierKey modifiers) override;

private:
    void updateTitle();

    sk_app::Window* fWindow;
    sk_app::Window::BackendType fBackendType;

    SkScalar fRotationAngle;
};

#endif
