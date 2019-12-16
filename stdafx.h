#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <pdh.h>
#include <psapi.h>

#ifdef _DEBUG
#include <iostream>
#endif
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>

#include "openvr.h"

#include "SFML/GpuPreference.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/OpenGL.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"

#include "pugixml.hpp"

#include "ScreenCapture.h"
