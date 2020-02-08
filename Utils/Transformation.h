#pragma once

class Transformation final
{
    glm::vec3 m_position;
    glm::quat m_rotation;

    glm::mat4 m_matrix;
    vr::HmdMatrix34_t m_vrMatrix;

    bool m_update;
    bool m_updated;
public:
    Transformation();
    ~Transformation();

    void SetPosition(const glm::vec3 &f_pos);
    inline const glm::vec3& GetPosition() const { return m_position; }
    void Move(const glm::vec3 &f_pos);

    void SetRotation(const glm::quat &f_rot);
    inline const glm::quat& GetRotation() const { return m_rotation; }

    inline const glm::mat4& GetMatrix() const { return m_matrix; }
    inline const vr::HmdMatrix34_t& GetMatrixVR() const { return m_vrMatrix; }

    void Update(const Transformation *f_parent = nullptr);
};
