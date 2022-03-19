#include "vk.h"
#include "../device_vtable.h"
#include "../window.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <errno.h>

#define VK_DEVICE ((gfx_vk_device_t*)device)

static const VkFormat attribute_types[] =
{
	VK_FORMAT_R32G32B32A32_SFLOAT,
	VK_FORMAT_R32G32B32A32_UINT,
	VK_FORMAT_R32G32B32A32_SINT,
	VK_FORMAT_R32G32B32_SFLOAT,
	VK_FORMAT_R32G32B32_UINT,
	VK_FORMAT_R32G32B32_SINT,
	VK_FORMAT_R32G32_SFLOAT,
	VK_FORMAT_R32G32_UINT,
	VK_FORMAT_R32G32_SINT,
	VK_FORMAT_R32_SFLOAT,
	VK_FORMAT_R32_UINT,
	VK_FORMAT_R32_SINT,
	VK_FORMAT_R16G16B16A16_SFLOAT,
	VK_FORMAT_R16G16B16A16_UNORM,
	VK_FORMAT_R16G16B16A16_SNORM,
	VK_FORMAT_R16G16B16A16_UINT,
	VK_FORMAT_R16G16B16A16_SINT,
	VK_FORMAT_R16G16_SFLOAT,
	VK_FORMAT_R16G16_UNORM,
	VK_FORMAT_R16G16_SNORM,
	VK_FORMAT_R16G16_UINT,
	VK_FORMAT_R16G16_SINT,
	VK_FORMAT_R8G8B8A8_UNORM,
	VK_FORMAT_R8G8B8A8_SNORM,
	VK_FORMAT_R8G8B8A8_UINT,
	VK_FORMAT_R8G8B8A8_SINT,
	VK_FORMAT_R8G8_UNORM,
	VK_FORMAT_R8G8_SNORM,
	VK_FORMAT_R8G8_UINT,
	VK_FORMAT_R8G8_SINT,
	VK_FORMAT_R8_UNORM,
	VK_FORMAT_R8_SNORM,
	VK_FORMAT_R8_UINT,
	VK_FORMAT_R8_SINT,
};

static const VkIndexType index_types[] =
{
	VK_INDEX_TYPE_UINT16,
	VK_INDEX_TYPE_UINT32,
};

static const VkCompareOp compare_functions[] =
{
	VK_COMPARE_OP_NEVER,
	VK_COMPARE_OP_LESS,
	VK_COMPARE_OP_LESS_OR_EQUAL,
	VK_COMPARE_OP_EQUAL,
	VK_COMPARE_OP_GREATER_OR_EQUAL,
	VK_COMPARE_OP_GREATER,
	VK_COMPARE_OP_NOT_EQUAL,
	VK_COMPARE_OP_ALWAYS,
};

static const VkBlendFactor blend_functions[] =
{
	VK_BLEND_FACTOR_ZERO,
	VK_BLEND_FACTOR_ONE,
	VK_BLEND_FACTOR_SRC_COLOR,
	VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
	VK_BLEND_FACTOR_DST_COLOR,
	VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
	VK_BLEND_FACTOR_SRC_ALPHA,
	VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
	VK_BLEND_FACTOR_DST_ALPHA,
	VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
	VK_BLEND_FACTOR_CONSTANT_COLOR,
	VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,
};

static const VkBlendOp blend_equations[] =
{
	VK_BLEND_OP_ADD,
	VK_BLEND_OP_SUBTRACT,
	VK_BLEND_OP_REVERSE_SUBTRACT,
	VK_BLEND_OP_MIN,
	VK_BLEND_OP_MAX,
};

static const VkStencilOp stencil_operations[] =
{
	VK_STENCIL_OP_KEEP,
	VK_STENCIL_OP_ZERO,
	VK_STENCIL_OP_REPLACE,
	VK_STENCIL_OP_INCREMENT_AND_CLAMP,
	VK_STENCIL_OP_INCREMENT_AND_WRAP,
	VK_STENCIL_OP_DECREMENT_AND_CLAMP,
	VK_STENCIL_OP_DECREMENT_AND_WRAP,
	VK_STENCIL_OP_INVERT,
};

static const VkPrimitiveTopology primitive_types[] =
{
	VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
	VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
};

