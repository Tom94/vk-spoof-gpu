cmake_minimum_required(VERSION 3.10)
project(vulkan-spoof-layer C)

find_package(PkgConfig REQUIRED)
pkg_check_modules(VULKAN REQUIRED vulkan)

add_library(VkLayer_spoof_gpu SHARED spoof_gpu.c)
target_include_directories(VkLayer_spoof_gpu PRIVATE ${VULKAN_INCLUDE_DIRS})
target_link_libraries(VkLayer_spoof_gpu PRIVATE ${CMAKE_DL_LIBS})
set_target_properties(VkLayer_spoof_gpu PROPERTIES
    C_STANDARD 11
    PREFIX ""
    LINK_FLAGS "-Wl,-Bsymbolic,--exclude-libs,ALL"
)

install(TARGETS VkLayer_spoof_gpu LIBRARY DESTINATION lib)
install(FILES VkLayer_spoof_gpu.json DESTINATION share/vulkan/explicit_layer.d)
