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

  MOCK_METHOD1(OnShellAdded, void(Shell* shell));
  MOCK_METHOD1(OnShellRemoved, void(Shell* shell));

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

// Make sure that the second invocation creates a new window.
IN_PROC_BROWSER_TEST_F(CameoBrowserMainTest, SecondLaunch) {
  MockShellRegistryObserver observer;
  cameo::ShellRegistry::Get()->AddObserver(&observer);
  Relaunch(GetCommandLineForRelaunch());

  Shell* new_shell = observer.WaitForSingleNewShell();
  EXPECT_TRUE(new_shell);
  EXPECT_CALL(observer, OnShellAdded(new_shell)).Times(1);
  ASSERT_EQ(2u, ShellRegistry::Get()->shells().size());
}

#if 0
IN_PROC_BROWSER_TEST_F(CameoBrowserMainTest,
                       ReuseBrowserInstanceWhenOpeningFile) {
  base::FilePath test_file_path = ui_test_utils::GetTestFilePath(
      base::FilePath(), base::FilePath().AppendASCII("empty.html"));
  CommandLine new_command_line(GetCommandLineForRelaunch());
  new_command_line.AppendArgPath(test_file_path);
  content::WindowedNotificationObserver observer(
        chrome::NOTIFICATION_TAB_ADDED,
        content::NotificationService::AllSources());
  Relaunch(new_command_line);
  observer.Wait();

  GURL url = net::FilePathToFileURL(test_file_path);
  content::WebContents* tab =
      browser()->tab_strip_model()->GetActiveWebContents();
  ASSERT_EQ(url, tab->GetController().GetActiveEntry()->GetVirtualURL());
}

// CameoBrowserMainTest.SecondLaunchWithIncognitoUrl is flaky on Win and Linux.
// http://crbug.com/130395
#if defined(OS_WIN) || defined(OS_LINUX)
#define MAYBE_SecondLaunchWithIncognitoUrl DISABLED_SecondLaunchWithIncognitoUrl
#else
#define MAYBE_SecondLaunchWithIncognitoUrl SecondLaunchWithIncognitoUrl
#endif

IN_PROC_BROWSER_TEST_F(CameoBrowserMainTest, MAYBE_SecondLaunchWithIncognitoUrl) {
  // We should start with one normal window.
  ASSERT_EQ(1u, chrome::GetTabbedBrowserCount(browser()->profile(),
                                              browser()->host_desktop_type()));

  // Run with --incognito switch and an URL specified.
  base::FilePath test_file_path = ui_test_utils::GetTestFilePath(
      base::FilePath(), base::FilePath().AppendASCII("empty.html"));
  CommandLine new_command_line(GetCommandLineForRelaunch());
  new_command_line.AppendSwitch(switches::kIncognito);
  new_command_line.AppendArgPath(test_file_path);

  Relaunch(new_command_line);

  // There should be one normal and one incognito window now.
  ui_test_utils::BrowserAddedObserver observer;
  Relaunch(new_command_line);
  observer.WaitForSingleNewBrowser();
  ASSERT_EQ(2u, chrome::GetTotalBrowserCount());

  ASSERT_EQ(1u, chrome::GetTabbedBrowserCount(browser()->profile(),
                                              browser()->host_desktop_type()));
}

IN_PROC_BROWSER_TEST_F(CameoBrowserMainTest, SecondLaunchFromIncognitoWithNormalUrl) {
  // We should start with one normal window.
  ASSERT_EQ(1u, chrome::GetTabbedBrowserCount(browser()->profile(),
                                              browser()->host_desktop_type()));

  // Create an incognito window.
  chrome::NewIncognitoWindow(browser());

  ASSERT_EQ(2u, chrome::GetTotalBrowserCount());
  ASSERT_EQ(1u, chrome::GetTabbedBrowserCount(browser()->profile(),
                                              browser()->host_desktop_type()));

  // Close the first window.
  Profile* profile = browser()->profile();
  chrome::HostDesktopType host_desktop_type = browser()->host_desktop_type();
  content::WindowedNotificationObserver observer(
        chrome::NOTIFICATION_BROWSER_CLOSED,
        content::NotificationService::AllSources());
  chrome::CloseWindow(browser());
  observer.Wait();

  // There should only be the incognito window open now.
  ASSERT_EQ(1u, chrome::GetTotalBrowserCount());
  ASSERT_EQ(0u, chrome::GetTabbedBrowserCount(profile, host_desktop_type));

  // Run with just an URL specified, no --incognito switch.
  base::FilePath test_file_path = ui_test_utils::GetTestFilePath(
      base::FilePath(), base::FilePath().AppendASCII("empty.html"));
  CommandLine new_command_line(GetCommandLineForRelaunch());
  new_command_line.AppendArgPath(test_file_path);
  content::WindowedNotificationObserver tab_observer(
        chrome::NOTIFICATION_TAB_ADDED,
        content::NotificationService::AllSources());
  Relaunch(new_command_line);
  tab_observer.Wait();

  // There should be one normal and one incognito window now.
  ASSERT_EQ(2u, chrome::GetTotalBrowserCount());
  ASSERT_EQ(1u, chrome::GetTabbedBrowserCount(profile, host_desktop_type));
}
#endif  // if 0
