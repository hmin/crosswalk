// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cameo/test/base/cameo_test_suite.h"

#include "base/command_line.h"
#include "base/file_util.h"
#include "base/memory/ref_counted.h"
#include "base/metrics/stats_table.h"
#include "base/path_service.h"
#include "base/process_util.h"
#include "base/stringprintf.h"
#include "base/utf_string_conversions.h"
#include "cameo/src/common/shell_content_client.h"
#include "cameo/src/browser/shell_content_browser_client.h"
#include "content/public/test/test_launcher.h"
#include "net/base/net_errors.h"
#include "net/base/net_util.h"
#include "net/dns/mock_host_resolver.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/resource/resource_handle.h"

#if defined(OS_POSIX)
#include "base/shared_memory.h"
#endif

namespace {

void RemoveSharedMemoryFile(const std::string& filename) {
  // Stats uses SharedMemory under the hood. On posix, this results in a file
  // on disk.
#if defined(OS_POSIX)
  base::SharedMemory memory;
  memory.Delete(filename);
#endif
}

// In many cases it may be not obvious that a test makes a real DNS lookup.
// We generally don't want to rely on external DNS servers for our tests,
// so this host resolver procedure catches external queries and returns a failed
// lookup result.
class LocalHostResolverProc : public net::HostResolverProc {
 public:
  LocalHostResolverProc() : HostResolverProc(NULL) {}

  virtual int Resolve(const std::string& host,
                      net::AddressFamily address_family,
                      net::HostResolverFlags host_resolver_flags,
                      net::AddressList* addrlist,
                      int* os_error) OVERRIDE {
    const char* kLocalHostNames[] = {"localhost", "127.0.0.1", "::1"};
    bool local = false;

    if (host == net::GetHostName()) {
      local = true;
    } else {
      for (size_t i = 0; i < arraysize(kLocalHostNames); i++)
        if (host == kLocalHostNames[i]) {
          local = true;
          break;
        }
    }

    // To avoid depending on external resources and to reduce (if not preclude)
    // network interactions from tests, we simulate failure for non-local DNS
    // queries, rather than perform them.
    // If you really need to make an external DNS query, use
    // net::RuleBasedHostResolverProc and its AllowDirectLookup method.
    if (!local) {
      DVLOG(1) << "To avoid external dependencies, simulating failure for "
          "external DNS lookup of " << host;
      return net::ERR_NOT_IMPLEMENTED;
    }

    return ResolveUsingPrevious(host, address_family, host_resolver_flags,
                                addrlist, os_error);
  }

 private:
  virtual ~LocalHostResolverProc() {}
};

class CameoTestSuiteInitializer : public testing::EmptyTestEventListener {
 public:
  CameoTestSuiteInitializer() {
  }

  virtual void OnTestStart(const testing::TestInfo& test_info) OVERRIDE {
    content_client_.reset(new cameo::ShellContentClient);
    content::SetContentClient(content_client_.get());

    browser_content_client_.reset(new cameo::ShellContentBrowserClient());
    SetBrowserClientForTesting(browser_content_client_.get());

    SetUpHostResolver();
  }

  virtual void OnTestEnd(const testing::TestInfo& test_info) OVERRIDE {
    browser_content_client_.reset();
    content_client_.reset();
    content::SetContentClient(NULL);

    TearDownHostResolver();
  }

 private:
  void SetUpHostResolver() {
    host_resolver_proc_ = new LocalHostResolverProc;
    scoped_host_resolver_proc_.reset(
        new net::ScopedDefaultHostResolverProc(host_resolver_proc_.get()));
  }

  void TearDownHostResolver() {
    scoped_host_resolver_proc_.reset();
    host_resolver_proc_ = NULL;
  }

  scoped_ptr<cameo::ShellContentClient> content_client_;
  scoped_ptr<cameo::ShellContentBrowserClient> browser_content_client_;

  scoped_refptr<LocalHostResolverProc> host_resolver_proc_;
  scoped_ptr<net::ScopedDefaultHostResolverProc> scoped_host_resolver_proc_;

  DISALLOW_COPY_AND_ASSIGN(CameoTestSuiteInitializer);
};

}  // namespace

CameoTestSuite::CameoTestSuite(int argc, char** argv)
    : content::ContentTestSuiteBase(argc, argv) {
}

CameoTestSuite::~CameoTestSuite() {
}

void CameoTestSuite::Initialize() {
  //cameo::RegisterPathProvider();

  if (!shell_bin_dir_.empty()) {
    PathService::Override(base::DIR_EXE, shell_bin_dir_);
    PathService::Override(base::DIR_MODULE, shell_bin_dir_);
  }

  // Initialize after overriding paths as some content paths depend on correct
  // values for DIR_EXE and DIR_MODULE.
  content::ContentTestSuiteBase::Initialize();

  // Force unittests to run using en-US so if we test against string
  // output, it'll pass regardless of the system language.
  ResourceBundle::InitSharedInstanceWithLocale("en-US", NULL);
//  base::FilePath resources_pack_path;
//  PathService::Get(cameo::FILE_RESOURCES_PACK, &resources_pack_path);
//  ResourceBundle::GetSharedInstance().AddDataPackFromPath(
//      resources_pack_path, ui::SCALE_FACTOR_NONE);

  stats_filename_ = base::StringPrintf("unit_tests-%d",
                                       base::GetCurrentProcId());
  RemoveSharedMemoryFile(stats_filename_);
  stats_table_.reset(new base::StatsTable(stats_filename_, 20, 200));
  base::StatsTable::set_current(stats_table_.get());

  testing::TestEventListeners& listeners =
      testing::UnitTest::GetInstance()->listeners();
  listeners.Append(new CameoTestSuiteInitializer);
}

content::ContentClient* CameoTestSuite::CreateClientForInitialization() {
  return new cameo::ShellContentClient();
}

void CameoTestSuite::Shutdown() {
  ResourceBundle::CleanupSharedInstance();

#if defined(OS_MACOSX) && !defined(OS_IOS)
  base::mac::SetOverrideFrameworkBundle(NULL);
#endif

  base::StatsTable::set_current(NULL);
  stats_table_.reset();
  RemoveSharedMemoryFile(stats_filename_);

  base::TestSuite::Shutdown();
}
