#include "stdafx.h"

#include "Utils/Transformation.h"
#include "Utils/Utils.h"

extern const glm::mat4 g_IdentityMatrix;
extern const glm::vec3 g_EmptyVector;
extern const glm::quat g_EmptyQuat;

Transformation::Transformation()
{
    m_position = g_EmptyVector;
    m_rotation = g_EmptyQuat;
    m_matrix = g_IdentityMatrix;
    m_update = false;
    m_updated = false;
}
Transformation::~Transformation()
{
}

void Transformation::SetPosition(const glm::vec3 &f_pos)
{
    if(m_position != f_pos)
    {
        std::memcpy(&m_position, &f_pos, sizeof(glm::vec3));
        m_update = true;
    }
}
void Transformation::Move(const glm::vec3 &f_pos)
{
    m_position += f_pos;
    m_update = true;
}

void Transformation::SetRotation(const glm::quat &f_rot)
{
    if(m_rotation != f_rot)
    {
        std::memcpy(&m_rotation, &f_rot, sizeof(glm::quat));
        m_update = true;
    }
}

void Transformation::Update(const Transformation *f_parent)
{
    m_updated = false;
    if(f_parent)
    {
        if(m_update || f_parent->m_updated)
        {
            m_matrix = glm::translate(g_IdentityMatrix, m_position)*glm::mat4_cast(m_rotation);
            m_matrix = f_parent->m_matrix*m_matrix;

            m_update = false;
            m_updated = true;
        }
    }
    else
    {
        if(m_update)
        {
            m_matrix = glm::translate(g_IdentityMatrix, m_position)*glm::mat4_cast(m_rotation);
            m_update = false;
            m_updated = true;
        }
    }
    if(m_updated) ConvertMatrix(m_matrix, m_vrMatrix);
}