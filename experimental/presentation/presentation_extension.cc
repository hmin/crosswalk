// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/experimental/presentation/presentation_extension.h"

#include "base/memory/scoped_ptr.h"
#include "base/values.h"
#include "content/public/browser/browser_thread.h"
#include "ipc/ipc_message.h"
#include "xwalk/experimental/presentation/presentation_creator_impl.h"
#include "xwalk/runtime/browser/runtime.h"

using xwalk::Runtime;
using xwalk::RuntimeRegistry;

// This will be generated from presentation_api.js.
extern const char kSource_presentation_api[];

namespace xwalk {
namespace experimental {

using content::BrowserThread;
using extensions::XWalkExtension;
using extensions::XWalkExtensionInstance;

PresentationExtension::PresentationExtension()
  : XWalkInternalExtension() {
  set_name("navigator.presentation");
  RuntimeRegistry::Get()->AddObserver(this);
}

PresentationExtension::~PresentationExtension() {
  RuntimeRegistry::Get()->RemoveObserver(this);
}

const char* PresentationExtension::GetJavaScriptAPI() {
  return kSource_presentation_api;
}

void PresentationExtension::OnRuntimeAdded(Runtime* runtime) {
  DLOG(INFO) << "A new runtime is added";
  creator_ = new PresentationCreatorImpl(runtime->web_contents());
}

void PresentationExtension::OnRuntimeRemoved(Runtime* runtime) {
}

void PresentationExtension::OnRuntimeAppIconChanged(Runtime* runtime) {
}

XWalkExtensionInstance* PresentationExtension::CreateInstance(
    const XWalkExtension::PostMessageCallback& post_message) {
  return new PresentationInstance(this, post_message);
}

//////////////////////////////////////////////////////////////////
//  PresentationInstance
//////////////////////////////////////////////////////////////////
PresentationInstance::PresentationInstance(PresentationExtension* extension,
  const XWalkExtension::PostMessageCallback& post_message)
    : XWalkInternalExtensionInstance(post_message),
      extension_(extension) {
  RegisterFunction("requestShow", &PresentationInstance::OnRequestShow);
}

PresentationInstance::~PresentationInstance() {
}

void PresentationInstance::HandleMessage(scoped_ptr<base::Value> msg) {
  if (!BrowserThread::CurrentlyOn(BrowserThread::UI)) {
    BrowserThread::PostTask(
      BrowserThread::UI, FROM_HERE,
      base::Bind(&XWalkInternalExtensionInstance::HandleMessage,
          base::Unretained(this), base::Passed(&msg)));
    return;
  }

  XWalkInternalExtensionInstance::HandleMessage(msg.Pass());
}

void PresentationInstance::OnRequestShow(const std::string& function_name,
                                         const std::string& callback_id,
                                         base::ListValue* args) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));

  if (!args || args->GetSize() != 3) {
    DLOG(WARNING) << "Invalid parameters passed to " << function_name;
    return;
  }

  int request_id = -1;
  int opener_routing_id = MSG_ROUTING_NONE;
  std::string url;
  if (!args->GetInteger(0, &request_id) ||
      !args->GetInteger(1, &opener_routing_id) ||
      !args->GetString(2, &url)) {
    DLOG(WARNING) << "Invalid parameter type passed to " << function_name;
    return;
  }

  scoped_ptr<base::DictionaryValue> response(new base::DictionaryValue);
  response->SetString("cmd", "RequestShowResponse");
  response->SetInteger("request_id", request_id);
  response->SetInteger("error_code", 0);
  response->SetString("display_id", "fake_id");

  PostMessageToJS(scoped_ptr<base::Value>(response.release()));
}

}  // namespace experimental
}  // namespace xwalk

