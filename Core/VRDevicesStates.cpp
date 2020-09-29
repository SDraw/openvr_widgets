#include "stdafx.h"

#include "Core/VRDevicesStates.h"
#include "Utils/Utils.h"

extern const glm::mat4 g_identityMatrix;
extern const glm::vec3 g_emptyVector;
extern const glm::quat g_emptyQuat;
extern const glm::vec4 g_zeroPoint;

glm::vec3 VRDevicesStates::ms_devicePos[] = { g_emptyVector };
glm::quat VRDevicesStates::ms_deviceRot[] = { g_emptyQuat };
glm::mat4 VRDevicesStates::ms_deviceMat[] = { g_identityMatrix };
float VRDevicesStates::ms_devicePower[] = { 0.f };

void VRDevicesStates::SetDeviceTransformation(const size_t f_device, const vr::HmdMatrix34_t &f_mat)
{
    if(f_device < VRDeviceIndex::VDI_Max)
    {
        ConvertMatrix(f_mat, ms_deviceMat[f_device]);
        ms_devicePos[f_device] = ms_deviceMat[f_device] * g_zeroPoint;
        ms_deviceRot[f_device] = glm::quat_cast(ms_deviceMat[f_device]);
    }
}

void VRDevicesStates::GetDevicePosition(const size_t f_device, glm::vec3 &f_pos)
{
    if(f_device < VRDeviceIndex::VDI_Max) std::memcpy(&f_pos, &ms_devicePos[f_device], sizeof(glm::vec3));
}

void VRDevicesStates::GetDeviceRotation(const size_t f_device, glm::quat &f_rot)
{
    if(f_device < VRDeviceIndex::VDI_Max) std::memcpy(&f_rot, &ms_deviceRot[f_device], sizeof(glm::quat));
}

void VRDevicesStates::GetDeviceMatrix(const size_t f_device, glm::mat4 &f_mat)
{
    if(f_device < VRDeviceIndex::VDI_Max) std::memcpy(&f_mat, &ms_deviceMat[f_device], sizeof(glm::mat4));
}

void VRDevicesStates::SetDevicePower(const size_t f_device, const float f_pow)
{
    if(f_device < VRDeviceIndex::VDI_Max) ms_devicePower[f_device] = f_pow;
}

const float VRDevicesStates::GetDevicePower(const size_t f_device)
{
    float l_result = 0.f;
    if(f_device < VRDeviceIndex::VDI_Max) l_result = ms_devicePower[f_device];
    return l_result;
}
