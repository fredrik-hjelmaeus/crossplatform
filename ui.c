#include "ui.h"
#include "app.h"
#include "api.h"

//----------------------------------------------------------------------------------------------//
// UI API
//----------------------------------------------------------------------------------------------//

Entity* ui_createPanel(AppState *app, Material material,vec3 position,vec3 scale,vec3 rotation,Entity* parent){
    Entity* entity = addEntity(app, MODEL);
    entity->uiComponent->active = 1;
    entity->boundingBoxComponent->active = 1;
    entity->boundingBoxComponent->boundingBox.min[0] = position[0];
    entity->boundingBoxComponent->boundingBox.min[1] = position[1];
    entity->boundingBoxComponent->boundingBox.min[2] = position[2];
    entity->boundingBoxComponent->boundingBox.max[0] = position[0] + scale[0];
    entity->boundingBoxComponent->boundingBox.max[1] = position[1] + scale[1];
    entity->boundingBoxComponent->boundingBox.max[2] = position[2] + scale[2];
    entity->uiComponent->uiNeedsUpdate = 1;
    entity->uiComponent->type = UITYPE_PANEL;

    setTransformData(entity,position,scale,rotation);

    // TODO: Create helper entity to display bounding box much like in ui_createButton
    // vertex data
    GLfloat vertices[] = {
        // Positions          // Colors           // Texture Coords    // Normals
        0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f,       0.0f,0.0f,1.0f, // top right
        0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 0.0f,       0.0f,0.0f,1.0f, // bottom right
        -0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,       0.0f,0.0f,1.0f, // bottom left
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f,       0.0f,0.0f,1.0f,  // top left 
    };
    GLuint bbIndices[] = {
        0, 1, 1,2, 2,3 ,3,0, 
    };
    Entity* boundingBoxEntity = addEntity(app, BOUNDING_BOX);
    ASSERT(boundingBoxEntity != NULL, "Failed to create bounding box entity");
    
    entity->uiComponent->boundingBoxEntityId = boundingBoxEntity->id;
    ASSERT(entity->uiComponent->boundingBoxEntityId != -1, "Failed to set bounding box entity id");

    createMesh(app, vertices,4,bbIndices,8,position,scale,rotation,&material,GL_LINES,VERTS_COLOR_ONEUV_INDICIES,boundingBoxEntity,true);
    return entity;
}
int ui_createRectangle(AppState *app, Material material,vec3 position,vec3 scale,vec3 rotation,Entity* parent){
    // vertex data
    GLfloat vertices[] = {
    // Positions          // Colors           // Texture Coords    // Normals
     0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f,       0.0f,0.0f,1.0f, // top right
     0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 0.0f,       0.0f,0.0f,1.0f, // bottom right
    -0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,       0.0f,0.0f,1.0f, // bottom left
    -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f,       0.0f,0.0f,1.0f,  // top left 
    }; 

    //OBSOLETE index data (counterclockwise)
    GLuint indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    Entity* entity = addEntity(app, MODEL);
    entity->uiComponent->active = 1;
    if(parent != NULL){
        entity->uiComponent->parent = parent;
        parent->uiComponent->childCount++;
        ASSERT(parent->uiComponent->childCount <= MAX_UI_CHILDREN, "Exceeded maximum number of children");
        parent->uiComponent->children[parent->uiComponent->childCount - 1] = entity->id;
    }
    entity->boundingBoxComponent->active = 1;
    entity->boundingBoxComponent->boundingBox.min[0] = position[0];
    entity->boundingBoxComponent->boundingBox.min[1] = position[1];
    entity->boundingBoxComponent->boundingBox.min[2] = position[2];
    entity->boundingBoxComponent->boundingBox.max[0] = position[0] + scale[0];
    entity->boundingBoxComponent->boundingBox.max[1] = position[1] + scale[1];
    entity->boundingBoxComponent->boundingBox.max[2] = position[2] + scale[2];
    entity->uiComponent->uiNeedsUpdate = 1;
    
    createMesh(app, vertices,4,indices,6,position,scale,rotation,&material,GL_TRIANGLES,VERTS_COLOR_ONEUV_INDICIES,entity,true);

    return entity->id;
}
/**
 * @brief Create a button
 * Create a button mesh in ui
 * @param diffuse - color of the rectangle
*/
void ui_createButton(AppState *app, Material material,vec3 position,vec3 scale,vec3 rotation, char* text,int eventType,Entity* parent){
    // vertex data <- TODO: This rectangle vertices array is used in many places. Move it to a separate file?
    GLfloat vertices[] = {
        // Positions          // Colors           // Texture Coords    // Normals
        0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f,       0.0f,0.0f,1.0f, // top right
        0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 0.0f,       0.0f,0.0f,1.0f, // bottom right
        -0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,       0.0f,0.0f,1.0f, // bottom left
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f,       0.0f,0.0f,1.0f,  // top left 
    }; 

    // OBSOLETE index data (counterclockwise)
    GLuint indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    Entity* entity = addEntity(app, MODEL);
    
    entity->uiComponent->active = 1;
    entity->boundingBoxComponent->active = 1;
    entity->boundingBoxComponent->boundingBox.min[0] = position[0];
    entity->boundingBoxComponent->boundingBox.min[1] = position[1];
    entity->boundingBoxComponent->boundingBox.min[2] = position[2];
    entity->boundingBoxComponent->boundingBox.max[0] = position[0] + scale[0];
    entity->boundingBoxComponent->boundingBox.max[1] = position[1] + scale[1];
    entity->boundingBoxComponent->boundingBox.max[2] = position[2] + scale[2];
    if(text != NULL){
        strncpy(entity->uiComponent->text, text, MAX_TEXT_LENGTH -1);
        entity->uiComponent->text[MAX_TEXT_LENGTH -1] = '\0';
    }
    entity->uiComponent->uiNeedsUpdate = 1;
    entity->uiComponent->eventType = eventType;
    entity->uiComponent->type = UITYPE_BUTTON;
    if(parent != NULL){
        entity->uiComponent->parent = parent;
        parent->uiComponent->childCount++;
        ASSERT(parent->uiComponent->childCount <= MAX_UI_CHILDREN, "Exceeded maximum number of children");
        parent->uiComponent->children[parent->uiComponent->childCount - 1] = entity->id;
        
    }
    
    createMesh(app, vertices,4,indices,6,position,scale,rotation,&material,GL_TRIANGLES,VERTS_COLOR_ONEUV_INDICIES,entity,true);

    // Bounding box, reuses the vertices from the rectangle
    GLuint bbIndices[] = {
        0, 1, 1,2, 2,3 ,3,0, 
    };
    Entity* boundingBoxEntity = addEntity(app, BOUNDING_BOX);
    ASSERT(boundingBoxEntity != NULL, "Failed to create bounding box entity");
    
    entity->uiComponent->boundingBoxEntityId = boundingBoxEntity->id;
    ASSERT(entity->uiComponent->boundingBoxEntityId != -1, "Failed to set bounding box entity id");

    createMesh(app, vertices,4,bbIndices,8,position,scale,rotation,&material,GL_LINES,VERTS_COLOR_ONEUV_INDICIES,boundingBoxEntity,true); 
}

