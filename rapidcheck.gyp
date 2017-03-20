 {
  'target_defaults': {
    'configurations': {
      'Debug': { },
      'Release': { }
    },
    'msvs_settings': {
      'VCLinkerTool': {
        #Allows for creation / output to console.
        #Console (/SUBSYSTEM:CONSOLE)
        'SubSystem': '1',

        #Console app, use main/wmain
        'EntryPointSymbol': 'mainCRTStartup',
      },
    },
    'ldflags': [
    ],
    'cflags': [
      '-std=c++11','-Werror', '-W', '-Wall', '-Wextra', '-Wno-unused-parameter', '-g', '-O0',
    ]
  },
  'targets': [
    {
      'target_name': 'rapidcheck',
      'type': 'static_library',
      'include_dirs': [
        'include',
      ],
      'sources': [
        'src/BeforeMinimalTestCase.cpp',
        'src/Check.cpp',
        'src/Classify.cpp',
        'src/GenerationFailure.cpp',
        'src/Log.cpp',
        'src/Random.cpp',
        'src/Show.cpp',
        'src/detail/Any.cpp',
        'src/detail/Assertions.cpp',
        'src/detail/Base64.cpp',
        'src/detail/Configuration.cpp',
        'src/detail/DefaultTestListener.cpp',
        'src/detail/FrequencyMap.cpp',
        'src/detail/ImplicitParam.cpp',
        'src/detail/LogTestListener.cpp',
        'src/detail/MapParser.cpp',
        'src/detail/MulticastTestListener.cpp',
        'src/detail/ParseException.cpp',
        'src/detail/Platform.cpp',
        'src/detail/Property.cpp',
        'src/detail/PropertyContext.cpp',
        'src/detail/ReproduceListener.cpp',
        'src/detail/Results.cpp',
        'src/detail/Serialization.cpp',
        'src/detail/StringSerialization.cpp',
        'src/detail/TestMetadata.cpp',
        'src/detail/TestParams.cpp',
        'src/detail/Testing.cpp',
        'src/gen/Numeric.cpp',
        'src/gen/Text.cpp',
        'src/gen/detail/ExecHandler.cpp',
        'src/gen/detail/GenerationHandler.cpp',
        'src/gen/detail/Recipe.cpp',
        'src/gen/detail/ScaleInteger.cpp',
      ],
    },
    {
      'target_name': 'counter',
      'type': 'executable',
      'dependencies': [
        'rapidcheck',
      ],
      'include_dirs': [
        'include',
      ],
      'sources': [
        'examples/counter/main.cpp',
      ],
    },
    {
      'target_name': 'classify',
      'type': 'executable',
      'dependencies': [
        'rapidcheck',
      ],
      'include_dirs': [
        'include',
      ],
      'sources': [
        'examples/classify/main.cpp',
      ],
    },
    {
      'target_name': 'database',
      'type': 'executable',
      'dependencies': [
        'rapidcheck',
      ],
      'include_dirs': [
        'include',
      ],
      'sources': [
        'examples/database/Database.cpp',
        'examples/database/DatabaseConnection.cpp',
        'examples/database/User.cpp',
        'examples/database/main.cpp',
      ],
    },
    {
      'target_name': 'mapparser',
      'type': 'executable',
      'dependencies': [
        'rapidcheck',
      ],
      'include_dirs': [
        'include',
      ],
      'sources': [
        'examples/mapparser/MapParser.cpp',
        'examples/mapparser/main.cpp',
      ],
    },
  ],
}
