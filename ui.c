#include "ui.h"
#include "globals.h"
#include "api.h"

//----------------------------------------------------------------------------------------------//
// UI API
//----------------------------------------------------------------------------------------------//

Entity* ui_createPanel(Material material,vec3 position,vec3 scale,vec3 rotation,Entity* parent){
    Entity* entity = addEntity(MODEL);
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
    Entity* boundingBoxEntity = addEntity(BOUNDING_BOX);
    ASSERT(boundingBoxEntity != NULL, "Failed to create bounding box entity");
    
    entity->uiComponent->boundingBoxEntityId = boundingBoxEntity->id;
    ASSERT(entity->uiComponent->boundingBoxEntityId != -1, "Failed to set bounding box entity id");

    createMesh(vertices,4,bbIndices,8,position,scale,rotation,&material,GL_LINES,VERTS_COLOR_ONEUV_INDICIES,boundingBoxEntity,true);
    return entity;
}
int ui_createRectangle(Material material,vec3 position,vec3 scale,vec3 rotation,Entity* parent){
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

    Entity* entity = addEntity(MODEL);
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
    
    createMesh(vertices,4,indices,6,position,scale,rotation,&material,GL_TRIANGLES,VERTS_COLOR_ONEUV_INDICIES,entity,true);

    return entity->id;
}
/**
 * @brief Create a button
 * Create a button mesh in ui
 * @param diffuse - color of the rectangle
*/
void ui_createButton(Material material,vec3 position,vec3 scale,vec3 rotation, char* text,int eventType,Entity* parent){
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

    Entity* entity = addEntity(MODEL);
    
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
    
    createMesh(vertices,4,indices,6,position,scale,rotation,&material,GL_TRIANGLES,VERTS_COLOR_ONEUV_INDICIES,entity,true);

    // Bounding box, reuses the vertices from the rectangle
    GLuint bbIndices[] = {
        0, 1, 1,2, 2,3 ,3,0, 
    };
    Entity* boundingBoxEntity = addEntity(BOUNDING_BOX);
    ASSERT(boundingBoxEntity != NULL, "Failed to create bounding box entity");
    
    entity->uiComponent->boundingBoxEntityId = boundingBoxEntity->id;
    ASSERT(entity->uiComponent->boundingBoxEntityId != -1, "Failed to set bounding box entity id");

    createMesh(vertices,4,bbIndices,8,position,scale,rotation,&material,GL_LINES,VERTS_COLOR_ONEUV_INDICIES,boundingBoxEntity,true); 
}

/**
 * @brief Create a input field
 * Create a input mesh in ui
 * @param diffuse - color of the rectangle
 * @return int - entity id
*/
int ui_createTextInput(Material material,vec3 position,vec3 scale,vec3 rotation, char* text,int eventType,Entity* parent){
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

    Entity* entity = addEntity(MODEL);
  
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

    createMesh(vertices,4,indices,6,position,scale,rotation,&material,GL_TRIANGLES,VERTS_COLOR_ONEUV_INDICIES,entity,true);

    // Bounding box, reuses the vertices from the rectangle
    GLuint bbIndices[] = {
        0, 1, 1,2, 2,3 ,3,0, 
    };
    Entity* boundingBoxEntity = addEntity(BOUNDING_BOX);
    ASSERT(boundingBoxEntity != NULL, "Failed to create bounding box entity");
    
    entity->uiComponent->boundingBoxEntityId = boundingBoxEntity->id;
    ASSERT(entity->uiComponent->boundingBoxEntityId != -1, "Failed to set bounding box entity id");

    createMesh(vertices,4,bbIndices,8,position,scale,rotation,&material,GL_LINES,VERTS_COLOR_ONEUV_INDICIES,boundingBoxEntity,true); 

    return entity->id;
}

/**
 * @brief Create a text field
 * Create a input mesh in ui
 * @param diffuse - color of the rectangle
 * @return int - entity id
*/
int ui_createTextField(Material material,vec3 position,vec3 scale,vec3 rotation, char* text,Entity* parent){
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

    Entity* entity = addEntity(MODEL);
    
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

    createMesh(vertices,4,indices,6,position,scale,rotation,&material,GL_TRIANGLES,VERTS_COLOR_ONEUV_INDICIES,entity,true);

    // Bounding box, reuses the vertices from the rectangle
    GLuint bbIndices[] = {
        0, 1, 1,2, 2,3 ,3,0, 
    };
    Entity* boundingBoxEntity = addEntity(BOUNDING_BOX);
    ASSERT(boundingBoxEntity != NULL, "Failed to create bounding box entity");
    
    entity->uiComponent->boundingBoxEntityId = boundingBoxEntity->id;
    ASSERT(entity->uiComponent->boundingBoxEntityId != -1, "Failed to set bounding box entity id");

    createMesh(vertices,4,bbIndices,8,position,scale,rotation,&material,GL_LINES,VERTS_COLOR_ONEUV_INDICIES,boundingBoxEntity,true); 

    return entity->id;
}