/**
 * @brief Create a input field
 * Create a input mesh in ui
 * @param diffuse - color of the rectangle
 * @return int - entity id
*/
int ui_createTextInput(AppState *app, Material material,vec3 position,vec3 scale,vec3 rotation, char* text,int eventType,Entity* parent){
    // vertex data
    GLfloat vertices[] = {
    // Positions          // Colors           // Texture Coords    // Normals
     0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f,       0.0f,0.0f,1.0f, // top right
     0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 0.0f,       0.0f,0.0f,1.0f, // bottom right
    -0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,       0.0f,0.0f,1.0f, // bottom left
    -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f,       0.0f,0.0f,1.0f,  // top left 
    }; 

    // OBSOLETE index data (counterclockwise)
    GLuint indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    Entity* entity = addEntity(app, MODEL);
  
    entity->uiComponent->active = 1;
    entity->boundingBoxComponent->active = 1;
    entity->boundingBoxComponent->boundingBox.min[0] = position[0];
    entity->boundingBoxComponent->boundingBox.min[1] = position[1];
    entity->boundingBoxComponent->boundingBox.min[2] = position[2];
    entity->boundingBoxComponent->boundingBox.max[0] = position[0] + scale[0];
    entity->boundingBoxComponent->boundingBox.max[1] = position[1] + scale[1];
    entity->boundingBoxComponent->boundingBox.max[2] = position[2] + scale[2];
    if (text != NULL) {
        strncpy(entity->uiComponent->text, text, MAX_TEXT_LENGTH - 1);
        entity->uiComponent->text[MAX_TEXT_LENGTH - 1] = '\0';
    }
    entity->uiComponent->uiNeedsUpdate = 1;
    entity->uiComponent->eventType = eventType;
    entity->uiComponent->type = UITYPE_INPUT;
    if(parent != NULL){
        entity->uiComponent->parent = parent;
        parent->uiComponent->childCount++;
        ASSERT(parent->uiComponent->childCount <= MAX_UI_CHILDREN, "Exceeded maximum number of children");
        parent->uiComponent->children[parent->uiComponent->childCount - 1] = entity->id;
    }

    createMesh(app, vertices,4,indices,6,position,scale,rotation,&material,GL_TRIANGLES,VERTS_COLOR_ONEUV_INDICIES,entity,true);

    // Bounding box, reuses the vertices from the rectangle
    GLuint bbIndices[] = {
        0, 1, 1,2, 2,3 ,3,0, 
    };
    Entity* boundingBoxEntity = addEntity(app, BOUNDING_BOX);
    ASSERT(boundingBoxEntity != NULL, "Failed to create bounding box entity");
    
    entity->uiComponent->boundingBoxEntityId = boundingBoxEntity->id;
    ASSERT(entity->uiComponent->boundingBoxEntityId != -1, "Failed to set bounding box entity id");

    createMesh(app, vertices,4,bbIndices,8,position,scale,rotation,&material,GL_LINES,VERTS_COLOR_ONEUV_INDICIES,boundingBoxEntity,true); 

    return entity->id;
}

