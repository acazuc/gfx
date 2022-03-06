#include "vk.h"
#include "../device_vtable.h"
#include "../window.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <errno.h>

#define VK_DEVICE ((gfx_vk_device_t*)device)

typedef struct gfx_vk_device_s
{
	gfx_device_t device;
	VkSurfaceCapabilitiesKHR surface_capabilities;
	VkSurfaceFormatKHR *surface_formats;
	uint32_t surface_formats_count;
	VkPresentModeKHR *present_modes;
	uint32_t present_modes_count;
	VkImageView *surface_image_views;
	uint32_t surface_image_views_count;
	VkImage *surface_images;
	uint32_t surface_images_count;
	VkFormat swap_chain_format;
	VkPhysicalDevice physical_device;
	VkCommandBuffer command_buffer;
	VkSwapchainKHR swap_chain;
	VkCommandPool command_pool;
	VkSurfaceKHR surface;
	VkInstance instance;
	VkDevice vk_device;
	VkQueue graphics_queue;
	VkQueue present_queue;
	uint32_t graphics_family;
	uint32_t present_family;
	VkPresentModeKHR present_mode;
} gfx_vk_device_t;

static const char *vk_err2str(VkResult result)
{
#define TEST_ERR(code) \
	case code: \
		return #code; \

	switch (result)
	{
		TEST_ERR(VK_SUCCESS)
		TEST_ERR(VK_NOT_READY)
		TEST_ERR(VK_TIMEOUT)
		TEST_ERR(VK_EVENT_SET)
		TEST_ERR(VK_EVENT_RESET)
		TEST_ERR(VK_INCOMPLETE)
		TEST_ERR(VK_ERROR_OUT_OF_HOST_MEMORY)
		TEST_ERR(VK_ERROR_OUT_OF_DEVICE_MEMORY)
		TEST_ERR(VK_ERROR_INITIALIZATION_FAILED)
		TEST_ERR(VK_ERROR_DEVICE_LOST)
		TEST_ERR(VK_ERROR_MEMORY_MAP_FAILED)
		TEST_ERR(VK_ERROR_LAYER_NOT_PRESENT)
		TEST_ERR(VK_ERROR_EXTENSION_NOT_PRESENT)
		TEST_ERR(VK_ERROR_FEATURE_NOT_PRESENT)
		TEST_ERR(VK_ERROR_INCOMPATIBLE_DRIVER)
		TEST_ERR(VK_ERROR_TOO_MANY_OBJECTS)
		TEST_ERR(VK_ERROR_FORMAT_NOT_SUPPORTED)
		TEST_ERR(VK_ERROR_FRAGMENTED_POOL)
		TEST_ERR(VK_ERROR_UNKNOWN)
		TEST_ERR(VK_ERROR_OUT_OF_POOL_MEMORY)
		TEST_ERR(VK_ERROR_INVALID_EXTERNAL_HANDLE)
		TEST_ERR(VK_ERROR_FRAGMENTATION)
		TEST_ERR(VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS)
		TEST_ERR(VK_ERROR_SURFACE_LOST_KHR)
		TEST_ERR(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR)
		TEST_ERR(VK_SUBOPTIMAL_KHR)
		TEST_ERR(VK_ERROR_OUT_OF_DATE_KHR)
		TEST_ERR(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR)
		TEST_ERR(VK_ERROR_VALIDATION_FAILED_EXT)
		TEST_ERR(VK_ERROR_INVALID_SHADER_NV)
		TEST_ERR(VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT)
		TEST_ERR(VK_ERROR_NOT_PERMITTED_EXT)
		TEST_ERR(VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT)
		TEST_ERR(VK_THREAD_IDLE_KHR)
		TEST_ERR(VK_THREAD_DONE_KHR)
		TEST_ERR(VK_OPERATION_DEFERRED_KHR)
		TEST_ERR(VK_OPERATION_NOT_DEFERRED_KHR)
		TEST_ERR(VK_PIPELINE_COMPILE_REQUIRED_EXT)
	}

	return "Unknown error";
#undef TEST_ERR
}

