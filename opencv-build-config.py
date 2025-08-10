# These options are optimized around minimizing binary size, particularly on x86/x86_64
# where the binaries tend to be larger even though very few devices use it.
# For the most part only the code that's used is included we don't need to explicitly omit
# most modules, but platform-specific optimizations take a large amount of space and some features
# depend on those and so need to be disabled for builds to work.

ANDROID_NATIVE_API_LEVEL = 26
cmake_common_vars = {
    'ANDROID_COMPILE_SDK_VERSION': 32,
    'ANDROID_TARGET_SDK_VERSION': 32,
    'ANDROID_MIN_SDK_VERSION': 26,
    'ANDROID_GRADLE_PLUGIN_VERSION': '8.12.0',
    'GRADLE_VERSION': '8.13',
    'KOTLIN_PLUGIN_VERSION': '2.1.20',

    # Options focused on trading performance for better size
    'CV_DISABLE_OPTIMIZATION': 'ON',
    'WITH_IPP': 'OFF',
    'CMAKE_BUILD_TYPE': 'MinSizeRel',

    'WITH_ANDROID_NATIVE_CAMERA': 'OFF',
    'WITH_TBB': 'OFF',
    'WITH_PROTOBUF': 'OFF',
    'WITH_QUIRC': 'OFF',
    'WITH_ZLIB_NG': 'OFF',
    'WITH_FLATBUFFERS': 'OFF',
    'WITH_ADE': 'OFF',

    'WITH_TIFF': 'OFF',
    'WITH_PNG': 'OFF',
    'WITH_JPEG': 'OFF',
    'WITH_WEBP': 'OFF',
    'WITH_OPENJPEG': 'OFF',
    'WITH_JASPER': 'OFF',
    'WITH_OPENEXR': 'OFF',

    'WITH_IMGCODEC_GIF': 'OFF',
    'WITH_IMGCODEC_HDR': 'OFF',
    'WITH_IMGCODEC_SUNRASTER': 'OFF',
    'WITH_IMGCODEC_PXM': 'OFF',
    'WITH_IMGCODEC_PFM': 'OFF',

    'CV_TRACE': 'OFF',
}
ABIs = [
    ABI("2", "armeabi-v7a", None, ndk_api_level=ANDROID_NATIVE_API_LEVEL, cmake_vars=cmake_common_vars),
    ABI("3", "arm64-v8a",   None, ndk_api_level=ANDROID_NATIVE_API_LEVEL, cmake_vars=cmake_common_vars),
    ABI("5", "x86_64",      None, ndk_api_level=ANDROID_NATIVE_API_LEVEL, cmake_vars=cmake_common_vars),
    ABI("4", "x86",         None, ndk_api_level=ANDROID_NATIVE_API_LEVEL, cmake_vars=cmake_common_vars),
]
