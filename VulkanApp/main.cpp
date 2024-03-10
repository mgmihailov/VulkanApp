#include <Precompiled.hpp>

#include <windows.h>

#include <iostream>
#include <unordered_map>

#define VOLK_IMPLEMENTATION

#include <volk/volk.h>

#define LAVA_ENABLE_LOGGING
#include <Logger.hpp>
#include <VulkanUtils.hpp>


std::unordered_map<void*, std::pair<int, int>> g_allocFreeMap;

void InitializeCustomAllocator(VkAllocationCallbacks& outAllocator, void* userData)
{
    outAllocator.pUserData = userData;

    outAllocator.pfnAllocation = [](void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) -> void*
    {
        VkApplicationInfo* appInfo = reinterpret_cast<VkApplicationInfo*>(pUserData);
        size_t alignedSize = sizeof(size_t) + alignment * ((size / alignment) + 1 * ((size % alignment) != 0));
        LAVA_LOG_DEBUG("[Allocator] Allocation by custom allocator for app '%s' - %zu bytes aligned size (%zu bytes size, %zu bytes alignment, %d allocation scope)",
            appInfo->pApplicationName, alignedSize, size, alignment, allocationScope);
        void* newMemory = malloc(alignedSize);

        size_t* asSizeT = reinterpret_cast<size_t*>(newMemory);
        *asSizeT = alignedSize;
        LAVA_LOG_DEBUG("[Allocator] Returning address %p (address plus magic: %p)", asSizeT + 1, newMemory);

        g_allocFreeMap[reinterpret_cast<void*>(asSizeT + 1)].first++;

        return asSizeT + 1;
    };

    outAllocator.pfnReallocation = [](void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope) -> void*
    {
        VkApplicationInfo* appInfo = reinterpret_cast<VkApplicationInfo*>(pUserData);
        size_t alignedSize = sizeof(size_t) + alignment * ((size / alignment) + 1 * ((size % alignment) != 0));
        LAVA_LOG_DEBUG("[Allocator][ThreadID: %ul] Reallocation by custom allocator for app '%s' - %zu bytes aligned size"\
            "(%p original address, %zu bytes size, %zu bytes alignment, %d allocation scope)",
            GetCurrentThreadId(), appInfo->pApplicationName, alignedSize, pOriginal, size, alignment, allocationScope);
        void* newMemory = malloc(alignedSize);
        size_t* asSizeT = reinterpret_cast<size_t*>(newMemory);
        *asSizeT = alignedSize;
        LAVA_LOG_DEBUG("[Allocator] Returning new address %p (address plus magic: %p)", asSizeT + 1, newMemory);
        g_allocFreeMap[reinterpret_cast<void*>(asSizeT + 1)].first++;

        size_t* originalAsSizeT = reinterpret_cast<size_t*>(pOriginal) - 1;
        memcpy((void*)(asSizeT + 1), pOriginal, (((*originalAsSizeT - sizeof(size_t)) < (alignedSize - sizeof(size_t))) ? (*originalAsSizeT - sizeof(size_t)) : (alignedSize - sizeof(size_t))));
        free(originalAsSizeT);
        g_allocFreeMap[pOriginal].second++;
        originalAsSizeT = nullptr;

        return asSizeT + 1;
    };

    outAllocator.pfnFree = [](void* pUserData, void* pMemory)
    {
        VkApplicationInfo* appInfo = reinterpret_cast<VkApplicationInfo*>(pUserData);

        if (pMemory == nullptr)
        {
            LAVA_LOG_DEBUG("[Allocator][ThreadID: %ul] Free by custom allocator for app'%s' - nullptr memory address detected", GetCurrentThreadId(), appInfo->pApplicationName);
            free(pMemory);
            return;
        }

        size_t* asSizeT = reinterpret_cast<size_t*>(pMemory) - 1;
        LAVA_LOG_DEBUG("[Allocator][ThreadID: %ul] Free by custom allocator for app '%s' - %p memory address (%zu bytes, address plus magic: %p)",
            GetCurrentThreadId(), appInfo->pApplicationName, pMemory, *asSizeT, asSizeT);
        free(asSizeT);
        g_allocFreeMap[pMemory].second++;
        asSizeT = nullptr;
    };

    outAllocator.pfnInternalAllocation = [](void* pUserData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope)
    {
        VkApplicationInfo* appInfo = reinterpret_cast<VkApplicationInfo*>(pUserData);
        LAVA_LOG_DEBUG("[Allocator] Internal allocation notification by custom allocator for app '%s' - %zu bytes allocation size, %d allocation type, %d allocation scope",
            appInfo->pApplicationName, size, allocationType, allocationScope);
    };

    outAllocator.pfnInternalFree = [](void* pUserData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope allocationScope)
    {
        VkApplicationInfo* appInfo = reinterpret_cast<VkApplicationInfo*>(pUserData);
        LAVA_LOG_DEBUG("[Allocator] Internal free notification by custom allocator for app '%s' - %zu bytes allocation size, %d allocation type, %d allocation scope",
            appInfo->pApplicationName, size, allocationType, allocationScope);
    };
}

