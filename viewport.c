#include "viewport.h"
#include "opengl_types.h"
#include "utils.h"
#include "globals.h"
#include "ecs.h"


/**
 * @brief When viewport size changes, this function is called to update the UI components positions
 */
void updateUIonViewportChange(
  float prevWidth, 
  float prevHeight,
  float ui_percentageWidth,
  float main_percentageWidth,
  float ui_percentageHeight
){
    for(int i = 0; i < MAX_ENTITIES; i++) {
        if(globals.entities[i].alive == 1 && globals.entities[i].uiComponent->active) {
            
            float px = globals.entities[i].transformComponent->position[0];
            float py = globals.entities[i].transformComponent->position[1];
    
            // Calculate new position
            px = px / ((prevWidth /  2.0) * ui_percentageWidth ) * ((float)globals.views.ui.rect.width  / 2.0);  
            py = py / ((prevHeight / 2.0) * ui_percentageHeight) * ((float)globals.views.ui.rect.height / 2.0); 
                                   
            globals.entities[i].transformComponent->position[0] = px;
            globals.entities[i].transformComponent->position[1] = py;
           
            // Bounding box
            globals.entities[i].boundingBoxComponent->boundingBox.min[0] = globals.entities[i].transformComponent->position[0];
            globals.entities[i].boundingBoxComponent->boundingBox.min[1] = globals.entities[i].transformComponent->position[1];
            globals.entities[i].boundingBoxComponent->boundingBox.max[0] = globals.entities[i].transformComponent->scale[0];
            globals.entities[i].boundingBoxComponent->boundingBox.max[1] = globals.entities[i].transformComponent->scale[1];

            globals.entities[i].transformComponent->modelNeedsUpdate = 1;
        }
    }
}

/**
 * @brief Recalculate the viewports when the window is resized.
 * TODO: This is fragile since the views are initialized with hardcoded pixel values 
 *       and only when resizing the window we transfer to percentage values.
 *       We also use the full view to determine what was the previous width & height of the window. 
 *       This is because when this function is called, the window has already been resized.
 *       This setup makes this function dependent on the full view beeing correct and also important
 *       to update full view correctly in this function.
 */
void recalculateViewports(int w, int h, struct Globals *globals){ 
    // Recalculate viewports
    float prevHeight = (float)globals->views.full.rect.height; 
    float prevWidth = (float)globals->views.full.rect.width; 

    // MAIN VIEW
    // Calc percentage height, using old height.
    float percentageHeight = (float)globals->views.main.rect.height / prevHeight; 

    // Update main height 
    globals->views.main.rect.height = (float)h * percentageHeight; 

    // Calc percentage width, using old width.
    float main_percentageWidth = (float)globals->views.main.rect.width / prevWidth; 

    // update main width
    globals->views.main.rect.width = (float)w * main_percentageWidth;
  
    // Update main x position
    float percentageX = (float)w / prevWidth; 
    globals->views.main.rect.x = percentageX * (float)globals->views.main.rect.x; 
  
    // Update main y position
    float percentageY = (float)h / prevHeight;
    globals->views.main.rect.y = percentageY * globals->views.main.rect.y; 
    
    // UI VIEW
    // Calc percentage height, using old height.
    percentageHeight = (float)globals->views.ui.rect.height / prevHeight; 

    // Update ui height
    globals->views.ui.rect.height = (float)h * percentageHeight;

    // Calc percentage width, using old width.
    float ui_percentageWidth = (float)globals->views.ui.rect.width / prevWidth;

    // update ui width
    globals->views.ui.rect.width = (float)w * ui_percentageWidth; 

    // Update ui x position
    float ui_percentageX = (float)w / prevWidth; 
    globals->views.ui.rect.x = ui_percentageX * (float)globals->views.ui.rect.x; 

    // Update ui y position
    percentageY = (float)h / prevHeight; 
    globals->views.ui.rect.y = percentageY * (float)globals->views.ui.rect.y;

    globals->views.ui.camera->aspectRatio = (float)globals->views.ui.rect.width / globals->views.ui.rect.height;
    globals->views.ui.camera->left = (-1 * (float)globals->views.ui.rect.width)/2.0f;
    globals->views.ui.camera->right = (float)globals->views.ui.rect.width /2.0f;
    globals->views.ui.camera->bottom = (-1 * (float)globals->views.ui.rect.height)/2.0f;
    globals->views.ui.camera->top = (float)globals->views.ui.rect.height/2.0f;
    globals->views.ui.camera->projectionMatrixNeedsUpdate = 1; 
    updateUIonViewportChange(prevWidth,prevHeight ,ui_percentageWidth,main_percentageWidth,percentageHeight);
    
    // update full view ( this should be done last in this fn, since full view is used to calculate new views)
    globals->views.full.rect.height = h;
    globals->views.full.rect.width = w;
    
    // Update the projection on the perspective main view camera
    globals->views.main.camera->projectionMatrixNeedsUpdate = 1;
}
void setViewport(struct View view) {
    glViewport(view.rect.x, view.rect.y, view.rect.width, view.rect.height);
}

void setViewportAndClear(View view){
    Rectangle rect = convertViewRectangleToSDLCoordinates(view,globals.views.full.rect.height);
   
    // Set the viewport
    glViewport(rect.x, rect.y, rect.width, rect.height);

     // Set the clear color
    glClearColor(view.clearColor.r, view.clearColor.g, view.clearColor.b, view.clearColor.a);

    // Clear the viewport
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

/**
 * @brief Set the viewport, scissor box and clear color for a view
 * Expects the view to be in opengl coordinates
 */
void setViewportWithScissorAndClear(View view) {
    Rectangle rect = convertViewRectangleToSDLCoordinates(view,globals.views.full.rect.height);
   
    // Set the viewport
    glViewport(rect.x, rect.y, rect.width, rect.height);

    // Set the scissor box
    glScissor(rect.x, rect.y, rect.width, rect.height);
    glEnable(GL_SCISSOR_TEST);

    // Set the clear color
    glClearColor(view.clearColor.r, view.clearColor.g, view.clearColor.b, view.clearColor.a);

    // Clear the viewport
    //glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Disable the scissor test
    glDisable(GL_SCISSOR_TEST);
}

