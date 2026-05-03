/*
 * vk-spoof-gpu -- Vulkan layer to spoof GPU name, ID, and vendor ID
 *
 * Copyright (C) 2026 Thomas Müller <contact@tom94.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>
#include <vulkan/vk_layer.h>

#define LAYER_NAME "VK_LAYER_spoof_gpu"

typedef struct {
    PFN_vkGetInstanceProcAddr pfn_get_instance_proc_addr;
    PFN_vkGetPhysicalDeviceProperties pfn_get_physical_device_properties;
    PFN_vkGetPhysicalDeviceProperties2 pfn_get_physical_device_properties2;
} instance_data;

/* single-instance shortcut — fine for game use */
static instance_data g_inst;

static void spoof_properties(VkPhysicalDeviceProperties *props)
{
    const char *vendor_id_str = getenv("SPOOF_VENDOR_ID");
    const char *device_id_str = getenv("SPOOF_DEVICE_ID");
    const char *device_name = getenv("SPOOF_DEVICE_NAME");

    if (vendor_id_str)
        props->vendorID = (uint32_t)strtoul(vendor_id_str, NULL, 0);

    if (device_id_str)
        props->deviceID = (uint32_t)strtoul(device_id_str, NULL, 0);

    if (device_name) {
        strncpy(props->deviceName, device_name,
            VK_MAX_PHYSICAL_DEVICE_NAME_SIZE - 1);
        props->deviceName[VK_MAX_PHYSICAL_DEVICE_NAME_SIZE - 1] = '\0';
    }
}

static VKAPI_ATTR void VKAPI_CALL spoof_GetPhysicalDeviceProperties(
    VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties *pProperties)
{
    g_inst.pfn_get_physical_device_properties(physicalDevice, pProperties);
    if (pProperties)
        spoof_properties(pProperties);
}

static VKAPI_ATTR void VKAPI_CALL spoof_GetPhysicalDeviceProperties2(
    VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2 *pProperties)
{
    g_inst.pfn_get_physical_device_properties2(physicalDevice, pProperties);
    if (pProperties)
        spoof_properties(&pProperties->properties);
}

static VKAPI_ATTR VkResult VKAPI_CALL spoof_CreateInstance(
    const VkInstanceCreateInfo *pCreateInfo,
    const VkAllocationCallbacks *pAllocator, VkInstance *pInstance)
{
    VkLayerInstanceCreateInfo *chain_info = (VkLayerInstanceCreateInfo *)pCreateInfo->pNext;
    while (chain_info &&
           !(chain_info->sType == VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO &&
             chain_info->function == VK_LAYER_LINK_INFO)) {
        chain_info = (VkLayerInstanceCreateInfo *)chain_info->pNext;
    }

    if (!chain_info)
        return VK_ERROR_INITIALIZATION_FAILED;

    PFN_vkGetInstanceProcAddr pfn_next = chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
    chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;

    PFN_vkCreateInstance pfn_create = (PFN_vkCreateInstance)pfn_next(NULL, "vkCreateInstance");
    VkResult ret = pfn_create(pCreateInfo, pAllocator, pInstance);
    if (ret != VK_SUCCESS)
        return ret;

    g_inst.pfn_get_instance_proc_addr = pfn_next;
    g_inst.pfn_get_physical_device_properties =
        (PFN_vkGetPhysicalDeviceProperties)pfn_next(*pInstance, "vkGetPhysicalDeviceProperties");
    g_inst.pfn_get_physical_device_properties2 =
        (PFN_vkGetPhysicalDeviceProperties2)pfn_next(*pInstance, "vkGetPhysicalDeviceProperties2");

    return VK_SUCCESS;
}

static VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL spoof_GetInstanceProcAddr(
    VkInstance instance, const char *pName)
{
    if (!strcmp(pName, "vkCreateInstance"))
        return (PFN_vkVoidFunction)spoof_CreateInstance;
    if (!strcmp(pName, "vkGetPhysicalDeviceProperties"))
        return (PFN_vkVoidFunction)spoof_GetPhysicalDeviceProperties;
    if (!strcmp(pName, "vkGetPhysicalDeviceProperties2"))
        return (PFN_vkVoidFunction)spoof_GetPhysicalDeviceProperties2;
    if (!strcmp(pName, "vkGetPhysicalDeviceProperties2KHR"))
        return (PFN_vkVoidFunction)spoof_GetPhysicalDeviceProperties2;
    if (!strcmp(pName, "vkGetInstanceProcAddr"))
        return (PFN_vkVoidFunction)spoof_GetInstanceProcAddr;

    if (g_inst.pfn_get_instance_proc_addr)
        return g_inst.pfn_get_instance_proc_addr(instance, pName);

    return NULL;
}

/* Layer entry points — exported */

VKAPI_ATTR VkResult VKAPI_CALL
vkNegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface *pVersionStruct)
{
    pVersionStruct->pfnGetInstanceProcAddr = spoof_GetInstanceProcAddr;
    pVersionStruct->pfnGetDeviceProcAddr = NULL;
    pVersionStruct->pfnGetPhysicalDeviceProcAddr = NULL;
    if (pVersionStruct->loaderLayerInterfaceVersion > 2)
        pVersionStruct->loaderLayerInterfaceVersion = 2;
    return VK_SUCCESS;
}