/**
 * @brief Create a text field
 * Create a input mesh in ui
 * @param diffuse - color of the rectangle
 * @return int - entity id
*/
int ui_createTextField(AppState *app, Material material,vec3 position,vec3 scale,vec3 rotation, char* text,Entity* parent){
    // vertex data
    GLfloat vertices[] = {
    // Positions          // Colors           // Texture Coords    // Normals
     0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f,       0.0f,0.0f,1.0f, // top right
     0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 0.0f,       0.0f,0.0f,1.0f, // bottom right
    -0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,       0.0f,0.0f,1.0f, // bottom left
    -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f,       0.0f,0.0f,1.0f,  // top left 
    }; 

    // OBSOLETE index data (counterclockwise)
    GLuint indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    Entity* entity = addEntity(app, MODEL);
    
    entity->uiComponent->active = 1;
    entity->boundingBoxComponent->active = 1;
    entity->boundingBoxComponent->boundingBox.min[0] = position[0];
    entity->boundingBoxComponent->boundingBox.min[1] = position[1];
    entity->boundingBoxComponent->boundingBox.min[2] = position[2];
    entity->boundingBoxComponent->boundingBox.max[0] = position[0] + scale[0];
    entity->boundingBoxComponent->boundingBox.max[1] = position[1] + scale[1];
    entity->boundingBoxComponent->boundingBox.max[2] = position[2] + scale[2];
    if(text != NULL){
        strncpy(entity->uiComponent->text, text, MAX_TEXT_LENGTH -1);
        entity->uiComponent->text[MAX_TEXT_LENGTH -1] = '\0';
    }
    entity->uiComponent->uiNeedsUpdate = 1;
    entity->uiComponent->type = UITYPE_TEXT;
    if(parent != NULL){
        entity->uiComponent->parent = parent;
        parent->uiComponent->childCount++;
        ASSERT(parent->uiComponent->childCount <= MAX_UI_CHILDREN, "Exceeded maximum number of children");
        parent->uiComponent->children[parent->uiComponent->childCount - 1] = entity->id;
    }

    createMesh(app, vertices,4,indices,6,position,scale,rotation,&material,GL_TRIANGLES,VERTS_COLOR_ONEUV_INDICIES,entity,true);

    // Bounding box, reuses the vertices from the rectangle
    GLuint bbIndices[] = {
        0, 1, 1,2, 2,3 ,3,0, 
    };
    Entity* boundingBoxEntity = addEntity(app, BOUNDING_BOX);
    ASSERT(boundingBoxEntity != NULL, "Failed to create bounding box entity");
    
    entity->uiComponent->boundingBoxEntityId = boundingBoxEntity->id;
    ASSERT(entity->uiComponent->boundingBoxEntityId != -1, "Failed to set bounding box entity id");

    createMesh(app, vertices,4,bbIndices,8,position,scale,rotation,&material,GL_LINES,VERTS_COLOR_ONEUV_INDICIES,boundingBoxEntity,true); 

    return entity->id;
}

