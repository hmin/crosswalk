// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_EXPERIMENTAL_PRESENTATION_PRESENTATION_DISPLAY_MANAGER_H
#define XWALK_EXPERIMENTAL_PRESENTATION_PRESENTATION_DISPLAY_MANAGER_H

#include <vector>

#include "base/observer_list.h"
#include "ui/gfx/display.h"
#include "ui/gfx/display_observer.h"

namespace xwalk {
namespace experimental {

class PresentationDisplayManager {
 public:
  static PresentationDisplayManager* Get();
  virtual ~PresentationDisplayManager();

  void AddSecondaryDisplay(const gfx::Display& display);
  void RemoveSecondaryDisplay(const gfx::Display& display);

  void AddObserver(gfx::DisplayObserver* observer);
  void RemoveObserver(gfx::DisplayObserver* observer);

  std::vector<gfx::Display> GetSecondaryDisplays() const {
    return secondary_displays_;
  }

  gfx::Display GetSecondaryDisplay(int display_id);

 private:
  PresentationDisplayManager();
  void Initialize();

  std::vector<gfx::Display> secondary_displays_;
  ObserverList<gfx::DisplayObserver> observers_;

  bool initialized_;
};

}  // namespace xwalk
}  // namespace experimental

#endif  // XWALK_EXPERIMENTAL_PRESENTATION_PRESENTATION_DISPLAY_MANAGER_H
