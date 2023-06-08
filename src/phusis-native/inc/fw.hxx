#ifndef PHUSISENGINE_FW_HXX
#define PHUSISENGINE_FW_HXX

#include <cstdalign>
#include <cstdarg>
#include <cstdbool>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include <iostream>

#include <array>
#include <vector>
#include <string>
#include <set>
#include <map>
#include <sstream>
#include <atomic>
#include <filesystem>
#include <algorithm>
#include <thread>
#include <queue>
#include <future>

#include <unistd.h>
#include <threads.h>

#define GLFW_INCLUDE_VULKAN
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#undef GLFW_INCLUDE_VULKAN

#include <glm/glm.hpp>

#define DLLEXPORT __attribute__((visibility("default")))

#define CCTOR \
friend class cctor; \
struct cctor \
{ \
    cctor(); \
}; \
static cctor __cctor

#endif //PHUSISENGINE_FW_HXX
