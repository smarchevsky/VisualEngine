#ifndef GAME_H
#define GAME_H
#include <entityx/entityx.h>
#include <memory>

namespace Visual {
class Scene;
class Camera;
class Renderer;
class IFrameBuffer;
} // namespace Visual
namespace ex = entityx;
namespace vi = Visual;

class PhysWorld;

class Game : public ex::EntityX {
  public:
    Game();
    void loadLevel();
    ~Game();

    void update(double dt);
    void render(vi::IFrameBuffer &frameBuffer);
    void addObject();

  private:
    std::unique_ptr<vi::Scene> m_visualScene;
    std::unique_ptr<vi::Renderer> m_renderer;
    std::shared_ptr<vi::Camera> m_camera;
    std::unique_ptr<PhysWorld> m_physWorld;
};

#endif // GAME_H
