// Copyright 2013 Intel Corp.
//
// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
#ifndef CAMEO_SRC_COMMON_CAMEO_NOTIFICATION_TYPES_H_
#define CAMEO_SRC_COMMON_CAMEO_NOTIFICATION_TYPES_H_

#include "build/build_config.h"
#include "content/public/browser/notification_types.h"

namespace cameo {

enum NotificationType {
  NOTIFICATION_CAMEO_START = content::NOTIFICATION_CONTENT_END,

  // Notify that a new Shell instance is created. The source is a
  // Source<Shell> containing the affected Shell. No details is provided.
  NOTIFICATION_SHELL_OPENED = NOTIFICATION_CAMEO_START,

  // Notify that a Shell instance is close. The source is a Source<Shell>
  // containing the affected Shell. No details is provided.
  NOTIFICATION_SHELL_CLOSED,

  NOTIFICATION_CAMEO_END,
};

}  // namespace cameo

#endif  // CAMEO_SRC_COMMON_CAMEO_NOTIFICATION_TYPES_H_
