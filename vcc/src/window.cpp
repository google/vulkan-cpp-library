/*
* Copyright 2016 Google Inc. All Rights Reserved.

* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at

* http://www.apache.org/licenses/LICENSE-2.0

* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#define NOMINMAX
#include <algorithm>
#include <cassert>
#include <chrono>
#include <vcc/command.h>
#include <vcc/physical_device.h>
#include <vcc/surface.h>
#include <vcc/window.h>
#ifdef _WIN32
#include <windowsx.h>
#endif // WIN32

namespace vcc {
namespace window {

input_callbacks_type::input_callbacks_type()
	: mouse_down_callback([](mouse_button_type, int, int) {return false; }),
	  mouse_up_callback([](mouse_button_type, int, int) {return false; }),
	  mouse_move_callback([](int, int) {return false; }),
	  key_down_callback([](keycode_type) {return false; }),
	  key_up_callback([](keycode_type) {return false; }) {}

input_callbacks_type &input_callbacks_type::set_mouse_down_callback(
		const mouse_press_callback_type &callback) {
	mouse_down_callback = callback;
	return *this;
}

	input_callbacks_type &input_callbacks_type::set_mouse_up_callback(
		const mouse_press_callback_type &callback) {
	mouse_up_callback = callback;
	return *this;
}
	input_callbacks_type &input_callbacks_type::set_mouse_move_callback(
		const mouse_move_callback_type &callback) {
	mouse_move_callback = callback;
	return *this;
}
	input_callbacks_type &input_callbacks_type::set_key_down_callback(
		const key_press_callback_type &callback) {
	key_down_callback = callback;
	return *this;
}
	input_callbacks_type &input_callbacks_type::set_key_up_callback(
		const key_press_callback_type &callback) {
	key_up_callback = callback;
	return *this;
}
input_callbacks_type &input_callbacks_type::set_touch_down_callback(
		const touch_press_callback_type &callback) {
	touch_down_callback = callback;
	return *this;
}
input_callbacks_type &input_callbacks_type::set_touch_up_callback(
		const touch_press_callback_type &callback) {
	touch_up_callback = callback;
	return *this;
}
input_callbacks_type &input_callbacks_type::set_touch_move_callback(
		const touch_move_callback_type &callback) {
	touch_move_callback = callback;
	return *this;
}

#ifdef _WIN32
const char *class_name = "vcc-vulkan";
#endif // WIN32

namespace internal {

window_data_type::~window_data_type() {
	render_thread.join();
#ifdef _WIN32
	DestroyWindow(window);
#endif // _WIN32
}

} // namespace internal

void resize(internal::window_data_type &data, VkExtent2D extent) {
	{
		// Check the surface capabilities and formats
		const VkPhysicalDevice physical_device(device::get_physical_device(*data.device));
		const VkSurfaceCapabilitiesKHR surfCapabilities(vcc::surface::physical_device_capabilities(physical_device, data.surface));

		// width and height are either both -1, or both not -1.
		data.extent = surfCapabilities.currentExtent.width == -1 ? extent : surfCapabilities.currentExtent;

		// If mailbox mode is available, use it, as is the lowest-latency non-
		// tearing mode.  If not, try IMMEDIATE which will usually be available,
		// and is fastest (though it tears).  If not, fall back to FIFO which is
		// always available.
		const std::vector<VkPresentModeKHR> presentModes(vcc::surface::physical_device_present_modes(physical_device, data.surface));
		VkPresentModeKHR swapchainPresentMode(*std::max_element(presentModes.begin(), presentModes.end(), [](VkPresentModeKHR mode1, VkPresentModeKHR mode2) {
			if (mode1 != mode2) {
				switch (mode2) {
				case VK_PRESENT_MODE_MAILBOX_KHR:
					return true;
				case VK_PRESENT_MODE_IMMEDIATE_KHR:
					return mode1 != VK_PRESENT_MODE_MAILBOX_KHR;
				case VK_PRESENT_MODE_FIFO_KHR:
					return mode1 != VK_PRESENT_MODE_MAILBOX_KHR && mode1 != VK_PRESENT_MODE_IMMEDIATE_KHR;
				}
			}
			return false;
		}));

		// Determine the number of VkImage's to use in the swap chain (we desire to
		// own only 1 image at a time, besides the images being displayed and
		// queued for display):
		uint32_t desiredNumberOfSwapchainImages = surfCapabilities.minImageCount + 1;
		if (surfCapabilities.maxImageCount > 0) {
			// Application might have to settle for fewer images than desired:
			desiredNumberOfSwapchainImages = std::min(surfCapabilities.maxImageCount, desiredNumberOfSwapchainImages);
		}
		const VkSurfaceTransformFlagBitsKHR preTransform(surfCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR : surfCapabilities.currentTransform);
		data.swapchain_images.clear();
		data.swapchain = vcc::swapchain::create(type::supplier<device::device_type>(data.device),
			vcc::swapchain::create_info_type{ std::ref(data.surface), desiredNumberOfSwapchainImages, data.format, data.color_space, data.extent,
				1, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_SHARING_MODE_EXCLUSIVE,{}, preTransform, 0, swapchainPresentMode, VK_TRUE, std::ref(data.swapchain) });
		std::vector<vcc::image::image_type> swapchain_images(vcc::swapchain::get_images(data.swapchain));
		data.swapchain_images.reserve(swapchain_images.size());

		for (vcc::image::image_type &swapchain_image : swapchain_images) {
			// Render loop will expect image to have been used before and in VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
			// layout and will change to COLOR_ATTACHMENT_OPTIMAL, so init the image to that state
			vcc::command_buffer::command_buffer_type command_buffer(std::move(vcc::command_buffer::allocate(type::supplier<device::device_type>(data.device), std::ref(data.cmd_pool), VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1).front()));
			vcc::command_buffer::compile(command_buffer, 0, VK_FALSE, 0, 0,
				vcc::command::pipeline_barrier(
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
					VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, {}, {},
					{
						vcc::command::image_memory_barrier{ 0, 0,
							VK_IMAGE_LAYOUT_UNDEFINED,
							VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
							VK_QUEUE_FAMILY_IGNORED,
							queue::get_family_index(data.present_queue),
							std::ref(swapchain_image),
							{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
						}
					}));
			vcc::queue::submit(data.present_queue, {}, { std::reference_wrapper<vcc::command_buffer::command_buffer_type>(command_buffer) }, {});

			vcc::image_view::image_view_type view(vcc::image_view::create(std::ref(swapchain_image), VK_IMAGE_VIEW_TYPE_2D, data.format,
			{ VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A },
			{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }));
			data.swapchain_images.push_back(swapchain_type(std::move(swapchain_image), std::move(view)));
		}
		data.resize_callback(data.extent, data.format, data.swapchain_images);
	}
}

namespace internal {

void window_data_type::draw() {
	VkResult err;
	uint32_t current_buffer;
	vcc::semaphore::semaphore_type present_complete_semaphore;
	do {
		present_complete_semaphore = vcc::semaphore::create(device);
		std::tie(err, current_buffer) = vcc::swapchain::acquire_next_image(
			swapchain, present_complete_semaphore);
		switch (err) {
		case VK_ERROR_OUT_OF_DATE_KHR:
			// swapchain is out of date (e.g. the window was resized) and
			// must be recreated:
			resize_callback(extent, format, swapchain_images);
			break;
		case VK_SUBOPTIMAL_KHR:
			VCC_PRINT("VK_SUBOPTIMAL_KHR");
			// swapchain is not as optimal as it could be, but the platform's
			// presentation engine will still present the image correctly.
			break;
		default:
			if (err) {
				VCC_PRINT("vcc::swapchain::acquire_next_image resulted in %u", err);
			}
			assert(!err);
			break;
		}
	} while (err == VK_ERROR_OUT_OF_DATE_KHR);
	// Assume the command buffer has been run on current_buffer before so
	// we need to set the image layout back to COLOR_ATTACHMENT_OPTIMAL
	vcc::command_buffer::command_buffer_type command_buffer(std::move(
		vcc::command_buffer::allocate(device, std::ref(cmd_pool),
			VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1).front()));
	vcc::command_buffer::compile(command_buffer, 0, VK_FALSE, 0, 0,
		vcc::command::pipeline_barrier(
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, {}, {},
			{
				vcc::command::image_memory_barrier{ 0,
					VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
					| VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
					VK_IMAGE_LAYOUT_UNDEFINED,
					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
					queue::get_family_index(present_queue),
					queue::get_family_index(*graphics_queue),
					std::ref(get_image(swapchain_images[current_buffer])),
					{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
				}
			}));
	vcc::queue::submit(*graphics_queue, {}, { std::ref(command_buffer) }, {});

	draw_callback(current_buffer);

	vcc::command_buffer::compile(command_buffer, 0, VK_FALSE, 0, 0,
		vcc::command::pipeline_barrier(
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, {}, {},
			{
				vcc::command::image_memory_barrier{
					VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
					0,
					VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
					VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
					queue::get_family_index(*graphics_queue),
					queue::get_family_index(present_queue),
					std::ref(get_image(swapchain_images[current_buffer])),
					{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 } }
			}));
	vcc::queue::submit(present_queue,
		{ vcc::queue::wait_semaphore{ std::ref(present_complete_semaphore) } },
		{ std::ref(command_buffer) }, {});

	err = vcc::queue::present(present_queue, {}, { std::ref(swapchain) },
		{ current_buffer });
	switch (err) {
	case VK_ERROR_OUT_OF_DATE_KHR:
		// swapchain is out of date (e.g. the window was resized) and
		// must be recreated:
		resize_callback(extent, format, swapchain_images);
		break;
	case VK_SUBOPTIMAL_KHR:
		// swapchain is not as optimal as it could be, but the platform's
		// presentation engine will still present the image correctly.
		VCC_PRINT("VK_SUBOPTIMAL_KHR");
		break;
	default:
		assert(!err);
	}

	vcc::queue::wait_idle(present_queue);
}

} // namespace internal

void initialize(internal::window_data_type &data,
#ifdef _WIN32
		HWND window
#elif defined(__ANDROID__)
		ANativeWindow *window
#endif // __ANDROID__
		) {
	data.surface = vcc::surface::create(data.instance,
#ifdef _WIN32
		data.connection,
#endif // _WIN32
		window);

#ifdef _WIN32
	data.window = window;
#endif // _WIN32

	data.present_queue = queue::get_present_queue(data.device, data.surface);

	data.cmd_pool = vcc::command_pool::create(data.device, 0,
		vcc::queue::get_family_index(data.present_queue));

	// Get the list of VkFormat's that are supported:
	const VkPhysicalDevice physical_device(device::get_physical_device(*data.device));
	const std::vector<VkSurfaceFormatKHR> surface_formats(
		vcc::surface::physical_device_formats(physical_device, data.surface));
	// If the format list includes just one entry of VK_FORMAT_UNDEFINED,
	// the surface has no preferred format.  Otherwise, at least one
	// supported format will be returned.
	// TODO(gardell): Pick a requested format
	assert(!surface_formats.empty());
	data.format = surface_formats.front().format == VK_FORMAT_UNDEFINED
		? VK_FORMAT_B8G8R8A8_UNORM : surface_formats.front().format;
	data.color_space = surface_formats.front().colorSpace;
}

#ifdef _WIN32
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	LONG_PTR user_data = GetWindowLongPtr(hWnd, GWLP_USERDATA);
	internal::window_data_type *window_data =
		reinterpret_cast<internal::window_data_type *>(user_data);

	switch (uMsg) {
	case WM_NCCREATE:
		SetWindowLongPtr(hWnd, GWLP_USERDATA,
			(LONG_PTR)((LPCREATESTRUCT)lParam)->lpCreateParams);
		break;
	case WM_CREATE:
		initialize(*window_data, hWnd);
		break;
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	case WM_SIZE:
		{
			const VkExtent2D extent{
				uint32_t(lParam & 0xffff), uint32_t(lParam & 0xffff0000 >> 16) };
			window_data->render_thread.post([extent, window_data]() {
				resize(*window_data, extent);
			});
			window_data->render_thread.set_drawing(true);
		}
		break;
	case WM_LBUTTONDOWN:
		window_data->input_callbacks.mouse_down_callback(mouse_button_left, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	case WM_MBUTTONDOWN:
		window_data->input_callbacks.mouse_down_callback(mouse_button_middle, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	case WM_RBUTTONDOWN:
		window_data->input_callbacks.mouse_down_callback(mouse_button_right, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	case WM_XBUTTONDOWN:
		switch (GET_XBUTTON_WPARAM(wParam)) {
		case XBUTTON1:
			window_data->input_callbacks.mouse_down_callback(mouse_button_4, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			break;
		case XBUTTON2:
			window_data->input_callbacks.mouse_down_callback(mouse_button_5, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			break;
		}
		break;
	case WM_LBUTTONUP:
		window_data->input_callbacks.mouse_up_callback(mouse_button_left, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	case WM_MBUTTONUP:
		window_data->input_callbacks.mouse_up_callback(mouse_button_middle, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	case WM_RBUTTONUP:
		window_data->input_callbacks.mouse_up_callback(mouse_button_right, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	case WM_XBUTTONUP:
		switch (GET_XBUTTON_WPARAM(wParam)) {
		case XBUTTON1:
			window_data->input_callbacks.mouse_up_callback(mouse_button_4, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			break;
		case XBUTTON2:
			window_data->input_callbacks.mouse_up_callback(mouse_button_5, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			break;
		}
		break;
	case WM_MOUSEMOVE:
		window_data->input_callbacks.mouse_move_callback(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		break;
	case WM_KEYDOWN:
		window_data->input_callbacks.key_down_callback(keycode_type(wParam));
		break;
	case WM_KEYUP:
		window_data->input_callbacks.key_up_callback(keycode_type(wParam));
		break;
	default:
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
#elif defined(__ANDROID__)

int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
    internal::window_data_type *window_data =
      (internal::window_data_type *) app->userData;
    switch (AInputEvent_getType(event)) {
      case AINPUT_EVENT_TYPE_MOTION: {
        const size_t count(AMotionEvent_getPointerCount(event));
				bool handled = false;
        switch (AMotionEvent_getAction(event)) {
          case AMOTION_EVENT_ACTION_MOVE:
            for (size_t i = 0; i < count; ++i) {
              const int32_t id(AMotionEvent_getPointerId(event, i));
              handled |= window_data->input_callbacks.touch_move_callback(id,
                AMotionEvent_getX(event, i), AMotionEvent_getY(event, i));
            }
            return !!handled;
          case AMOTION_EVENT_ACTION_DOWN:
            for (size_t i = 0; i < count; ++i) {
              const int32_t id(AMotionEvent_getPointerId(event, i));
              handled |= window_data->input_callbacks.touch_down_callback(id,
                AMotionEvent_getX(event, i), AMotionEvent_getY(event, i));
            }
            return !!handled;
          case AMOTION_EVENT_ACTION_UP:
            for (size_t i = 0; i < count; ++i) {
              const int32_t id(AMotionEvent_getPointerId(event, i));
              handled |= window_data->input_callbacks.touch_up_callback(id,
                AMotionEvent_getX(event, i), AMotionEvent_getY(event, i));
            }
            return !!handled;
        }
        } break;
    }
    return 0;
}

/**
 * Process the next main command.
 */
