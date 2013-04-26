// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/test/base/in_process_browser_test.h"

#include "base/auto_reset.h"
#include "base/bind.h"
#include "base/command_line.h"
#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/lazy_instance.h"
#include "base/path_service.h"
#include "base/strings/string_number_conversions.h"
#include "base/test/test_file_util.h"
#include "cameo/src/renderer/shell_content_renderer_client.h"
#include "cameo/test/base/cameo_test_suite.h"
#include "cameo/test/base/test_launcher_utils.h"
#include "content/public/common/content_switches.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/notification_types.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_browser_thread.h"
#include "content/public/test/test_launcher.h"
#include "content/public/test/test_navigation_observer.h"
#include "content/public/test/test_utils.h"
#include "net/dns/mock_host_resolver.h"
#include "net/test/test_server.h"
#include "ui/compositor/compositor_switches.h"

namespace {

// Passed as value of kTestType.
const char kBrowserTestType[] = "browser";

// Used when running in single-process mode.
base::LazyInstance<cameo::ShellContentRendererClient>::Leaky
    g_cameo_content_renderer_client = LAZY_INSTANCE_INITIALIZER;

}  // namespace

InProcessBrowserTest::InProcessBrowserTest()
    : shell_(NULL),
      exit_when_last_shell_closes_(true)
    {
  CreateTestServer(base::FilePath(FILE_PATH_LITERAL("cameo/test/data")));
}

InProcessBrowserTest::~InProcessBrowserTest() {
}

void InProcessBrowserTest::SetUp() {
  CommandLine* command_line = CommandLine::ForCurrentProcess();
  // Allow subclasses to change the command line before running any tests.
  SetUpCommandLine(command_line);
  // Add command line arguments that are used by all InProcessBrowserTests.
  PrepareTestCommandLine(command_line);

  // Single-process mode is not set in BrowserMain, so process it explicitly,
  // and set up renderer.
  if (command_line->HasSwitch(switches::kSingleProcess)) {
    content::SetRendererClientForTesting(
        &g_cameo_content_renderer_client.Get());
  }

  host_resolver_ = new net::RuleBasedHostResolverProc(NULL);

  // See http://en.wikipedia.org/wiki/Web_Proxy_Autodiscovery_Protocol
  // We don't want the test code to use it.
  host_resolver_->AddSimulatedFailure("wpad");

  net::ScopedDefaultHostResolverProc scoped_host_resolver_proc(
      host_resolver_.get());

  BrowserTestBase::SetUp();
}

void InProcessBrowserTest::PrepareTestCommandLine(CommandLine* command_line) {
  // Propagate commandline settings from test_launcher_utils.
  test_launcher_utils::PrepareBrowserCommandLineForTests(command_line);
}

void InProcessBrowserTest::TearDown() {
  BrowserTestBase::TearDown();
}

cameo::Shell* InProcessBrowserTest::CreateShell() {
  return NULL;
}

CommandLine InProcessBrowserTest::GetCommandLineForRelaunch() {
  CommandLine new_command_line(CommandLine::ForCurrentProcess()->GetProgram());
  CommandLine::SwitchMap switches =
      CommandLine::ForCurrentProcess()->GetSwitches();
  new_command_line.AppendSwitch(content::kLaunchAsBrowser);

  for (CommandLine::SwitchMap::const_iterator iter = switches.begin();
        iter != switches.end(); ++iter) {
    new_command_line.AppendSwitchNative((*iter).first, (*iter).second);
  }
  return new_command_line;
}

void InProcessBrowserTest::RunTestOnMainThreadLoop() {
  // Pump startup related events.
  content::RunAllPendingInMessageLoop();

  SetUpOnMainThread();

  if (!HasFatalFailure())
    RunTestOnMainThread();

  // Invoke cleanup and quit even if there are failures. This is similar to
  // gtest in that it invokes TearDown even if Setup fails.
  CleanUpOnMainThread();
  // Sometimes tests leave Quit tasks in the MessageLoop (for shame), so let's
  // run all pending messages here to avoid preempting the QuitBrowsers tasks.
  // TODO(jbates) Once crbug.com/134753 is fixed, this can be removed because it
  // will not be possible to post Quit tasks.
  content::RunAllPendingInMessageLoop();

  QuitAllRunningShells();
}

void InProcessBrowserTest::QuitAllRunningShells() {
}