static const VkPolygonMode fill_modes[] =
{
	VK_POLYGON_MODE_POINT,
	VK_POLYGON_MODE_LINE,
	VK_POLYGON_MODE_FILL,
};

static VkCullModeFlagBits cull_modes[] =
{
	VK_CULL_MODE_NONE,
	VK_CULL_MODE_FRONT_BIT,
	VK_CULL_MODE_BACK_BIT,
};

static const VkFrontFace front_faces[] =
{
	VK_FRONT_FACE_CLOCKWISE,
	VK_FRONT_FACE_COUNTER_CLOCKWISE,
};

static const VkColorComponentFlagBits color_masks[] =
{
	0,
	VK_COLOR_COMPONENT_R_BIT,
	VK_COLOR_COMPONENT_G_BIT,
	VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_R_BIT,
	VK_COLOR_COMPONENT_B_BIT,
	VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_R_BIT,
	VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_G_BIT,
	VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_R_BIT,
	VK_COLOR_COMPONENT_A_BIT,
	VK_COLOR_COMPONENT_A_BIT | VK_COLOR_COMPONENT_R_BIT,
	VK_COLOR_COMPONENT_A_BIT | VK_COLOR_COMPONENT_G_BIT,
	VK_COLOR_COMPONENT_A_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_R_BIT,
	VK_COLOR_COMPONENT_A_BIT | VK_COLOR_COMPONENT_B_BIT,
	VK_COLOR_COMPONENT_A_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_R_BIT,
	VK_COLOR_COMPONENT_A_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_G_BIT,
	VK_COLOR_COMPONENT_A_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_R_BIT,
};

typedef struct gfx_vk_device_s
{
	gfx_device_t device;
	VkAllocationCallbacks allocation_callbacks;
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
	enum gfx_primitive_type primitive;
} gfx_vk_device_t;

#define ALLOCATION_CALLBACKS NULL //&VK_DEVICE->allocation_callbacks

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
	}

	return "unknown error";
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
		//if (device_properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		//	continue;
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
	VkResult result = vkCreateDevice(VK_DEVICE->physical_device, &create_info, ALLOCATION_CALLBACKS, &VK_DEVICE->vk_device);
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
	VkResult result = vkCreateSwapchainKHR(VK_DEVICE->vk_device, &create_info, ALLOCATION_CALLBACKS, &VK_DEVICE->swap_chain);
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
		VkResult result = vkCreateImageView(VK_DEVICE->vk_device, &create_info, ALLOCATION_CALLBACKS, &image_views[i]);
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
	VkResult result = vkCreateCommandPool(VK_DEVICE->vk_device, &create_info, ALLOCATION_CALLBACKS, &VK_DEVICE->command_pool);
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
		vkDestroyImageView(VK_DEVICE->vk_device, VK_DEVICE->surface_image_views[i], ALLOCATION_CALLBACKS);
	vkFreeCommandBuffers(VK_DEVICE->vk_device, VK_DEVICE->command_pool, 1, &VK_DEVICE->command_buffer);
	vkDestroyCommandPool(VK_DEVICE->vk_device, VK_DEVICE->command_pool, ALLOCATION_CALLBACKS);
	vkDestroySwapchainKHR(VK_DEVICE->vk_device, VK_DEVICE->swap_chain, ALLOCATION_CALLBACKS);
	vkDestroySurfaceKHR(VK_DEVICE->instance, VK_DEVICE->surface, ALLOCATION_CALLBACKS);
	vkDestroyInstance(VK_DEVICE->instance, NULL); /* XXX: allocation callbacks */
	vkDestroyDevice(VK_DEVICE->vk_device, NULL); /* XXX: allocation callbacks */
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
	range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	range.baseMipLevel = 0;
	range.levelCount = 1;
	range.baseArrayLayer = 0;
	range.layerCount = 1;
	for (uint32_t i = 0; i < VK_DEVICE->surface_images_count; ++i)
		vkCmdClearColorImage(VK_DEVICE->command_buffer, VK_DEVICE->surface_images[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &vk_color, 1, &range);
}

