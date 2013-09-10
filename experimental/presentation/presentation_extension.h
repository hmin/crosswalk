// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXPERIMENTAL_PRESENTATION_PRESENTATION_EXTENSION_H
#define XWALK_EXPERIMENTAL_PRESENTATION_PRESENTATION_EXTENSION_H

#include "xwalk/extensions/browser/xwalk_extension_internal.h"

#include <map>

#include "base/memory/linked_ptr.h"
#include "xwalk/runtime/browser/runtime_registry.h"

namespace xwalk {
namespace experimental {

class PresentationCreatorImpl;

class PresentationExtension : public extensions::XWalkInternalExtension,
                           public RuntimeRegistryObserver {
 public:
  PresentationExtension();
  virtual ~PresentationExtension();

  // XWalkExtension implementation.
  virtual const char* GetJavaScriptAPI() OVERRIDE;
  virtual extensions::XWalkExtensionInstance* CreateInstance() OVERRIDE;

 private:
  // RuntimeRegistryObserver impls.
  virtual void OnRuntimeAdded(xwalk::Runtime* runtime) OVERRIDE;
  virtual void OnRuntimeRemoved(xwalk::Runtime* runtime) OVERRIDE;
  virtual void OnRuntimeAppIconChanged(xwalk::Runtime* runtime) OVERRIDE;

  // Mapping of routing id and presentation creator. Each Runtime instance
  // should have a presentation creator to handle the presentation showing
  // request from renderer process.
  typedef std::map<Runtime*, linked_ptr<PresentationCreatorImpl> >
      PresentationCreatorMap;
  PresentationCreatorMap creator_map_;
};

class PresentationInstance : public extensions::XWalkInternalExtensionInstance {
 public:
  PresentationInstance(PresentationExtension* extension);
  virtual ~PresentationInstance();

  virtual void HandleMessage(scoped_ptr<base::Value> msg) OVERRIDE;

 private:
  void OnRequestShow(const std::string& function_name,
                     const std::string& callback_id,
                     base::ListValue* args);

  void InitializeDisplayManagerOnUI();

  PresentationExtension* extension_;
};

}  // namespace experimental
}  // namespace xwalk

#endif  // XWALK_EXPERIMENTAL_PRESENTATION_PRESENTATION_EXTENSION_H
