// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.runtime.extension.api.presentation;

import android.content.Context;
import android.content.DialogInterface;
import android.hardware.display.DisplayManager;
import android.util.Log;
import android.util.JsonReader;
import android.util.JsonWriter;
import android.util.SparseArray;
import android.view.Display;
import java.io.IOException;
import java.io.StringReader;
import java.io.StringWriter;
import java.util.ArrayList;
import org.chromium.base.ThreadUtils;
import org.xwalk.core.XWalkPresentationView;
import org.xwalk.runtime.extension.XWalkExtension;
import org.xwalk.runtime.extension.XWalkExtensionContext;

/**
 * A XWalk extension for Presentation API spec implementation on Android. It is
 * placed under experimental namespace since the spec is still in editor draft
 * and might be changed in future after a wider discussion.
 *
 * Presentation API spec: http://otcshare.github.io/presentation-spec/index.html
 */
public class PresentationExtension extends XWalkExtension {
    public final static String TAG = "PresentationExtension";
    public final static String JS_API_PATH = "js_api/presentation_api.js";
    public final static String NAME = "navigator.experimental.presentation";

    // Tag names:
    private final static String TAG_CMD = "cmd";
    private final static String TAG_DATA = "data";
    private final static String TAG_REQUEST_ID = "requestId";

    // Command messages:
    private final static String CMD_DISPLAY_AVAILABLE_CHANGE = "DisplayAvailableChange";
    private final static String CMD_QUERY_DISPLAY_AVAILABILITY = "QueryDisplayAvailability";
    private final static String CMD_REQUEST_SHOW = "RequestShow";
    private final static String CMD_SHOW_FAILED = "ShowFailed";
    private final static String CMD_SHOW_SUCCEEDED = "ShowSucceeded";

    // Error messages:
    private final static String ERROR_INVALID_ACCESS = "InvalidAccessError";
    private final static String ERROR_INVALID_STATE = "InvalidStateError";
    private final static String ERROR_NOT_FOUND = "NotFoundError";

    private DisplayManager mDisplayManager;

    // Holds all available presentation displays connected to the system.
    private final SparseArray<Display> mDisplayList = new SparseArray<Display>();

    // The pending request for showing presentation.
    private int mPendingRequestId;

    // The only presentation is currently showing on the display.
    private XWalkPresentationView mPresentationView;

    /*
     * Listens for displays to be added, removed or changed.
     * We use it to check the presentation display availability and notify the availability
     * listeners if it is changed. The presentation display become available when the first
     * secondardy display is added, and unavailable when the last secondary display is removed.
     */
    private final DisplayManager.DisplayListener mDisplayListener =
            new DisplayManager.DisplayListener() {
        @Override
        public void onDisplayAdded(int displayId) {
            Display display = mDisplayManager.getDisplay(displayId);
            addSecondaryDisplay(displayId, display);
        }

        @Override
        public void onDisplayRemoved(int displayId) {
            removeSecondaryDisplay(displayId);
        }

        @Override
        public void onDisplayChanged(int displayId) {
            // TODO(hmin): Figure out the behaviour when the display is changed.
        }
    };

    public PresentationExtension(String name, String jsApi, XWalkExtensionContext context) {
        super(name, jsApi, context);
        mPendingRequestId = -1;
        mPresentationView = null;
        mDisplayManager = (DisplayManager) mExtensionContext.getContext().getSystemService(Context.DISPLAY_SERVICE);
    }

    private String getDisplayCategory() {
        return DisplayManager.DISPLAY_CATEGORY_PRESENTATION;
    }

    private void notifyAvailabilityChanged(boolean isAvailable) {
        StringWriter contents = new StringWriter();
        JsonWriter writer = new JsonWriter(contents);

        try {
            writer.beginObject();
            writer.name(TAG_CMD).value(CMD_DISPLAY_AVAILABLE_CHANGE);
            writer.name(TAG_DATA).value(isAvailable);
            writer.endObject();

            broadcastMessage(contents.toString());
        } catch (IOException e) {
            Log.e(TAG, "Error: " + e.toString());
        }
    }

    private void notifyRequestShowSucceed(int instanceId, int requestId, int presentationId) {
        StringWriter contents = new StringWriter();
        JsonWriter writer = new JsonWriter(contents);

        try {
            writer.beginObject();
            writer.name(TAG_CMD).value(CMD_SHOW_SUCCEEDED);
            writer.name(TAG_REQUEST_ID).value(requestId);
            writer.name(TAG_DATA).value(presentationId);
            writer.endObject();

            postMessage(instanceId, contents.toString());
        } catch (IOException e) {
            Log.e(TAG, "Error: " + e.toString());
        }
    }