static void vk_clear_depth_stencil(gfx_device_t *device, const gfx_render_target_t *render_target, float depth, uint8_t stencil)
{
	VkClearDepthStencilValue vk_depth_stencil;
	vk_depth_stencil.depth = depth;
	vk_depth_stencil.stencil = stencil;
	VkImageSubresourceRange range;
	range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	range.baseMipLevel = 0;
	range.levelCount = 1;
	range.baseArrayLayer = 0;
	range.layerCount = 1;
	//vkCmdClearDepthStencilImage(VK_DEVICE->command_buffer, image, image_layout, &vk_depth_stencil, 1, &range
}

static void vk_draw_indexed_instanced(gfx_device_t *device, uint32_t count, uint32_t offset, uint32_t prim_count)
{
	vkCmdDrawIndexed(VK_DEVICE->command_buffer, count, prim_count, offset, 0, 0);
#ifndef NDEBUG
	switch (VK_DEVICE->primitive)
	{
		case GFX_PRIMITIVE_TRIANGLES:
			device->triangles_count += count / 3 * prim_count;
			break;
		case GFX_PRIMITIVE_POINTS:
			device->points_count += count * prim_count;
			break;
		case GFX_PRIMITIVE_LINES:
			device->lines_count += count / 2 * prim_count;
			break;
	}
	device->draw_calls_count++;
#endif
}

static void vk_draw_instanced(gfx_device_t *device, uint32_t count, uint32_t offset, uint32_t prim_count)
{
	vkCmdDraw(VK_DEVICE->command_buffer, count, prim_count, offset, 0);
#ifndef NDEBUG
	switch (VK_DEVICE->primitive)
	{
		case GFX_PRIMITIVE_TRIANGLES:
			device->triangles_count += count / 3;
			break;
		case GFX_PRIMITIVE_POINTS:
			device->points_count += count;
			break;
		case GFX_PRIMITIVE_LINES:
			device->lines_count += count / 2;
			break;
	}
	device->draw_calls_count++;
#endif
}

static void vk_draw_indexed(gfx_device_t *device, uint32_t count, uint32_t offset)
{
	vkCmdDrawIndexed(VK_DEVICE->command_buffer, count, 1, offset, 0, 0);
#ifndef NDEBUG
	switch (VK_DEVICE->primitive)
	{
		case GFX_PRIMITIVE_TRIANGLES:
			device->triangles_count += count / 3;
			break;
		case GFX_PRIMITIVE_POINTS:
			device->points_count += count;
			break;
		case GFX_PRIMITIVE_LINES:
			device->lines_count += count / 2;
			break;
	}
	device->draw_calls_count++;
#endif
}

static void vk_draw(gfx_device_t *device, uint32_t count, uint32_t offset)
{
	vkCmdDraw(VK_DEVICE->command_buffer, count, 1, offset, 0);
#ifndef NDEBUG
	switch (VK_DEVICE->primitive)
	{
		case GFX_PRIMITIVE_TRIANGLES:
			device->triangles_count += count / 3;
			break;
		case GFX_PRIMITIVE_POINTS:
			device->points_count += count;
			break;
		case GFX_PRIMITIVE_LINES:
			device->lines_count += count / 2;
			break;
	}
	device->draw_calls_count++;
#endif
}

static bool vk_create_blend_state(gfx_device_t *device, gfx_blend_state_t *state, bool enabled, enum gfx_blend_function src_c, enum gfx_blend_function dst_c, enum gfx_blend_function src_a, enum gfx_blend_function dst_a, enum gfx_blend_equation equation_c, enum gfx_blend_equation equation_a, enum gfx_color_mask color_mask)
{
	assert(!state->handle.u64);
	state->device = device;
	state->handle.ptr = (void*)1;
	state->enabled = enabled;
	state->src_c = src_c;
	state->dst_c = dst_c;
	state->src_a = src_a;
	state->dst_a = dst_a;
	state->equation_c = equation_c;
	state->equation_a = equation_a;
	state->color_mask = color_mask;
	return true;
}

static void vk_delete_blend_state(gfx_device_t *device, gfx_blend_state_t *state)
{
	(void)device;
	if (!state || !state->handle.ptr)
		return;
	state->handle.ptr = NULL;
}

