project(VulkanEngine)

set(SHADER_DIR ${CMAKE_SOURCE_DIR}/base/shaders)
file(GLOB SHADERS ${SHADER_DIR}/*.vert ${SHADER_DIR}/*.frag)

foreach(SHADER IN LISTS SHADERS)
    get_filename_component(FILENAME ${SHADER} NAME)
    add_custom_command(OUTPUT ${SHADER_DIR}/${FILENAME}.spv
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${SHADER} -o ${SHADER_DIR}/${FILENAME}.spv
        DEPENDS ${SHADER}
        COMMENT "Compiling ${FILENAME}")
list(APPEND SPV_SHADERS ${SHADER_DIR}/${FILENAME}.spv)
endForeach()

add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})

set(SOURCES backend/Vulkan/VulkanInstance.cpp
			backend/Vulkan/PhysicalDevice.cpp
			backend/Vulkan/SwapChain.cpp
			backend/Vulkan/Buffer.cpp
			backend/Vulkan/VulkanBackend.cpp
			backend/Vulkan/Framebuffer.cpp
			backend/Vulkan/RenderPass.cpp
			backend/Vulkan/Pipeline.cpp
			backend/Vulkan/CommandBuffer.cpp
			backend/Vulkan/ShaderUnit.cpp
			backend/Vulkan/DescriptorPool.cpp
			frontend/frontend.cpp
			app/Application.cpp
			main.cpp
			geometry/Model.cpp)

add_executable(VulkanEngine ${SOURCES} ${SHADERS})

make_directory(${CMAKE_CURRENT_BINARY_DIR}/base)
make_directory(${CMAKE_CURRENT_BINARY_DIR}/base/shaders)

foreach(SHADER IN LISTS SHADERS)
	get_filename_component(FILENAME ${SHADER} NAME)
	add_custom_command(TARGET VulkanEngine POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy ${SHADER}.spv ${CMAKE_BINARY_DIR}/base/shaders/${FILENAME}.spv
	)
endforeach()

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

include_directories(${CMAKE_SOURCE_DIR} ${VULKAN_INCLUDE_DIRS})
target_link_libraries(VulkanEngine Vulkan::Vulkan glfw)