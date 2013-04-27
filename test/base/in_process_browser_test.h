// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_TEST_BASE_IN_PROCESS_BROWSER_TEST_H_
#define CHROME_TEST_BASE_IN_PROCESS_BROWSER_TEST_H_

#include "base/compiler_specific.h"
#include "base/files/scoped_temp_dir.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "content/public/common/page_transition_types.h"
#include "content/public/test/browser_test.h"
#include "content/public/test/browser_test_base.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace cameo {
class Shell;
}

class CommandLine;

namespace content {
class ContentRendererClient;
}

namespace net {
class RuleBasedHostResolverProc;
}

// Base class for tests wanting to bring up a browser in the unit test process.
// Writing tests with InProcessBrowserTest is slightly different than that of
// other tests. This is necessitated by InProcessBrowserTest running a message
// loop. To use InProcessBrowserTest do the following:
// . Use the macro IN_PROC_BROWSER_TEST_F to define your test.
// . Your test method is invoked on the ui thread. If you need to block until
//   state changes you'll need to run the message loop from your test method.
//   For example, if you need to wait till a find bar has completely been shown
//   you'll need to invoke content::RunMessageLoop. When the message bar is
//   shown, invoke MessageLoop::current()->Quit() to return control back to your
//   test method.
// . If you subclass and override SetUp, be sure and invoke
//   InProcessBrowserTest::SetUp. (But see also SetUpOnMainThread,
//   SetUpInProcessBrowserTestFixture and other related hook methods for a
//   cleaner alternative).
//
// Following three hook methods are called in sequence before calling
// BrowserMain(), thus no browser has been created yet. They are mainly for
// setting up the environment for running the browser.
// . SetUpCommandLine()
// . SetUpInProcessBrowserTestFixture()
//
// SetUpOnMainThread() is called just after creating the default browser object
// and before executing the real test code. It's mainly for setting up things
// related to the browser object and associated window, like opening a new Tab
// with a testing page loaded.
//
// CleanUpOnMainThread() is called just after executing the real test code to
// do necessary cleanup before the browser is torn down.
//
// TearDownInProcessBrowserTestFixture() is called after BrowserMain() exits to
// cleanup things setup for running the browser.
//
// By default InProcessBrowserTest creates a single Browser (as returned from
// the CreateBrowser method). You can obviously create more as needed.

// InProcessBrowserTest disables the sandbox when running.
//
// See ui_test_utils for a handful of methods designed for use with this class.
//
// It's possible to write browser tests that span a restart by splitting each
// run of the browser process into a separate test. Example:
//
// IN_PROC_BROWSER_TEST_F(Foo, PRE_Bar) {
//   do something
// }
//
// IN_PROC_BROWSER_TEST_F(Foo, Bar) {
//   verify something persisted from before
// }
//
//  This is recursive, so PRE_PRE_Bar would run before PRE_BAR.
class InProcessBrowserTest : public content::BrowserTestBase {
 public:
  InProcessBrowserTest();
  virtual ~InProcessBrowserTest();

  // Configures everything for an in process browser test, then invokes
  // BrowserMain. BrowserMain ends up invoking RunTestOnMainThreadLoop.
  virtual void SetUp() OVERRIDE;

  // Restores state configured in SetUp.
  virtual void TearDown() OVERRIDE;

 protected:
  // Returns the shell instance created by CreateShell.
  cameo::Shell* shell() const { return shell_; }

  // Override this to add any custom cleanup code that needs to be done on the
  // main thread before the browser is torn down.
  virtual void CleanUpOnMainThread() {}

  // BrowserTestBase:
  virtual void RunTestOnMainThreadLoop() OVERRIDE;

  // Creates a shell instance, wait for the shell to finish loading and
  // show the window.
  cameo::Shell* CreateShell();

  // Return a CommandLine object that is used to relaunch the browser_test
  // binary as a browser process. This function is deliberately not defined on
  // the Mac because re-using an existing browser process when launching from
  // the command line isn't a concept that we support on the Mac; AppleEvents
  // are the Mac solution for the same need. Any test based on these functions
  // doesn't apply to the Mac.
  CommandLine GetCommandLineForRelaunch();

  // Returns the host resolver being used for the tests. Subclasses might want
  // to configure it inside tests.
  net::RuleBasedHostResolverProc* host_resolver() {
    return host_resolver_.get();
  }

  void SetExitWhenLastShellCloses(bool value) {
    exit_when_last_shell_closes_ = value;
  }

 private:

  // Quits all open shells and waits until there are no more shells.
  void QuitAllShells();

  // Prepare command line that will be used to launch the child browser process
  // with an in-process test.
  void PrepareTestCommandLine(CommandLine* command_line);

  // Browser created from CreateBrowser.
  cameo::Shell* shell_;

  // Host resolver to use during the test.
  scoped_refptr<net::RuleBasedHostResolverProc> host_resolver_;

  // Temporary user data directory. Used only when a user data directory is not
  // specified in the command line.
  base::ScopedTempDir temp_user_data_dir_;

  // True if we should exit the tests after the last shell instance closes.
  bool exit_when_last_shell_closes_;
};

#endif  // CHROME_TEST_BASE_IN_PROCESS_BROWSER_TEST_H_