// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_RENDERER_XWALK_PRESENTATION_MODULE_H
#define XWALK_EXTENSIONS_RENDERER_XWALK_PRESENTATION_MODULE_H

#include "base/memory/scoped_ptr.h"
#include "xwalk/extensions/renderer/xwalk_module_system.h"

namespace content {
class RenderView;
class RenderViewObserver;
}

namespace xwalk {
namespace extensions {

class PresentationMessageListener;

// Provides extra JS binding function for navigator.presentation API code.
class XWalkPresentationModule : public XWalkNativeModule {
 public:
  XWalkPresentationModule();
  virtual ~XWalkPresentationModule();

 private:
  // Returns the current render view.
  static content::RenderView* GetCurrentRenderView();

  // Called by JavaScript to retrieve the view id of the current render view.
  static void GetViewIDFromCurrentContext(
      const v8::FunctionCallbackInfo<v8::Value>& info);

  // Called by JavaScript to retrive 'window' object from the current context.
  static void GetWindowContext(const v8::FunctionCallbackInfo<v8::Value>& args);

  // Called by JavaScript to send a request to show presentation.
  static void RequestShowPresentation(
      const v8::FunctionCallbackInfo<v8::Value>& args);

  // XWalkNativeModule implementations.
  virtual v8::Handle<v8::Object> NewInstance() OVERRIDE;

  v8::Persistent<v8::ObjectTemplate> object_template_;

  // Observe the current render view.
  scoped_ptr<PresentationMessageListener> message_listener_;

  DISALLOW_COPY_AND_ASSIGN(XWalkPresentationModule);
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_RENDERER_XWALK_PRESENTATION_MODULE_H


