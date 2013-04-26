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
    'target_name': 'cameo_tests',
    'type': 'executable',
    'dependencies': [
      'test_support_common',
    ],
    'include_dirs' : [
      '..',
    ],
    'sources': [
      'test/base/run_all_unittests.cc',
    ], 
  }], # cameo_tests target
}