static bool get_queues_id(gfx_device_t *device, VkPhysicalDevice physical_device)
{
	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, NULL);
	VkQueueFamilyProperties *queue_families = GFX_MALLOC(sizeof(*queue_families) * queue_family_count);
	if (!queue_families)
	{
		GFX_ERROR_CALLBACK("can't allocate queue families: %s (%d)", strerror(errno), errno);
		return false;
	}
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families);
	bool graphics_found = false;
	bool present_found = false;
	for (uint32_t i = 0; i < queue_family_count; ++i)
	{
		if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			VK_DEVICE->graphics_family = i;
			graphics_found = true;
		}
		VkBool32 surface_support = 0;
		vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, VK_DEVICE->surface, &surface_support);
		if (surface_support)
		{
			VK_DEVICE->present_family = i;
			present_found = true;
		}
	}
	GFX_FREE(queue_families);
	return graphics_found && present_found;
}

static bool support_extensions(VkPhysicalDevice physical_device)
{
	uint32_t extensions_count;
	vkEnumerateDeviceExtensionProperties(physical_device, NULL, &extensions_count, NULL);
	VkExtensionProperties *extensions = GFX_MALLOC(sizeof(*extensions) * extensions_count);
	if (!extensions)
	{
		GFX_ERROR_CALLBACK("device extensions allocation failed: %s (%d)", strerror(errno), errno);
		return false;
	}
	vkEnumerateDeviceExtensionProperties(physical_device, NULL, &extensions_count, extensions);
	bool support_swapchain = false;
	for (uint32_t i = 0; i < extensions_count; ++i)
	{
		if (!strcmp(extensions[i].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME))
			support_swapchain = true;
	}
	GFX_FREE(extensions);
	return support_swapchain;
}

static bool get_physical_device(gfx_device_t *device)
{
	VkResult result;
	uint32_t device_count = 0;
	result = vkEnumeratePhysicalDevices(VK_DEVICE->instance, &device_count, NULL);
	if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("can't enumerate physical devices: %s (%d)", vk_err2str(result), result);
		return false;
	}
	if (device_count == 0)
	{
		GFX_ERROR_CALLBACK("no physical devices available");
		return false;
	}
	VkPhysicalDevice *devices = GFX_MALLOC(sizeof(*devices) * device_count);
	if (devices == NULL)
	{
		GFX_ERROR_CALLBACK("can't allocate physical devices: %s (%d)", strerror(errno), errno);
		return false;
	}
	result = vkEnumeratePhysicalDevices(VK_DEVICE->instance, &device_count, devices);
	if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("can't enumerate physical devices: %s (%d)", vk_err2str(result), result);
		GFX_FREE(devices);
		return false;
	}
	for (uint32_t i = 0; i < device_count; ++i)
	{
		VkPhysicalDeviceProperties device_properties;
		vkGetPhysicalDeviceProperties(devices[i], &device_properties);
		if (device_properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			continue;
		if (!get_queues_id(device, devices[i]))
			continue;
		if (!support_extensions(devices[i]))
			continue;
		uint32_t formats_count;
		vkGetPhysicalDeviceSurfaceFormatsKHR(devices[i], VK_DEVICE->surface, &formats_count, NULL);
		if (!formats_count)
			continue;
		uint32_t present_modes_count;
		vkGetPhysicalDeviceSurfacePresentModesKHR(devices[i], VK_DEVICE->surface, &present_modes_count, NULL);
		if (!present_modes_count)
			continue;
		VkSurfaceFormatKHR *formats = GFX_MALLOC(sizeof(*formats) * formats_count);
		if (!formats)
		{
			GFX_ERROR_CALLBACK("formats allocation failed: %s (%d)", strerror(errno), errno);
			GFX_FREE(devices);
			return false;
		}
		vkGetPhysicalDeviceSurfaceFormatsKHR(devices[i], VK_DEVICE->surface, &formats_count, formats);
		VkPresentModeKHR *present_modes = GFX_MALLOC(sizeof(*present_modes) * present_modes_count);
		if (!present_modes)
		{
			GFX_ERROR_CALLBACK("present modes allocation failed: %s (%d)", strerror(errno), errno);
			GFX_FREE(formats);
			GFX_FREE(devices);
			return false;
		}
		vkGetPhysicalDeviceSurfacePresentModesKHR(devices[i], VK_DEVICE->surface, &present_modes_count, present_modes);
		VK_DEVICE->present_modes = present_modes;
		VK_DEVICE->present_modes_count = present_modes_count;
		VK_DEVICE->surface_formats = formats;
		VK_DEVICE->surface_formats_count = formats_count;
		VK_DEVICE->physical_device = devices[i];
		GFX_FREE(devices);
		return true;
	}
	GFX_ERROR_CALLBACK("can't find suitable physical device");
	GFX_FREE(devices);
	return false;
}

