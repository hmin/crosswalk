// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/experimental/presentation/presentation_impl.h"

#include "base/bind.h"
#include "base/bind_helpers.h"
#include "base/logging.h"
#include "base/message_loop.h"
#include "content/public/browser/web_contents.h"
#include "xwalk/experimental/presentation/presentation_delegate.h"

using content::WebContents;

namespace xwalk {
namespace experimental {

// static
PresentationImpl* PresentationImpl::Create(WebContents* web_contents) {
  return new PresentationImpl(web_contents);
}

PresentationImpl::PresentationImpl(WebContents* web_contents)
  : web_contents_(web_contents),
    delegate_(NULL),
    window_(NULL) {
  web_contents_->SetDelegate(this);
  PlatformCreatePresentation();
}

PresentationImpl::~PresentationImpl() {
  DLOG(INFO) << "dtor";
  if (delegate_)
    delegate_->OnPresentationDestroyed(this);
}

void PresentationImpl::Show() {
  PlatformShowPresentation();
}

void PresentationImpl::Close() {
  PlatformClosePresentation();
}

void PresentationImpl::OnWindowDestroyed() {
  base::MessageLoop::current()->PostTask(
      FROM_HERE, base::Bind(&base::DeletePointer<PresentationImpl>, this));
}

void PresentationImpl::OnDisplayError() {
  PlatformClosePresentation();
}

void PresentationImpl::CloseContents(WebContents* source) {
  DCHECK(source == web_contents_);
  DLOG(INFO) << "close contents";
  PlatformClosePresentation();
}

}  // namespace experimental
}  // namespace xwalk