static bool vk_create_depth_stencil_state(gfx_device_t *device, gfx_depth_stencil_state_t *state, bool depth_write, bool depth_test, enum gfx_compare_function depth_compare, bool stencil_enabled, uint32_t stencil_write_mask, enum gfx_compare_function stencil_compare, uint32_t stencil_reference, uint32_t stencil_compare_mask, enum gfx_stencil_operation stencil_fail, enum gfx_stencil_operation stencil_zfail, enum gfx_stencil_operation stencil_pass)
{
	assert(!state->handle.u64);
	state->device = device;
	state->handle.ptr = (void*)1;
	state->depth_write = depth_write;
	state->depth_test = depth_test;
	state->depth_compare = depth_compare;
	state->stencil_enabled = stencil_enabled;
	state->stencil_write_mask = stencil_write_mask;
	state->stencil_compare = stencil_compare;
	state->stencil_reference = stencil_reference;
	state->stencil_compare_mask = stencil_compare_mask;
	state->stencil_fail = stencil_fail;
	state->stencil_zfail = stencil_zfail;
	state->stencil_pass = stencil_pass;
	return true;
}

static void vk_delete_depth_stencil_state(gfx_device_t *device, gfx_depth_stencil_state_t *state)
{
	if (!state || !state->handle.ptr)
		return;
	state->handle.ptr = NULL;
}

static bool vk_create_rasterizer_state(gfx_device_t *device, gfx_rasterizer_state_t *state, enum gfx_fill_mode fill_mode, enum gfx_cull_mode cull_mode, enum gfx_front_face front_face, bool scissor)
{
	assert(!state->handle.ptr);
	state->device = device;
	state->handle.ptr = (void*)1;
	state->fill_mode = fill_mode;
	state->cull_mode = cull_mode;
	state->front_face = front_face;
	state->scissor = scissor;
	return true;
}

static void vk_delete_rasterizer_state(gfx_device_t *device, gfx_rasterizer_state_t *state)
{
	if (!state || !state->handle.ptr)
		return;
	state->handle.ptr = NULL;
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
	assert(!state->handle.ptr);
	state->handle.ptr = (void*)1;
	memcpy(state->binds, binds, sizeof(*binds) * count);
	state->count = count;
	state->index_type = index_type;
	state->index_buffer = index_buffer;
	return true;
}

static void vk_bind_attributes_state(gfx_device_t *device, const gfx_attributes_state_t *state, const gfx_input_layout_t *input_layout)
{
	assert(state->handle.ptr);
	VkBuffer buffers[8];
	VkDeviceSize offsets[8];
	for (uint32_t i = 0; i < state->count; ++i)
	{
		buffers[i] = state->binds[i].buffer->handle.ptr;
		offsets[i] = state->binds[i].offset;
	}
	vkCmdBindVertexBuffers(VK_DEVICE->command_buffer, 0, state->count, buffers, offsets);
	if (state->index_buffer)
		vkCmdBindIndexBuffer(VK_DEVICE->command_buffer, state->index_buffer->handle.ptr, 0, index_types[state->index_type]);
}

static void vk_delete_attributes_state(gfx_device_t *device, gfx_attributes_state_t *state)
{
	if (!state || !state->handle.ptr)
		return;
	state->handle.ptr = NULL;
}

static bool vk_create_input_layout(gfx_device_t *device, gfx_input_layout_t *input_layout, const gfx_input_layout_bind_t *binds, uint32_t count, const gfx_shader_state_t *shader_state)
{
	(void)shader_state;
	assert(!input_layout->handle.ptr);
	input_layout->handle.ptr = (void*)1;
	input_layout->device = device;
	memcpy(input_layout->binds, binds, sizeof(*binds) * count);
	input_layout->count = count;
	return true;
}

static void vk_delete_input_layout(gfx_device_t *device, gfx_input_layout_t *input_layout)
{
	(void)device;
	if (!input_layout || !input_layout->handle.u64)
		return;
	input_layout->handle.u64 = 0;
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
	VkShaderModuleCreateInfo shader_create_info;
	shader_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shader_create_info.pNext = NULL;
	shader_create_info.flags = 0;
	shader_create_info.codeSize = len;
	shader_create_info.pCode = (const uint32_t*)data;
	VkShaderModule shaderModule;
	VkResult result = vkCreateShaderModule(VK_DEVICE->vk_device, &shader_create_info, ALLOCATION_CALLBACKS, (VkShaderModule*)&shader->handle.ptr);
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
	vkDestroyShaderModule(VK_DEVICE->vk_device, (VkShaderModule)shader->handle.ptr, ALLOCATION_CALLBACKS);
	shader->handle.ptr = NULL;
}