/**
 * @brief Create a button
 * Create a button mesh in ui
 * @param diffuse - color of the rectangle
 * @return - int entity id
*/
int ui_createSlider(AppState *app, Material mat1,Material mat2, vec3 position,vec3 scale,vec3 rotation, EventType eventType, Entity* parent){
    // vertex data
    GLfloat vertices[] = {
        // Positions          // Colors           // Texture Coords    // Normals
        0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f,       0.0f,0.0f,1.0f, // top right
        0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 0.0f,       0.0f,0.0f,1.0f, // bottom right
        -0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,       0.0f,0.0f,1.0f, // bottom left
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f,       0.0f,0.0f,1.0f,  // top left 
    }; 

    // OBSOLETE index data (counterclockwise)
    GLuint indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    Entity* entity = addEntity(app, MODEL);
    // Slider consist of :
    // - button rectangle you interact with.
    vec3 btnRectangleScale = {20.0,20.0,2};
    vec3 btnRectanglePosition;
    btnRectanglePosition[0] = position[0];
    btnRectanglePosition[1] = position[1];
    btnRectanglePosition[2] = position[2];
   // ui_createRectangle(mat1,(vec3){position[0],position[1]+5,position[2]},btnRectangleScale,rotation,NULL);
   int rangeEntityId = ui_createRectangle(app, mat2, (vec3){position[0],position[1]+(scale[1]/2),position[2]}, (vec3){scale[0],10.0,scale[2]}, rotation, NULL);
    // - thin rectangle you slide along
    // - thin rectangle showing "progress" of slide on top of the other thin rectangle.
    // - slider drives a value and can also be driven by that value. Twoway.
    
    entity->uiComponent->active = 1;
    entity->uiComponent->eventType = eventType;
    entity->uiComponent->sliderRange = (float)scale[0];
    entity->uiComponent->sliderRangeEntityId = rangeEntityId;
    entity->boundingBoxComponent->active = 1;
    entity->boundingBoxComponent->boundingBox.min[0] = position[0];
    entity->boundingBoxComponent->boundingBox.min[1] = position[1]+5;
    entity->boundingBoxComponent->boundingBox.min[2] = position[2];
    entity->boundingBoxComponent->boundingBox.max[0] = position[0] + 20;
    entity->boundingBoxComponent->boundingBox.max[1] = position[1]+ 5 + 20;
    entity->boundingBoxComponent->boundingBox.max[2] = position[2] + scale[2];
    entity->uiComponent->uiNeedsUpdate = 1;
    entity->uiComponent->type = UITYPE_SLIDER;
    if(parent != NULL){
        entity->uiComponent->parent = parent;
        parent->uiComponent->childCount++;
        ASSERT(parent->uiComponent->childCount <= MAX_UI_CHILDREN, "Exceeded maximum number of children");
        parent->uiComponent->children[parent->uiComponent->childCount - 1] = entity->id;
    }
    
   createMesh(app, vertices,4,indices,6,(vec3){position[0],position[1]+5,position[2]},btnRectangleScale,rotation,&mat1,GL_TRIANGLES,VERTS_COLOR_ONEUV_INDICIES,entity,true);

    // Bounding box, reuses the vertices from the rectangle
    GLuint bbIndices[] = {
        0, 1, 1,2, 2,3 ,3,0, 
    };
    Entity* boundingBoxEntity = addEntity(app, BOUNDING_BOX);
    ASSERT(boundingBoxEntity != NULL, "Failed to create bounding box entity");
    
    entity->uiComponent->boundingBoxEntityId = boundingBoxEntity->id;
    ASSERT(entity->uiComponent->boundingBoxEntityId != -1, "Failed to set bounding box entity id");

    createMesh(app, vertices,4,bbIndices,8,(vec3){position[0],position[1]+5,position[2]},btnRectangleScale,rotation,&mat2,GL_LINES,VERTS_COLOR_ONEUV_INDICIES,boundingBoxEntity,true); 

    return entity->id;
}
/**
 * @brief Create a Checkbox
 * Create a button checkbox
*/
void ui_createCheckbox(AppState *app, Material mat1,Material mat2, vec3 position,vec3 scale,vec3 rotation, int eventType,bool checked,Entity* parent){
    // vertex data
    GLfloat vertices[] = {
        // Positions          // Colors           // Texture Coords    // Normals
        0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f,       0.0f,0.0f,1.0f, // top right
        0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 0.0f,       0.0f,0.0f,1.0f, // bottom right
        -0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,       0.0f,0.0f,1.0f, // bottom left
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f,       0.0f,0.0f,1.0f,  // top left 
    }; 

    // OBSOLETE index data (counterclockwise)
    GLuint indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

   Entity* entity = addEntity(app, MODEL);

   float borderLine = 4.0;
   int depth = 2;
   vec3 newPos = { position[0]+borderLine*0.5, position[1]+borderLine*0.5, position[2]+depth };
   vec3 newScale = { scale[0] - borderLine , scale[1] - borderLine , scale[2] };
   
    
    entity->uiComponent->active = 1;
    entity->boundingBoxComponent->active = 1;
    entity->boundingBoxComponent->boundingBox.min[0] = position[0];
    entity->boundingBoxComponent->boundingBox.min[1] = position[1];
    entity->boundingBoxComponent->boundingBox.min[2] = position[2];
    entity->boundingBoxComponent->boundingBox.max[0] = position[0] + scale[0];
    entity->boundingBoxComponent->boundingBox.max[1] = position[1] + scale[1];
    entity->boundingBoxComponent->boundingBox.max[2] = position[2] + scale[2];
    entity->uiComponent->uiNeedsUpdate = 1;
    entity->uiComponent->type = UITYPE_CHECKBOX;
    entity->uiComponent->checkedEntityId = ui_createRectangle(app, mat2,newPos,newScale,rotation,NULL);
    entity->uiComponent->eventType = eventType;
    entity->uiComponent->checked = checked;
    if(parent != NULL){
        entity->uiComponent->parent = parent;
        parent->uiComponent->childCount++;
        ASSERT(parent->uiComponent->childCount <= MAX_UI_CHILDREN, "Exceeded maximum number of children");
        parent->uiComponent->children[parent->uiComponent->childCount - 1] = entity->id;
    } 
    
    createMesh(app, vertices,4,indices,6,position,scale,rotation,&mat1,GL_TRIANGLES,VERTS_COLOR_ONEUV_INDICIES,entity,true);

    // Visual representation of Bounding box, reuses the vertices from the rectangle
    GLuint bbIndices[] = {
        0, 1, 1,2, 2,3 ,3,0, 
    };
    Entity* boundingBoxEntity = addEntity(app, BOUNDING_BOX);
    ASSERT(boundingBoxEntity != NULL, "Failed to create bounding box entity");
    
    entity->uiComponent->boundingBoxEntityId = boundingBoxEntity->id;
    ASSERT(entity->uiComponent->boundingBoxEntityId != -1, "Failed to set bounding box entity id");

    createMesh(app, vertices,4,bbIndices,8,position,scale,rotation,&mat2,GL_LINES,VERTS_COLOR_ONEUV_INDICIES,boundingBoxEntity,true); 
}

