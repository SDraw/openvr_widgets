#include "stdafx.h"

extern const float g_pi = glm::pi<float>();
extern const float g_piHalf = g_pi*0.5f;
extern const glm::mat4 g_identityMatrix(1.f);
extern const glm::ivec2 g_emptyIVector2(0);
extern const glm::vec3 g_emptyVector(0.f);
extern const glm::quat g_emptyQuat(1.f, 0.f, 0.f, 0.f);
extern const glm::vec4 g_zeroPoint(0.f, 0.f, 0.f, 1.f);
extern const glm::vec3 g_axisX(1.f, 0.f, 0.f);
extern const glm::vec3 g_axisY(0.f, 1.f, 0.f);
extern const glm::vec3 g_axisZN(0.f, 0.f, -1.f);
extern const sf::Color g_clearColor(0U, 0U, 0U, 127U);
extern const unsigned char g_dummyTextureData[]
{
    0x7FU, 0x7FU, 0x7FU, 0xFF,
    0xF7U, 0x94U, 0x1DU, 0xFF,
    0xF7U, 0x94U, 0x1DU, 0xFF,
    0x7FU, 0x7FU, 0x7FU, 0xFF
};
