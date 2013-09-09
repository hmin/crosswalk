// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.extension.api;

public class PresentationDisplayManager {
    private static PresentationDisplayManager gDisplayManager = null;

    private int mNativeImpl = 0;

    @SuppressWarnings("unused")
    @CalledByNative
    private static void CreatePresentationDisplayManager() {
        assert (gDisplayManager == null);
        gDisplayManager = new PresentationDisplayManager();
    }

    public PresentationDisplayManager() {
        mNativeImpl = nativeInitPresentationDisplayManager();
        // TODO: Query all display list.
    }

    private void addSecondaryDisplay(int displayId, String displayName) {
        nativeAddSecondaryDisplay(mNativeImpl, displayId, displayName);
    }

    private void removeSecondaryDisplay(int display_id) {
        nativeRemoveSecondaryDisplay(mNativeImpl, display_id);
    }

    private native void nativeInitPresentationDisplayManager();
    private native void nativeAddSecondaryDisplay(int mNativeImpl, int displayId, String displayName);
    private native void nativeRemoveSecondaryDisplay(int mNativeImpl, int displayId);

}


