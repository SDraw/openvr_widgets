#pragma once

void ConvertMatrix(const vr::HmdMatrix34_t &f_matVR, glm::mat4 &f_mat);
void ConvertMatrix(const glm::mat4 &f_mat, vr::HmdMatrix34_t &f_matVR);
void GetRotationToPoint(const glm::vec3 &f_pointA, const glm::vec3 &f_pointB, const glm::quat &f_rotationA, glm::quat &f_result);

size_t ReadEnumVector(const std::string &f_val, const std::vector<std::string> &f_vec);
size_t ReadEnumVector(const char *f_val, const std::vector<std::string> &f_vec);

void ExtractScreenCaptureImage(const SL::Screen_Capture::Image &f_img, unsigned char *f_dst, size_t f_size);

#ifdef __linux__
unsigned long long GetTickCount64();
#endif