void PrintCustomAllocatorStats()
{
    size_t allocFreeDiscrepancies = 0;
    size_t allocFreeSymmetries = 0;
    for (const auto& pair : g_allocFreeMap)
    {
        if (pair.second.first - pair.second.second != 0)
        {
            ++allocFreeDiscrepancies;
        }
        else
        {
            ++allocFreeSymmetries;
        }
    }

    LAVA_LOG_DEBUG("Alloc / free discrepancies: %zu", allocFreeDiscrepancies);
    LAVA_LOG_DEBUG("Alloc / free symmetries: %zu", allocFreeSymmetries);
}

void InitializeDebugUtilsMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& utilsMessenger, void* userData, void* next)
{
    utilsMessenger.pNext = next;
    utilsMessenger.flags = 0;
    VkDebugUtilsMessageSeverityFlagsEXT severityFlags = VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
        | VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
        | VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
        | VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    utilsMessenger.messageSeverity = severityFlags;
    VkDebugUtilsMessageTypeFlagsEXT typeFlags = VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
        | VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
        | VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
        /*| VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT*/; // TODO: Check if this one is available?
    utilsMessenger.messageType = typeFlags;
    utilsMessenger.pfnUserCallback = [](VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageTypes,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) -> VkBool32
    {
        std::string messageSeverityStr;
        Lava::DebugUtilsMessageSeverityFlagToString(messageSeverity, messageSeverityStr);
        std::string messageTypesStr;
        Lava::DebugUtilsMessageTypeFlagsToString(messageTypes, messageTypesStr);
        LAVA_LOG_DEBUG("[DEBUG_UTILS_MESSENGER_CREATE_INFO] Message severity: %s, message types: %s",
            messageSeverityStr.c_str(),
            messageTypesStr.c_str());

        return VK_FALSE;
    };
    utilsMessenger.pUserData = userData;
}

void InitializeDebugReportCallbackCreateInfo(VkDebugReportCallbackCreateInfoEXT& reportCallback, void* userData, void* next)
{
    reportCallback.pNext = next;
    VkDebugReportFlagsEXT flags = VkDebugReportFlagBitsEXT::VK_DEBUG_REPORT_INFORMATION_BIT_EXT
        | VkDebugReportFlagBitsEXT::VK_DEBUG_REPORT_WARNING_BIT_EXT
        | VkDebugReportFlagBitsEXT::VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT
        | VkDebugReportFlagBitsEXT::VK_DEBUG_REPORT_ERROR_BIT_EXT
        | VkDebugReportFlagBitsEXT::VK_DEBUG_REPORT_DEBUG_BIT_EXT;
    reportCallback.flags = flags;
    reportCallback.pfnCallback = [](VkDebugReportFlagsEXT flags,
        VkDebugReportObjectTypeEXT objectType,
        uint64_t object,
        size_t location,
        int32_t messageCode,
        const char* pLayerPrefix,
        const char* pMessage,
        void* pUserData) -> VkBool32
    {
        LAVA_LOG_DEBUG("[DEBUG_CALLBACK_CREATE_INSTANCE] Flags: %d, Object type: %d, Layer prefix: %s, Message: %s",
            flags, objectType, pLayerPrefix, pMessage);

        return VK_FALSE;
    };
    reportCallback.pUserData = userData;
}

