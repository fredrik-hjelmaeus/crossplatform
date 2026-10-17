#include "shadows.h"
#include "linmath.h"
#include "types.h"
#include "app.h"


/**
 * @param index
 */
void createDirectionalLightSpace(AppState *app, int index){
        Entity* light = &app->entities[app->lights[index].entityId];
        int lightSpaceIndex = app->lights[index].lightSpaceMatrixIndex[0];

        mat4x4 lightProjection, lightView;
        float near_plane = 0.1f, far_plane = 100.0f;

        // Orthographic projection covering the scene
        mat4x4_ortho(lightProjection, -10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);

        // Light position at (10, 0, 0), looking at (0, 0, 0), up is (0, 1, 0)
        mat4x4_look_at(
            lightView,                          // view
            light->transformComponent->position,// eye
            (vec3){0.0f, 0.0f, 0.0f},           // center
            (vec3){0.0f, 1.0f, 0.0f}            // up
        );

        // Multiply to get light-space matrix
        mat4x4_mul(app->lightSpaceMatrix[lightSpaceIndex], (const float (*)[4])lightProjection, (const float (*)[4])lightView); 
}
