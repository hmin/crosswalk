// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXTENSIONS_RENDERER_XWALK_PRESENTATION_MODULE_H
#define XWALK_EXTENSIONS_RENDERER_XWALK_PRESENTATION_MODULE_H

#include "xwalk/extensions/renderer/xwalk_module_system.h"

namespace xwalk {
namespace extensions {

// Provides extra JS binding function for navigator.presentation API code.
class XWalkPresentationModule : public XWalkNativeModule {
 public:
  XWalkPresentationModule();
  virtual ~XWalkPresentationModule();

 private:
  // XWalkNativeModule implementations.
  virtual v8::Handle<v8::Object> NewInstance() OVERRIDE;

  v8::Persistent<v8::ObjectTemplate> object_template_;
};

}  // namespace extensions
}  // namespace xwalk

#endif  // XWALK_EXTENSIONS_RENDERER_XWALK_PRESENTATION_MODULE_H


