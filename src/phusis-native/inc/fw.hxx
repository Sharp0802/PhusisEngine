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

#include <unistd.h>
#include <threads.h>

#include <vulkan/vulkan.h>

#define CCTOR \
friend class cctor; \
struct cctor \
{ \
    cctor(); \
}; \
static cctor __cctor

#endif //PHUSISENGINE_FW_HXX