static bool create_device(gfx_device_t *device)
{
	static const char *extensions[] =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};
	float queue_priority = 1;
	VkDeviceQueueCreateInfo queues_create_info[2];
	queues_create_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queues_create_info[0].pNext = NULL;
	queues_create_info[0].flags = 0;
	queues_create_info[0].queueFamilyIndex = VK_DEVICE->graphics_family;
	queues_create_info[0].queueCount = 1;
	queues_create_info[0].pQueuePriorities = &queue_priority;
	queues_create_info[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queues_create_info[1].pNext = NULL;
	queues_create_info[1].flags = 0;
	queues_create_info[1].queueFamilyIndex = VK_DEVICE->present_family;
	queues_create_info[1].queueCount = 1;
	queues_create_info[1].pQueuePriorities = &queue_priority;
	VkDeviceCreateInfo create_info;
	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = 0;
	create_info.queueCreateInfoCount = sizeof(queues_create_info) / sizeof(*queues_create_info);
	create_info.pQueueCreateInfos = queues_create_info;
	create_info.enabledLayerCount = 0;
	create_info.ppEnabledLayerNames = NULL;
	create_info.enabledExtensionCount = sizeof(extensions) / sizeof(*extensions);
	create_info.ppEnabledExtensionNames = extensions;
	create_info.pEnabledFeatures = NULL;
	VkResult result = vkCreateDevice(VK_DEVICE->physical_device, &create_info, NULL, &VK_DEVICE->vk_device);
	if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("can't create device: %s (%d)", strerror(errno), errno);
		return false;
	}
	vkGetDeviceQueue(VK_DEVICE->vk_device, VK_DEVICE->graphics_family, 0, &VK_DEVICE->graphics_queue);
	vkGetDeviceQueue(VK_DEVICE->vk_device, VK_DEVICE->present_family, 0, &VK_DEVICE->present_queue);
	return true;
}

