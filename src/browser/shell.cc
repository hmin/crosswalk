// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/src/browser/shell.h"

#include <string>
#include <utility>

#include "base/auto_reset.h"
#include "base/command_line.h"
#include "base/message_loop.h"
#include "base/path_service.h"
#include "base/stringprintf.h"
#include "base/string_number_conversions.h"
#include "base/string_util.h"
#include "base/utf_string_conversions.h"
#include "cameo/src/browser/shell_browser_main_parts.h"
#include "cameo/src/browser/shell_content_browser_client.h"
#include "cameo/src/browser/shell_devtools_frontend.h"
#include "cameo/src/browser/shell_javascript_dialog_manager.h"
#include "cameo/src/browser/shell_registry.h"
#include "cameo/src/common/shell_switches.h"
#include "content/public/browser/devtools_manager.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/notification_details.h"
#include "content/public/browser/notification_source.h"
#include "content/public/browser/notification_types.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"
#include "content/public/common/renderer_preferences.h"

// Content area size for newly created windows.
static const int kDefaultWindowWidth = 800;
static const int kDefaultWindowHeight = 600;

namespace cameo {

Shell::Shell(content::WebContents* web_contents)
    : devtools_frontend_(NULL),
      is_fullscreen_(false),
      is_devtools_(false),
      window_(NULL),
      url_edit_view_(NULL),
#if defined(OS_WIN) && !defined(USE_AURA)
      default_edit_wnd_proc_(0),
#endif
      headless_(false) {
  const CommandLine& command_line = *CommandLine::ForCurrentProcess();
  if (command_line.HasSwitch(switches::kHeadless)) {
    headless_ = true;
  }

  registrar_.Add(
      this,
      content::NOTIFICATION_WEB_CONTENTS_TITLE_UPDATED,
      content::Source<content::WebContents>(web_contents));

  ShellRegistry::Get()->AddShell(this);
}

Shell::~Shell() {
  PlatformCleanUp();

  ShellRegistry::Get()->RemoveShell(this);

  if (ShellRegistry::Get()->shells().empty())
    MessageLoop::current()->PostTask(FROM_HERE, MessageLoop::QuitClosure());
}

Shell* Shell::CreateShell(content::WebContents* web_contents) {
  Shell* shell = new Shell(web_contents);

  shell->web_contents_.reset(web_contents);
  web_contents->SetDelegate(shell);

  shell->PlatformCreateWindow(kDefaultWindowWidth, kDefaultWindowHeight);

  shell->PlatformSetContents();

  shell->PlatformResizeSubViews();

  return shell;
}

// static
void Shell::Initialize() {
  PlatformInitialize(gfx::Size(kDefaultWindowWidth, kDefaultWindowHeight));
}

Shell* Shell::CreateNewWindow(content::BrowserContext* browser_context,
                              const GURL& url,
                              content::SiteInstance* site_instance,
                              int routing_id,
                              const gfx::Size& initial_size) {
  content::WebContents::CreateParams create_params(
      browser_context, site_instance);
  create_params.routing_id = routing_id;
  if (!initial_size.IsEmpty())
    create_params.initial_size = initial_size;
  else
    create_params.initial_size =
        gfx::Size(kDefaultWindowWidth, kDefaultWindowHeight);
  content::WebContents* web_contents =
      content::WebContents::Create(create_params);
  Shell* shell = CreateShell(web_contents);
  if (!url.is_empty())
    shell->LoadURL(url);
  return shell;
}

void Shell::LoadURL(const GURL& url) {
  LoadURLForFrame(url, std::string());
}

void Shell::LoadURLForFrame(const GURL& url, const std::string& frame_name) {
  content::NavigationController::LoadURLParams params(url);
  params.transition_type = content::PageTransitionFromInt(
      content::PAGE_TRANSITION_TYPED |
      content::PAGE_TRANSITION_FROM_ADDRESS_BAR);
  params.frame_name = frame_name;
  web_contents_->GetController().LoadURLWithParams(params);
  web_contents_->GetView()->Focus();
}

void Shell::GoBackOrForward(int offset) {
  web_contents_->GetController().GoToOffset(offset);
  web_contents_->GetView()->Focus();
}

void Shell::Reload() {
  web_contents_->GetController().Reload(false);
  web_contents_->GetView()->Focus();
}

void Shell::Stop() {
  web_contents_->Stop();
  web_contents_->GetView()->Focus();
}

void Shell::UpdateNavigationControls() {
  int current_index = web_contents_->GetController().GetCurrentEntryIndex();
  int max_index = web_contents_->GetController().GetEntryCount() - 1;

  PlatformEnableUIControl(BACK_BUTTON, current_index > 0);
  PlatformEnableUIControl(FORWARD_BUTTON, current_index < max_index);
  PlatformEnableUIControl(STOP_BUTTON, web_contents_->IsLoading());
}

void Shell::ShowDevTools() {
  if (devtools_frontend_) {
    devtools_frontend_->Focus();
    return;
  }
  devtools_frontend_ = ShellDevToolsFrontend::Show(web_contents());
  devtools_frontend_->set_inspected_shell(this);
}

void Shell::CloseDevTools() {
  if (!devtools_frontend_)
    return;
  devtools_frontend_->Close();
  devtools_frontend_ = NULL;
}

gfx::NativeView Shell::GetContentView() {
  if (!web_contents_.get())
    return NULL;
  return web_contents_->GetView()->GetNativeView();
}

content::WebContents* Shell::OpenURLFromTab(
    content::WebContents* source, const content::OpenURLParams& params) {
  // The only one we implement for now.
  DCHECK(params.disposition == CURRENT_TAB);
  source->GetController().LoadURL(
      params.url, params.referrer, params.transition, std::string());
  return source;
}

void Shell::LoadingStateChanged(content::WebContents* source) {
  UpdateNavigationControls();
  PlatformSetIsLoading(source->IsLoading());
}

void Shell::ToggleFullscreenModeForTab(content::WebContents* web_contents,
                                       bool enter_fullscreen) {
#if defined(OS_ANDROID)
  PlatformToggleFullscreenModeForTab(web_contents, enter_fullscreen);
#endif
  if (is_fullscreen_ != enter_fullscreen) {
    is_fullscreen_ = enter_fullscreen;
    web_contents->GetRenderViewHost()->WasResized();
  }
}

bool Shell::IsFullscreenForTabOrPending(
    const content::WebContents* web_contents) const {
#if defined(OS_ANDROID)
  return PlatformIsFullscreenForTabOrPending(web_contents);
#else
  return is_fullscreen_;
#endif
}

void Shell::RequestToLockMouse(content::WebContents* web_contents,
                               bool user_gesture,
                               bool last_unlocked_by_target) {
  web_contents->GotResponseToLockMouseRequest(true);
}

void Shell::CloseContents(content::WebContents* source) {
  Close();
}

bool Shell::CanOverscrollContent() const {
  return false;
}

void Shell::WebContentsCreated(
    content::WebContents* source_contents,
    int64 source_frame_id,
    const string16& frame_name,
    const GURL& target_url,
    content::WebContents* new_contents) {
  CreateShell(new_contents);
}

void Shell::DidNavigateMainFramePostCommit(content::WebContents* web_contents) {
  PlatformSetAddressBarURL(web_contents->GetURL());
}

content::JavaScriptDialogManager* Shell::GetJavaScriptDialogManager() {
  if (!dialog_manager_.get())
    dialog_manager_.reset(new ShellJavaScriptDialogManager());
  return dialog_manager_.get();
}

void Shell::ActivateContents(content::WebContents* contents) {
  contents->GetRenderViewHost()->Focus();
}

void Shell::DeactivateContents(content::WebContents* contents) {
  contents->GetRenderViewHost()->Blur();
}

void Shell::Observe(int type,
                    const content::NotificationSource& source,
                    const content::NotificationDetails& details) {
  if (type == content::NOTIFICATION_WEB_CONTENTS_TITLE_UPDATED) {
    std::pair<content::NavigationEntry*, bool>* title =
        content::Details<std::pair<content::NavigationEntry*, bool> >(
            details).ptr();

    if (title->first) {
      string16 text = title->first->GetTitle();
      PlatformSetTitle(text);
    }
  }
}

}  // namespace cameo