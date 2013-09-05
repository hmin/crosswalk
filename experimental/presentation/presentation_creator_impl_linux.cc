// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/presentation/presentation_creator_impl.h"

#include <gtk/gtk.h>

#include "content/public/browser/web_contents_view.h"

namespace content {

namespace {

int kDefaultWidth = 800;
int kDefaultHeight = 600;

gboolean OnWindowDestroyed(GtkWidget* widget, gpointer userdata) {
  PresentationCreator* creator = static_cast<PresentationCreator*>(userdata);
  creator->ClosePresentationContents();
  return FALSE;
}

}

void PresentationCreatorImpl::PlatformCreatePresentationWindow() {
  if (presentation_web_contents_) {
    gtk_widget_set_size_request(
        presentation_web_contents_->GetView()->GetNativeView(),
        kDefaultWidth, kDefaultHeight);
  }

  presentation_window_ = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
  gtk_window_set_title(presentation_window_, "Presentation Window");
  g_signal_connect(G_OBJECT(presentation_window_), "destroy",
                   G_CALLBACK(OnWindowDestroyed), this);

  gtk_window_set_default_size(presentation_window_,
      kDefaultWidth, kDefaultHeight);
  gtk_widget_show(GTK_WIDGET(presentation_window_));
}

void PresentationCreatorImpl::PlatformAttachWebContentsToWindow() {
  WebContentsView* content_view = presentation_web_contents_->GetView();
  gtk_widget_show(content_view->GetNativeView());
  gtk_container_add(GTK_CONTAINER(presentation_window_),
                    content_view->GetNativeView());
}

void PresentationCreatorImpl::PlatformClosePresentationWindow() {
  if (presentation_window_)
    gtk_widget_destroy(GTK_WIDGET(presentation_window_));
  presentation_window_ = NULL;
}

}  // namespace
