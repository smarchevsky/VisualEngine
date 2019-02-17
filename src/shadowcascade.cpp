#include "shadowcascade.h"
#include "camera.h"
#include "shader.h"

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Visual {
ShadowCascade::ShadowCascade(int size) {
    mShadowMapSize = size;

    // Directional light shadow map buffer
    glGenFramebuffers(1, &mCascadedShadowFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, mCascadedShadowFBO);

    glGenTextures(1, &mCascadedTextureArray);
    glBindTexture(GL_TEXTURE_2D_ARRAY, mCascadedTextureArray);

    glBindTexture(GL_TEXTURE_2D_ARRAY, mCascadedTextureArray);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, mShadowMapSize, mShadowMapSize, SHADOW_MAP_CASCADE_COUNT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mCascadedTextureArray, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    // restore default FBO
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void ShadowCascade::updateCascades(const Camera &camera) {
    float cascadeSplits[SHADOW_MAP_CASCADE_COUNT];

    float nearClip = camera.getNear();
    float farClip = camera.getFar();
    float clipRange = farClip - nearClip;

    float minZ = nearClip;
    float maxZ = nearClip + clipRange;

    float range = maxZ - minZ;
    float ratio = maxZ / minZ;

    // Calculate split depths based on view camera furstum
    // Based on method presentd in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
    for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++) {
        float p = (i + 1) / static_cast<float>(SHADOW_MAP_CASCADE_COUNT);
        float log = minZ * std::pow(ratio, p);
        float uniform = minZ + range * p;

        float cascadeSplitLambda = 0.95f;
        float d = cascadeSplitLambda * (log - uniform) + uniform;
        cascadeSplits[i] = (d - nearClip) / clipRange;
    }

    // Calculate orthographic projection matrix for each cascade
    float lastSplitDist = 0.0;
    for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++) {
        float splitDist = cascadeSplits[i];

        glm::vec3 frustumCorners[8] = {
            glm::vec3(-1.0f, 1.0f, -1.0f),
            glm::vec3(1.0f, 1.0f, -1.0f),
            glm::vec3(1.0f, -1.0f, -1.0f),
            glm::vec3(-1.0f, -1.0f, -1.0f),
            glm::vec3(-1.0f, 1.0f, 1.0f),
            glm::vec3(1.0f, 1.0f, 1.0f),
            glm::vec3(1.0f, -1.0f, 1.0f),
            glm::vec3(-1.0f, -1.0f, 1.0f),
        };

        // Project frustum corners into world space
        glm::mat4 invCam = glm::inverse(camera.getProjection() * camera.getView()); // TEST IT
        for (uint32_t i = 0; i < 8; i++) {
            glm::vec4 invCorner = invCam * glm::vec4(frustumCorners[i], 1.0f);
            frustumCorners[i] = invCorner / invCorner.w;
        }

        for (uint32_t i = 0; i < 4; i++) {
            glm::vec3 dist = frustumCorners[i + 4] - frustumCorners[i];
            frustumCorners[i + 4] = frustumCorners[i] + (dist * splitDist);
            frustumCorners[i] = frustumCorners[i] + (dist * lastSplitDist);
        }

        // Get frustum center
        glm::vec3 frustumCenter = glm::vec3(0.0f);
        for (uint32_t i = 0; i < 8; i++) {
            frustumCenter += frustumCorners[i];
        }
        frustumCenter /= 8.0f;

        float radius = 0.0f;
        for (uint32_t i = 0; i < 8; i++) {
            float distance = glm::length(frustumCorners[i] - frustumCenter);
            radius = glm::max(radius, distance);
        }
        radius = std::ceil(radius * 16.0f) / 16.0f;

        glm::vec3 maxExtents = glm::vec3(radius);
        glm::vec3 minExtents = -maxExtents;

        glm::vec3 lightPos = glm::vec3();
        glm::vec3 lightDir = normalize(-lightPos); // what the stupid logic ????????
        lightViewMatrix = glm::lookAt(frustumCenter - lightDir * -minExtents.z, frustumCenter, glm::vec3(0.0f, 1.0f, 0.0f));
        lightOrthoMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f, maxExtents.z - minExtents.z);

        // Store split distance and matrix in cascade
        cascades[i].splitDepth = (camera.getNear() + splitDist * clipRange) * -1.0f;
        cascades[i].viewProjMatrix = lightOrthoMatrix * lightViewMatrix;

        lastSplitDist = cascadeSplits[i];
    }
}

void ShadowCascade::draw() {
    auto shadowShader = ShaderShadow::get();
    //Now finally render the scene into FBO
    shadowShader->use();
    glBindFramebuffer(GL_FRAMEBUFFER, mCascadedShadowFBO);
    glViewport(0, 0, mShadowMapSize, mShadowMapSize);

    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, mCascadedTextureArray, 0, cascadeIterator);

    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_DEPTH_CLAMP);
    glCullFace(GL_FRONT);

    glm::mat4 lightViewProjection = lightOrthoMatrix * lightViewMatrix;
    //glUniformMatrix4fv(currentShader.getUniformLocation("lightViewProjectionMatrix"), 1, GL_FALSE, &lightViewProjection[0][0]);
    shadowShader->setViewProjection(&lightViewProjection[0][0]);

    //This function just renders the depth of the 4 cubes we previously saw
    // RENDER HERE
    //renderDepthCPU(cubeVAO,currentShader);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //currentShader.unbindShader();
    glDisable(GL_DEPTH_CLAMP);
}
} // namespace Visual
