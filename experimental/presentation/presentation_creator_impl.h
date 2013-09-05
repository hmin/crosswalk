// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXPERIMENTAL_PRESENTATION_PRESENTATION_CREATOR_IMPL_H
#define XWALK_EXPERIMENTAL_PRESENTATION_PRESENTATION_CREATOR_IMPL_H

#include <map>
#include <vector>

#include "base/memory/scoped_ptr.h"
#include "content/public/browser/web_contents_observer.h"
#include "googleurl/src/gurl.h"
#include "xwalk/experimental/presentation/presentation_delegate.h"

namespace content {
class WebContents;
}

namespace xwalk {
namespace experimental {

class PresentationImpl;

// A presentation creator is responsible for the presentation creation requested
// from the web contents.
class PresentationCreatorImpl : public PresentationDelegate,
                           public content::WebContentsObserver {
 public:
  explicit PresentationCreatorImpl(content::WebContents* web_contents);
  virtual ~PresentationCreatorImpl();

 private:
  // Represent a request for showing presentation from renderer process.
  struct ShowRequest {
    // The request id assigned by the presentation creator, and is unqiue within
    // a RVH instance.
    int request_id;
    // The target URL of the presentation to be showed.
    GURL target_url;
    // The target display on which the presentation will be showed.
    std::string display_id;
  };

  // PresentationDelegate implementations.
  virtual void OnPresentationDestroyed(PresentationImpl* presentation) OVERRIDE;

  // WebContentsObserver implementations.
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;

  // PresentationCreator implementations.
  virtual bool CanCreatePresentation(const GURL& url);
  virtual void CreateNewPresentation(int request_id,
                                     int opener_id,
                                     const std::string& url);

  void OnRequestShowPresentation(int request_id,
                                 int opener_id,
                                 const std::string& url);

  typedef std::vector<PresentationImpl*> PresentationList;
  PresentationList presentation_list_;

  // The available displays connected to the system. Will be updated if the
  // display is added/removed.
  std::vector<std::string> available_displays_;

  DISALLOW_COPY_AND_ASSIGN(PresentationCreatorImpl);
};

}  // namespace experimental
}  // namespace xwalk

#endif  // XWALK_EXPERIMENTAL_PRESENTATION_PRESENTATION_CREATOR_IMPL_H
