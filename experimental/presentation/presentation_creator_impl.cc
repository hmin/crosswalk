// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/experimental/presentation/presentation_creator_impl.h"

#include "content/common/view_messages.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/site_instance.h"
#include "ui/gfx/display.h"
#include "xwalk/experimental/presentation/presentation_impl.h"
#include "xwalk/extensions/common/xwalk_extension_messages.h"

using content::WebContents;

namespace xwalk {
namespace experimental {

PresentationCreatorImpl::PresentationCreatorImpl(WebContents* web_contents)
  : WebContentsObserver(web_contents) {
}

PresentationCreatorImpl::~PresentationCreatorImpl() {
  PresentationList::iterator it = presentation_list_.begin();
  for (;it != presentation_list_.end(); ++it) {
    PresentationImpl* impl = *it;
    // Must reset the delegate of each PresentationImpl since we are in the
    // destructor of PresentationDelegate here.
    impl->set_delegate(NULL);
    impl->Close();
  }

  presentation_list_.clear();
}

bool PresentationCreatorImpl::OnMessageReceived(const IPC::Message& message) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(PresentationCreatorImpl, message)
    IPC_MESSAGE_HANDLER(XWalkViewHostMsg_RequestShowPresentation,
                        OnRequestShowPresentation)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void PresentationCreatorImpl::OnRequestShowPresentation(
    int request_id, int opener_id, const std::string& url) {
  DLOG(INFO) << "Request id: " << request_id << " opener id: " << opener_id
             << " target url" << url;
}

void PresentationCreatorImpl::CreateNewPresentation(WebContents* web_contents) {
  PresentationImpl* impl = PresentationImpl::Create(web_contents);
  impl->set_delegate(this);
  presentation_list_.push_back(impl);

  impl->Show();
}

bool PresentationCreatorImpl::CanCreatePresentation(const GURL& url) {
  // TODO(hmin): Need to figure out the policy of permission control.
  // A presentation is allowed to show if:
  // o There is at least one available external display for use, and
  // o User grant the permission of showing presentation on the external display.
  return true;
}

void PresentationCreatorImpl::OnPresentationDestroyed(PresentationImpl* impl) {
  PresentationList::iterator it = presentation_list_.begin();
  for (;it != presentation_list_.end(); ++it)
    if (*it == impl) break;

  if (it != presentation_list_.end()) {
//    PresentationImpl* impl = *it;

//    int presentation_routing_id = impl->web_contents()->GetRoutingID();
//    Send(new ViewMsg_ClosePresentation(routing_id(), presentation_routing_id));
//
    presentation_list_.erase(it);
  }
}

}  // namespace experimental
}  // namespace xwalk
