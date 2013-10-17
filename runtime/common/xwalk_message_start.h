// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XWALK_RUNTIME_COMMON_XWALK_MESSAGES_H_
#define XWALK_RUNTIME_COMMON_XWALK_MESSAGES_H_

#include "ipc/ipc_message_start.h"

// Used by IPC_BEGIN_MESSAGES so that each message class defined for xwalk
// starts from a unique base. It is used for IPC logging code to figure out
// the message class from its ID. The LastIPCMsgStart enum is defined in
// ipc/ipc_message_start.h.
enum {
  XWalkViewMsgStart = LastIPCMsgStart + 1,
  XWalkViewHostMsgStart,
  XWalkLastIPCMsgStart  // Must be the last one.
};

#endif



