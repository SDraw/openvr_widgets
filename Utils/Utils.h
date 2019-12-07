#pragma once

void ConvertMatrix(const vr::HmdMatrix34_t &f_matVR, glm::mat4 &f_mat);
void ConvertMatrix(const glm::mat4 &f_mat, vr::HmdMatrix34_t &f_matVR);
size_t ReadEnumVector(const std::string &f_val, const std::vector<std::string> &f_vec);
size_t ReadEnumVector(const char *f_val, const std::vector<std::string> &f_vec);
