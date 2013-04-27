# Copyright 2013 Intel Corp.

{
  'targets': [
  {
    'target_name': 'test_support_common',
    'type': 'static_library',
    'dependencies': [
      'cameo',
      'cameo_lib',
      'cameo_resources',
      '../base/base.gyp:test_support_base',
      '../base/base.gyp:base_prefs_test_support',
      '../content/content.gyp:content_app',
      '../content/content.gyp:test_support_content',
      '../net/net.gyp:net',
      '../net/net.gyp:net_test_support',
      '../skia/skia.gyp:skia',
      '../testing/gmock.gyp:gmock',
      '../testing/gtest.gyp:gtest',
      '../third_party/zlib/zlib.gyp:zlib',
    ],
    'export_dependent_settings': [
      '../base/base.gyp:test_support_base',
    ],
    'include_dirs': [
      '..',
    ],
    'sources': [
      'test/base/cameo_test_suite.cc',
      'test/base/cameo_test_suite.h',
    ],
    'conditions': [
      ['toolkit_uses_gtk == 1', {
        'dependencies' : [
          '../build/linux/system.gyp:gtk',
          '../build/linux/system.gyp:ssl',
        ],
      }],
      ['OS=="win"', {
        'include_dirs': [
          '<DEPTH>/third_party/wtl/include',
        ],
      }],
    ],
  },  # test_support_common target

  {
    'target_name': 'unit_tests',
    'type': 'executable',
    'dependencies': [
      'test_support_common',
      '../testing/gtest.gyp:gtest',
    ],
    'include_dirs' : [
      '..',
    ],
    'sources': [
      'src/common/shell_content_client_unittest.cc',
      'test/base/run_all_unittests.cc',
    ], 
  }, # unit_tests target

  {
    'target_name': 'browser_tests',
    'type': 'executable',
    'dependencies': [
      'test_support_common',
      '../skia/skia.gyp:skia',
      '../testing/gtest.gyp:gtest',
      '../testing/gmock.gyp:gmock',
    ],
    'include_dirs': [
      '..',
    ],
    'defines': [
      'HAS_OUT_OF_PROC_TEST_RUNNER',
    ],
    'sources': [
      'src/browser/shell_browser_main_browsertest.cc',
      'test/base/cameo_test_launcher.cc',
      'test/base/in_process_browser_test.cc',
      'test/base/in_process_browser_test.h',
      'test/base/test_launcher_utils.cc',
      'test/base/test_launcher_utils.h',
    ], 
  }], # browser_tests target
}