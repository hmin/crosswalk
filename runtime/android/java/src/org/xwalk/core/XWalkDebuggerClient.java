// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

/**
 * The debugger client interface which is responsible for handling the
 * response message from the debugger agent.
 */
public interface XWalkDebuggerClient {
    public void handleMessage(String message);
}
