#ifndef UI_H
#define UI_H

typedef struct RenderState RenderState;


typedef struct AppState AppState;

#include "types.h"
#include "ecs-entity.h"

//-------
// UI API
//-------
void togglePanel(AppState *app, void *params);
void lightDirectionChange(AppState *app, void *params);
void toggleShadow(AppState *app, int entityId);
void toggleGlobalShadows(RenderState *renderState, void *params);
void onButtonClick(AppState *app);
/**
 * @brief Create a panel
 * Create a panel in ui. No visibility, only a bounding box.
 * @param material - material for bounding box
*/
Entity* ui_createPanel(AppState *app, Material material,vec3 position,vec3 scale,vec3 rotation,Entity* parent);
/**
 * @brief Create a rectangle
 * Create a rectangle mesh
*/
int ui_createRectangle(AppState *app, Material material,vec3 position,vec3 scale,vec3 rotation,Entity* parent);
/**
 * @brief Create a button
 * Create a button mesh in ui
*/
void ui_createButton(AppState *app, Material material,vec3 position,vec3 scale,vec3 rotation, char* text,int eventType,Entity* parent);
/**
 * @brief Create a input field
 * Create a input mesh in ui
 * @return in - entity id
*/
int ui_createTextInput(AppState *app, Material material,vec3 position,vec3 scale,vec3 rotation, char* text,int eventType,Entity* parent);
/**
 * @brief Create a text field
 * Create a mesh in ui
 * @return - int entity id
*/
int ui_createTextField(AppState *app, Material material,vec3 position,vec3 scale,vec3 rotation, char* text,Entity* parent);
/**
 * @brief Create a slider. 
 * @param eventType - The eventType is what is triggered when we use the slider
 * Usually the event trigger is on mouse down and mouse pos is deemed within ui slider bounding box.
 * @return - int entity id
 */
int ui_createSlider(AppState *app, Material mat1,Material mat2, vec3 position,vec3 scale,vec3 rotation, EventType eventType, Entity* parent);
void ui_createCheckbox(AppState *app, Material mat1,Material mat2, vec3 position,vec3 scale,vec3 rotation, int eventType,bool checked,Entity* parent);

// ----------------------------------
// Association structs for UI elements.
// -----------------------------------
// When we create a UI element, we wan't it to control something. 
// These structs creates this association.

typedef struct {
  int lightEntityId;
  int sliderEntityId;
  int textInputEntityId;
} LightDirectionAssociation;

#endif // UI_H
