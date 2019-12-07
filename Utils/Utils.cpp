#include "stdafx.h"

#include "Utils/Utils.h"

extern const float g_Pi = glm::pi<float>();
extern const glm::mat4 g_IdentityMatrix(1.f);
extern const glm::vec3 g_EmptyVector(0.f);
extern const glm::quat g_EmptyQuat(1.f, 0.f, 0.f, 0.f);
extern const glm::vec4 g_ZeroPoint(0.f, 0.f, 0.f, 1.f);
extern const glm::vec3 g_AxisY(0.f, 1.f, 0.f);
extern const glm::vec3 g_AxisZN(0.f, 0.f, -1.f);

void ConvertMatrix(const vr::HmdMatrix34_t &f_matVR, glm::mat4 &f_mat)
{
    for(int i = 0; i < 4; i++)
    {
        for(int j = 0; j < 3; j++) f_mat[i][j] = f_matVR.m[j][i];
    }
    for(int i = 0; i < 3; i++) f_mat[i][3] = 0.f;
    f_mat[3][3] = 1.f;
}
void ConvertMatrix(const glm::mat4 &f_mat, vr::HmdMatrix34_t &f_matVR)
{
    for(int i = 0; i < 3; i++)
    {
        for(int j = 0; j < 4; j++) f_matVR.m[i][j] = f_mat[j][i];
    }
}

size_t ReadEnumVector(const std::string &f_val, const std::vector<std::string> &f_vec)
{
    size_t l_result = std::numeric_limits<size_t>::max();
    for(auto iter = f_vec.begin(), iterEnd = f_vec.end(); iter != iterEnd; ++iter)
    {
        if(!iter->compare(f_val))
        {
            l_result = std::distance(f_vec.begin(), iter);
            break;
        }
    }
    return l_result;
}
size_t ReadEnumVector(const char *f_val, const std::vector<std::string> &f_vec)
{
    size_t l_result = std::numeric_limits<size_t>::max();
    for(auto iter = f_vec.begin(), iterEnd = f_vec.end(); iter != iterEnd; ++iter)
    {
        if(!iter->compare(f_val))
        {
            l_result = std::distance(f_vec.begin(), iter);
            break;
        }
    }
    return l_result;
}
