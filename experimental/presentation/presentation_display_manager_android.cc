// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/experimental/presentation/presentation_display_manager.h"

#include "jni/PresentationDisplayManager_jni.h"

namespace xwalk {
namespace experimental {

bool RegisterPresentationDisplayManager(JNI* env) {
  return RegisterNativesImpl(env);
}

void PresentationDisplayManager::EnsureInitialized() {
  if (initialized_)
    return;
  initialized_ = true;
}

static jint initPresentationDisplayManager(JNIEnv* env, jobject* obj) {
  PresentationDisplayManager* display_manager = new PresentationDisplayManager();
  display_manager->EnsureInitialized();
  return reinterpret_cast<jint>(display_manager);
}

static void addSecondaryDisplay(JNIEnv* env,
                                jobject* obj,
                                jint display_id,
                                jstring display_name) {
}

static void removeSecondaryDisplay(JNIEnv* env,
                                   jobject* obj,
                                   jint display_id) {
}

}  // namespace xwalk
}  // namespace experimental

