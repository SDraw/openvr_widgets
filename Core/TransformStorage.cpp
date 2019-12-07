#include "stdafx.h"

#include "Core/TransformStorage.h"
#include "Utils/Utils.h"

extern const glm::mat4 g_IdentityMatrix;
extern const glm::vec3 g_EmptyVector;
extern const glm::quat g_EmptyQuat;
extern const glm::vec4 g_ZeroPoint;

glm::vec3 TransformStorage::ms_hmdPos = g_EmptyVector;
glm::quat TransformStorage::ms_hmdRot = g_EmptyQuat;
glm::mat4 TransformStorage::ms_hmdMat = g_IdentityMatrix;
glm::vec3 TransformStorage::ms_leftHandPos = g_EmptyVector;
glm::quat TransformStorage::ms_leftHandRot = g_EmptyQuat;
glm::mat4 TransformStorage::ms_leftHandMat = g_IdentityMatrix;
glm::vec3 TransformStorage::ms_rightHandPos = g_EmptyVector;
glm::quat TransformStorage::ms_rightHandRot = g_EmptyQuat;
glm::mat4 TransformStorage::ms_rightHandMat = g_IdentityMatrix;

void TransformStorage::SetHmdTransformation(const vr::HmdMatrix34_t &f_mat)
{
    ConvertMatrix(f_mat, ms_hmdMat);
    ms_hmdPos = ms_hmdMat*g_ZeroPoint;
    ms_hmdRot = glm::quat_cast(ms_hmdMat);
}
void TransformStorage::SetLeftHandTransformation(const vr::HmdMatrix34_t &f_mat)
{
    ConvertMatrix(f_mat, ms_leftHandMat);
    ms_leftHandPos = ms_leftHandMat*g_ZeroPoint;
    ms_leftHandRot = glm::quat_cast(ms_leftHandMat);
}
void TransformStorage::SetRightHandTransformation(const vr::HmdMatrix34_t &f_mat)
{
    ConvertMatrix(f_mat, ms_rightHandMat);
    ms_rightHandPos = ms_rightHandMat*g_ZeroPoint;
    ms_rightHandRot = glm::quat_cast(ms_rightHandMat);
}