static VkSurfaceFormatKHR *get_surface_format(gfx_device_t *device)
{
	for (uint32_t i = 0; i < VK_DEVICE->surface_formats_count; ++i)
	{
		VkSurfaceFormatKHR *format = &VK_DEVICE->surface_formats[i];
		if (format->format != VK_FORMAT_B8G8R8A8_SRGB
		 || format->colorSpace != VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			continue;
		return format;
	}
	return &VK_DEVICE->surface_formats[0];
}

static VkPresentModeKHR get_present_mode(gfx_device_t *device)
{
	for (uint32_t i = 0; i < VK_DEVICE->present_modes_count; ++i)
	{
		if (VK_DEVICE->present_modes[i] == VK_DEVICE->present_mode)
			return VK_DEVICE->present_mode;
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D get_extent(gfx_device_t *device)
{
	if (VK_DEVICE->surface_capabilities.currentExtent.width != UINT32_MAX)
		return VK_DEVICE->surface_capabilities.currentExtent;
	VkExtent2D extent;
	extent.width = device->window->width;
	extent.height = device->window->height;
	if (extent.width < VK_DEVICE->surface_capabilities.minImageExtent.width)
		extent.width = VK_DEVICE->surface_capabilities.minImageExtent.width;
	else if (extent.height > VK_DEVICE->surface_capabilities.maxImageExtent.width)
		extent.height = VK_DEVICE->surface_capabilities.maxImageExtent.width;
	if (extent.height < VK_DEVICE->surface_capabilities.minImageExtent.height)
		extent.height = VK_DEVICE->surface_capabilities.minImageExtent.height;
	else if (extent.height > VK_DEVICE->surface_capabilities.maxImageExtent.height)
		extent.height = VK_DEVICE->surface_capabilities.maxImageExtent.height;
	return extent;
}

static bool create_swapchain(gfx_device_t *device)
{
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VK_DEVICE->physical_device, VK_DEVICE->surface, &VK_DEVICE->surface_capabilities);
	VkSurfaceFormatKHR *surface_format = get_surface_format(device);
	VkPresentModeKHR present_mode = get_present_mode(device);
	VkExtent2D extent = get_extent(device);
	uint32_t image_count = VK_DEVICE->surface_capabilities.minImageCount + 1;
	if (VK_DEVICE->surface_capabilities.maxImageCount && image_count > VK_DEVICE->surface_capabilities.maxImageCount)
		image_count = VK_DEVICE->surface_capabilities.maxImageCount;
	VK_DEVICE->swap_chain_format = surface_format->format;
	VkSwapchainCreateInfoKHR create_info;
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.pNext = NULL;
	create_info.flags = 0;
	create_info.surface = VK_DEVICE->surface;
	create_info.minImageCount = image_count;
	create_info.imageFormat = surface_format->format;
	create_info.imageColorSpace = surface_format->colorSpace;
	create_info.imageExtent = extent;
	create_info.imageArrayLayers = 1;
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	if (VK_DEVICE->graphics_family != VK_DEVICE->present_family)
	{
		uint32_t families[2] = {VK_DEVICE->graphics_family, VK_DEVICE->present_family};
		create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		create_info.queueFamilyIndexCount = 2;
		create_info.pQueueFamilyIndices = families;
	}
	else
	{
		create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		create_info.queueFamilyIndexCount = 0;
		create_info.pQueueFamilyIndices = NULL;
	}
	create_info.preTransform = VK_DEVICE->surface_capabilities.currentTransform;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	create_info.presentMode = present_mode;
	create_info.clipped = VK_TRUE;
	create_info.oldSwapchain = VK_DEVICE->swap_chain;
	VkResult result = vkCreateSwapchainKHR(VK_DEVICE->vk_device, &create_info, NULL, &VK_DEVICE->swap_chain);
	if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("can't create swapchain: %s (%d)", vk_err2str(result), result);
		return false;
	}
	uint32_t images_count = 0;
	result = vkGetSwapchainImagesKHR(VK_DEVICE->vk_device, VK_DEVICE->swap_chain, &images_count, NULL);
	if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("can't get swapchain images count: %s (%d)", vk_err2str(result), result);
		return false;
	}
	VkImage *images = GFX_MALLOC(sizeof(*images) * images_count);
	if (!images)
	{
		GFX_ERROR_CALLBACK("can't allocate images: %s (%d)", strerror(errno), errno);
		return false;
	}
	result = vkGetSwapchainImagesKHR(VK_DEVICE->vk_device, VK_DEVICE->swap_chain, &images_count, images);
	if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("can't get swapchain images: %s (%d)", vk_err2str(result), result);
		return false;
	}
	GFX_FREE(VK_DEVICE->surface_images);
	VK_DEVICE->surface_images = images;
	VK_DEVICE->surface_images_count = images_count;
	return true;
}

static bool create_image_views(gfx_device_t *device)
{
	uint32_t image_views_count = VK_DEVICE->surface_images_count;
	VkImageView *image_views = GFX_MALLOC(sizeof(*image_views) * image_views_count);
	if (!image_views)
	{
		GFX_ERROR_CALLBACK("can't allocate image views: %s (%d)", strerror(errno), errno);
		return false;
	}
	for (uint32_t i = 0; i < image_views_count; ++i)
	{
		VkImageViewCreateInfo create_info;
		create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		create_info.pNext = NULL;
		create_info.flags = 0;
		create_info.image = VK_DEVICE->surface_images[i];
		create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		create_info.format = VK_DEVICE->swap_chain_format;
		create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.subresourceRange.aspectMask = 0;
		create_info.subresourceRange.baseMipLevel = 0;
		create_info.subresourceRange.levelCount = 1;
		create_info.subresourceRange.baseArrayLayer = 0;
		create_info.subresourceRange.layerCount = 1;
		VkResult result = vkCreateImageView(VK_DEVICE->vk_device, &create_info, NULL, &image_views[i]);
		if (result != VK_SUCCESS)
		{
			GFX_ERROR_CALLBACK("can't allocate image views: %s (%d)", vk_err2str(result), result);
			GFX_FREE(image_views);
			return false;
		}
	}
	VK_DEVICE->surface_image_views_count = image_views_count;
	VK_DEVICE->surface_image_views = image_views;
	return true;
}