static bool vk_create_shader_state(gfx_device_t *device, gfx_shader_state_t *shader_state, const gfx_shader_t **shaders, uint32_t shaders_count, const gfx_shader_attribute_t *attributes, const gfx_shader_constant_t *constants, const gfx_shader_sampler_t *samplers)
{
	VkResult result;
	assert(!shader_state->handle.ptr);
	uint32_t constant_layouts_count = 0;
	uint32_t sampler_layouts_count = 0;
	while (constants[constant_layouts_count].name)
		constant_layouts_count++;
	while (samplers[sampler_layouts_count].name)
		sampler_layouts_count++;
	VkDescriptorSetLayout *layouts = GFX_MALLOC(sizeof(*layouts) * (sampler_layouts_count + constant_layouts_count));
	if (!layouts)
	{
		GFX_ERROR_CALLBACK("can't allocate layouts: %s (%d)", strerror(errno), errno);
		return false;
	}
	for (uint32_t i = 0; i < constant_layouts_count; ++i)
	{
		VkDescriptorSetLayoutBinding binding;
		binding.binding = i;
		binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		binding.descriptorCount = 1;
		binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
		binding.pImmutableSamplers = NULL;

		VkDescriptorSetLayoutCreateInfo layout_create_info;
		layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layout_create_info.pNext = NULL;
		layout_create_info.flags = 0;
		layout_create_info.bindingCount = 1;
		layout_create_info.pBindings = &binding;
		result = vkCreateDescriptorSetLayout(VK_DEVICE->vk_device, &layout_create_info, ALLOCATION_CALLBACKS, (VkDescriptorSetLayout*)&layouts[i]);
		if (result != VK_SUCCESS)
		{
			/* XXX: release all previously created */
			GFX_ERROR_CALLBACK("can't create descriptor set layout: %s (%d)", vk_err2str(result), result);
			return false;
		}
	}
	for (uint32_t i = 0; i < sampler_layouts_count; ++i)
	{
		VkDescriptorSetLayoutBinding binding;
		binding.binding = i;
		binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		binding.descriptorCount = 1;
		binding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
		binding.pImmutableSamplers = NULL;

		VkDescriptorSetLayoutCreateInfo layout_create_info;
		layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layout_create_info.pNext = NULL;
		layout_create_info.flags = 0;
		layout_create_info.bindingCount = 1;
		layout_create_info.pBindings = &binding;
		result = vkCreateDescriptorSetLayout(VK_DEVICE->vk_device, &layout_create_info, ALLOCATION_CALLBACKS, (VkDescriptorSetLayout*)&layouts[constant_layouts_count + i]);
		if (result != VK_SUCCESS)
		{
			/* XXX: release all previously created */
			GFX_ERROR_CALLBACK("can't create descriptor set layout: %s (%d)", vk_err2str(result), result);
			return false;
		}
	}

	VkPipelineLayoutCreateInfo pipeline_create_info;
	pipeline_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_create_info.pNext = NULL;
	pipeline_create_info.flags = 0;
	pipeline_create_info.setLayoutCount = constant_layouts_count + sampler_layouts_count;
	pipeline_create_info.pSetLayouts = layouts;
	pipeline_create_info.pushConstantRangeCount = 0;
	pipeline_create_info.pPushConstantRanges = NULL;
	result = vkCreatePipelineLayout(VK_DEVICE->vk_device, &pipeline_create_info, ALLOCATION_CALLBACKS, (VkPipelineLayout*)&shader_state->pipeline_layout.ptr);
	if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("can't create pipeline layout: %s (%d)", vk_err2str(result), result);
		return false;
	}
	return true;
}

