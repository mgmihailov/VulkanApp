#pragma once

namespace Lava
{

#ifdef LAVA_ENABLE_LOGGING
#define LAVA_LOG(severity, fmt, ...) printf("[" severity "] " fmt "\n", __VA_ARGS__)
#else
#define LAVA_LOG(severity, fmt, ...)
#endif

#define LAVA_LOG_INFO(fmt, ...) LAVA_LOG("INFO", fmt, __VA_ARGS__)
#define LAVA_LOG_VERBOSE(fmt, ...) LAVA_LOG("VERBOSE", fmt, __VA_ARGS__)
#define LAVA_LOG_DEBUG(fmt, ...) LAVA_LOG("DEBUG", fmt, __VA_ARGS__)
#define LAVA_LOG_WARNING(fmt, ...) LAVA_LOG("WARNING", fmt, __VA_ARGS__)
#define LAVA_LOG_ERROR(fmt, ...) LAVA_LOG("ERROR", fmt, __VA_ARGS__)

}