static bool create_command_pool(gfx_device_t *device)
{
	VkCommandPoolCreateInfo create_info;
	create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	create_info.queueFamilyIndex = 0;
	VkResult result = vkCreateCommandPool(VK_DEVICE->vk_device, &create_info, NULL, &VK_DEVICE->command_pool);
	if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("can't create vulkan command pool: %s (%d)", vk_err2str(result), result);
		return false;
	}
	return true;
}

static bool create_command_buffers(gfx_device_t *device)
{
	VkCommandBufferAllocateInfo allocate_info;
	allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocate_info.pNext = NULL;
	allocate_info.commandPool = VK_DEVICE->command_pool;
	allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocate_info.commandBufferCount = 1;
	VkResult result = vkAllocateCommandBuffers(VK_DEVICE->vk_device, &allocate_info, &VK_DEVICE->command_buffer);
	if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("can't create vulkan command buffer: %s (%d)", vk_err2str(result), result);
		return false;
	}
	return true;
}

static bool vk_ctr(gfx_device_t *device, gfx_window_t *window)
{
	VkResult result;
	VK_DEVICE->present_mode = VK_PRESENT_MODE_FIFO_KHR;
	if (!gfx_device_vtable.ctr(device, window))
		return false;
	if (!get_physical_device(device))
		return false;
	if (!create_device(device))
		return false;
	if (!create_swapchain(device))
		return false;
	if (!create_image_views(device))
		return false;
	if (!create_command_pool(device))
		return false;
	if (!create_command_buffers(device))
		return false;
	return true;
}

static void vk_dtr(gfx_device_t *device)
{
	for (uint32_t i = 0; i < VK_DEVICE->surface_image_views_count; ++i)
		vkDestroyImageView(VK_DEVICE->vk_device, VK_DEVICE->surface_image_views[i], NULL);
	vkFreeCommandBuffers(VK_DEVICE->vk_device, VK_DEVICE->command_pool, 1, &VK_DEVICE->command_buffer);
	vkDestroyCommandPool(VK_DEVICE->vk_device, VK_DEVICE->command_pool, NULL);
	vkDestroySwapchainKHR(VK_DEVICE->vk_device, VK_DEVICE->swap_chain, NULL);
	vkDestroySurfaceKHR(VK_DEVICE->instance, VK_DEVICE->surface, NULL);
	vkDestroyInstance(VK_DEVICE->instance, NULL);
	vkDestroyDevice(VK_DEVICE->vk_device, NULL);
	GFX_FREE(VK_DEVICE->surface_image_views);
	GFX_FREE(VK_DEVICE->surface_formats);
	GFX_FREE(VK_DEVICE->surface_images);
	GFX_FREE(VK_DEVICE->present_modes);
	gfx_device_vtable.dtr(device);
}

static void vk_tick(gfx_device_t *device)
{
	gfx_device_vtable.tick(device);
}

static void vk_clear_color(gfx_device_t *device, const gfx_render_target_t *render_target, enum gfx_render_target_attachment attachment, vec4f_t color)
{
	VkClearColorValue vk_color;
	vk_color.float32[0] = color.x;
	vk_color.float32[1] = color.y;
	vk_color.float32[2] = color.z;
	vk_color.float32[3] = color.w;
	VkImageSubresourceRange range;
	//range.aspectMask = ;
	range.baseMipLevel = 0;
	//range.levelCount = ;
	//range.baseArrayLayer = ;
	//range.layerCount = ;
	//vkCmdClearColorImage(VK_DEVICE->command_buffer, image, image_layout, &vk_color, 1, &range);
}

