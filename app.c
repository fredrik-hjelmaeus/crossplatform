#include "app.h"

//------------------------------------------------------
// Global variables / Initialization
//------------------------------------------------------
AppState appDefaults(void) {
    return (AppState){
    
    .window=NULL,
    .renderer=NULL,
    .event={0},
    .gl_context=NULL,
    .running=1,
    .entities=NULL,
    .vertex_count=0,
    .delta_time=0.0f,
    .overideDrawMode=GL_TRIANGLES,
    .overrideDrawModeBool=0,
    .views = {
        .ui={{0,   0, width, height}, {0.0f, 1.0f, 0.0f, 1.0f},NULL,false},
        .main={{0, 0, width, height}, {1.0f, 0.0f, 0.0f, 1.0f},NULL,false},
        .full={{0, 0, width, height}, {0.0f, 0.0f, 0.0f, 1.0f} ,NULL,false},
    },
    .currentSceneMode=SCENE_MODE_NONE,

    // Mouse
    .firstMouse=1,
    .mouseXpos=0.0f,
    .mouseYpos=0.0f,
    .mouseLeftButtonPressed=false, // mouse down check every frame
    .prevMouseLeftDown=false,
    .mouseDoubleClick=false,
    .deselectCondition=false,
    .mouseDragged=false,
    .mouseDragStart={-10000.0f,-10000.0f},
    .mouseDragPreviousFrame={0.0f,0.0f},

    .drawBoundingBoxes=false,
    .render=true,
    .gpuFontData={0,0,0,0,0,0,GL_TRIANGLES},
    .charScale=0.5f,
    .fontSize=26,
    .textColor={187.0/255.0,188.0/255.0,196.0/255.0,1.0},
    .unitScale=100.0f, // 1 unit = 1 cm 
    .culling=false,
    .drawCallsCounter=0,
    .debugDrawCalls=false,
    //.assetArena=NULL,
    .materials=NULL,
    .materialsCount=0,
    .materialsCapacity=15,
    .objDataCapacity=20000,
    .lights={{0}},
    .lightsCount=0,
    .focusedEntityId=-1,
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

    // Cursor
    .cursorEntityId=-1,
    .cursorBlinkTime=0.5f,
    .cursorSelectionActive=false,
    .cursorDragStart=-1.0f,
    .cursorTextSelection={0,0},
    
    .frameCount = 0,
    .prevTick = 0,
    .showUI = true,
    .shadows = true,
    };
}
