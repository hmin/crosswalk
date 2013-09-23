// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core;

import android.app.Activity;
import android.app.Presentation;
import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.Display;

public class XWalkPresentationView extends Presentation {

    private final static String TAG = "XWalkPresentationView";
    private String mUrl;
    private XWalkView mContentView;
    private Context mContext;
    private Activity mActivity;
    private Delegate mDelegate = null;

    public XWalkPresentationView(Context context, Activity activity, Display display, String targetUrl) {
        super(context, display);
        mContext = context;
        mActivity = activity;
        mUrl = targetUrl;
    }

    public void setDelegate(Delegate delegate) {
        mDelegate = delegate;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mContentView = new XWalkView(mContext, mActivity);
        mContentView.setXWalkClient(new XWalkClient() {
            @Override
            public void onPageFinished(XWalkView view, String url) {
                if (mDelegate != null)
                    mDelegate.onLoadingFinished(url, view.getContentID());
            }

            @Override
            public void onCloseWindow(XWalkView view) {
                cancel();
            }
        });
        mContentView.loadUrl(mUrl);
        setContentView(mContentView);
    }

    public interface Delegate {
        public void onLoadingFinished(String url, int routingId);
    }
}
