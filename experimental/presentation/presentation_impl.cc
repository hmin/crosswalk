// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/experimental/presentation/presentation_impl.h"

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/logging.h"
#include "base/message_loop.h"
#include "content/public/browser/web_contents.h"
#include "ui/gfx/display.h"
#include "ui/gfx/screen.h"

#include "xwalk/experimental/presentation/presentation_delegate.h"

using content::WebContents;

namespace xwalk {
namespace experimental {

// static
PresentationImpl* PresentationImpl::Create(WebContents* web_contents,
                                           int64 display_id) {
  return new PresentationImpl(web_contents, display_id);
}

PresentationImpl::PresentationImpl(WebContents* web_contents,
                                   int64 display_id)
  : web_contents_(web_contents),
    delegate_(NULL),
    window_(NULL),
    display_id_(display_id) {
  web_contents_->SetDelegate(this);
}

PresentationImpl::~PresentationImpl() {
  DLOG(INFO) << "destroy presentation.";
  if (delegate_)
    delegate_->OnPresentationDestroyed(this);
}

void PresentationImpl::Show() {
#if !defined(OS_ANDROID)
  if (!window_) {
    gfx::Screen* screen = gfx::Screen::GetNativeScreen();
    if (display_id_ != screen->GetPrimaryDisplay().id()) {
      // TODO: Implement to create window on the secondary display.
      NOTIMPLEMENTED();
      return;
    }

    DCHECK(web_contents_);

    NativeAppWindow::CreateParams params;
    params.delegate = this;
    params.web_contents = web_contents_.get();
    params.bounds = gfx::Rect(0, 0, 640, 480);

    window_ = NativeAppWindow::Create(params);
  }

  window_->Show();
#else
  NOTIMPLEMENTED();
#endif  // OS_ANDROID
}

void PresentationImpl::Close() {
#if !defined(OS_ANDROID)
  window_->Close();
#else
  NOTIMPLEMENTED();
#endif
}

void PresentationImpl::OnWindowDestroyed() {
  base::MessageLoop::current()->PostTask(
      FROM_HERE, base::Bind(&base::DeletePointer<PresentationImpl>, this));
}

void PresentationImpl::OnDisplayError() {
  Close();
}

void PresentationImpl::CloseContents(WebContents* source) {
  DCHECK(source == web_contents_);
  Close();
}

}  // namespace experimental
}  // namespace xwalk
