// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXPERIMENTAL_PRESENTATION_PRESENTATION_DELEGATE_H
#define XWALK_EXPERIMENTAL_PRESENTATION_PRESENTATION_DELEGATE_H

namespace xwalk {
namespace experimental {

class PresentationImpl;

class PresentationDelegate {
 public:
  virtual ~PresentationDelegate() {}

  virtual void OnPresentationDestroyed(PresentationImpl* presentation) = 0;
};

}  // namespace experimental
}  // namespace xwalk

#endif  // XWALK_EXPERIMENTAL_PRESENTATION_PRESENTATION_DELEGATE_H