static void vk_delete_shader_state(gfx_device_t *device, gfx_shader_state_t *shader_state)
{
	if (!shader_state || !shader_state->handle.ptr)
		return;
	vkDestroyPipelineLayout(VK_DEVICE->vk_device, (VkPipelineLayout)shader_state->pipeline_layout.ptr, ALLOCATION_CALLBACKS);
	shader_state->handle.ptr = NULL;
}

static void vk_bind_constant(gfx_device_t *device, uint32_t bind, const gfx_buffer_t *buffer, uint32_t size, uint32_t offset)
{
	//vkCmdBindDescriptorSets(VK_DEVICE->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, bind, 1, &descriptor_sets[i], 1, &offset);
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

static bool vk_create_pipeline_state(gfx_device_t *device, gfx_pipeline_state_t *state, const gfx_shader_state_t *shader_state, const gfx_rasterizer_state_t *rasterizer, const gfx_depth_stencil_state_t *depth_stencil, const gfx_blend_state_t *blend, const gfx_input_layout_t *input_layout, enum gfx_primitive_type primitive)
{
	assert(state && !state->handle.ptr);
	state->primitive = primitive;

	uint32_t shader_stages_count = 2;
	VkPipelineShaderStageCreateInfo shader_stages[3];
	shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stages[0].pNext = NULL;
	shader_stages[0].flags = 0;
	shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shader_stages[0].module = (VkShaderModule)shader_state->vertex_shader.ptr;
	shader_stages[0].pName = "main";
	shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stages[1].pNext = NULL;
	shader_stages[1].flags = 0;
	shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shader_stages[1].module = (VkShaderModule)shader_state->fragment_shader.ptr;
	shader_stages[1].pName = "main";
	if (shader_state->geometry_shader.ptr)
	{
		shader_stages_count++;
		shader_stages[2].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[2].pNext = NULL;
		shader_stages[2].flags = 0;
		shader_stages[2].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shader_stages[2].module = (VkShaderModule)shader_state->geometry_shader.ptr;
		shader_stages[2].pName = "main";
	}

	VkVertexInputAttributeDescription input_attribute_descriptions[8];
	VkVertexInputBindingDescription input_binding_descriptions[8];
	for (uint32_t i = 0; i < input_layout->count; ++i)
	{
		input_attribute_descriptions[i].location = i;
		input_attribute_descriptions[i].binding = i;
		input_attribute_descriptions[i].format = attribute_types[input_layout->binds[i].type];
		input_attribute_descriptions[i].offset = input_layout->binds[i].offset;
		input_binding_descriptions[i].binding = i;
		input_binding_descriptions[i].stride = input_layout->binds[i].stride;
		input_binding_descriptions[i].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	}

	VkPipelineVertexInputStateCreateInfo vertex_input_create_info;
	vertex_input_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_create_info.pNext = NULL;
	vertex_input_create_info.flags = 0;
	vertex_input_create_info.vertexBindingDescriptionCount = input_layout->count;
	vertex_input_create_info.pVertexBindingDescriptions = input_binding_descriptions;
	vertex_input_create_info.vertexAttributeDescriptionCount = input_layout->count;
	vertex_input_create_info.pVertexAttributeDescriptions = input_attribute_descriptions;

	VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info;
	input_assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly_create_info.pNext = NULL;
	input_assembly_create_info.flags = 0;
	input_assembly_create_info.topology = primitive_types[primitive];
	input_assembly_create_info.primitiveRestartEnable = VK_FALSE;

	VkPipelineRasterizationStateCreateInfo rasterization_create_info;
	rasterization_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterization_create_info.pNext = NULL;
	rasterization_create_info.flags = 0;
	rasterization_create_info.depthClampEnable = VK_TRUE;
	rasterization_create_info.rasterizerDiscardEnable = VK_TRUE;
	rasterization_create_info.polygonMode = fill_modes[rasterizer->fill_mode];
	rasterization_create_info.cullMode = cull_modes[rasterizer->cull_mode];
	rasterization_create_info.frontFace = front_faces[rasterizer->front_face];
	rasterization_create_info.depthBiasEnable = VK_FALSE;
	rasterization_create_info.depthBiasConstantFactor = 0;
	rasterization_create_info.depthBiasClamp = 0;
	rasterization_create_info.depthBiasSlopeFactor = 0;
	rasterization_create_info.lineWidth = 1;

	VkPipelineMultisampleStateCreateInfo multisample_create_info;
	multisample_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisample_create_info.pNext = NULL;
	multisample_create_info.flags = 0;
	multisample_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; //XXX
	multisample_create_info.sampleShadingEnable = VK_FALSE; //XXX
	multisample_create_info.minSampleShading = 1; //XXX
	multisample_create_info.pSampleMask = NULL; //XXX
	multisample_create_info.alphaToCoverageEnable = VK_FALSE; //XXX
	multisample_create_info.alphaToOneEnable = VK_FALSE; //XXX

	VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info;
	depth_stencil_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil_create_info.pNext = NULL;
	depth_stencil_create_info.flags = 0;
	depth_stencil_create_info.depthTestEnable = depth_stencil->depth_test;
	depth_stencil_create_info.depthWriteEnable = depth_stencil->depth_write;
	depth_stencil_create_info.depthCompareOp = compare_functions[depth_stencil->depth_compare];
	depth_stencil_create_info.depthBoundsTestEnable = VK_TRUE;
	depth_stencil_create_info.stencilTestEnable = depth_stencil->stencil_enabled;
	depth_stencil_create_info.front.failOp = stencil_operations[depth_stencil->stencil_fail];
	depth_stencil_create_info.front.passOp = stencil_operations[depth_stencil->stencil_pass];
	depth_stencil_create_info.front.depthFailOp = stencil_operations[depth_stencil->stencil_zfail];
	depth_stencil_create_info.front.compareOp = compare_functions[depth_stencil->stencil_compare];
	depth_stencil_create_info.front.compareMask = depth_stencil->stencil_compare_mask;
	depth_stencil_create_info.front.writeMask = depth_stencil->stencil_write_mask;
	depth_stencil_create_info.front.reference = depth_stencil->stencil_reference;
	depth_stencil_create_info.back.failOp = stencil_operations[depth_stencil->stencil_fail];
	depth_stencil_create_info.back.passOp = stencil_operations[depth_stencil->stencil_pass];
	depth_stencil_create_info.back.depthFailOp = stencil_operations[depth_stencil->stencil_zfail];
	depth_stencil_create_info.back.compareOp = compare_functions[depth_stencil->stencil_compare];
	depth_stencil_create_info.back.compareMask = depth_stencil->stencil_compare_mask;
	depth_stencil_create_info.back.writeMask = depth_stencil->stencil_write_mask;
	depth_stencil_create_info.back.reference = depth_stencil->stencil_reference;
	depth_stencil_create_info.minDepthBounds = 0;
	depth_stencil_create_info.maxDepthBounds = 1;

	VkPipelineColorBlendAttachmentState color_blend_attachment;
	color_blend_attachment.blendEnable = blend->enabled;
	color_blend_attachment.srcColorBlendFactor = blend_functions[blend->src_c];
	color_blend_attachment.dstColorBlendFactor = blend_functions[blend->dst_c];
	color_blend_attachment.colorBlendOp = blend_equations[blend->equation_c];
	color_blend_attachment.srcAlphaBlendFactor = blend_functions[blend->src_a];
	color_blend_attachment.dstAlphaBlendFactor = blend_functions[blend->dst_a];
	color_blend_attachment.alphaBlendOp = blend_equations[blend->equation_a];
	color_blend_attachment.colorWriteMask = color_masks[blend->color_mask];

	VkPipelineColorBlendStateCreateInfo color_blend_create_info;
	color_blend_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blend_create_info.pNext = NULL;
	color_blend_create_info.flags = 0;
	color_blend_create_info.logicOpEnable = VK_FALSE;
	color_blend_create_info.logicOp = 0;
	color_blend_create_info.attachmentCount = 1;
	color_blend_create_info.pAttachments = &color_blend_attachment;
	color_blend_create_info.blendConstants[0] = 1;
	color_blend_create_info.blendConstants[1] = 1;
	color_blend_create_info.blendConstants[2] = 1;
	color_blend_create_info.blendConstants[3] = 1;

	VkDynamicState dynamic_states[] =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
		VK_DYNAMIC_STATE_LINE_WIDTH,
	};

	VkPipelineDynamicStateCreateInfo dynamic_state_create_info;
	dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state_create_info.pNext = NULL;
	dynamic_state_create_info.flags = 0;
	dynamic_state_create_info.dynamicStateCount = sizeof(dynamic_states) / sizeof(*dynamic_states);
	dynamic_state_create_info.pDynamicStates = dynamic_states;

	VkGraphicsPipelineCreateInfo create_info;
	create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	create_info.pNext = NULL;
	create_info.flags = 0;
	create_info.stageCount = shader_stages_count;
	create_info.pStages = shader_stages;
	create_info.pVertexInputState = &vertex_input_create_info;
	create_info.pInputAssemblyState = &input_assembly_create_info;
	create_info.pTessellationState = NULL;
	create_info.pViewportState = NULL;
	create_info.pRasterizationState = &rasterization_create_info;
	create_info.pMultisampleState = &multisample_create_info;
	create_info.pDepthStencilState = &depth_stencil_create_info;
	create_info.pColorBlendState = &color_blend_create_info;
	create_info.pDynamicState = &dynamic_state_create_info;
	create_info.layout = (VkPipelineLayout)shader_state->pipeline_layout.ptr;
	create_info.renderPass = ;
	create_info.subpass = ;
	create_info.basePipelineHandle = VK_NULL_HANDLE;
	create_info.basePipelineIndex = -1;
	VkResult result = vkCreateGraphicsPipelines(VK_DEVICE->vk_device, VK_NULL_HANDLE, 1, &create_info, NULL, (VkPipeline*)&state->handle.ptr);
	if (result != VK_SUCCESS)
	{
		GFX_ERROR_CALLBACK("can't create graphics pipeline: %s (%d)", vk_err2str(result), result);
		return false;
	}
	return true;
}