void engine_handle_cmd(struct android_app* app, int32_t cmd) {
    internal::window_data_type *data =
        (internal::window_data_type *) app->userData;
    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            // The system has asked us to save our current state.  Do so.
            break;
        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            if (app->window != NULL) {
              vcc::window::initialize(*data, app->window);
              vcc::window::resize(*data, VkExtent2D{
                (uint32_t) ANativeWindow_getWidth(app->window),
                (uint32_t) ANativeWindow_getHeight(app->window)});
              data->render_thread.set_drawing(true);
            }
            break;
        case APP_CMD_WINDOW_RESIZED:
            vcc::window::resize(*data, VkExtent2D{
              (uint32_t) ANativeWindow_getWidth(app->window),
              (uint32_t) ANativeWindow_getHeight(app->window)});
        	break;
        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
            break;
        case APP_CMD_GAINED_FOCUS:
            // When our app gains focus, we start monitoring the accelerometer.
            data->render_thread.set_drawing(true);
            break;
        case APP_CMD_LOST_FOCUS:
            // When our app loses focus, we stop monitoring the accelerometer.
            // This is to avoid consuming battery while not being used.
            data->render_thread.set_drawing(false);
            break;
    }
}

#endif // __ANDROID__

window_type create(
#ifdef _WIN32
	HINSTANCE hinstance,
#elif defined(__ANDROID__)
	android_app* state,
#endif // __ANDROID__
	const type::supplier<instance::instance_type> &instance,
	const type::supplier<device::device_type> &device,
	const type::supplier<queue::queue_type> &graphics_queue,
	VkExtent2D extent, VkFormat format, const std::string &title) {

	std::unique_ptr<internal::window_data_type> data(new internal::window_data_type(
#ifdef _WIN32
		hinstance,
#elif defined(__ANDROID__)
		state,
#endif // _WIN32
		instance,  graphics_queue, extent, device));

#ifdef _WIN32
	WNDCLASSEX  win_class;
	win_class.cbSize = sizeof(WNDCLASSEX);

	if (!GetClassInfoEx(hinstance, class_name, &win_class)) {
		// Initialize the window class structure:
		win_class.style = CS_HREDRAW | CS_VREDRAW;
		win_class.lpfnWndProc = WndProc;
		win_class.cbClsExtra = 0;
		win_class.cbWndExtra = 0;
		win_class.hInstance = hinstance; // hInstance
		win_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		win_class.hCursor = LoadCursor(NULL, IDC_ARROW);
		win_class.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		win_class.lpszMenuName = NULL;
		win_class.lpszClassName = class_name;
		win_class.hIconSm = LoadIcon(NULL, IDI_WINLOGO);
		// Register window class:
		if (!RegisterClassEx(&win_class)) {
			throw vcc::vcc_exception("RegisterClassEx");
		}
	}

	// Create window with the registered class:
	RECT wr = { 0, 0, (LONG) extent.width, (LONG) extent.height };
	AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
	if (!CreateWindowEx(0, class_name, title.c_str(),
		WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_SYSMENU,
		100, 100, wr.right - wr.left, wr.bottom - wr.top, NULL, NULL, hinstance, data.get())) {
		throw vcc::vcc_exception("CreateWindowEx failed");
	}
#elif defined(__ANDROID__)
	state->userData = data.get();
	state->onAppCmd = engine_handle_cmd;
	state->onInputEvent = engine_handle_input;
#endif // _WIN32

	return window_type{std::move(data)};
}

int run(window_type &window, const resize_callback_type &resize_callback,
	const draw_callback_type &draw_callback,
	const input_callbacks_type &input_callbacks) {
	window.data->resize_callback = resize_callback;
	window.data->draw_callback = draw_callback;
	window.data->input_callbacks = input_callbacks;
	window.data->render_thread.start();
#ifdef _WIN32
	MSG msg;
	for (;;) {
		GetMessage(&msg, NULL, 0, 0);
		if (msg.message == WM_QUIT) {
			break;
		} else {
			/* Translate and dispatch to event queue*/
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	window.data->render_thread.join();
	return (int) msg.wParam;

#elif defined(__ANDROID__)
	for (;;) {
		// Read all pending events.
		int events;
		android_poll_source *source;
		for (int ident;
				(ident = ALooper_pollAll(-1, NULL, &events, (void**) &source)) >= 0;) {

			if (source) {
				source->process(window.data->state, source);
			}

			if (window.data->state->destroyRequested != 0) {
				window.data->render_thread.join();
				return 0;
			}
		}
	}
#endif // __ANDROID__
}

}  // namespace window
}  // namespace vcc