void onButtonClick(AppState *app) {

    for(int i = 0; i < MAX_ENTITIES; i++){
       if(app->entities[i].alive == 1 && app->entities[i].uiComponent->active != 1 && app->entities[i].transformComponent->active == 1){
            app->entities[i].transformComponent->rotation[1] += 0.01f;
            app->entities[i].transformComponent->modelNeedsUpdate = 1;
       }
    }
    printf("Button pressed!\n");
}

void toggleGlobalShadows(AppState *app, void *params){
    if(params != NULL){
       // printf("unused param %d \n",(int)params);
    }
    app->shadows = !app->shadows;
    printf("global shadows ? %d \n",app->shadows);
}

void toggleShadow(AppState *app, int entityId){
        app->entities[entityId].lightComponent->castShadows = !app->entities[entityId].lightComponent->castShadows;
        printf("shadow state on entityiID %d: %d \n",entityId,app->entities[entityId].lightComponent->castShadows);
}

void lightDirectionChange(AppState *app, void *params){
    LightDirChangeParams lightDirectionChange = *(LightDirChangeParams *)params;
    printf("entity ID %d \n",lightDirectionChange.entityId);
    app->entities[app->lights[0].entityId].lightComponent->direction[lightDirectionChange.index] = app->entities[lightDirectionChange.entityId].uiComponent->sliderValue;
}

