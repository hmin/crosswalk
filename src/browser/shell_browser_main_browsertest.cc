// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/test/ui/ui_test.h"

#include "base/command_line.h"
#include "base/file_util.h"
#include "base/path_service.h"
#include "base/process_util.h"
#include "cameo/src/common/cameo_notification_types.h"
#include "cameo/src/browser/shell.h"
#include "cameo/src/browser/shell_registry.h"
#include "cameo/test/base/in_process_browser_test.h"
#include "cameo/test/base/test_launcher_utils.h"
#include "content/public/browser/navigation_controller.h"
#include "content/public/browser/navigation_entry.h"
#include "content/public/browser/notification_details.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/notification_source.h"
#include "content/public/test/test_utils.h"
#include "content/public/browser/web_contents.h"
#include "content/public/test/test_utils.h"
#include "net/base/net_util.h"
#include "testing/gmock/include/gmock/gmock.h"

using cameo::Shell;
using cameo::ShellRegistry;
using cameo::ShellVector;
using testing::_;

// A mock observer to listen shell registry changes.
class MockShellRegistryObserver : public cameo::ShellRegistryObserver {
 public:
  MockShellRegistryObserver()
      : notification_observer_(cameo::NOTIFICATION_SHELL_CLOSED,
          content::NotificationService::AllSources()) {
    const ShellVector& shells = ShellRegistry::Get()->shells();
    for (ShellVector::const_iterator it = shells.begin();
         it != shells.end(); ++it)
      original_shells_.push_back(*it);
  }
  virtual ~MockShellRegistryObserver() {}

  MOCK_METHOD1(OnShellAdded, void(Shell*));
  MOCK_METHOD1(OnShellRemoved, void(Shell*));

  Shell* WaitForSingleNewShell() {
    notification_observer_.Wait();
    const ShellVector& shells = ShellRegistry::Get()->shells();
    for (ShellVector::const_iterator it = shells.begin();
         it != shells.end(); ++it) {
      ShellVector::iterator target =
          std::find(original_shells_.begin(), original_shells_.end(), *it);
      // Not found means a new one.
      if (target == original_shells_.end())
        return *it;
    }
    return NULL;
  }

 private:
  std::vector<Shell*> original_shells_;
  content::WindowedNotificationObserver notification_observer_;

  DISALLOW_COPY_AND_ASSIGN(MockShellRegistryObserver);
};

class CameoBrowserMainTest : public InProcessBrowserTest {
 public:
  CameoBrowserMainTest() {}

  void Relaunch(const CommandLine& new_command_line) {
    base::LaunchProcess(new_command_line, base::LaunchOptions(), NULL);
  }
};

// FIXME(hmin): Since the browser process is not shared by multiple launch,
// this test is disabled to avoid floody launches.
IN_PROC_BROWSER_TEST_F(CameoBrowserMainTest, DISABLED_SecondLaunch) {
  MockShellRegistryObserver observer;
  ShellRegistry::Get()->AddObserver(&observer);
  Relaunch(GetCommandLineForRelaunch());

  Shell* second_shell = NULL;
  EXPECT_TRUE(second_shell = observer.WaitForSingleNewShell());
  EXPECT_CALL(observer, OnShellAdded(second_shell)).Times(1);
  ASSERT_EQ(2u, ShellRegistry::Get()->shells().size());

  ShellRegistry::Get()->RemoveObserver(&observer);
}

IN_PROC_BROWSER_TEST_F(CameoBrowserMainTest, ShowAndCloseDevTools) {
  MockShellRegistryObserver observer;
  cameo::ShellRegistry::Get()->AddObserver(&observer);

  // A default Shell instance is created at startup time.
  size_t len = ShellRegistry::Get()->shells().size();
  EXPECT_EQ(1u, len);

  // Show devtools will create a new Shell instance.
  EXPECT_CALL(observer, OnShellAdded(_)).Times(1);
  shell()->ShowDevTools();
  content::RunAllPendingInMessageLoop();
  EXPECT_EQ(len + 1u, ShellRegistry::Get()->shells().size());

  // On closing devtools, the Shell will also be removed.
  EXPECT_CALL(observer, OnShellRemoved(_)).Times(1);
  shell()->CloseDevTools();
  content::RunAllPendingInMessageLoop();
  EXPECT_EQ(len, ShellRegistry::Get()->shells().size());

  ShellRegistry::Get()->RemoveObserver(&observer);
}
