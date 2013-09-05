// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/experimental/presentation/presentation_creator_impl.h"

#include "content/common/view_messages.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/render_process_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/site_instance.h"
#include "content/public/browser/web_contents.h"
#include "ui/gfx/display.h"
#include "xwalk/experimental/presentation/presentation_impl.h"
#include "xwalk/extensions/common/xwalk_extension_messages.h"

using content::BrowserContext;
using content::NavigationController;
using content::SiteInstance;
using content::WebContents;

namespace xwalk {
namespace experimental {

PresentationCreatorImpl::PresentationCreatorImpl(WebContents* web_contents)
  : WebContentsObserver(web_contents) {
  DLOG(INFO) << "## PresentationCreatorImpl ctor";
}

PresentationCreatorImpl::~PresentationCreatorImpl() {
  DLOG(INFO) << "## PresentationCreatorImpl dtor";
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
  // TODO: permission checking here.
  CreateNewPresentation(request_id, opener_id, url);
}

void PresentationCreatorImpl::CreateNewPresentation(
    int request_id, int opener_id, const std::string& url) {
  GURL target_url(url);
  BrowserContext* browser_context = web_contents()->GetBrowserContext();
  WebContents::CreateParams params(browser_context,
      web_contents()->GetSiteInstance());

  WebContents* new_contents = WebContents::Create(params);

  NavigationController::LoadURLParams load_params(target_url);
  new_contents->GetController().LoadURLWithParams(load_params);

  int view_id = MSG_ROUTING_NONE;
  if (new_contents->GetRenderViewHost()->GetProcess()->GetID() ==
      web_contents()->GetRenderViewHost()->GetProcess()->GetID())
    view_id = new_contents->GetRoutingID();

  PresentationImpl* impl = PresentationImpl::Create(new_contents);
  impl->set_delegate(this);
  impl->Show();
  presentation_list_.push_back(impl);

  DLOG(INFO) << "New presentation: " << view_id;

  Send(new XWalkViewMsg_ShowPresentationSucceeded(routing_id(),
                                                  request_id,
                                                  view_id));
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
