#include "stdafx.h"

#include "Utils/Utils.h"

extern const float g_Pi = glm::pi<float>();
extern const glm::mat4 g_IdentityMatrix(1.f);
extern const glm::ivec2 g_EmptyIVector2(0);
extern const glm::vec3 g_EmptyVector(0.f);
extern const glm::quat g_EmptyQuat(1.f, 0.f, 0.f, 0.f);
extern const glm::vec4 g_ZeroPoint(0.f, 0.f, 0.f, 1.f);
extern const glm::vec3 g_AxisX(1.f, 0.f, 0.f);
extern const glm::vec3 g_AxisY(0.f, 1.f, 0.f);
extern const glm::vec3 g_AxisZN(0.f, 0.f, -1.f);
extern const sf::Color g_ClearColor(0U, 0U, 0U, 127U);
extern const unsigned char g_DummyTextureData[] = {
    0x7FU, 0x7FU, 0x7FU, 0xFF,
    0xF7U, 0x94U, 0x1DU, 0xFF,
    0xF7U, 0x94U, 0x1DU, 0xFF,
    0x7FU, 0x7FU, 0x7FU, 0xFF
};

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

// Function from example code, possibly can be optimized even more
void ExtractAndConvertToRGBA(const SL::Screen_Capture::Image &img, unsigned char *dst, size_t dst_size)
{
#ifdef _DEBUG
    assert(dst_size >= static_cast<size_t>(SL::Screen_Capture::Width(img) * SL::Screen_Capture::Height(img) * sizeof(SL::Screen_Capture::ImageBGRA)));
#endif
    auto imgsrc = StartSrc(img);
    auto imgdist = dst;
    for(auto h = 0; h < Height(img); h++)
    {
        auto startimgsrc = imgsrc;
        for(auto w = 0; w < Width(img); w++)
        {
            *imgdist++ = imgsrc->R;
            *imgdist++ = imgsrc->G;
            *imgdist++ = imgsrc->B;
            *imgdist++ = 255U;
            imgsrc++;
        }
        imgsrc = SL::Screen_Capture::GotoNextRow(img, startimgsrc);
    }
}

void GetRotationToPoint(const glm::vec3 &f_pointA, const glm::vec3 &f_pointB, const glm::quat &f_rotationA, glm::quat &f_result)
{
    glm::vec3 l_dir = f_pointA - f_pointB;
    l_dir = glm::normalize(l_dir);

    glm::vec3 l_up = f_rotationA*g_AxisY;
    glm::vec3 l_crossA = glm::cross(l_up, l_dir);
    l_crossA = glm::normalize(l_crossA);

    glm::vec3 l_crossB = glm::cross(l_dir, l_crossA);
    l_crossB = glm::normalize(l_crossB);

    glm::mat3 l_rotMat(l_crossA, l_crossB, l_dir);
    f_result = l_rotMat;
}

void SendWinAPIMessage(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    if(SendMessage(hWnd, Msg, wParam, lParam)) PostMessage(hWnd, Msg, wParam, lParam);
}