static void vk_clear_depth_stencil(gfx_device_t *device, const gfx_render_target_t *render_target, float depth, uint8_t stencil)
{
	VkClearDepthStencilValue vk_depth_stencil;
	vk_depth_stencil.depth = depth;
	vk_depth_stencil.stencil = stencil;
	VkImageSubresourceRange range;
	//range.aspectMask = ;
	range.baseMipLevel = 0;
	//range.levelCount = ;
	//range.baseArrayLayer = ;
	//range.layerCount = ;
	//vkCmdClearDepthStencilImage(VK_DEVICE->command_buffer, image, image_layout, &vk_depth_stencil, 1, &range
}

static void vk_draw_indexed_instanced(gfx_device_t *device, enum gfx_primitive_type primitive, uint32_t count, uint32_t offset, uint32_t prim_count)
{
	vkCmdDrawIndexed(VK_DEVICE->command_buffer, count, prim_count, offset, 0, 0);
}

static void vk_draw_instanced(gfx_device_t *device, enum gfx_primitive_type primitive, uint32_t count, uint32_t offset, uint32_t prim_count)
{
	vkCmdDraw(VK_DEVICE->command_buffer, count, prim_count, offset, 0);
}

static void vk_draw_indexed(gfx_device_t *device, enum gfx_primitive_type primitive, uint32_t count, uint32_t offset)
{
	vkCmdDrawIndexed(VK_DEVICE->command_buffer, count, 1, offset, 0, 0);
}

static void vk_draw(gfx_device_t *device, enum gfx_primitive_type primitive, uint32_t count, uint32_t offset)
{
	vkCmdDraw(VK_DEVICE->command_buffer, count, 1, offset, 0);
}

static bool vk_create_blend_state(gfx_device_t *device, gfx_blend_state_t *state, bool enabled, enum gfx_blend_function src_c, enum gfx_blend_function dst_c, enum gfx_blend_function src_a, enum gfx_blend_function dst_a, enum gfx_blend_equation equation_c, enum gfx_blend_equation equation_a)
{
	//VkPipelineColorBlendStateCreateInfo
	return true;
}

static void vk_delete_blend_state(gfx_device_t *device, gfx_blend_state_t *state)
{
}

static bool vk_create_depth_stencil_state(gfx_device_t *device, gfx_depth_stencil_state_t *state, bool depth_write, bool depth_test, enum gfx_compare_function depth_compare, bool stencil_enabled, uint32_t stencil_write_mask, enum gfx_compare_function stencil_compare, uint32_t stencil_reference, uint32_t stencil_compare_mask, enum gfx_stencil_operation stencil_fail, enum gfx_stencil_operation stencil_zfail, enum gfx_stencil_operation stencil_pass)
{
	//VkPipelineDepthStencilStateCreateInfo
	return true;
}

static void vk_delete_depth_stencil_state(gfx_device_t *device, gfx_depth_stencil_state_t *state)
{
}

static bool vk_create_rasterizer_state(gfx_device_t *device, gfx_rasterizer_state_t *state, enum gfx_fill_mode fill_mode, enum gfx_cull_mode cull_mode, enum gfx_front_face front_face, bool scissor)
{
	//VkPipelineRasterizationStateCreateInfo
	return true;
}

static void vk_delete_rasterizer_state(gfx_device_t *device, gfx_rasterizer_state_t *state)
{
}

static bool vk_create_buffer(gfx_device_t *device, gfx_buffer_t *buffer, enum gfx_buffer_type type, const void *data, uint32_t size, enum gfx_buffer_usage usage)
{
	return true;
}

static void vk_set_buffer_data(gfx_device_t *device, gfx_buffer_t *buffer, const void *data, uint32_t size, uint32_t offset)
{
}

static void vk_delete_buffer(gfx_device_t *device, gfx_buffer_t *buffer)
{
}

static bool vk_create_attributes_state(gfx_device_t *device, gfx_attributes_state_t *state, const gfx_attribute_bind_t *binds, uint32_t count, const gfx_buffer_t *index_buffer, enum gfx_index_type index_type)
{
	//VkPipelineVertexInputStateCreateInfo
	return true;
}

