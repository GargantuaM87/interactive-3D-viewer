#ifndef GUI_H
#define GUI_H

#include "../headers/camera.h"
#include "../include/glm/glm.hpp"
#include "../include/glm/gtc/matrix_transform.hpp"
#include "../include/glm/gtc/type_ptr.hpp"

class GUIVariables {
    public:
        glm::vec3 lightPos = glm::vec3(0.0, 2.5, 0.0); // point light position
        glm::vec3 lightColor = glm::vec3(1.0f); // point light color
        glm::vec3 lightDir = glm::vec3(-2.0f, 4.0f, 2.0f); // directional light direction
        float exposure = 1.0f;
        bool bloom = true;
};

#endif
