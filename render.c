#include "render.h"

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
