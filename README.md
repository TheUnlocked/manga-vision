# manga-vision

This is a library which analyzes pages of manga to extract useful information. It is designed for
use on Android devices.

## Features

Currently the feature set is very small. The following features are supported:

- Spread detection (i.e. do two adjacent pages form a spread)

manga-vision uses a custom build of the OpenCV Android SDK which optimizes for making as small a
binary as possible. As of v1, for a universal APK using this library, it will add ~13MB to the
uncompressed size, and about a quarter of that to platform-specific APKs.

## Building

You must compile the OpenCV SDK before you can build this project. The OpenCV source code is
included as a git submodule, and a command to compile it is in `.github/workflows/build.yml`.

The only artifact from this project is an AAR file, which can be used in libraries and apps.
It can be created with `./gradlew assembleRelease` and the output can be found in
`mangavision/build/outputs/aar/mangavision-release.aar`
