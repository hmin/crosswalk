// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXPERIMENTAL_PRESENTATION_PRESENTATION_IMPL_H
#define XWALK_EXPERIMENTAL_PRESENTATION_PRESENTATION_IMPL_H

#include "base/memory/scoped_ptr.h"
#include "content/public/browser/web_contents_delegate.h"
#include "ui/gfx/native_widget_types.h"

namespace content {
class WebContents;
}

namespace xwalk {
namespace experimental {

class PresentationDelegate;

// Represents a presentation contents showing on the target display. It
// associates with a WebContents and a native window.
//
// A Presentation window is a special window. It is frameless and always in
// fullscreen mode. The native window will be closed if one of the following
// conditions is satisfied:
//   o By user interaction (e.g. Alt+F4), or
//   o Web contents is closed (e.g. window.close), or
//   o The opener web contents is closed, or
//   o An error occurs on the target display (e.g. the display is disconnected).
//
// When the native window gets destroyed, the Presentation will be also deleted,
// so the client code won't need to delete it by manual.
class PresentationImpl : public content::WebContentsDelegate {
 public:
  static PresentationImpl* Create(content::WebContents* source,
                                  int64 display_id);
  ~PresentationImpl();

  content::WebContents* web_contents() const { return web_contents_.get(); }

  int64 display_id() const { return display_id_; }
  void set_display_id(int64 id) { display_id_ = id; }

  void set_delegate(PresentationDelegate* delegate) { delegate_ = delegate; }

  // Show the presentation on the target display.
  void Show();

  // Close the presentation. Note that Presentation object will be destroyed
  // once the Close is called. An attempt to reference it after Close method
  // is called would cause unpredicatable behaviour!.
  void Close();

  // Get called when the native window is destroyed.
  void OnWindowDestroyed();

  // Get called when an error occurs on the display used for showing this
  // presentation.
  void OnDisplayError();

 private:
  explicit PresentationImpl(content::WebContents* web_contents, int64 display_id);

  // WebContentsDelegate impl.
  virtual void CloseContents(content::WebContents* source) OVERRIDE;

  // The below functions are implemented in platform specific file.
  void PlatformCreatePresentation();
  void PlatformShowPresentation();
  void PlatformClosePresentation();

  // The associated web contents of presentation to be showed.
  scoped_ptr<content::WebContents> web_contents_;

  // The delegate of Presentation. May be NULL.
  PresentationDelegate* delegate_;

  // The native window to hold the |web_contents_|.
  gfx::NativeWindow window_;

  // The display used for showing the presentation window.
  int64 display_id_;

  DISALLOW_COPY_AND_ASSIGN(PresentationImpl);
};

}  // namespace experimental
}  // namespace xwalk

#endif  // XWALK_EXPERIMENTAL_PRESENTATION_PRESENTATION_IMPL_H
