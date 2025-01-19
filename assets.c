#include "assets.h"

struct Material objectMaterial;
struct Material lightMaterial;
struct Material depthMapMaterial;
struct Material textInputUiMat;
struct Material flatColorUiDarkGrayMat;
struct Material flatColorUiGrayMat;
struct Material uiBoundingBoxMat;

void initAssets(){
   TextureData containerTextureData = loadTexture("./Assets/container.jpg");
   GLuint containerMap = setupTexture(containerTextureData);
   TextureData containerTwoTextureData = loadTexture("./Assets/container2.png");
   GLuint containerTwoMap = setupTexture(containerTwoTextureData);
   TextureData containerTwoSpecTextureData = loadTexture("./Assets/container2_specular.png");
   GLuint containerTwoSpecularMap = setupTexture(containerTwoSpecTextureData);


    objectMaterial = (struct Material){
        .active = 1,
        .name = "objectMaterial",
        .ambient = (Color){1.0f, 1.0f, 1.0f, 1.0f},  // NOT used,controlled from lightmaterial
        .diffuse = (Color){0.5f, 0.0f, 0.0f, 1.0f},  // used when diffuseMapOpacity lower than 1.0 (may not be true anymore)
        .specular = (Color){0.0f, 0.0f, 0.0f, 1.0f}, // NOT used
        .shininess = 32.0f,                           // used
        .diffuseMap = containerTwoMap,               // used
        .diffuseMapOpacity = 1.0f,                  // used
        .specularMap = containerTwoSpecularMap,      // used
    };
    /*  textureUIMatExample = (struct Material){ // Saved as example of how to use a texture with ui element.
        .active = 1,
        .name = "textureUIMatExample",
        .ambient = (Color){0.0f, 0.0f, 0.0f, 1.0f},  // NOT used
        .diffuse = (Color){0.5f, 0.5f, 0.0f, 1.0f},  // used when diffuseMapOpacity lower than 1.0
        .specular = (Color){0.0f, 0.0f, 0.0f, 1.0f}, // NOT used
        .shininess = 4.0f,                           // NOT used
        .diffuseMap = containerTwoMap,               // used
        .diffuseMapOpacity = 1.0f,                    // used
        .specularMap = containerTwoSpecularMap,      // NOT used
    }; */
     depthMapMaterial = (struct Material){ // Saved as example of how to use a texture with ui element.
        .active = 1,
        .name = "depthMapMaterial",
        .ambient = (Color){0.0f, 0.0f, 0.0f, 1.0f},  // NOT used
        .diffuse = (Color){0.0f, 0.5f, 0.0f, 1.0f},  // used when diffuseMapOpacity lower than 1.0
        .specular = (Color){0.0f, 0.0f, 0.0f, 1.0f}, // NOT used
        .shininess = 4.0f,                           // NOT used
        .diffuseMap = containerTwoMap,               // used
        .diffuseMapOpacity = 1.0f,                    // used
        .specularMap = containerTwoSpecularMap,      // NOT used
        .isPostProcessMaterial = true,
    }; 
    textInputUiMat = (struct Material){ // Saved as example of how to use a texture with ui element.
        .active = 1,
        .name = "textInputUiMat",
        .ambient = (Color){0.0f, 0.0f, 0.0f, 1.0f},  // NOT used
        .diffuse = DARK_INPUT_FIELD,  // used when diffuseMapOpacity lower than 1.0
        .specular = (Color){0.0f, 0.0f, 0.0f, 1.0f}, // NOT used
        .shininess = 4.0f,                           // NOT used
        .diffuseMap = containerTwoMap,               // used
        .diffuseMapOpacity = 0.0f,                    // used
        .specularMap = containerTwoSpecularMap,      // NOT used
    }; 
     flatColorUiDarkGrayMat = (struct Material){
        .active = 1,
        .name = "flatColorUiDarkGrayMat",
        .ambient = (Color){0.0f, 0.0f, 0.0f, 1.0f},  // NOT used
        .diffuse = DARK_GRAY_COLOR,  // used when diffuseMapOpacity lower than 1.0
        .specular = (Color){0.0f, 0.0f, 0.0f, 1.0f}, // NOT used
        .shininess = 4.0f,                           // NOT used
        .diffuseMap = containerTwoMap,               // used
        .diffuseMapOpacity = 0.0f,                    // used
        .specularMap = containerTwoSpecularMap,      // NOT used
    }; 
    flatColorUiGrayMat = (struct Material){
        .active = 1,
        .name = "flatColorUiGrayMat",
        .ambient = (Color){0.0f, 0.0f, 0.0f, 1.0f},  // NOT used
        .diffuse = GRAY_COLOR,  // used when diffuseMapOpacity lower than 1.0
        .specular = (Color){0.0f, 0.0f, 0.0f, 1.0f}, // NOT used
        .shininess = 4.0f,                           // NOT used
        .diffuseMap = containerTwoMap,               // used
        .diffuseMapOpacity = 0.0f,                    // used
        .specularMap = containerTwoSpecularMap,      // NOT used
    }; 
    uiBoundingBoxMat = (struct Material){
        .active = 1,
        .name = "uiBoundingBoxMat",
        .ambient = (Color){0.0f, 0.0f, 0.0f, 1.0f},  // NOT used
        .diffuse = BOUNDINGBOX_COLOR,  // used when diffuseMapOpacity lower than 1.0
        .specular = (Color){0.0f, 0.0f, 0.0f, 1.0f}, // NOT used
        .shininess = 4.0f,                           // NOT used
        .diffuseMap = containerTwoMap,               // used
        .diffuseMapOpacity = 0.0f,                    // used
        .specularMap = containerTwoSpecularMap,      // NOT used
    }; 
    lightMaterial = (struct Material){
        .active = 1,
        .name = "lightMaterial",
        .ambient = (Color){1.0f, 1.0f, 1.0f, 1.0f},  // used       <-- ambient strength & tint
        .diffuse = (Color){1.0f, 1.0f, 1.0f, 1.0f},  // used       <-- diffuse strength & tint
        .specular = (Color){1.0f,1.0f, 1.0f, 1.0f}, // used        <-- specular strength & tint
        .shininess = 32.0f,                         // NOT used
        .diffuseMap = containerMap,                  // NOT used
        .diffuseMapOpacity = 1.0f,                  // NOT used
        .specularMap = containerMap,                 // NOT used
    };
}