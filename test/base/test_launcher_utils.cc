// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/test/base/test_launcher_utils.h"

#include "base/command_line.h"
#include "base/environment.h"
#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "base/path_service.h"
#include "base/strings/string_number_conversions.h"
#include "chrome/common/chrome_paths.h"
#include "chrome/common/chrome_switches.h"
#include "ui/gl/gl_switches.h"

namespace test_launcher_utils {

void PrepareBrowserCommandLineForTests(CommandLine* command_line) {
  // Enable info level logging by default so that we can see when bad
  // stuff happens, but honor the flags specified from the command line.
  if (!command_line->HasSwitch(switches::kEnableLogging))
    command_line->AppendSwitch(switches::kEnableLogging);
  if (!command_line->HasSwitch(switches::kLoggingLevel))
    command_line->AppendSwitchASCII(switches::kLoggingLevel, "0");  // info

  // Don't collect GPU info, load GPU blacklist, or schedule a GPU blacklist
  // auto-update.
  command_line->AppendSwitch(switches::kSkipGpuDataLoading);
}

bool OverrideGLImplementation(CommandLine* command_line,
                              const std::string& implementation_name) {
  if (command_line->HasSwitch(switches::kUseGL))
    return false;

  command_line->AppendSwitchASCII(switches::kUseGL, implementation_name);

  return true;
}

}  // namespace test_launcher_utils