void togglePanel(AppState *app, void *params){
    if(params != NULL){
        printf("unhandled params \n");
    }
 //   BoundingBox boundary;
    for(int i = 0; i < MAX_ENTITIES; i++){
       if(app->entities[i].alive == 1 && app->entities[i].uiComponent->active != 1 && app->entities[i].uiComponent->clicked){
        printf("Sclicked %d \n",app->entities[i].id);
        
            if(app->entities[i].uiComponent->parent != NULL){
                printf("parentId %d ",app->entities[i].uiComponent->parent->id);
            }
       }
    }
   /*  for(int i = 0; i < MAX_ENTITIES; i++){
       if(app->entities[i].alive == 1 && app->entities[i].uiComponent->active == 1 && app->entities[i].transformComponent->active == 1){
      
        if(isPointInsideRect(boundary, (vec2){ app->entities[i].uiComponent->boundingBox.x, app->entities[i].uiComponent->boundingBox.y}) 
        && isPointInsideRect(boundary, (vec2){ app->entities[i].uiComponent->boundingBox.x+app->entities[i].uiComponent->boundingBox.x 
        + app->entities[i].uiComponent->boundingBox.width, app->entities[i].uiComponent->boundingBox.y
        + app->entities[i].uiComponent->boundingBox.height})){
             if(app->entities[i].transformComponent->scale[1] > 0.0f){
                app->entities[i].transformComponent->scale[1] -= 0.61f;
            }
            app->entities[i].transformComponent->modelNeedsUpdate = 1;
        }

        
           
            
       }
    } */
   // printf("Panel toggled!\n");
}

