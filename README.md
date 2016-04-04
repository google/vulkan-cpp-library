# vulkan-cpp

Vulkan abstraction library using C++11 for memory, resource management and type and thread safety as well as system independency.
The goal is to be able to quickly write Vulkan code that is readable and clearly states its purpose, instead of being overwhelmed by pointer arithmetics, memory alignment and the lovely "Vulkan/OpenGL" black screen of death.

### Resource management

Buffer content and push constants are all typed, referenced and revision counted. The library handles synchronizing your buffers between client and host lazily,
layouts data according to std140, std430, interleaved or linear depending on your need. Similar is provided for specialization constants but without revision counting.

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

// Submit a command buffer which depend on the buffer.
// Will cause a flush of the vertex_buffer.
queue::submit(queue, {}, command_buffers, {});
```

The `mutated_vec3_array` is a movable only lock of the underlying array. Unlike the array object, the mutable object provides a full vector-like
interface whereas the array object only exposes a read-only version.

*Because the data is strongly typed, there are a lot of opportunities for type checking, especially against SPIR-V. This is in the works!*

### Memory management
All Vulkan objects are encapsulated and hidden in C++ classes. These are movable only and destroy the underlying Vulkan objects when they go out of scope.

This makes memory management as easy as is expected with C++, simply move your object to a safe place and use `std::ref` whenever another object
needs to keep a reference, or, move your object into a `std::shared_ptr` for reference counting. All functions that take a supplier<T> in this library
will keep a reference to an object, where functions only using the argument for the scope of the function take immediate references. Supplier has overloads for movable objects (takes ownership), `std::shared_ptr` `std::unique_ptr`
as well as `std::reference_wrapper` (`std::ref`).

### Multithreading
In the works. The idea is to cover the locking required according to the Vulkan specification, 2.4 Threading Behavior.

### Android support.
In the works. Waiting for the NDK :)

### Xlib/xcb
In the works.

### Install
## Visual Studio 2015
Only 2015 is supported. The C++11 support in previous versions is not sufficient.
* Open the vulkan-cpp.sln,
* click "Property Manager" (next to "Solution Explorer", bottom left corner),
* Expand any of the projects and configurations,
* right-click on Microsoft.Cpp.Win32.user or Microsoft.Cpp.x64.user and select properties.
* Under Common Properties > User Macros,
* edit the VulkanSdk, GoogleTestDir, PngDir, GlmDir and GliDir to point to the locations of the respective libraries.
* The project should compile, both Win32 and x64 as well as Release and Debug are setup.

### Acknowledgements
* This library optionally uses [OpenGL Mathematics, glm.](http://glm.g-truc.net/0.9.7/index.html)
* vcc-image uses [libpng](http://www.libpng.org/) for loading VK_IMAGE_TILING_LINEAR images.
* vcc-image uses [OpenGL Image, gli](http://gli.g-truc.net/) for loading VK_IMAGE_TILING_OPTIMAL images.
* The demos contain image resources by [Emil Persson, aka Humus](http://www.humus.name).

This is not an official Google product.
This is purely a project made by a Google employee.
