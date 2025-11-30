# Android Native Activity Template

A minimal, production-ready template for building pure C++ Android applications using NativeActivity. No Java or Kotlin code required.

## Features

- ✅ **Pure C++ Development** - Zero Java/Kotlin code (`android:hasCode="false"`)
- ✅ **Minimal Dependencies** - Only standard NDK libraries (native_app_glue, android, log)
- ✅ **Modern Build System** - AGP 8.2.0, CMake 3.22.1, Gradle 8.x
- ✅ **Clean Architecture** - Simple event loop with proper lifecycle handling
- ✅ **Ready to Use** - Complete project structure with Gradle wrapper included

## Requirements

- Android Studio (Arctic Fox or later)
- Android NDK 23.1.7779620 or later
- CMake 3.22.1 or later
- Target API: Android 7.0 (API 24) and above

## Getting Started

### Clone and Build

```bash
git clone <your-repo-url>
cd android-native-activity-template
./gradlew assembleDebug
```

### Open in Android Studio

1. Open Android Studio
2. Select "Open an Existing Project"
3. Navigate to the project directory
4. Wait for Gradle sync to complete
5. Run on device/emulator

## Project Structure

```
.
├── app/
│   ├── src/main/
│   │   ├── cpp/
│   │   │   └── main.cpp          # Entry point (android_main)
│   │   ├── AndroidManifest.xml   # NativeActivity configuration
│   │   └── res/                  # App resources (icons, strings)
│   └── build.gradle              # App-level build configuration
├── CMakeLists.txt                # Native build configuration
├── build.gradle                  # Project-level build configuration
└── settings.gradle               # Gradle project settings
```

## Customization

### Change Package Name

1. Update `namespace` and `applicationId` in `app/build.gradle`
2. Update app name in `app/src/main/res/values/strings.xml`

### Add Native Libraries

Edit `CMakeLists.txt` to add your source files and link libraries:

```cmake
set(SOURCES
    app/src/main/cpp/main.cpp
    app/src/main/cpp/your_file.cpp  # Add your files here
)

target_link_libraries(native-activity
    android
    native_app_glue
    log
    # Add your libraries here
)
```

### Modify Entry Point

The main entry point is `android_main()` in `app/src/main/cpp/main.cpp`. This function is called when your app starts.