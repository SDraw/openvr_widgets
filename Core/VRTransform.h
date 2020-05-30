#pragma once
class VRTransform final
{
    static glm::vec3 ms_hmdPos;
    static glm::quat ms_hmdRot;
    static glm::mat4 ms_hmdMat;

    static glm::vec3 ms_leftHandPos;
    static glm::quat ms_leftHandRot;
    static glm::mat4 ms_leftHandMat;
    static float ms_leftHandPower;

    static glm::vec3 ms_rightHandPos;
    static glm::quat ms_rightHandRot;
    static glm::mat4 ms_rightHandMat;
    static float ms_rightHandPower;
public:
    static inline const glm::vec3& GetHmdPosition() { return ms_hmdPos; }
    static inline const glm::quat& GetHmdRotation() { return ms_hmdRot; }
    static inline const glm::mat4& GetHmdMatrix() { return ms_hmdMat; }

    static inline const glm::vec3& GetLeftHandPosition() { return ms_leftHandPos; }
    static inline const glm::quat& GetLeftHandRotation() { return ms_leftHandRot; }
    static inline const glm::mat4& GetLeftHandMatrix() { return ms_leftHandMat; }
    static inline const float GetLeftHandPower() { return ms_leftHandPower; }

    static inline const glm::vec3& GetRightHandPosition() { return ms_rightHandPos; }
    static inline const glm::quat& GetRightHandRotation() { return ms_rightHandRot; }
    static inline const glm::mat4& GetRightHandMatrix() { return ms_rightHandMat; }
    static inline const float GetRightHandPower() { return ms_rightHandPower; }
protected:
    static void SetHmdTransformation(const vr::HmdMatrix34_t &f_mat);

    static void SetLeftHandTransformation(const vr::HmdMatrix34_t &f_mat);
    static void SetLeftHandPower(float f_pow);

    static void SetRightHandTransformation(const vr::HmdMatrix34_t &f_mat);
    static void SetRightHandPower(float f_pow);

    friend class Core;
};