static void vk_bind_attributes_state(gfx_device_t *device, const gfx_attributes_state_t *state, const gfx_input_layout_t *input_layout)
{
}

static void vk_delete_attributes_state(gfx_device_t *device, gfx_attributes_state_t *state)
{
}

static bool vk_create_input_layout(gfx_device_t *device, gfx_input_layout_t *input_layout, const gfx_input_layout_bind_t *binds, uint32_t count, const gfx_program_t *program)
{
	return true;
}

static void vk_delete_input_layout(gfx_device_t *device, gfx_input_layout_t *input_layout)
{
}

static bool vk_create_texture(gfx_device_t *device, gfx_texture_t *texture, enum gfx_texture_type type, enum gfx_format format, uint8_t lod, uint32_t width, uint32_t height, uint32_t depth)
{
	return true;
}

static void vk_set_texture_data(gfx_device_t *device, gfx_texture_t *texture, uint8_t lod, uint32_t offset, uint32_t width, uint32_t height, uint32_t depth, uint32_t size, const void *data)
{
}

static void vk_set_texture_addressing(gfx_device_t *device, gfx_texture_t *texture, enum gfx_texture_addressing addressing_s, enum gfx_texture_addressing addressing_t, enum gfx_texture_addressing addressing_r)
{
}

static void vk_set_texture_filtering(gfx_device_t *device, gfx_texture_t *texture, enum gfx_filtering min_filtering, enum gfx_filtering mag_filtering, enum gfx_filtering mip_filtering)
{
}

static void vk_set_texture_anisotropy(gfx_device_t *device, gfx_texture_t *texture, uint32_t anisotropy)
{
}

static void vk_set_texture_levels(gfx_device_t *device, gfx_texture_t *texture, uint32_t min_level, uint32_t max_level)
{
}

static void vk_delete_texture(gfx_device_t *device, gfx_texture_t *texture)
{
}

static bool vk_create_shader(gfx_device_t *device, gfx_shader_t *shader, enum gfx_shader_type type, const uint8_t *data, uint32_t len)
{
	VkShaderModuleCreateInfo create_info;
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = 0;
	create_info.codeSize = len;
	create_info.pCode = (const uint32_t*)data;
	VkShaderModule shaderModule;
	VkResult result = vkCreateShaderModule(VK_DEVICE->vk_device, &create_info, NULL, (VkShaderModule*)&shader->handle.ptr);
	if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("can't create shader module: %s (%d)", vk_err2str(result), result);
		return false;
	}
	return true;
}

static void vk_delete_shader(gfx_device_t *device, gfx_shader_t *shader)
{
	if (!shader || !shader->handle.ptr)
		return;
	vkDestroyShaderModule(VK_DEVICE->vk_device, (VkShaderModule)shader->handle.ptr, NULL);
	shader->handle.ptr = NULL;
}

static bool vk_create_program(gfx_device_t *device, gfx_program_t *program, const gfx_shader_t *vertex_shader, const gfx_shader_t *fragment_shader, const gfx_shader_t *geometry_shader, const gfx_program_attribute_t *attributes, const gfx_program_constant_t *constants, const gfx_program_sampler_t *samplers)
{
	assert(!program->handle.ptr);
	VkPipelineShaderStageCreateInfo vert_info;
	vert_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vert_info.pNext = NULL;
	vert_info.flags = 0;
	vert_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vert_info.module = (VkShaderModule)vertex_shader->handle.ptr;
	vert_info.pName = "main";
	VkPipelineShaderStageCreateInfo frag_info;
	frag_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	frag_info.pNext = NULL;
	frag_info.flags = 0;
	frag_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	frag_info.module = (VkShaderModule)fragment_shader->handle.ptr;
	frag_info.pName = "main";
	VkPipelineShaderStageCreateInfo shaderStages[] = {vert_info, frag_info};
	return true;
}

static void vk_delete_program(gfx_device_t *device, gfx_program_t *program)
{
	if (!program || !program->handle.ptr)
		return;
	program->handle.ptr = NULL;
}

static void vk_bind_constant(gfx_device_t *device, uint32_t bind, const gfx_buffer_t *buffer, uint32_t size, uint32_t offset)
{
	//vkCmdBindDescriptorSets
}