int main(int argc, char** argv)
{
    auto res = volkInitialize();

    if (res != VK_SUCCESS)
    {
        LAVA_LOG_ERROR("Failed to initialize Volk!Make sure it's installed on your system.");
        return 1;
    }
    else
    {
        LAVA_LOG_INFO("Volk successfully initialized!");
    }

    // Query instance-level functionality supported by the implementation
    // ================================
    uint32_t apiVersion;
    vkEnumerateInstanceVersion(&apiVersion);

    LAVA_LOG_INFO("Instance-level functionality supported by the implementation %d.%d.%d.",
        VK_API_VERSION_MAJOR(apiVersion), VK_API_VERSION_MINOR(apiVersion), VK_API_VERSION_PATCH(apiVersion));
    // ================================

    // Query available layers
    // ================================
    uint32_t layersCount;
    res = vkEnumerateInstanceLayerProperties(&layersCount, nullptr);

    if (res != VK_SUCCESS)
    {
        LAVA_LOG_ERROR("Got an error while enumerating layer count. Result: %d", res);
    }

    std::vector<VkLayerProperties> layerProps(layersCount);
    vkEnumerateInstanceLayerProperties(&layersCount, layerProps.data());

    if (res != VK_SUCCESS)
    {
        LAVA_LOG_ERROR("Got an error while enumerating layers. Result: %d", res);
    }

    LAVA_LOG_INFO("Layers count: %d. Printing them all:", layersCount);

    for (size_t i = 0; i < layersCount; ++i)
    {
        // Query instance extensions per layer
        // ================================
        uint32_t extensionsCount;
        vkEnumerateInstanceExtensionProperties(layerProps[i].layerName, &extensionsCount, nullptr);
        std::vector<VkExtensionProperties> extensions(extensionsCount);
        vkEnumerateInstanceExtensionProperties(layerProps[i].layerName, &extensionsCount, extensions.data());
        LAVA_LOG_INFO("Found %d extensions for layer '%s'. Printing them all:", extensionsCount, layerProps[i].layerName);
        for (size_t j = 0; j < extensionsCount; ++j)
        {
            LAVA_LOG_INFO("%s", extensions[j].extensionName);
        }
        // ================================
    }
    //================================

    // Query extensions supported by the Vulkan implementation or implicitly
    // enabled layers
    //================================
    uint32_t extensionsCount;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionsCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, extensions.data());
    LAVA_LOG_INFO("Found %d extensions. Printing them all:", extensionsCount);
    for (size_t i = 0; i < extensionsCount; ++i)
    {
        LAVA_LOG_INFO("%s", extensions[i].extensionName);
    }
    //================================

    VkApplicationInfo vkAppInfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
    vkAppInfo.pNext = nullptr;
    vkAppInfo.pApplicationName = "Vulkan App";
    vkAppInfo.applicationVersion = 0;
    vkAppInfo.pEngineName = "Lava";
    vkAppInfo.engineVersion = 0;
    vkAppInfo.apiVersion = apiVersion; // Use the max supported one

    // Create debug utils messenger during VkCreateInstance
    //================================
    VkDebugUtilsMessengerCreateInfoEXT utilsMessenger{ VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
    InitializeDebugUtilsMessengerCreateInfo(utilsMessenger, &vkAppInfo, nullptr);
    //================================

    // Create debug report callback during VkCreateInstance
    //================================
    VkDebugReportCallbackCreateInfoEXT reportCallback{ VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT };
    InitializeDebugReportCallbackCreateInfo(reportCallback, &vkAppInfo, &utilsMessenger); // TODO: Add the other debug layer as pNext
    //================================

    const size_t EXTENSION_COUNT = 1;
    const char* enabledExtensions[EXTENSION_COUNT] = { "VK_EXT_debug_report" };

    VkInstanceCreateInfo vkInstanceCreateInfo{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    vkInstanceCreateInfo.pNext = &reportCallback;
    vkInstanceCreateInfo.flags = 0;
    vkInstanceCreateInfo.pApplicationInfo = &vkAppInfo;
    vkInstanceCreateInfo.enabledLayerCount = 0; // Fill in later
    vkInstanceCreateInfo.ppEnabledLayerNames = nullptr; // Fill in later
    vkInstanceCreateInfo.enabledExtensionCount = EXTENSION_COUNT;
    vkInstanceCreateInfo.ppEnabledExtensionNames = enabledExtensions;

    //VkAllocationCallbacks vkAllocator{};
    //InitializeCustomAllocator(vkAllocator, &vkAppInfo);

    VkInstance vkInstance;
    res = vkCreateInstance(&vkInstanceCreateInfo, nullptr/*&vkAllocator*/, &vkInstance);

    if (res != VK_SUCCESS)
    {
        LAVA_LOG_ERROR("Couldn't create Vulkan instance. Error: %d", res);
        return 1;
    }
    else
    {
        LAVA_LOG_INFO("Successfully created Vulkan instance!");
    }

    volkLoadInstance(vkInstance);

    vkDestroyInstance(vkInstance, nullptr/*&vkAllocator*/);

    LAVA_LOG_INFO("Successfully destroyed Vulkan instance!");

    //PrintCustomAllocatorStats();

    return 0;
}