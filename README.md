# vulkan-cpp

Vulkan abstraction library using C++11 for memory, resource management, type and thread safety as well as system independency.
The goal is to be able to quickly write Vulkan code that is type safe, readable and clearly states its purpose, instead of being overwhelmed by pointer arithmetics, memory alignment and the "Vulkan/OpenGL" black screen of death.

### Resource management

Buffer content is typed, referenced and revision counted. Push and specialization constants are typed. The library synchronizes your buffers between client and host lazily,
layouts data according to std140, std430, interleaved or linear depending on your need.

```C++
vec3_array vertices;
vec3_array normals;
vec2_array texcoords;

buffer_type vertex_buffer(create(interleaved_std140, ...,
  std::ref(vertices), std::ref(texcoords), std::ref(normals)));
```

The above `vertices`, `normals` and `texcoords` can be updated at any time, using the following syntax:
```C++
mutated_vec3_array m = mutate(vertices);
m[0] = 1.f;
```

Submitting a command buffer which depend on the buffer will cause a flush of said buffer.
```C++
queue::submit(queue, {}, command_buffers, {});
```

The `mutated_vec3_array` is a movable only lock of the underlying array. The `mutated_*_array` provides a full `std::vector` like interface where `*_array` provides a read-only interface.

*Because the data is strongly typed, there are a lot of opportunities for type checking, especially against SPIR-V. This is in the works!*

### Memory management
All Vulkan objects are encapsulated and hidden in C++ classes. These are movable only and destroy the underlying Vulkan objects when they go out of scope.

This makes memory management as easy as is expected with C++, simply move your object to a safe place and use `std::ref` whenever another object
needs to keep a reference, or, move your object into a `std::shared_ptr` for reference counting. All functions that take a `supplier<T>` in this library
will keep a reference to the object, where functions taking a reference will use the argument only for the scope of the function. `supplier<T>` has overloads for rvalue references (takes ownership), `std::shared_ptr` `std::unique_ptr`, `std::reference_wrapper` (`std::ref`) and `function<T&()>`.

### Multithreading
The library is thread-safe as required by the Vulkan specification, `2.5 Threading Behavior`, `Externally Synchronized Parameters`, `Externally Synchronized Parameter Lists`.
Notice that `Implicit Externally Synchronized Parameters` is not included.

### Xlib/xcb
In the works.

### Install

## Android
Install the latest Android SDK and NDK, the pre-release of Android N (API level 24) is required.
Unit and integration tests are not supported.

To compile and install the library and samples, run:
ANDROID_NDK_HOME=/<your-path>/android-ndk-r11c/ ANDROID_HOME=/<your-path>/android-sdk-linux/ ./gradlew installDebug


## Visual Studio 2015
Only 2015 is supported. The C++11 support in previous versions is not sufficient.
* Open the vulkan-cpp.sln,
* click "Property Manager" (next to "Solution Explorer", bottom left corner),
* Expand any of the projects and configurations,
* right-click on Microsoft.Cpp.Win32.user or Microsoft.Cpp.x64.user and select properties.
* Under Common Properties > User Macros,
* edit the VulkanSdk, GoogleTestDir, PngDir, GlmDir and GliDir to point to the locations of the respective libraries.
* The project should now compile. Win32 and x64 in Release and Debug configurations are setup.

### Acknowledgements
* This library optionally uses [OpenGL Mathematics, glm.](http://glm.g-truc.net/0.9.7/index.html)
* vcc-image uses [libpng](http://www.libpng.org/) for loading VK_IMAGE_TILING_LINEAR images.
* vcc-image uses [OpenGL Image, gli](http://gli.g-truc.net/) for loading VK_IMAGE_TILING_OPTIMAL images.
* The demos contain image resources by [Emil Persson, aka Humus](http://www.humus.name).
* The demos contain image resources by [Julian Herzog](https://commons.wikimedia.org/wiki/File:Normal_map_example_with_scene_and_result.png).

This is not an official Google product.
This is purely a project made by a Google employee.
