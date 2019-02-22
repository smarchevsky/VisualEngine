
#include "camera.h"
#include "control.h"
#include "logger.h"
#include "material.h"
#include "mesh.h"
#include "meshdata.h"
#include "model.h"
#include "renderer.h"
#include "scene.h"
#include "texture.h"
#include "window.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>

#include <cassert>
#include <memory>

using namespace std;
namespace vi = Visual;

int main() {

    try {
        Window window(1024, 768, "Main window", false);
        window.setCullMode(vi::IFrameBuffer::Cull::Front);
        vi::Scene scene;

        //        struct Object {
        //            glm::dmat4 m_tr = glm::dmat4(1);
        //            vi::Model m_m;
        //            Object(){

        //            }
        //        };

        //        for (int i = 0; i < 30; ++i) {
        //            glm::sphericalRand(30.0);
        //        }

        //auto texture = std::make_shared<vi::Texture>("../GameTest2/textures/jupiter_diffuse.jpg");
        auto sphereMesh = std::make_shared<vi::Mesh>(vi::MeshPrimitives::sphere(40, vi::MeshData::Type::VTN));

        auto sphereMaterial = std::make_shared<vi::MaterialPBR>(vi::Color{0.6, 0.04, 0.1});
        glm::dmat4 sphereTransform = glm::translate(glm::dmat4(1), glm::dvec3(0, -1.3, 0));
        vi::Model sphereModel(&scene, sphereMesh, sphereMaterial, glm::value_ptr(sphereTransform));

        auto sphereMaterial2 = std::make_shared<vi::MaterialPBR>(vi::Color{0.3, 0.04, 0.8});
        glm::dmat4 sphereTransform2 = glm::translate(glm::dmat4(1), glm::dvec3(0, 1.3, 0));
        vi::Model sphereModel2(&scene, sphereMesh, sphereMaterial2, glm::value_ptr(sphereTransform2));

        auto sphereMaterial3 = std::make_shared<vi::MaterialPBR>(vi::Color{0.8, 0.8, 0.8});
        glm::dmat4 sphereTransform3 = glm::translate(glm::dmat4(1), glm::dvec3(40, 1.3, 0));
        vi::Model sphereModel3(&scene, sphereMesh, sphereMaterial3, glm::value_ptr(sphereTransform3));

        vi::Color planeColor{0.1, 0.1, 0.1};
        auto planeMaterial = std::make_shared<vi::MaterialPBR>(planeColor);
        glm::dmat4 planeTransform(1);
        planeTransform = glm::translate(planeTransform, glm::dvec3(0, 0, -1));
        auto planeMesh = std::make_shared<vi::Mesh>(vi::MeshPrimitives::plane(3000.0f, vi::MeshData::Type::VTN));
        vi::Model plane(&scene, planeMesh, planeMaterial, glm::value_ptr(planeTransform));

        vi::Camera camera(glm::dvec3(10, 10, 0), glm::dvec3(0, 0, 0));
        double xx = M_PI_2 - 0.01, yy = -0.5;
        double distance = 5.0;

        DLOG("main loop");
        vi::Renderer renderer;
        while (window.active()) {

            glm::dvec3 vec = {1, 0, 0};
            {
                xx += Control::mousePos().x * 0.02;
                yy += Control::mousePos().y * 0.02;
                yy = yy > M_PI_2 ? M_PI_2 - 0.001 : yy;
                yy = yy < -M_PI_2 ? -M_PI_2 + 0.001 : yy;
                vec = glm::rotate(vec, yy, glm::dvec3(0, 1, 0));
                vec = glm::rotate(vec, xx, glm::dvec3(0, 0, 1));
                distance *= 1.0 + Control::scrollOffset() * 0.1;
            }

            camera.set(vec * distance, glm::dvec3(0.0, 0.0, 0.0));
            renderer.draw(scene, camera, window);

            window.refresh();
        }
    } catch (const std::exception &except) {
        std::cerr << except.what() << std::endl;
    } catch (...) {
        std::cerr << "Undefined shit happened" << std::endl;
    }
    return 0;
}
