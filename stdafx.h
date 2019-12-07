#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <string>
#include <vector>
#include <chrono>
#include <thread>

#include "openvr.h"

#include "SFML/GpuPreference.hpp"
#include "SFML/Graphics.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"

#include "pugixml.hpp"
