// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXPERIMENTAL_PRESENTATION_PRESENTATION_EXTENSION_H
#define XWALK_EXPERIMENTAL_PRESENTATION_PRESENTATION_EXTENSION_H

#include "xwalk/extensions/browser/xwalk_extension_internal.h"

namespace xwalk {
namespace experimental {

class PresentationExtension : public extensions::XWalkInternalExtension {
 public:
  PresentationExtension();
  virtual ~PresentationExtension();

  // XWalkExtension implementation.
  virtual const char* GetJavaScriptAPI() OVERRIDE;
  virtual extensions::XWalkExtensionInstance* CreateInstance(
      const extensions::XWalkExtension::PostMessageCallback& post_msg) OVERRIDE;
};

class PresentationInstance : public extensions::XWalkInternalExtensionInstance {
 public:
  PresentationInstance(PresentationExtension* extension,
      const extensions::XWalkExtension::PostMessageCallback& post_message);
  virtual ~PresentationInstance();

  virtual void HandleMessage(scoped_ptr<base::Value> msg) OVERRIDE;

 private:
  void OnRequestShow(const std::string& function_name,
                     const std::string& callback_id,
                     base::ListValue* args);

  PresentationExtension* extension_;
};

}  // namespace experimental
}  // namespace xwalk

#endif  // XWALK_EXPERIMENTAL_PRESENTATION_PRESENTATION_EXTENSION_H


