// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/experimental/presentation/presentation_extension.h"

#include "base/memory/scoped_ptr.h"
#include "base/stl_util.h"
#include "base/values.h"
#include "content/public/browser/browser_thread.h"
#include "ipc/ipc_message.h"
#include "xwalk/experimental/presentation/presentation_creator_impl.h"
#include "xwalk/experimental/presentation/presentation_display_manager.h"
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
  set_name("navigator.experimental.presentation");
  RuntimeRegistry::Get()->AddObserver(this);
}

PresentationExtension::~PresentationExtension() {
  RuntimeRegistry::Get()->RemoveObserver(this);
}

const char* PresentationExtension::GetJavaScriptAPI() {
  return kSource_presentation_api;
}

void PresentationExtension::OnRuntimeAdded(Runtime* runtime) {
  // A Runtime instance won't be added twice.
  DCHECK(!ContainsKey(creator_map_, runtime));
  DLOG(INFO) << "On new runtime added";
  linked_ptr<PresentationCreatorImpl> impl(
      new PresentationCreatorImpl(runtime->web_contents()));
  creator_map_.insert(
      std::pair<Runtime*, linked_ptr<PresentationCreatorImpl> >(runtime, impl));
}

void PresentationExtension::OnRuntimeRemoved(Runtime* runtime) {
  PresentationCreatorMap::iterator it = creator_map_.find(runtime);
  if (it != creator_map_.end())
    creator_map_.erase(it);
}

void PresentationExtension::OnRuntimeAppIconChanged(Runtime* runtime) {
}

XWalkExtensionInstance* PresentationExtension::CreateInstance() {
  DLOG(INFO) << "## new presentation instance is created";
  return new PresentationInstance(this);
}

//////////////////////////////////////////////////////////////////
//  PresentationInstance
//////////////////////////////////////////////////////////////////
PresentationInstance::PresentationInstance(PresentationExtension* extension)
    : XWalkInternalExtensionInstance(),
      extension_(extension) {
  InitializeDisplayManagerOnUI();
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

void PresentationInstance::InitializeDisplayManagerOnUI() {
  if (!BrowserThread::CurrentlyOn(BrowserThread::UI)) {
    BrowserThread::PostTask(
      BrowserThread::UI, FROM_HERE,
      base::Bind(&PresentationInstance::InitializeDisplayManagerOnUI,
          base::Unretained(this)));
    return;
  }

  PresentationDisplayManager::Get()->EnsureInitialized();
}

}  // namespace experimental
}  // namespace xwalk

