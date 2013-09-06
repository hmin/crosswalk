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

#include "xwalk/experimental/presentation/presentation_display_manager.h"
#include "xwalk/experimental/presentation/presentation_impl.h"
#include "xwalk/extensions/common/xwalk_extension_messages.h"

using content::BrowserContext;
using content::NavigationController;
using content::SiteInstance;
using content::WebContents;

namespace xwalk {
namespace experimental {

// The request to show presentation is denied.
const char* kInvalidAccessError = "InvalidAccessError";
// There is no available secondary display for use.
const char* kNotFoundError = "NotFoundError";

bool PresentationCreatorImpl::display_available_ = false;

PresentationCreatorImpl::PresentationCreatorImpl(WebContents* web_contents)
  : WebContentsObserver(web_contents) {
  DLOG(INFO) << "## PresentationCreatorImpl ctor";
  PresentationDisplayManager::Get()->AddObserver(this);
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
  PresentationDisplayManager::Get()->RemoveObserver(this);
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

void PresentationCreatorImpl::OnDisplayBoundsChanged(
    const gfx::Display& display) {
}

void PresentationCreatorImpl::OnDisplayAdded(const gfx::Display& new_display) {
  if (!display_available_) {
    DLOG(INFO) << "display added";
    display_available_ = true;
    Send(new XWalkViewMsg_DisplayAvailableChange(routing_id(),
                                                 display_available_));
  }
}

void PresentationCreatorImpl::OnDisplayRemoved(const gfx::Display& old_display) {
  if (display_available_ &&
      PresentationDisplayManager::Get()->GetSecondaryDisplays().size() == 0) {
    display_available_ = false;
    Send(new XWalkViewMsg_DisplayAvailableChange(routing_id(),
                                                 display_available_));
  }
}


void PresentationCreatorImpl::OnRequestShowPresentation(
    int request_id, int opener_id, const std::string& url) {
  DLOG(INFO) << "Request id: " << request_id << " opener id: " << opener_id
             << " target url" << url;

  GURL target_url(url);
  // TODO(hmin): We might need to popup a permission dialog to ask if the
  // end-user grants the presentation showing right. Here we bypass it since
  // it is implemented in an runtime, not a browser.
  if (!CanCreatePresentation(target_url)) {
    Send(new XWalkViewMsg_ShowPresentationFailed(routing_id(),
                                                 request_id,
                                                 kInvalidAccessError));
    return;
  }

   std::vector<gfx::Display> displays =
      PresentationDisplayManager::Get()->GetSecondaryDisplays();
   if (displays.size() == 0) {
     Send(new XWalkViewMsg_ShowPresentationFailed(routing_id(),
                                                  request_id,
                                                  kNotFoundError));
     return;
   }

   // TODO(hmin): We should popup a display selection dialog to allow end-user
   // select the target display in case of there are more than one display
   // available for presentation. Here we always select the first secondary
   // display as target display for simplicity.
   int64 selected_display = displays[0].id();
   CreateNewPresentation(request_id, selected_display, url);
}

void PresentationCreatorImpl::CreateNewPresentation(
    int request_id, int64 display_id, const std::string& url) {
  GURL target_url(url);
  BrowserContext* browser_context = web_contents()->GetBrowserContext();

  // The site instance used to create the new web contents is the same as the
  // opener, such that the new wbe contents can be in the same process as
  // the opener if they are sharing the same origin.
  WebContents::CreateParams params(browser_context,
      web_contents()->GetSiteInstance());

  WebContents* new_contents = WebContents::Create(params);
  NavigationController::LoadURLParams load_params(target_url);
  new_contents->GetController().LoadURLWithParams(load_params);

  int view_id = MSG_ROUTING_NONE;
  if (new_contents->GetRenderViewHost()->GetProcess()->GetID() ==
      web_contents()->GetRenderViewHost()->GetProcess()->GetID())
    view_id = new_contents->GetRoutingID();

  PresentationImpl* impl = PresentationImpl::Create(new_contents, display_id);
  impl->set_delegate(this);
  impl->Show();
  presentation_list_.push_back(impl);

  Send(new XWalkViewMsg_ShowPresentationSucceeded(routing_id(),
                                                  request_id,
                                                  view_id));
}

bool PresentationCreatorImpl::CanCreatePresentation(const GURL& url) {
  return true;
}

void PresentationCreatorImpl::OnPresentationDestroyed(PresentationImpl* impl) {
  PresentationList::iterator it = presentation_list_.begin();
  for (;it != presentation_list_.end(); ++it)
    if (*it == impl) break;

  if (it != presentation_list_.end())
    presentation_list_.erase(it);
}

}  // namespace experimental
}  // namespace xwalk
