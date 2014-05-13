// Copyright (c) 2014 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.xwalk.core.sample;

import org.xwalk.core.XWalkView;
import org.xwalk.core.XWalkPreferences;

import android.animation.Animator;
import android.animation.AnimatorListenerAdapter;
import android.animation.AnimatorSet;
import android.animation.ObjectAnimator;
import android.app.Activity;
import android.os.Bundle;
import android.widget.Button;
import android.widget.LinearLayout;
import android.view.View;

/**
 * Sample code to show how to use ANIMATED_XWALK_VIEW preference key to create
 * animated XWalkView and apply alpha animation or scale animation on it.
 */
public class AnimatableXWalkViewActivity extends Activity {
    private final static float ANIMATION_FACTOR = 0.6f;
    private Button mRunAnimationButton;

    private XWalkView mWebView;

    private void startAnimation() {
        AnimatorSet combo = new AnimatorSet();

        float targetAlpha = mWebView.getAlpha() == 1.f ? ANIMATION_FACTOR : 1.f;
        float targetScaleFactor = mWebView.getScaleX() == 1.f ? ANIMATION_FACTOR : 1.f;

        ObjectAnimator fade = ObjectAnimator.ofFloat(mWebView,
                "alpha", mWebView.getAlpha(), targetAlpha);
        ObjectAnimator scaleX = ObjectAnimator.ofFloat(mWebView,
                "scaleX", mWebView.getScaleX(), targetScaleFactor);
        ObjectAnimator scaleY = ObjectAnimator.ofFloat(mWebView,
                "scaleY", mWebView.getScaleY(), targetScaleFactor);

        combo.setDuration(400);
        combo.playTogether(fade, scaleX, scaleY);
        combo.start();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // ANIMATABLE_XWALK_VIEW preference key MUST be set before XWalkView creation.
        XWalkPreferences.setValue(XWalkPreferences.ANIMATABLE_XWALK_VIEW, true);

        setContentView(R.layout.animatable_xwview_layout);

        mRunAnimationButton = (Button) findViewById(R.id.run_animation);
        mRunAnimationButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                startAnimation();
            }
        });

        mWebView = (XWalkView) findViewById(R.id.xwalkview);
        mWebView.load("http://www.baidu.com", null);
    }

    @Override
    protected void onPause() {
        super.onPause();
        mWebView.onHide();
    }

    @Override
    protected void onResume() {
        super.onResume();
        mWebView.onShow();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        mWebView.onDestroy();
    }
}