    private void notifyRequestShowFail(int instanceId, int requestId, String errorMessage) {
        StringWriter contents = new StringWriter();
        JsonWriter writer = new JsonWriter(contents);

        try {
            writer.beginObject();
            writer.name(TAG_CMD).value(CMD_SHOW_FAILED);
            writer.name(TAG_REQUEST_ID).value(requestId);
            writer.name(TAG_DATA).value(errorMessage);
            writer.endObject();

            postMessage(instanceId, contents.toString());
        } catch (IOException e) {
            Log.e(TAG, "Error: " + e.toString());
        }
    }

    public void addSecondaryDisplay(int displayId, Display display) {
        mDisplayList.put(displayId, display);
        if (mDisplayList.size() == 1)
            notifyAvailabilityChanged(true);
    }

    public void removeSecondaryDisplay(int displayId) {
        mDisplayList.remove(displayId);
        if (mDisplayList.size() == 0)
            notifyAvailabilityChanged(false);
    }

    @Override
    public void onMessage(int instanceId, String message) {
        StringReader contents = new StringReader(message);
        JsonReader reader = new JsonReader(contents);

        int requestId = -1;
        String cmd = null;
        String url = null;
        try {
            reader.beginObject();
            while (reader.hasNext()) {
                String name = reader.nextName();
                if (name.equals(TAG_CMD)) {
                    cmd = reader.nextString();
                } else if (name.equals(TAG_REQUEST_ID)) {
                    requestId = reader.nextInt();
                } else if (name.equals("url")) {
                    url = reader.nextString();
                } else {
                    reader.skipValue();
                }
            }
            reader.endObject();
            if (cmd != null && cmd.equals(CMD_REQUEST_SHOW) && requestId >= 0)
                handleRequestShow(instanceId, url, requestId);
            else
                Log.d(TAG, "Unknown command: " + cmd);
        } catch (IOException e) {
            Log.d(TAG, "Error: " + e);
        }
    }

    private void handleRequestShow(final int instanceId, String url, int requestId) {
        final String targetUrl;
        if (url.startsWith("http://") || url.startsWith("https://"))
            targetUrl = url;
        else
            targetUrl = "file:///android_asset/" + url;

        final int pendingRequest = requestId;

        if (mDisplayList.size() == 0) {
            Log.d(TAG, "No available display");
            notifyRequestShowFail(instanceId, requestId, ERROR_NOT_FOUND);
        } else {
            final Display selectedDisplay = mDisplayList.valueAt(0);
            ThreadUtils.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    // Only allow one Presentation to be created on the display.
                    if (mPresentationView != null) {
                        notifyRequestShowFail(instanceId, pendingRequest, ERROR_INVALID_ACCESS);
                        return;
                    }

                    if (!selectedDisplay.isValid()) {
                        notifyRequestShowFail(instanceId, pendingRequest, ERROR_INVALID_STATE);
                        return;
                    }

                    mPresentationView = new XWalkPresentationView(mExtensionContext.getContext(),
                        mExtensionContext.getActivity(), selectedDisplay, targetUrl);

                    mPresentationView.setDelegate(new XWalkPresentationView.Delegate() {
                        @Override
                        public void onLoadingFinished(String url, int routingId) {
                            notifyRequestShowSucceed(instanceId, pendingRequest, routingId);
                        }
                    });

                    mPresentationView.setOnDismissListener(new DialogInterface.OnDismissListener() {
                        @Override
                        public void onDismiss(DialogInterface dialog) {
                            if (dialog == mPresentationView)
                                mPresentationView = null;
                        }
                    });
                    mPresentationView.show();
                }
            });
        }
    }

    @Override
    public String onSyncMessage(int instanceId, String message) {
        if (message.equals(CMD_QUERY_DISPLAY_AVAILABILITY)) {
            return mDisplayList.size() != 0 ? "true" : "false";
        } else {
            Log.e(TAG, "Unexpected sync message received: " + message);
            return "";
        }
    }

    @Override
    public void onResume() {
        // With DISPLAY_CATEGORY_PRESENTATION category, the first display in
        // the returned display array is the most preferred presentation display
        // sorted by display manager.
        Display[] displays = mDisplayManager.getDisplays(getDisplayCategory());

        // We should fire an event for the fact the display is unavailable now
        // if the cached display list is not empty.
        if (displays.length == 0 && mDisplayList.size() > 0)
            notifyAvailabilityChanged(false);

        mDisplayList.clear();

        for (Display d : displays)
            addSecondaryDisplay(d.getDisplayId(), d);

        // Register the listener to display manager
        mDisplayManager.registerDisplayListener(mDisplayListener, null);
    }

    @Override
    public void onPause() {
        Log.d(TAG, "onPause is called");
        mDisplayManager.unregisterDisplayListener(mDisplayListener);
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy is called");
    }
}
