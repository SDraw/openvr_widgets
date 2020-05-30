#include "stdafx.h"

extern const float g_Pi = glm::pi<float>();
extern const float g_PiHalf = g_Pi*0.5f;
extern const glm::mat4 g_IdentityMatrix(1.f);
extern const glm::ivec2 g_EmptyIVector2(0);
extern const glm::vec3 g_EmptyVector(0.f);
extern const glm::quat g_EmptyQuat(1.f, 0.f, 0.f, 0.f);
extern const glm::vec4 g_ZeroPoint(0.f, 0.f, 0.f, 1.f);
extern const glm::vec3 g_AxisX(1.f, 0.f, 0.f);
extern const glm::vec3 g_AxisY(0.f, 1.f, 0.f);
extern const glm::vec3 g_AxisZN(0.f, 0.f, -1.f);
extern const sf::Color g_ClearColor(0U, 0U, 0U, 127U);
extern const unsigned char g_DummyTextureData[] = {
    0x7FU, 0x7FU, 0x7FU, 0xFF,
    0xF7U, 0x94U, 0x1DU, 0xFF,
    0xF7U, 0x94U, 0x1DU, 0xFF,
    0x7FU, 0x7FU, 0x7FU, 0xFF
};
