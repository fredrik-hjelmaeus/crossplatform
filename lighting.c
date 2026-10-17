#include "render.h"
#include "lighting.h"
#include "app.h"
#include "linmath.h"
#include "shadows.h"
#include <stdio.h>

/**
 * @param index
 */
void createPointLightSpace(RenderState *renderState, AppState *app, int index){
    Entity* light = &app->entities[app->lights[index].entityId];
    float near_plane = 0.001f, far_plane = 60.0f;
    mat4x4 lightProjection, lightView;
    mat4x4_perspective(lightProjection,90,1.0,near_plane,far_plane);

    vec3 targets[6] = {
        {1.0f,  0.0f,  0.0f},  // Right
        {-1.0f,  0.0f,  0.0f},  // Left
        {0.0f,  1.0f,  0.0f},  // Up
        {0.0f, -1.0f,  0.0f},  // Down
        {0.0f,  0.0f,  1.0f},  // Front
        {0.0f,  0.0f, -1.0f}   // Back
    };

    vec3 ups[6] = {
        {0.0f, -1.0f,  0.0f},  // Right
        {0.0f, -1.0f,  0.0f},  // Left
        {0.0f,  0.0f,  1.0f},  // Up
        {0.0f,  0.0f, -1.0f},  // Down
        {0.0f, -1.0f,  0.0f},  // Front
        {0.0f, -1.0f,  0.0f}   // Back
    };

    for(int i = 0; i < 6; i++){
        ASSERT(index > MAX_LIGHTS,"index larger than MAX_LIGHTS");
        int lightSpaceIndex = app->lights[index].lightSpaceMatrixIndex[i]; 
        ASSERT(lightSpaceIndex > MAX_LIGHTSPACES,"Light space index out of bounds");

        // 6 projections with diff. view targets, up,down,left,right,front,back HERE!<--
        // front
        if(i == 0) mat4x4_look_at(lightView, light->transformComponent->position, targets[i],ups[i]);
      //  if(i == 0) mat4x4_look_at(lightView, light->transformComponent->position, (vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, 1.0f, 0.0f});
        // back
        if(i == 1) mat4x4_look_at(lightView, light->transformComponent->position,  targets[i], ups[i]);
       // if(i == 1) mat4x4_look_at(lightView, light->transformComponent->position, (vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, -1.0f, 0.0f});
        // left
       // if(i == 2) mat4x4_look_at(lightView, light->transformComponent->position, (vec3){0.0f, 0.0f, 0.0f}, (vec3){-1.0f, 1.0f, 0.0f});
        if(i == 2) mat4x4_look_at(lightView, light->transformComponent->position,  targets[i], ups[i]);
        // right
    //    if(i == 3) mat4x4_look_at(lightView, light->transformComponent->position, (vec3){0.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 0.0f});
        if(i == 3) mat4x4_look_at(lightView, light->transformComponent->position, targets[i], ups[i]);
        // up
       // if(i == 4) mat4x4_look_at(lightView, light->transformComponent->position, (vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, 1.0f, -1.0f});
        if(i == 4) mat4x4_look_at(lightView, light->transformComponent->position,  targets[i], ups[i]);
        // down
       // if(i == 5) mat4x4_look_at(lightView, light->transformComponent->position, (vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, 1.0f, 1.0f});
        if(i == 5) mat4x4_look_at(lightView, light->transformComponent->position,  targets[i], ups[i]);

        mat4x4_mul(renderState->lightSpaceMatrix[lightSpaceIndex], (const float (*)[4])lightProjection, (const float (*)[4])lightView);
    }
}

/**
 * @param index
 */
void createSpotLightSpace(RenderState *renderState, AppState *app, int index){
    // ASSERT(true, "NOT YET IMPLEMENTED");
    Entity* light = &app->entities[app->lights[index].entityId];
    int lightSpaceIndex = app->lights[index].lightSpaceMatrixIndex[0];
    printf("lsi %i \n",lightSpaceIndex);
    mat4x4 lightProjection, lightView;
    float near_plane = 0.001f, far_plane = 60.0f;
    mat4x4_ortho(lightProjection, -60.0f, 60.0f, -60.0f, 60.0f, near_plane, far_plane);
    mat4x4_look_at(lightView, light->transformComponent->position, (vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, 1.0f, 0.0f});
    mat4x4_mul(renderState->lightSpaceMatrix[lightSpaceIndex], (const float (*)[4])lightProjection, (const float (*)[4])lightView);
}

void createLightSpace(RenderState *renderState, AppState *app){
    for(int i = 0; i < app->lightsCount; i++){
        if(app->lights[i].lightType == DIRECTIONAL){
            createDirectionalLightSpace(renderState, app, i);
        }
        if(app->lights[i].lightType == SPOT){
            //createDirectionalLightSpace(i);
          //  createSpotLightSpace(i);
        }
        if(app->lights[i].lightType == POINT){
           // createDirectionalLightSpace(i);
            //createPointLightSpace(i);
        }
    }
}