static void vk_bind_samplers(gfx_device_t *device, uint32_t start, uint32_t count, const gfx_texture_t **texture)
{
	//vkCmdBindDescriptorSets
}

static bool vk_create_render_target(gfx_device_t *device, gfx_render_target_t *render_target)
{
	return true;
}

static void vk_delete_render_target(gfx_device_t *device, gfx_render_target_t *render_target)
{
}

static void vk_bind_render_target(gfx_device_t *device, const gfx_render_target_t *render_target)
{
}

static void vk_set_render_target_texture(gfx_device_t *device, gfx_render_target_t *render_target, enum gfx_render_target_attachment attachment, const gfx_texture_t *texture)
{
}

static void vk_set_render_target_draw_buffers(gfx_device_t *device, gfx_render_target_t *render_target, uint32_t *render_buffers, uint32_t render_buffers_count)
{
}

static void vk_resolve_render_target(gfx_device_t *device, const gfx_render_target_t *src, const gfx_render_target_t *dst, uint32_t buffers, uint32_t color_src, uint32_t color_dst)
{
}

static bool vk_create_pipeline_state(gfx_device_t *device, gfx_pipeline_state_t *state, const gfx_program_t *program, const gfx_rasterizer_state_t *rasterizer, const gfx_depth_stencil_state_t *depth_stencil, const gfx_blend_state_t *blend, const gfx_input_layout_t *input_layout)
{
	//VkGraphicsPipelineCreateInfo
	//VkPipeline graphicsPipeline;
	//vkCreateGraphicsPipelines
}

static void vk_delete_pipeline_state(gfx_device_t *device, gfx_pipeline_state_t *state)
{
}

static void vk_bind_pipeline_state(gfx_device_t *device, const gfx_pipeline_state_t *state)
{
}

static void vk_set_viewport(gfx_device_t *device, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
	VkViewport viewports[8];
	for (size_t i = 0; i < 8; ++i)
	{
		viewports[i].x = x;
		viewports[i].y = y;
		viewports[i].width = width;
		viewports[i].height = height;
		viewports[i].minDepth = 0;
		viewports[i].maxDepth = 1;
	}
	vkCmdSetViewport(VK_DEVICE->command_buffer, 0, 8, viewports);
}

static void vk_set_scissor(gfx_device_t *device, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
	VkRect2D rect;
	rect.offset.x = x;
	rect.offset.y = y;
	rect.extent.width = width;
	rect.extent.height = height;
	vkCmdSetScissor(VK_DEVICE->command_buffer, 0, 1, &rect);
}

static void vk_set_line_width(gfx_device_t *device, float line_width)
{
	vkCmdSetLineWidth(VK_DEVICE->command_buffer, line_width);
}

static void vk_set_point_size(gfx_device_t *device, float point_size)
{
	(void)device;
	(void)point_size;
}

static gfx_device_vtable_t vk_vtable =
{
	GFX_DEVICE_VTABLE_DEF(vk)
};

void gfx_vk_set_swap_interval(gfx_device_t *device, int interval)
{
	if (interval < 0)
		VK_DEVICE->present_mode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
	else if (!interval)
		VK_DEVICE->present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
	else
		VK_DEVICE->present_mode = VK_PRESENT_MODE_FIFO_KHR;
	if (create_swapchain(device))
		create_image_views(device);
}

void gfx_vk_swap_buffers(gfx_device_t *device)
{
	//vkQueuePresentKHR
}

gfx_device_t *gfx_vk_device_new(gfx_window_t *window, VkInstance instance, VkSurfaceKHR surface)
{
	gfx_vk_device_t *device = GFX_MALLOC(sizeof(*device));
	if (!device)
		return NULL;
	memset(device, 0, sizeof(*device));
	gfx_device_t *dev = &device->device;
	dev->vtable = &vk_vtable;
	device->instance = instance;
	device->surface = surface;
	if (!dev->vtable->ctr(dev, window))
	{
		dev->vtable->dtr(dev);
		GFX_FREE(device);
		return NULL;
	}
	return dev;
}
