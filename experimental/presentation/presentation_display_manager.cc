// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xwalk/experimental/presentation/presentation_display_manager.h"

#include "base/command_line.h"
#include "content/public/browser/browser_thread.h"
#include "ui/gfx/screen.h"

#if defined(TOOLKIT_GTK)
#include "gdk/gdkx.h"
#include "gtk/gtk.h"
#endif

using content::BrowserThread;

namespace xwalk {
namespace experimental {

// Use the primary display as presentation display. Mainly for testing.
const char* kUsePrimaryDisplay = "use-primary-display";

static PresentationDisplayManager* g_display_manager = NULL;

PresentationDisplayManager::PresentationDisplayManager()
    : initialized_(false),
      observers_(new ObserverListThreadSafe<Observer>()) {
  DCHECK(g_display_manager == NULL);
  g_display_manager = this;
}

PresentationDisplayManager::~PresentationDisplayManager() {
  DCHECK(g_display_manager);
  g_display_manager = NULL;
}

// static
PresentationDisplayManager* PresentationDisplayManager::Get() {
  if (!g_display_manager)
    new PresentationDisplayManager;
  return g_display_manager;
}

void PresentationDisplayManager::EnsureInitialized() {
  if (!BrowserThread::CurrentlyOn(BrowserThread::UI)) {
    BrowserThread::PostTask(BrowserThread::UI, FROM_HERE,
        base::Bind(&PresentationDisplayManager::EnsureInitialized,
                   base::Unretained(this)));
    return;
  }

  if (initialized_)
    return;

#if defined(TOOLKIT_GTK)
  CommandLine* command_line = CommandLine::ForCurrentProcess();
  gfx::Display primary_display =
      gfx::Screen::GetNativeScreen()->GetPrimaryDisplay();

  // We use the primary display id as the internal display id. An internal
  // display won't be used for showing presentation unless an explicit option
  // --use-primary-display is specified on the command line.
  gfx::Display::SetInternalDisplayId(primary_display.id());

  GdkScreen* screen = gdk_screen_get_default();
  gint num_of_displays = gdk_screen_get_n_monitors(screen);
  for (gint i = 0; i < num_of_displays; ++i) {
    gfx::Display display(i);
    // If using primary display option is specified via command line, we should
    // treat the primary display as secondary display. Mainly for testing
    // purpose.
    if (!command_line->HasSwitch(kUsePrimaryDisplay) && display.IsInternal())
      continue;

    AddSecondaryDisplay(display);
  }
#else
  NOTIMPLEMENTED() << "Only implemented for GTK";
#endif

  initialized_ = true;
}

void PresentationDisplayManager::AddObserver(Observer* obs) {
  observers_->AddObserver(obs);
}

void PresentationDisplayManager::RemoveObserver(Observer* obs) {
  observers_->RemoveObserver(obs);
}

void PresentationDisplayManager::AddSecondaryDisplay(
    const gfx::Display& display) {
  secondary_displays_.push_back(display);
  // Notify its observers when the first secondary display is added.
  if (secondary_displays_.size() == 1)
    observers_->Notify(&Observer::OnDisplayAvailabilityChanged, true);
}

void PresentationDisplayManager::RemoveSecondaryDisplay(
    const gfx::Display& display) {
  std::vector<gfx::Display>::iterator it = secondary_displays_.begin();
  for (; it != secondary_displays_.end(); ++it)
    if (it->id() == display.id()) break;

  if (it == secondary_displays_.end())
    return;

  secondary_displays_.erase(it);
  // Notify its observers when the last secondary display is removed.
  if (secondary_displays_.size() == 0)
    observers_->Notify(&Observer::OnDisplayAvailabilityChanged, false);
}

gfx::Display PresentationDisplayManager::GetDisplayInfo(int display_id) {
  gfx::Display ret;
  std::vector<gfx::Display>::iterator it = secondary_displays_.begin();
  for (; it != secondary_displays_.end(); ++it)
    if (it->id() != display_id) continue;

  if (it != secondary_displays_.end())
    ret = *it;

  return ret;
}

}  // namespace experimental
}  // namespace xwalk
