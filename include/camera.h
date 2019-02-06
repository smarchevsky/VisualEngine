#ifndef CAMERA_H
#define CAMERA_H
#include <glm/glm.hpp>

using mat4f = glm::mat<4, 4, float, glm::highp>;
using mat4d = glm::mat<4, 4, double, glm::highp>;
using vec3f = glm::vec<3, float, glm::highp>;
using vec3d = glm::vec<3, double, glm::highp>;
using vec4f = glm::vec<4, float, glm::highp>;
using vec4d = glm::vec<4, double, glm::highp>;

namespace Visual {
class Camera {
  public:
    Camera(vec3d pos, vec3d aim);
    void updateView(double dt);
    void set(glm::dvec3 pos, glm::dvec3 aim, glm::dvec3 up = {0.0, 0.0, 1.0}) { m_pos = pos, m_aim = aim, m_up = up; }
    void setFOV(float fov);
    void setFOV(double fov) { setFOV(static_cast<float>(fov)); };
    void setAspectRatio(float ar);
    mat4d getView();
    const mat4f &getProjection() { return m_projection; };
    const vec3d &getPos() { return m_pos; }
    void move(vec3d offset);
    void rotate(vec3d angle);

  private:
    vec3d m_pos, m_aim, m_up;
    float m_near = 0.1f, m_far = 10000.f, m_fov = 1.f, m_aspectRatio = 1.f;

    mat4f m_projection;
    void updateProjection();
};
} // namespace Visual
#endif // CAMERA_H