static void vk_delete_pipeline_state(gfx_device_t *device, gfx_pipeline_state_t *state)
{
	if (!state || !state->handle.ptr)
		return;
	vkDestroyPipeline(VK_DEVICE->vk_device, (VkPipeline)state->handle.ptr, NULL);
	state->handle.ptr = NULL;
}

static void vk_bind_pipeline_state(gfx_device_t *device, const gfx_pipeline_state_t *state)
{
	assert(state);
	VK_DEVICE->primitive = state->primitive;
	vkCmdBindPipeline(VK_DEVICE->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, (VkPipeline)state->handle.ptr);
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

static const gfx_device_vtable_t vk_vtable =
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

static void *vk_allocation(void *userdata, size_t size, size_t alignment, VkSystemAllocationScope scope)
{
	uint8_t *addr = GFX_MALLOC(size + alignment);
	size_t delta = (size_t)addr % alignment;
	if (!delta)
		return addr;
	return addr - delta + alignment;
}

static void *vk_realloaction(void *userdata, void *original, size_t size, size_t alignment, VkSystemAllocationScope scope)
{
	uint8_t *addr = GFX_REALLOC(original, size + alignment);
	size_t delta = (size_t)addr % alignment;
	if (!delta)
		return addr;
	return addr - delta + alignment;
}

static void vk_free(void *userdata, void *memory)
{
	GFX_FREE(memory);
}

static void vk_internal_allocation(void *userdata, size_t size, VkInternalAllocationType type, VkSystemAllocationScope scope)
{
}

static void vk_internal_free(void *userdata, size_t size, VkInternalAllocationType type, VkSystemAllocationScope scope)
{
}

gfx_device_t *gfx_vk_device_new(gfx_window_t *window, VkInstance instance, VkSurfaceKHR surface)
{
	gfx_vk_device_t *device = GFX_MALLOC(sizeof(*device));
	if (!device)
		return NULL;
	memset(device, 0, sizeof(*device));
	device->allocation_callbacks.pUserData = NULL;
	device->allocation_callbacks.pfnAllocation = NULL;//vk_allocation;
	device->allocation_callbacks.pfnReallocation = NULL;//vk_realloaction;
	device->allocation_callbacks.pfnFree = NULL;//vk_free;
	device->allocation_callbacks.pfnInternalAllocation = NULL;//vk_internal_allocation;
	device->allocation_callbacks.pfnInternalFree = NULL;//vk_internal_free;
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
