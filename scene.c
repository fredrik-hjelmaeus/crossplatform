#include "render.h"
#include "scene.h"
#include "app.h"
#include "assets.h"
#include "ui.h"
#include "api.h"
#include "utils.h"
#include "opengl.h"
#include "text.h"

void changeSceneMode(SceneMode mode,AppState *app){
  app->currentSceneMode = mode;
}


/**
 * Initialize the scene
 * Create the 3d and ui scene objects
 * 3d objects put into main viewport will be in an 3d-enviroment and treated normally.
 * objects created in the ui viewport will be in a 2d-environment and act more like a web ui element.
*/
void initScene(RenderState *renderState, AppState *app){

   changeSceneMode(SCENE_MODE_NORMAL,app);

   app->views.main.camera->position[1] = 35.0f;
   app->views.main.camera->position[2] = 35.0f;
   app->views.main.camera->target[0] = 0.0f;
   app->views.main.camera->target[1] = 0.0f;
   app->views.main.camera->target[2] = 0.0f;
    
    // Assets
    initFont(renderState, app);
   
    setupMaterial(&renderState->depthMapBuffer, "shaders/depthMapBuffer_vert.glsl", "shaders/depthMapBuffer_frag.glsl");
  
    depthshadow_createFrameBuffer(&renderState->depthMapBuffer);
    depthshadow_createDepthTexture(renderState);
    //depthshadow_createDepthCubemap();
  //  depthshadow_configureFrameBuffer(&renderState->depthMapBuffer);
    //depthshadow_configureCubeMapFrameBuffer(&renderState->depthMapBuffer);
    initAssets(renderState);
  
 //  TextureData oldBricksTest = loadTexture("./Assets/oldbricks.jpg");
  


   // ObjGroup* truck = obj_loadFile("./Assets/truck.obj"); // Not supported atm, need .obj group support.
  // ObjGroup* cornell_box = obj_loadFile("./Assets/cornell_box.obj");  
   // ObjGroup* bunny = obj_loadFile("./Assets/bunny2.obj");
  /*  ObjGroup* plane = obj_loadFile("./Assets/plane.obj"); 
    ObjGroup* objExample = obj_loadFile("./Assets/Two_adjoining_squares_with_vertex_normals.obj");
    ObjGroup* sphere = obj_loadFile("./Assets/blender_sphere3.obj");
    ObjGroup* triangleVolumes = obj_loadFile("./Assets/triangle_volumes.obj");
    ObjGroup* teapot = obj_loadFile("./Assets/teapot.obj");*/
  //  ObjGroup* dragon = obj_loadFile("./Assets/dragon.obj");   
   // ObjGroup* textured_objects = obj_loadFile("./Assets/textured_objects.obj");
    ObjGroup* skp_arbetsrum = obj_loadFile(renderState, app, "./Assets/arbetsrum_FINAL.obj");
 
    // Main viewport objects (3d scene) x,y,z coords is a world space coordinate (not yet implemented?).
  
  /*   createObject(&cornell_box->objData[0],(vec3){-5.0f, 5.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}); 
    createObject(&cornell_box->objData[1],(vec3){-5.0f, 5.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}); 
    createObject(&cornell_box->objData[2],(vec3){-5.0f, 5.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}); 
    createObject(&cornell_box->objData[3],(vec3){-5.0f, 5.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}); 
    createObject(&cornell_box->objData[4],(vec3){-5.0f, 5.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}); 
    createObject(&cornell_box->objData[5],(vec3){-5.0f, 5.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}); 
    createObject(&cornell_box->objData[6],(vec3){-5.0f, 5.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}); 
    createObject(&cornell_box->objData[7],(vec3){-5.0f, 5.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});   */
 
   
   // createPoint((vec3){-5.0f, -5.0f, 0.0f});
    //createObject(VIEWPORT_MAIN,&plane->objData[0],(vec3){5.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
   /*  for(int i = 0; i < 500; i++){
        createObject(&truck->objData[i],(vec3){0.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    } */
  
    createObject(app, &skp_arbetsrum->objData[0],(vec3){0.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    createObject(app, &skp_arbetsrum->objData[1],(vec3){0.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    createObject(app, &skp_arbetsrum->objData[2],(vec3){0.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    createObject(app, &skp_arbetsrum->objData[3],(vec3){0.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}); 
  
  /*   createObject(&truck->objData[0],(vec3){1.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    createObject(&truck->objData[1],(vec3){1.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    createObject(&truck->objData[2],(vec3){1.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    createObject(&truck->objData[3],(vec3){1.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    createObject(&truck->objData[4],(vec3){1.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    createObject(&truck->objData[5],(vec3){1.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    createObject(&truck->objData[6],(vec3){1.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    createObject(&truck->objData[7],(vec3){1.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    createObject(&truck->objData[8],(vec3){1.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    createObject(&truck->objData[9],(vec3){1.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}); */
    
    //createObject(VIEWPORT_MAIN,&objExample->objData[0],(vec3){5.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    //createObject(VIEWPORT_MAIN,&sphere->objData[0],(vec3){3.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    //createObject(VIEWPORT_MAIN,&triangleVolumes->objData[0],(vec3){4.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});
    //createObject(VIEWPORT_MAIN,&teapot->objData[0],(vec3){0.0f, 0.0f, 0.0f}, (vec3){0.25f, 0.25f, 0.25f}, (vec3){-90.0f, 0.0f, 0.0f}); 
   // createObject(&dragon->objData[0],(vec3){0.0f, 0.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});   
   // createObject(&bunny->objData[0],(vec3){6.0f, 0.0f, 0.0f}, (vec3){10.0f, 10.0f, 10.0f}, (vec3){0.0f, 0.0f, 0.0f});   
 //   createObject(&textured_objects->objData[0],(vec3){2.0f, 1.0f, -6.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});  
   // createObject(&textured_objects->objData[1],(vec3){2.0f, 1.0f, -6.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f});     
    

    // lights
    int lightId = createLight(renderState, app, lightMaterial,(vec3){5.0f,5.0f, 0.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f},(vec3){0.0f, -1.0f, 0.0f},DIRECTIONAL);
   // createLight(lightMaterial,(vec3){10.7f, 1.2f, 5.0f}, (vec3){0.25f, 0.25f, 0.25f}, (vec3){0.0f, 0.0f, 0.0f},(vec3){-0.2f, -1.0f, -0.3f},POINT);
   // createLight(lightMaterial,(vec3){-1.0f, 10.0f, 1.0f}, (vec3){0.25f, 0.25f, 0.25f}, (vec3){0.0f, 0.0f, 0.0f},(vec3){-0.2f, -1.0f, -0.3f},SPOT);
 /*   createLight(lightMaterial,(vec3){0.25f, 3.5f, 0.0f}, (vec3){0.25f, 0.25f, 0.25f}, (vec3){0.0f, 0.0f, 0.0f},(vec3){0.0f, -1.0f, 0.0f},SPOT);
    createLight(lightMaterial,(vec3){3.0f, 2.0f, 1.0f}, (vec3){0.25f, 0.25f, 0.25f}, (vec3){0.0f, 0.0f, 0.0f},(vec3){-0.2f, -1.0f, -0.3f},SPOT);
    createLight(lightMaterial,(vec3){0.0f, 1.0f, -5.0f}, (vec3){0.25f, 0.25f, 0.25f}, (vec3){0.0f, 0.0f, 0.0f},(vec3){-0.2f, -1.0f, -0.3f},SPOT);
   
    createLight(lightMaterial,(vec3){10.7f, -5.2f, -3.0f}, (vec3){0.25f, 0.25f, 0.25f}, (vec3){0.0f, 0.0f, 0.0f},(vec3){-0.2f, -1.0f, -0.3f},POINT);
    createLight(lightMaterial,(vec3){-12.7f, 5.2f, 2.0f}, (vec3){0.25f, 0.25f, 0.25f}, (vec3){0.0f, 0.0f, 0.0f},(vec3){-0.2f, -1.0f, -0.3f},POINT);
    createLight(lightMaterial,(vec3){-8.7f, 0.2f, -5.0f}, (vec3){0.25f, 0.25f, 0.25f}, (vec3){0.0f, 0.0f, 0.0f},(vec3){-0.2f, -1.0f, -0.3f},POINT); */

    // Primitives
    createPlane(app, objectMaterial, (vec3){0.0f, -3.0f, 0.0f}, (vec3){50.0f, 50.0f, 50.0f}, (vec3){-90.0f, 0.0f, 0.0f});
    //createCube(objectMaterial,(vec3){10.0f, 3.0f, 12.0f}, (vec3){1.0f, 1.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}); 
  
    // Frame buffer quad
  //  ui_createRectangle(showDepthMapMaterial, (vec3){0.0f, 0.0f,5.0f}, (vec3){800.0f, 600.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f},NULL);

    //debug_drawFrustum();

   // UI scene objects creation (2d scene) x,y coords where x = 0 is left and y = 0 is top and x,y is pixel positions. (controlled by the uiPositionSystem)
   // Scale is in pixels, 100.0f is 100 pixels etc.
   // z position will be z-depth, much like in DOM in web.Use this to control draw order.
   // TODO: implement rotation, it is atm not affecting. 
 
    // UI Settings Panel
    Entity* settingsPanel = ui_createPanel(app, uiBoundingBoxMat,(vec3){545.0f, 5.0f, 3.0f}, (vec3){250.0f, 350.0f, 1.0f},(vec3){0.0f, 0.0f, 0.0f},NULL);
   
    ui_createButton(app, flatColorUiGrayMat, (vec3){545.0f, 5.0f, 0.0f}, (vec3){250.0f, 25.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}, "Header", EVENT_TOGGLE_UI_PANEL, settingsPanel);
 
    ui_createTextInput(app, textInputUiMat, (vec3){680.0f, 35.0f, 1.0f}, (vec3){110.0f, 25.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}, "TextInput",EVENT_NONE,settingsPanel);
    ui_createTextField(app, flatColorUiDarkGrayMat, (vec3){555.0f, 35.0f, 1.0f}, (vec3){75.0f, 25.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}, "TextField",settingsPanel);
    // Row 2 Slider
  //  ui_createTextField(flatColorUiDarkGrayMat, (vec3){555.0f, 65.0f, 1.0f}, (vec3){75.0f, 25.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}, "Slider Text Field",settingsPanel);
   // ui_createSlider(flatColorUiGrayMat,textInputUiMat, (vec3){650.0f, 65.0f, 1.0f}, (vec3){75.0f, 25.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f},settingsPanel);
   
   
 /*   ui_createTextInput(textInputUiMat, (vec3){730.0f, 65.0f, 1.0f}, (vec3){60.0f, 25.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}, "1.00",NUMBER_INPUT,settingsPanel);
    // Row 3 Checkbox
    ui_createTextField(flatColorUiGrayMat, (vec3){555.0f, 100.0f, 1.0f}, (vec3){75.0f, 25.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}, "Global shadows",settingsPanel);
    ui_createCheckbox(flatColorUiGrayMat,flatColorUiDarkGrayMat, (vec3){650.0f, 100.0f, 1.0f}, (vec3){20.0f, 20.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f},TOGGLE_GLOBAL_SHADOWS,true,settingsPanel);
   

    // Row 4 List
   
    for(int i=0; i < app->lightsCount; i++){
        yPos += 35.0f;
        Event toggleShadow = {
            .type = TOGGLE_CAST_SHADOW,
            .bPtr = &app->entities[app->lights[i].entityId].lightComponent->castShadows,
            .subscriberCount = 0,
            .subscribers = {0}
        };

      // char* lightName[20];
       char* textCopy = (char*)arena_Alloc(&app->uiArena, 99 * sizeof(char));
       sprintf(textCopy, "id: %d, type: %d", app->lights[i].entityId,app->lights[i].type);
       // strcpy(lightName, "customtext"); // Copies "custom text" into the lightName array.
       // itoa(app->lights[i].entityId,lightName,10);
       // lightName = 'c';
       // sprintf(lightName,"%i", app->lights[i].entityId); //app->entities[app->lights[i].entityId].id
        ui_createTextField(flatColorUiGrayMat, (vec3){555.0f, yPos, 1.0f}, (vec3){75.0f, 25.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}, textCopy , settingsPanel);
        ui_createCheckbox(flatColorUiGrayMat,flatColorUiDarkGrayMat, (vec3){650.0f, yPos, 1.0f}, (vec3){20.0f, 20.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f},&toggleShadow,true,settingsPanel);
    } */
   
    // Row 5 Slider
    float yPos = 135.0f;
    yPos += 35.0f;
    ui_createTextField(app, flatColorUiDarkGrayMat, (vec3){555.0f, yPos, 1.0f}, (vec3){75.0f, 25.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}, "Position x",settingsPanel);
    int sliderId = ui_createSlider(app, flatColorUiGrayMat,textInputUiMat, (vec3){650.0f, yPos, 1.0f}, (vec3){75.0f, 25.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f},EVENT_LIGHT_DIR_X_CHANGED,settingsPanel);
    int textInputId = ui_createTextInput(app, textInputUiMat, (vec3){730.0f, yPos, 1.0f}, (vec3){60.0f, 25.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}, "0.00",EVENT_LIGHT_DIR_X_CHANGED,settingsPanel);
    // Row 6 Slider
    yPos += 35.0f;
    ui_createTextField(app, flatColorUiDarkGrayMat, (vec3){555.0f, yPos, 1.0f}, (vec3){75.0f, 25.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}, "Position y",settingsPanel);
    int sliderIdSix = ui_createSlider(app, flatColorUiGrayMat,textInputUiMat, (vec3){650.0f, yPos, 1.0f}, (vec3){75.0f, 25.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f},EVENT_LIGHT_DIR_Y_CHANGED,settingsPanel);
    int textInputIdSix = ui_createTextInput(app, textInputUiMat, (vec3){730.0f, yPos, 1.0f}, (vec3){60.0f, 25.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}, "5.00",EVENT_LIGHT_DIR_Y_CHANGED,settingsPanel);
    

    // Setup associations
    LightDirectionAssociation* lightAssociation = (LightDirectionAssociation*)malloc(sizeof(LightDirectionAssociation));
    lightAssociation->lightEntityId = lightId;
    lightAssociation->sliderEntityId = sliderId;
    lightAssociation->textInputEntityId = textInputId;
    
    LightDirectionAssociation* lightAssociation2 = (LightDirectionAssociation*)malloc(sizeof(LightDirectionAssociation));
    lightAssociation2->lightEntityId = lightId;
    lightAssociation2->sliderEntityId = sliderIdSix;
    lightAssociation2->textInputEntityId = textInputIdSix;

    // Event subscriptions
    Subscriber sub = { .eventType = EVENT_LIGHT_DIR_X_CHANGED , .callback = onDirectionLightChangeX, .association = lightAssociation };
    subscribeToEvent(sub);
    Subscriber sub2 = { .eventType = EVENT_LIGHT_DIR_Y_CHANGED , .callback = onDirectionLightChangeY, .association = lightAssociation2 };
    subscribeToEvent(sub2);
    
   /*  ui_createTextField(flatColorUiDarkGrayMat, (vec3){555.0f, yPos, 1.0f}, (vec3){75.0f, 25.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}, "Direction y",settingsPanel);
    Event lightDirYsliderChange;
    lightDirYsliderChange.type = CHANGE_Y_DIRECTION;
    ui_createSlider(flatColorUiGrayMat,textInputUiMat, (vec3){650.0f, yPos, 1.0f}, (vec3){75.0f, 25.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f},lightDirYsliderChange,settingsPanel);
    Event lightDirYtextChange;
    lightDirYtextChange.type = CHANGE_Y_DIRECTION;
    ui_createTextInput(textInputUiMat, (vec3){730.0f, yPos, 1.0f}, (vec3){60.0f, 25.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}, "1.00",lightDirYtextChange,settingsPanel);
    // Row 7 Slider
    yPos += 35.0f;
    ui_createTextField(flatColorUiDarkGrayMat, (vec3){555.0f, yPos, 1.0f}, (vec3){75.0f, 25.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}, "Direction z",settingsPanel);
    Event lightDirZsliderChange;
    lightDirZsliderChange.type = CHANGE_Z_DIRECTION;
    ui_createSlider(flatColorUiGrayMat,textInputUiMat, (vec3){650.0f, yPos, 1.0f}, (vec3){75.0f, 25.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f},lightDirZsliderChange,settingsPanel);
    Event lightDirZtextChange;
    lightDirZtextChange.type = CHANGE_Z_DIRECTION;
    ui_createTextInput(textInputUiMat, (vec3){730.0f, yPos, 1.0f}, (vec3){60.0f, 25.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}, "1.00",lightDirZtextChange,settingsPanel); */

  /*   ui_createRectangle(flatColorUiGrayMat, (vec3){545.0f, 30.0f, 0.0f}, (vec3){5.0f, 300.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f},settingsPanel);
    ui_createRectangle(flatColorUiDarkGrayMat, (vec3){550.0f, 30.0f, 1.0f}, (vec3){245.0f, 300.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f},settingsPanel);
    // Bottom pane
    ui_createButton(flatColorUiGrayMat, (vec3){545.0f, 355.0f, 0.0f}, (vec3){250.0f, 50.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}, "Header2",TOGGLE_PANEL,NULL);
    ui_createRectangle(flatColorUiGrayMat, (vec3){545.0f, 405.0f, 0.0f}, (vec3){10.0f, 300.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f},NULL);
    ui_createRectangle(flatColorUiDarkGrayMat, (vec3){555.0f, 405.0f, 1.0f}, (vec3){240.0f, 300.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f},NULL);    */
    
   

    //ui_slider(flatColorUiDarkGrayMat, (vec3){545.0f, 55.0f, 0.0f}, (vec3){250.0f, 50.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}, "Slider",onSliderChange);
    //ui_colorPicker(flatColorUiDarkGrayMat, (vec3){545.0f, 105.0f, 0.0f}, (vec3){250.0f, 50.0f, 1.0f}, (vec3){0.0f, 0.0f, 0.0f}, "ColorPicker",onColorPickerChange);
    //ui_checkbox
    //ui_open_file (texture,obj-model,.fh-packfile,)


 //ui_createRectangle(uiMaterial, (vec3){765.0f, 5.0f, 0.0f}, (vec3){35.0f, 50.0f, 100.0f}, (vec3){0.0f, 0.0f, 0.0f});

 // Textured button
 //ui_createButton(uiMaterial, (vec3){150.0f, 0.0f, 0.0f}, (vec3){150.0f, 50.0f, 100.0f}, (vec3){0.0f, 0.0f, 0.0f}, "Rotate",onButtonClick);
  
   // TODO: create slider or input for ui using this
  //printf("materials-list (%d): \n",app->materialsCount);
   // for(int i = 0; i < app->materialsCount; i++){
    //    printf("(%d),%s \n",i,app->materials[i].name);
   /*       printf("shininess %f \n",app->materials[i].shininess);
        printf("ambient.r %f \n",app->materials[i].ambient.r);
        printf("ambient.g %f \n",app->materials[i].ambient.g);
        printf("ambient.b %f \n",app->materials[i].ambient.b);
        printf("diffuse.r %f \n",app->materials[i].diffuse.r);
        printf("diffuse.g %f \n",app->materials[i].diffuse.g);
        printf("diffuse.b %f \n",app->materials[i].diffuse.b);
        printf("specular.r %f \n",app->materials[i].specular.r);
        printf("specular.g %f \n",app->materials[i].specular.g);
        printf("specular.b %f \n",app->materials[i].specular.b);
        printf("ior/Ni %f \n",app->materials[i].ior);
        printf("alpha %f \n",app->materials[i].alpha); 
        printf("diffuseMap %d \n",app->materials[i].diffuseMap);
        printf("ambientMap %d \n",app->materials[i].ambientMap);
        printf("specularMap %d \n",app->materials[i].specularMap);
        printf("shininessMap %d \n\n",app->materials[i].shininessMap);  */
 // }   

}
