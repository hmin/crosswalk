// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/experimental/presentation/presentation_impl.h"

#include <gtk/gtk.h>

#include "content/public/browser/web_contents.h"
#include "content/public/browser/web_contents_view.h"

namespace xwalk {
namespace experimental {

namespace {

// The dimension is just used for testing purpose.
int kDefaultWidth = 800;
int kDefaultHeight = 600;

gboolean OnWindowDestroyedCallback(GtkWidget* widget, gpointer userdata) {
  DLOG(INFO) << "### window destroyed";
  PresentationImpl* impl = static_cast<PresentationImpl*>(userdata);
  impl->OnWindowDestroyed();
  return FALSE;
}  // namespace

}

void PresentationImpl::PlatformCreatePresentation() {
  if (window_)
    return;

  if (!display_id_.empty()) {
    NOTIMPLEMENTED();
    return;
  }

  DCHECK(web_contents_);

  window_ = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));

  gtk_window_set_title(window_, "Presentation Window");
  g_signal_connect(G_OBJECT(window_), "destroy",
                   G_CALLBACK(OnWindowDestroyedCallback), this);

  gtk_window_set_default_size(window_, kDefaultWidth, kDefaultHeight);
  gtk_widget_set_size_request(
      web_contents_->GetView()->GetNativeView(),
      kDefaultWidth, kDefaultHeight);

}

void PresentationImpl::PlatformShowPresentation() {
  // Attach to the web contents.
  gfx::NativeView native_view = web_contents_->GetView()->GetNativeView();
  gtk_widget_show(native_view);
  gtk_container_add(GTK_CONTAINER(window_), native_view);
  gtk_widget_show_all(GTK_WIDGET(window_));
}

void PresentationImpl::PlatformClosePresentation() {
  if (window_)
    gtk_widget_destroy(GTK_WIDGET(window_));
  window_ = NULL;
}

}  // namespace experimental

}  // namespace xwalk
