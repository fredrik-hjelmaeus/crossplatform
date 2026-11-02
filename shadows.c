#include "render.h"
#include "shadows.h"
#include "linmath.h"
#include "types.h"
#include "app.h"


/**
 * Builds the matrices needed to see the scene from 
 * the directional light’s point of view.
 * 
 * - Creates an ortographic projection
 * - Creates an light view matrix
 * - Combines the view & proj matrices for the light
 */
void createDirectionalLightSpace(RenderState *renderState, AppState *app, int index)
{
    // Get light from lights-list.
    Entity* light = &app->entities[app->lights[index].entityId];
    int lightSpaceIndex = app->lights[index].lightSpaceMatrixIndex[0];

    // Calc target using dir & pos from light.
    vec3 target;
    vec3_add(target, light->transformComponent->position, light->lightComponent->direction);

    mat4x4 lightProjection, lightView;
    float near_plane = 0.1f, far_plane = 100.0f;

    // Orthographic projection covering the scene
    mat4x4_ortho(lightProjection, -10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);

    // Light position 
    mat4x4_look_at(
        lightView,                          // view
        light->transformComponent->position,// eye
        target,                             // center
        (vec3){0.0f, 0.0f, 1.0f}            // up
    );

    // Multiply to get light-space matrix
    mat4x4_mul(
        renderState->lightSpaceMatrix[lightSpaceIndex], 
        (const float (*)[4])lightProjection, 
        (const float (*)[4])lightView
    ); 
}
