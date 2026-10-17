#include "types.h"
#include "utils.h"
#include "app.h"
#include "ui.h"


void tests_ui_coordinate_conversion(void){
   // SDL TO UI
    SDLVector2 myVec; 
    myVec.x = 400.0f;
    myVec.y = 300.0f;
    UIVector2 r = convertSDLToUI(myVec,width,height); // x - (width / 2), (height / 2) - y  = 0 , 0
    ASSERT((int)r.x == 0 && (int)r.y == 0,"convertSDLToUI failed on 400,300");
    myVec.x = 545.0f;
    myVec.y = 5.0f;
    r = convertSDLToUI(myVec,width,height); // 545 - (width / 2), height / 2 - 5.0 = 145 , 295
    ASSERT((int)r.x == 145 && (int)r.y == 295,"convertSDLToUI failed on 545,5");
    myVec.x = 0.0f;
    myVec.y = 0.0f;
    r = convertSDLToUI(myVec,width,height); // 0 - (width / 2), height / 2 - 0 = -400 , 300
    ASSERT((int)r.x == -400 && (int)r.y == 300,"convertSDLToUI failed on 0,0");
    myVec.x = 800.0f;
    myVec.y = 600.0f;
    r= convertSDLToUI(myVec,width,height); // 800 - (width /2 ), height /2 - 600 = 400, -300
    ASSERT((int)r.x == 400 && (int)r.y == -300,"convertSDLToUI failed on 800,600");

    // UI TO SDL
    UIVector2 myUiVec;
    myUiVec.x = 336.0f;
    myUiVec.y = 202.0f;
    SDLVector2 res = convertUIToSDL(myUiVec,width,height); 
    ASSERT((int)res.x == 736 && (int)res.y == 98,"convert ui to sdl failed on 336,202");
    myUiVec.x = -336.0f;
    myUiVec.y = -202.0f;
    res = convertUIToSDL(myUiVec,width,height);
    ASSERT((int)res.x == 64 && (int)res.y == 398,"convert ui to sdl failed on -336,-202");

    printf("tests_ui_coordinate_conversion: ");
    printf(TEXT_COLOR_GREEN "SUCCESS \n" TEXT_COLOR_RESET);
}

void tests_binary_conversion(void){
    int result = convertBinaryToDecimal(1111);
    ASSERT(result == 15, "convertBinaryToDecimal failed on 1111");
    long long binTwo = 1001001;  
    result = convertBinaryToDecimal(binTwo);
    ASSERT(result == 73, "convertBinaryToDecimal failed on 1001001");
    long long twentyFive = 11001;  
    result = convertBinaryToDecimal(twentyFive);
    ASSERT(result == 25, "convertBinaryToDecimal failed on twentyFive"); 

    printf("tests_binary_conversion: ");
    printf(TEXT_COLOR_GREEN "SUCCESS \n" TEXT_COLOR_RESET);
}

void tests_events(AppState *app){
   // This is a test setup aswell as doc for event-flow.
   // Meant to be used mainly for one-entity or one-time occurances.
   // This leaves systems to do updates on batches and we can avoid having to
   // shoe-horn in these one-time updates we now can use event flows for.
   
   // There are several event flows.
   // 1. ui mapping, where we setup an association struct where we provide the actors id for 
   // that particular event. 
   // The function we provide should know what to do with actors id and provided event payload.
   // 2. input mapping for keyboard input & mouse input etc.

   // Test of 1. ui mapping:
   changeSceneMode(SCENE_MODE_NORMAL,app); // sets mode
   
   LightDirectionAssociation* lightAss = (LightDirectionAssociation*)malloc(sizeof(LightDirectionAssociation));
   
   lightAss->lightEntityId = 2;
   lightAss->sliderEntityId = 3;
   lightAss->textInputEntityId = 4;

   Subscriber sub = { .eventType = EVENT_LIGHT_DIR_X_CHANGED, .callback = onDirectionLightChangeX, .association = lightAss };
   
   subscribeToEvent(sub); // event sub
   
   // Fake invoke phase two. When slider or textInput has found an update on itself, it will send out this event.
   // In this case it will provide new x value. 
   // onDirectionLightChangeUpdateUI will have to know how to update lightAss with the payload.
   TransformPayload tpo = {.objectEntityId = 3, .t = 1.0 };
   Event translateObjectEvent = {.type = EVENT_LIGHT_DIR_X_CHANGED, .payload.transform = tpo };
   pushEvent(translateObjectEvent);
   
   processEvents(app); // walk eventqueue and executes events

   // Test of 2. -> input-mapping -> subscribe -> process -> event -> action
   // Incomplete/Broken atm.
   //addKeyBinding(102,translateObjectEvent,SCENE_MODE_NORMAL); // key mapping
   //processKeyInput(102,app->currentSceneMode); // trigger event / push to eventqueue
   //exit(0);

    printf("tests_events: ");
    printf(TEXT_COLOR_GREEN "SUCCESS \n" TEXT_COLOR_RESET);
}
