#include "stdafx.h"

#include "Core/VRTransform.h"
#include "Utils/Utils.h"

extern const glm::mat4 g_IdentityMatrix;
extern const glm::vec3 g_EmptyVector;
extern const glm::quat g_EmptyQuat;
extern const glm::vec4 g_ZeroPoint;

glm::vec3 VRTransform::ms_hmdPos = g_EmptyVector;
glm::quat VRTransform::ms_hmdRot = g_EmptyQuat;
glm::mat4 VRTransform::ms_hmdMat = g_IdentityMatrix;
glm::vec3 VRTransform::ms_leftHandPos = g_EmptyVector;
glm::quat VRTransform::ms_leftHandRot = g_EmptyQuat;
glm::mat4 VRTransform::ms_leftHandMat = g_IdentityMatrix;
glm::vec3 VRTransform::ms_rightHandPos = g_EmptyVector;
glm::quat VRTransform::ms_rightHandRot = g_EmptyQuat;
glm::mat4 VRTransform::ms_rightHandMat = g_IdentityMatrix;

void VRTransform::SetHmdTransformation(const vr::HmdMatrix34_t &f_mat)
{
    ConvertMatrix(f_mat, ms_hmdMat);
    ms_hmdPos = ms_hmdMat*g_ZeroPoint;
    ms_hmdRot = glm::quat_cast(ms_hmdMat);
}
void VRTransform::SetLeftHandTransformation(const vr::HmdMatrix34_t &f_mat)
{
    ConvertMatrix(f_mat, ms_leftHandMat);
    ms_leftHandPos = ms_leftHandMat*g_ZeroPoint;
    ms_leftHandRot = glm::quat_cast(ms_leftHandMat);
}
void VRTransform::SetRightHandTransformation(const vr::HmdMatrix34_t &f_mat)
{
    ConvertMatrix(f_mat, ms_rightHandMat);
    ms_rightHandPos = ms_rightHandMat*g_ZeroPoint;
    ms_rightHandRot = glm::quat_cast(ms_rightHandMat);
}
