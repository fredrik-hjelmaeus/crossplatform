#include "render.h"
#include "app.h"
#include "opengl.h"
#include "ecs.h"
#include "fh-sdl.h"
#include "viewport.h"
#include "lighting.h"
#include "utils.h"

//------------------------------------------------------
// Render variables / Initialization
//------------------------------------------------------
RenderState renderDefaults(void) {
    return (RenderState){
    
    .renderer=NULL,
    .gl_context=NULL,
    .overideDrawMode=GL_TRIANGLES,
    .overrideDrawModeBool=0,

    .drawBoundingBoxes=false,
    .render=true,
    .gpuFontData={0,0,0,0,0,0,GL_TRIANGLES},
    .culling=false,
    .drawCallsCounter=0,
    .debugDrawCalls=false,

    .blinnMode=false,
    .gamma=false,
    .depthMapBuffer={0},
    .frameBuffer={0},
    .depthMap=0,
    .depthCubemap=0,
    .lightSpaceMatrix={{0}},
    .postProcessBuffer={0},
    .showDepthMap=false,
    .shadowWidth=2048,
    .shadowHeight=2048,
    .shadows = true,
    };
}

void render(RenderState *renderState, AppState *app){

    // Clear the entire window
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    // Set culling
     if(renderState->culling){
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        glFrontFace(GL_CCW);
    }else {
        glDisable(GL_CULL_FACE);
    } 

    // Render without ui on wasm
    #ifdef __EMSCRIPTEN__
    setViewport(app->views.full);
     for(int i = 0; i < MAX_ENTITIES; i++) {
        if(app->entities[i].alive == 1) {
            if(app->entities[i].meshComponent->active == 1) { // && app->entities[i].uiComponent->active == 0
                Color* diff = &app->entities[i].materialComponent->diffuse;
                Color* amb = &app->entities[i].materialComponent->ambient;
                Color* spec = &app->entities[i].materialComponent->specular;
                GLfloat shin = app->entities[i].materialComponent->shininess;
                GLuint diffMap = app->entities[i].materialComponent->diffuseMap;
                bool useDiffMap = app->entities[i].materialComponent->useDiffuseMap;
                renderMesh(renderState, app, app->entities[i].meshComponent->gpuData,app->entities[i].transformComponent,diff,amb,spec,shin,diffMap,app->views.main.camera,useDiffMap);
            }
        }
    }
    #else
   // Native/Desktop
   
   // Render depth map
   // TODO: GL_CULL_FACE to avoid Peter panning?
   if(renderState->shadows){
    createLightSpace(renderState, app);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, renderState->depthMapBuffer.FBO);
    
    glEnable(GL_DEPTH_TEST);
        for(int i = 0; i < MAX_ENTITIES; i++) {
            if(app->entities[i].alive == 1) {
                if(app->entities[i].meshComponent->active == 1) {
                    if(app->entities[i].uiComponent->active != 1){
                        if(!app->entities[i].materialComponent->isPostProcessMaterial){
                            depthshadow_renderToDepthTexture(renderState, app, app->entities[i].meshComponent->gpuData,app->entities[i].transformComponent);
                        }
                    }
                }
            }
        }

    // Enable color buffer writes again
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
   }
  
   // Render main view & 3d objects
   setViewportAndClear(app, app->views.full);
   setFontProjection(&renderState->gpuFontData,app->views.full);
    for(int i = 0; i < MAX_ENTITIES; i++) {
        if(app->entities[i].alive == 1 && app->entities[i].visible) {
            if(app->entities[i].meshComponent->active == 1) {
                if(app->entities[i].uiComponent->active != 1){
                   if(!app->entities[i].materialComponent->isPostProcessMaterial){
                     renderMesh(renderState, app,  app->entities[i].meshComponent->gpuData, app->entities[i].transformComponent, app->views.main.camera,app->entities[i].materialComponent );
                   }
                    
                }
            }
        }
    }
    
    // Render GL_LINES & GL_POINTS(particles)
    for(int i = 0; i < MAX_ENTITIES; i++){
        if((app->entities[i].alive == 1 && app->entities[i].lineComponent->active == 1 && app->entities[i].visible) 
        || (app->entities[i].alive == 1 && app->entities[i].pointComponent->active == 1 && app->entities[i].visible)){
            if(app->entities[i].lineComponent->active == 1){
              renderLine(app->entities[i].lineComponent->gpuData,app->entities[i].transformComponent,app->views.main.camera,app->entities[i].lineComponent->color);
            }
            if(app->entities[i].pointComponent->active == 1){
                // NOTE: point Size is not drawing anything else than 1.0 in windows/wsl2.
                // Get point size range ( use this to debug pointSize or later at init to tell support or not of pointsize )
                //GLfloat    pointSizeRange[2];
                //glGetFloatv ( GL_ALIASED_POINT_SIZE_RANGE, pointSizeRange );
                // Print the point size range
                //printf("Point size range: min = %f, max = %f\n", pointSizeRange[0], pointSizeRange[1]);
                renderPoints(
                    app->entities[i].pointComponent->gpuData,
                    app->entities[i].transformComponent, 
                    app->views.main.camera, 
                    app->entities[i].pointComponent->color,
                    app->entities[i].pointComponent->pointSize
                );
            }
             
               
            
        }
    }

    // Render ui scene & ui objects
    if(app->showUI){
        for(int i = 0; i < MAX_ENTITIES; i++) {
            if(app->entities[i].alive == 1 && app->entities[i].visible) {
                if(app->entities[i].meshComponent->active == 1) {
                            // render ui, could be overhead with switching viewports?. profile.    
                            if(renderState->drawBoundingBoxes){
                                if(app->entities[i].tag == BOUNDING_BOX){
                                    renderMesh(renderState, app, app->entities[i].meshComponent->gpuData,app->entities[i].transformComponent,app->views.ui.camera, app->entities[i].materialComponent);
                                }
                            }else {
                                if(app->entities[i].uiComponent->active == 1 && app->entities[i].tag != BOUNDING_BOX){
                                    renderMesh(renderState, app, app->entities[i].meshComponent->gpuData,app->entities[i].transformComponent,app->views.ui.camera, app->entities[i].materialComponent);
                                }
                            }
                }
            }
        }
        
        // render ui text
        setFontProjection(&renderState->gpuFontData,app->views.ui);
        glDisable(GL_DEPTH_TEST);
        for(int i = 0; i < MAX_ENTITIES; i++) {
            if(app->entities[i].alive == 1 && app->entities[i].visible) {
                    if(app->entities[i].uiComponent->active == 1){
                        if(strlen(app->entities[i].uiComponent->text) > 0){
                        
                            // convert transform position to viewport space
                            vec2 result;
                            convertUIcoordinateToWindowcoordinates(
                                app->views.ui,
                                app->entities[i].transformComponent,
                                app->views.full.rect.height,
                                app->views.full.rect.width,
                                result);
                        
                                // align text center vertically
                                result[1] -= (float)app->characters[0].Size[1] / 4.0;
                            renderText(app, 
                                &renderState->gpuFontData, 
                                app->entities[i].uiComponent->text, 
                                result[0],result[1],
                                app->charScale,app->textColor);
                        }
                    }
            }
        }  
     }

   
    
   
    #endif

    // Swap the window buffers to show the new frame
    swapWindow(app);
}


