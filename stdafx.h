#pragma once

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <pdh.h>
#include <psapi.h>
#ifdef _DEBUG
#include <iostream>
#endif
#endif

#ifdef __linux__
#include <iostream>
#include <unistd.h>
#include <limits.h>
#endif
#include <string>
#include <vector>
#include <set>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>

#include "openvr.h"

#include "SFML/GpuPreference.hpp"
#include "SFML/Graphics.hpp"

#include "ScreenCapture.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"

#include "pugixml.hpp"