/**
 * @brief Create a button
 * Create a button mesh in ui
 * @param diffuse - color of the rectangle
 * @return - int entity id
*/
int ui_createSlider(Material mat1,Material mat2, vec3 position,vec3 scale,vec3 rotation, EventType eventType, Entity* parent){
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

    Entity* entity = addEntity(MODEL);
    // Slider consist of :
    // - button rectangle you interact with.
    vec3 btnRectangleScale = {20.0,20.0,2};
    vec3 btnRectanglePosition;
    btnRectanglePosition[0] = position[0];
    btnRectanglePosition[1] = position[1];
    btnRectanglePosition[2] = position[2];
   // ui_createRectangle(mat1,(vec3){position[0],position[1]+5,position[2]},btnRectangleScale,rotation,NULL);
   int rangeEntityId = ui_createRectangle(mat2,(vec3){position[0],position[1]+(scale[1]/2),position[2]},(vec3){scale[0],10.0,scale[2]},rotation,NULL);
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
    
   createMesh(vertices,4,indices,6,(vec3){position[0],position[1]+5,position[2]},btnRectangleScale,rotation,&mat1,GL_TRIANGLES,VERTS_COLOR_ONEUV_INDICIES,entity,true);

    // Bounding box, reuses the vertices from the rectangle
    GLuint bbIndices[] = {
        0, 1, 1,2, 2,3 ,3,0, 
    };
    Entity* boundingBoxEntity = addEntity(BOUNDING_BOX);
    ASSERT(boundingBoxEntity != NULL, "Failed to create bounding box entity");
    
    entity->uiComponent->boundingBoxEntityId = boundingBoxEntity->id;
    ASSERT(entity->uiComponent->boundingBoxEntityId != -1, "Failed to set bounding box entity id");

    createMesh(vertices,4,bbIndices,8,(vec3){position[0],position[1]+5,position[2]},btnRectangleScale,rotation,&mat2,GL_LINES,VERTS_COLOR_ONEUV_INDICIES,boundingBoxEntity,true); 

    return entity->id;
}
/**
 * @brief Create a Checkbox
 * Create a button checkbox
*/
void ui_createCheckbox(Material mat1,Material mat2, vec3 position,vec3 scale,vec3 rotation, int eventType,bool checked,Entity* parent){
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

   Entity* entity = addEntity(MODEL);

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
    entity->uiComponent->checkedEntityId = ui_createRectangle(mat2,newPos,newScale,rotation,NULL);
    entity->uiComponent->eventType = eventType;
    entity->uiComponent->checked = checked;
    if(parent != NULL){
        entity->uiComponent->parent = parent;
        parent->uiComponent->childCount++;
        ASSERT(parent->uiComponent->childCount <= MAX_UI_CHILDREN, "Exceeded maximum number of children");
        parent->uiComponent->children[parent->uiComponent->childCount - 1] = entity->id;
    } 
    
    createMesh(vertices,4,indices,6,position,scale,rotation,&mat1,GL_TRIANGLES,VERTS_COLOR_ONEUV_INDICIES,entity,true);

    // Visual representation of Bounding box, reuses the vertices from the rectangle
    GLuint bbIndices[] = {
        0, 1, 1,2, 2,3 ,3,0, 
    };
    Entity* boundingBoxEntity = addEntity(BOUNDING_BOX);
    ASSERT(boundingBoxEntity != NULL, "Failed to create bounding box entity");
    
    entity->uiComponent->boundingBoxEntityId = boundingBoxEntity->id;
    ASSERT(entity->uiComponent->boundingBoxEntityId != -1, "Failed to set bounding box entity id");

    createMesh(vertices,4,bbIndices,8,position,scale,rotation,&mat2,GL_LINES,VERTS_COLOR_ONEUV_INDICIES,boundingBoxEntity,true); 
}

