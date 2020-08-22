#pragma once

enum VRDeviceIndex : size_t
{
    VDI_Hmd = 0U,
    VDI_LeftController,
    VDI_RightController,
    VDI_FirstTracker,
    VDI_SecondTracker,
    VDI_ThirdTracker,

    VDI_Max
};

class VRDevicesStates final
{
    static glm::vec3 ms_devicePos[VRDeviceIndex::VDI_Max];
    static glm::quat ms_deviceRot[VRDeviceIndex::VDI_Max];
    static glm::mat4 ms_deviceMat[VRDeviceIndex::VDI_Max];
    static float ms_devicePower[VRDeviceIndex::VDI_Max];
public:
    static void GetDevicePosition(const size_t f_device, glm::vec3 &f_pos);
    static void GetDeviceRotation(const size_t f_device, glm::quat &f_rot);
    static void GetDeviceMatrix(const size_t f_device, glm::mat4 &f_mat);
    static const float GetDevicePower(const size_t f_device);
protected:
    static void SetDeviceTransformation(const size_t f_device, const vr::HmdMatrix34_t &f_mat);
    static void SetDevicePower(const size_t f_device, const float f_pow);

    friend class Core;
};
