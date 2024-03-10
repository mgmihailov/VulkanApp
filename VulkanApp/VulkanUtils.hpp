#pragma once

#include <volk/volk.h>


namespace Lava
{
    static void DebugUtilsMessageSeverityFlagToString(VkDebugUtilsMessageSeverityFlagBitsEXT severity, std::string& outSeverityString)
    {
        switch (severity)
        {
            case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            {
                outSeverityString = "VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT";
            }
            break;
            case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            {
                outSeverityString = "VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT";
            }
            break;
            case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            {
                outSeverityString = "VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT";
            }
            break;
            case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            {
                outSeverityString = "VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT";
            }
            break;
        }
    }

    static void DebugUtilsMessageTypeFlagsToString(VkDebugUtilsMessageTypeFlagsEXT types, std::string& outTypesString)
    {
        if ((types & VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) != 0)
        {
            outTypesString.append("VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT, ");
        }

        if ((types & VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) != 0)
        {
            outTypesString.append("VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT, ");
        }

        if ((types & VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) != 0)
        {
            outTypesString.append("VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT, ");
        }

        if ((types & VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT) != 0)
        {
            outTypesString.append("VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT, ");
        }

        outTypesString.erase(outTypesString.size() - 2);
    }
}