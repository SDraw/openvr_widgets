#include "stdafx.h"

#include "Core/Core.h"

float getAngle(const glm::vec3 & dir1, const glm::vec3 & dir2, const glm::vec3 & norm, bool signedAngle)
{	
	float dot = glm::dot(dir1,dir2);
	dot = glm::clamp(dot, -1.0f, 1.0f);
	float angle = acos(dot);

	if (signedAngle)
	{
		glm::vec3 cross = glm::cross(dir1,dir2);
		float sign = glm::dot(norm,cross);
		if (sign < 0) angle *= -1.0f;
	}

	return angle;
}

#ifdef _DEBUG
int main(void)
{
#else
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
#endif
    Core *l_core = new Core();
    if(l_core->Init())
    {
        while(l_core->DoPulse());
        l_core->Terminate();
    }
    delete l_core;

    return EXIT_SUCCESS;
}