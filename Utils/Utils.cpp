#include "stdafx.h"

#include "Utils/Utils.h"

extern const glm::vec3 g_axisY;

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

void GetRotationToPoint(const glm::vec3 &f_pointA, const glm::vec3 &f_pointB, const glm::quat &f_rotationA, glm::quat &f_result)
{
    glm::vec3 l_dir = (f_pointA - f_pointB);
    l_dir = glm::normalize(l_dir);

    const glm::vec3 l_up = f_rotationA*g_axisY;
    glm::vec3 l_crossA = glm::cross(l_up, l_dir);
    l_crossA = glm::normalize(l_crossA);

    glm::vec3 l_crossB = glm::cross(l_dir, l_crossA);
    l_crossB = glm::normalize(l_crossB);

    const glm::mat3 l_rotMat(l_crossA, l_crossB, l_dir);
    f_result = l_rotMat;
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

void ExtractScreenCaptureImage(const SL::Screen_Capture::Image &f_img, unsigned char *f_dst, size_t f_size)
{
#ifdef _DEBUG
    assert(f_size >= static_cast<size_t>(SL::Screen_Capture::Width(f_img) * SL::Screen_Capture::Height(f_img) * sizeof(SL::Screen_Capture::ImageBGRA)));
#endif
    unsigned char *l_startDst = f_dst;
    const SL::Screen_Capture::ImageBGRA *l_startSrc = SL::Screen_Capture::StartSrc(f_img);
    if(SL::Screen_Capture::isDataContiguous(f_img)) std::memcpy(l_startDst, l_startSrc, f_size);
    else
    {
        const size_t l_lineSize = sizeof(SL::Screen_Capture::ImageBGRA) * SL::Screen_Capture::Width(f_img);
        for(int i = 0, j = SL::Screen_Capture::Height(f_img); i < j; i++)
        {
            std::memcpy(l_startDst, l_startSrc, l_lineSize);
            l_startDst += l_lineSize;
            l_startSrc = SL::Screen_Capture::GotoNextRow(f_img, l_startSrc);
        }
    }
}

#ifdef __linux__
unsigned long long GetTickCount64()
{
    struct timespec l_ts;
    clock_gettime(CLOCK_MONOTONIC, &l_ts);
    return (uint64_t)(l_ts.tv_nsec / 1000000) + ((uint64_t)l_ts.tv_sec * 1000ull);
}
#endif
