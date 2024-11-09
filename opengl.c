#include "types.h"
#include "linmath.h"
#include "globals.h"
#include "opengl.h"


/** 
 * @brief Setup buffer to render GL_LINES. Very similar to setupMesh, 
 * only difference is that we only need color & position attributes.
*/
void setupLine(Line* lines, int lineCount, GpuData* buffer) {

    buffer->numIndicies = 0;
    buffer->drawMode = GL_LINES;
    buffer->vertexCount = lineCount;
    glGenVertexArrays(1, &(buffer->VAO));
    glGenBuffers(1, &(buffer->VBO));
    glGenBuffers(1, &buffer->EBO);
    glBindVertexArray(buffer->VAO);

    // Bind/Activate VBO
    glBindBuffer(GL_ARRAY_BUFFER, buffer->VBO);

    // Copy vertices to buffer
    glBufferData(GL_ARRAY_BUFFER, lineCount * sizeof(Line), lines, GL_STATIC_DRAW);

    // Bind/Activate EBO
  //  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->EBO);

    // Copy indices to buffer
   // glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(unsigned int), indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    
    // Unbind VBO/buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Unbind VAO/vertex array
    glBindVertexArray(0);
}

/** 
 * @brief Setup buffer to render a mesh.
*/
void setupMesh(Vertex* vertices, int vertexCount, unsigned int* indices, int indexCount, GpuData* buffer) {

    buffer->numIndicies = indexCount;
    glGenVertexArrays(1, &(buffer->VAO));
    glGenBuffers(1, &(buffer->VBO));
    glGenBuffers(1, &buffer->EBO);
    glBindVertexArray(buffer->VAO);

    // Bind/Activate VBO
    glBindBuffer(GL_ARRAY_BUFFER, buffer->VBO);

    // Copy vertices to buffer
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(Vertex), vertices, GL_STATIC_DRAW);

    // Bind/Activate EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->EBO);

    // Copy indices to buffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(unsigned int), indices, GL_STATIC_DRAW);

    // This line tells OpenGL how to interpret the vertex data
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    
    // Texture attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    // Normal attribute
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
    glEnableVertexAttribArray(3);

    // Unbind VBO/buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Unbind VAO/vertex array
    glBindVertexArray(0);
}

void setupMaterial(GpuData* buffer,const char* vertexPath,const char* fragmentPath){
     #ifdef __EMSCRIPTEN__
        char* vertexShaderSource = readFile("shaders/wasm/mesh_vertex_wasm.glsl"); // TODO: fix path
        char* fragmentShaderSource = readFile("shaders/wasm/mesh_fragment_wasm.glsl"); //  TODO: fix path
    #else
        char* vertexShaderSource = readFile(vertexPath);
        char* fragmentShaderSource = readFile(fragmentPath);
    #endif

    if(fragmentShaderSource == NULL || vertexShaderSource == NULL) {
        printf("Error loading shader source\n");
        return;
    }
    
    printf("OpenGL ES version: %s\n", glGetString(GL_VERSION));

    // Compile shaders
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    if(vertexShader == 0) {
        printf("Error creating vertex shader\n");
        return;
    }
    glShaderSource(vertexShader, 1, (const GLchar* const*)&vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // Check for shader compile errors
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
    }

    // Fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    if(fragmentShader == 0) {
        printf("Error creating fragment shader\n");
        return;
    }
    glShaderSource(fragmentShader, 1, (const GLchar* const*)&fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
 
    // Check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
    }

    // free memory of shader sources
    free(vertexShaderSource);
    free(fragmentShaderSource);

    // Link shaders
    buffer->shaderProgram = glCreateProgram();
    glAttachShader(buffer->shaderProgram, vertexShader);
    glAttachShader(buffer->shaderProgram, fragmentShader);
    glLinkProgram(buffer->shaderProgram);

    printf("Shader program: %d\n", buffer->shaderProgram);

    // Check for linking errors
    glGetProgramiv(buffer->shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(buffer->shaderProgram, 512, NULL, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
    }
}

void renderLine(GpuData* buffer,TransformComponent* transformComponent, Camera* camera,Color lineColor){
    // Check if camera is NULL
    if (camera == NULL) {
        fprintf(stderr, "Error: camera is NULL\n");
        return;
    }
     
   // Set shader
   glUseProgram(buffer->shaderProgram);

   // Set uniforms
   GLint lineColorLoc = glGetUniformLocation(buffer->shaderProgram, "lineColor");
   glUniform4f(lineColorLoc, lineColor.r,lineColor.g,lineColor.b,lineColor.a);
   
   GLint modelLoc = glGetUniformLocation(buffer->shaderProgram, "model");
   GLint viewLoc =  glGetUniformLocation(buffer->shaderProgram, "view");
   GLint projLoc =  glGetUniformLocation(buffer->shaderProgram, "projection");

   glUniformMatrix4fv(modelLoc,1,GL_FALSE,&transformComponent->transform[0][0]);
   glUniformMatrix4fv(viewLoc, 1,GL_FALSE,&camera->view[0][0]);
   glUniformMatrix4fv(projLoc, 1, GL_FALSE,&camera->projection[0][0]);

  // Bind buffer
  glBindVertexArray(buffer->VAO);
  
  glDrawArrays(buffer->drawMode,0,buffer->vertexCount);

  // Unbind buffer
  glBindVertexArray(0);
}

/**
 * @brief Render a mesh
 * Further optimizations:
 * Cache Uniform Locations: Cache the uniform locations during shader initialization to avoid querying them every frame.
 * Update Uniforms Only When Necessary: Track changes to uniform values and update them only when they change.
 * Use Uniform Buffer Objects (UBOs): For frequently changing uniforms, consider using UBOs to batch updates and reduce the number of API calls.
 * Minimize State Changes: Reduce the number of state changes (e.g., binding textures, shaders) by grouping draw calls that use the same state.
 * Ex of caching uniform locations:
 * typedef struct {
    GLuint shaderProgram;
    GLint modelLoc;
    GLint viewLoc;
    GLint projectionLoc;
    GLint colorLoc;
    GLint ambientLoc;
    GLint useDiffuseMapLoc;
    GLint texture1Loc;
} ShaderProgram;

void initializeShaderProgram(ShaderProgram* shaderProgram, const char* vertexPath, const char* fragmentPath) {
    // Compile and link shaders (not shown)
    shaderProgram->shaderProgram = compileAndLinkShaders(vertexPath, fragmentPath);

    // Cache uniform locations
    shaderProgram->modelLoc = glGetUniformLocation(shaderProgram->shaderProgram, "model");
    shaderProgram->viewLoc = glGetUniformLocation(shaderProgram->shaderProgram, "view");
    shaderProgram->projectionLoc = glGetUniformLocation(shaderProgram->shaderProgram, "projection");
    shaderProgram->colorLoc = glGetUniformLocation(shaderProgram->shaderProgram, "color");
    shaderProgram->ambientLoc = glGetUniformLocation(shaderProgram->shaderProgram, "ambient");
    shaderProgram->useDiffuseMapLoc = glGetUniformLocation(shaderProgram->shaderProgram, "useDiffuseMap");
    shaderProgram->texture1Loc = glGetUniformLocation(shaderProgram->shaderProgram, "texture1");
}
 */
void renderMesh(GpuData* buffer,TransformComponent* transformComponent, Camera* camera,MaterialComponent* material) {
 
    // Check if camera is NULL
    if (camera == NULL) {
        fprintf(stderr, "Error: camera is NULL\n");
        return;
    }
     
    // Set shader
    glUseProgram(buffer->shaderProgram);
 
    // Assign diffuseMap to texture1 slot
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, material->diffuseMap);
    glUniform1i(glGetUniformLocation(buffer->shaderProgram, "material.diffuse"), 0);

    // Assign specularMap to texture2 slot
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, material->specularMap);
    glUniform1i(glGetUniformLocation(buffer->shaderProgram, "material.specular"), 1);

    // Set diffuseMapOpacity uniform
    glUniform1f(glGetUniformLocation(buffer->shaderProgram, "material.diffuseMapOpacity"), material->diffuseMapOpacity);

    // Set the diffuseColor uniform
    GLint diffuseColorLocation = glGetUniformLocation(buffer->shaderProgram, "material.diffuseColor");
    glUniform4f(diffuseColorLocation, material->diffuse.r, material->diffuse.g, material->diffuse.b, material->diffuse.a);

    // Set the ambient uniform
    GLint ambientLocation = glGetUniformLocation(buffer->shaderProgram, "ambient");
    glUniform4f(ambientLocation, material->ambient.r, material->ambient.g, material->ambient.b, material->ambient.a);

    // Set the shininess uniform
    GLint shininessLocation = glGetUniformLocation(buffer->shaderProgram, "material.shininess");
    glUniform1f(shininessLocation, material->shininess);

    // Set the specular uniform
    GLint specularLocation = glGetUniformLocation(buffer->shaderProgram, "specular");
    glUniform4f(specularLocation, material->specular.r, material->specular.g, material->specular.b, material->specular.a);

    // Spotlights
   Entity spotLightEntityOne_ = globals.lights[0];
   Entity* spotLightEntityOne = &spotLightEntityOne_;
    if(spotLightEntityOne != NULL && spotLightEntityOne->lightComponent != NULL){
 
        // Set lightColor uniform
        GLint lightColorLocation = glGetUniformLocation(buffer->shaderProgram, "lightColor");
        glUniform3f(lightColorLocation, 0.0f,0.0f,1.0f);

        // Set the light ambient uniform 
        GLint lightAmbientLocation = glGetUniformLocation(buffer->shaderProgram, "spotLights[0].ambient");
        glUniform3f(lightAmbientLocation, spotLightEntityOne->lightComponent->ambient.r, spotLightEntityOne->lightComponent->ambient.g, spotLightEntityOne->lightComponent->ambient.b);

        // Set the light diffuse uniform
        GLint lightDiffuseLocation = glGetUniformLocation(buffer->shaderProgram, "spotLights[0].diffuse");
        glUniform3f(lightDiffuseLocation, spotLightEntityOne->lightComponent->diffuse.r, spotLightEntityOne->lightComponent->diffuse.g, spotLightEntityOne->lightComponent->diffuse.b);

        // Set the light specular uniform
        GLint lightSpecularLocation = glGetUniformLocation(buffer->shaderProgram, "spotLights[0].specular");
        glUniform3f(lightSpecularLocation, spotLightEntityOne->lightComponent->specular.r, spotLightEntityOne->lightComponent->specular.g, spotLightEntityOne->lightComponent->specular.b);

        // Set the light position uniform
        GLint lightPositionLocation = glGetUniformLocation(buffer->shaderProgram, "spotLights[0].position");
        glUniform3f(lightPositionLocation, spotLightEntityOne->transformComponent->position[0], spotLightEntityOne->transformComponent->position[1], spotLightEntityOne->transformComponent->position[2]);
      
        // Set the light direction uniform
        GLint lightDirLocation = glGetUniformLocation(buffer->shaderProgram, "spotLights[0].direction");
        glUniform3f(lightDirLocation, spotLightEntityOne->lightComponent->direction[0], spotLightEntityOne->lightComponent->direction[1], spotLightEntityOne->lightComponent->direction[2]);

        // Set the light constant uniform
        GLint lightConstantLocation = glGetUniformLocation(buffer->shaderProgram, "spotLights[0].constant");
        glUniform1f(lightConstantLocation, spotLightEntityOne->lightComponent->constant);

        // Set the light linear uniform
        GLint lightLinearLocation = glGetUniformLocation(buffer->shaderProgram, "spotLights[0].linear");
        glUniform1f(lightLinearLocation, spotLightEntityOne->lightComponent->linear);

        // Set the light quadratic uniform
        GLint lightQuadraticLocation = glGetUniformLocation(buffer->shaderProgram, "spotLights[0].quadratic");
        glUniform1f(lightQuadraticLocation, spotLightEntityOne->lightComponent->quadratic);

        // Set the light cutOff uniform
        GLint lightCutOffLocation = glGetUniformLocation(buffer->shaderProgram, "spotLights[0].cutOff");
        glUniform1f(lightCutOffLocation, spotLightEntityOne->lightComponent->cutOff);

        // Set the light outerCutOff uniform
        GLint lightOuterCutOffLocation = glGetUniformLocation(buffer->shaderProgram, "spotLights[0].outerCutOff");
        glUniform1f(lightOuterCutOffLocation, spotLightEntityOne->lightComponent->outerCutOff);
    }
   Entity spotLightEntityTwo_ = globals.lights[1];
   Entity* spotLightEntityTwo = &spotLightEntityTwo_;
    if(spotLightEntityTwo != NULL && spotLightEntityTwo->lightComponent != NULL){
 
        // Set lightColor uniform
        GLint lightColorLocation = glGetUniformLocation(buffer->shaderProgram, "lightColor");
        glUniform3f(lightColorLocation, 0.0f,0.0f,1.0f);

        // Set the light ambient uniform 
        GLint lightAmbientLocation = glGetUniformLocation(buffer->shaderProgram, "spotLights[1].ambient");
        glUniform3f(lightAmbientLocation, spotLightEntityTwo->lightComponent->ambient.r, spotLightEntityTwo->lightComponent->ambient.g, spotLightEntityTwo->lightComponent->ambient.b);

        // Set the light diffuse uniform
        GLint lightDiffuseLocation = glGetUniformLocation(buffer->shaderProgram, "spotLights[1].diffuse");
        glUniform3f(lightDiffuseLocation, spotLightEntityTwo->lightComponent->diffuse.r, spotLightEntityTwo->lightComponent->diffuse.g, spotLightEntityTwo->lightComponent->diffuse.b);

        // Set the light specular uniform
        GLint lightSpecularLocation = glGetUniformLocation(buffer->shaderProgram, "spotLights[1].specular");
        glUniform3f(lightSpecularLocation, spotLightEntityTwo->lightComponent->specular.r, spotLightEntityTwo->lightComponent->specular.g, spotLightEntityTwo->lightComponent->specular.b);

        // Set the light position uniform
        GLint lightPositionLocation = glGetUniformLocation(buffer->shaderProgram, "spotLights[1].position");
        glUniform3f(lightPositionLocation, spotLightEntityTwo->transformComponent->position[0], spotLightEntityTwo->transformComponent->position[1], spotLightEntityTwo->transformComponent->position[2]);
      
        // Set the light direction uniform
        GLint lightDirLocation = glGetUniformLocation(buffer->shaderProgram, "spotLights[1].direction");
        glUniform3f(lightDirLocation, spotLightEntityTwo->lightComponent->direction[0], spotLightEntityTwo->lightComponent->direction[1], spotLightEntityTwo->lightComponent->direction[2]);

        // Set the light constant uniform
        GLint lightConstantLocation = glGetUniformLocation(buffer->shaderProgram, "spotLights[1].constant");
        glUniform1f(lightConstantLocation, spotLightEntityTwo->lightComponent->constant);

        // Set the light linear uniform
        GLint lightLinearLocation = glGetUniformLocation(buffer->shaderProgram, "spotLights[1].linear");
        glUniform1f(lightLinearLocation, spotLightEntityTwo->lightComponent->linear);

        // Set the light quadratic uniform
        GLint lightQuadraticLocation = glGetUniformLocation(buffer->shaderProgram, "spotLights[1].quadratic");
        glUniform1f(lightQuadraticLocation, spotLightEntityTwo->lightComponent->quadratic);

        // Set the light cutOff uniform
        GLint lightCutOffLocation = glGetUniformLocation(buffer->shaderProgram, "spotLights[1].cutOff");
        glUniform1f(lightCutOffLocation, spotLightEntityTwo->lightComponent->cutOff);

        // Set the light outerCutOff uniform
        GLint lightOuterCutOffLocation = glGetUniformLocation(buffer->shaderProgram, "spotLights[1].outerCutOff");
        glUniform1f(lightOuterCutOffLocation, spotLightEntityTwo->lightComponent->outerCutOff);
    }
   Entity spotLightEntityThree_ = globals.lights[2];
   Entity* spotLightEntityThree = &spotLightEntityThree_;
    if(spotLightEntityThree != NULL && spotLightEntityThree->lightComponent != NULL){
 
        // Set lightColor uniform
        GLint lightColorLocation = glGetUniformLocation(buffer->shaderProgram, "lightColor");
        glUniform3f(lightColorLocation, 0.0f,0.0f,1.0f);

        // Set the light ambient uniform 
        GLint lightAmbientLocation = glGetUniformLocation(buffer->shaderProgram, "spotLights[2].ambient");
        glUniform3f(lightAmbientLocation, spotLightEntityThree->lightComponent->ambient.r, spotLightEntityThree->lightComponent->ambient.g, spotLightEntityThree->lightComponent->ambient.b);

        // Set the light diffuse uniform
        GLint lightDiffuseLocation = glGetUniformLocation(buffer->shaderProgram, "spotLights[2].diffuse");
        glUniform3f(lightDiffuseLocation, spotLightEntityThree->lightComponent->diffuse.r, spotLightEntityThree->lightComponent->diffuse.g, spotLightEntityThree->lightComponent->diffuse.b);

        // Set the light specular uniform
        GLint lightSpecularLocation = glGetUniformLocation(buffer->shaderProgram, "spotLights[2].specular");
        glUniform3f(lightSpecularLocation, spotLightEntityThree->lightComponent->specular.r, spotLightEntityThree->lightComponent->specular.g, spotLightEntityThree->lightComponent->specular.b);

        // Set the light position uniform
        GLint lightPositionLocation = glGetUniformLocation(buffer->shaderProgram, "spotLights[2].position");
        glUniform3f(lightPositionLocation, spotLightEntityThree->transformComponent->position[0], spotLightEntityThree->transformComponent->position[1], spotLightEntityThree->transformComponent->position[2]);
      
        // Set the light direction uniform
        GLint lightDirLocation = glGetUniformLocation(buffer->shaderProgram, "spotLights[2].direction");
        glUniform3f(lightDirLocation, spotLightEntityThree->lightComponent->direction[0], spotLightEntityThree->lightComponent->direction[1], spotLightEntityThree->lightComponent->direction[2]);

        // Set the light constant uniform
        GLint lightConstantLocation = glGetUniformLocation(buffer->shaderProgram, "spotLights[2].constant");
        glUniform1f(lightConstantLocation, spotLightEntityThree->lightComponent->constant);

        // Set the light linear uniform
        GLint lightLinearLocation = glGetUniformLocation(buffer->shaderProgram, "spotLights[2].linear");
        glUniform1f(lightLinearLocation, spotLightEntityThree->lightComponent->linear);

        // Set the light quadratic uniform
        GLint lightQuadraticLocation = glGetUniformLocation(buffer->shaderProgram, "spotLights[2].quadratic");
        glUniform1f(lightQuadraticLocation, spotLightEntityThree->lightComponent->quadratic);

        // Set the light cutOff uniform
        GLint lightCutOffLocation = glGetUniformLocation(buffer->shaderProgram, "spotLights[2].cutOff");
        glUniform1f(lightCutOffLocation, spotLightEntityThree->lightComponent->cutOff);

        // Set the light outerCutOff uniform
        GLint lightOuterCutOffLocation = glGetUniformLocation(buffer->shaderProgram, "spotLights[2].outerCutOff");
        glUniform1f(lightOuterCutOffLocation, spotLightEntityThree->lightComponent->outerCutOff);
    }
   Entity spotLightEntityFour_ = globals.lights[3];
   Entity* spotLightEntityFour = &spotLightEntityFour_;
    if(spotLightEntityFour != NULL && spotLightEntityFour->lightComponent != NULL){
 
        // Set lightColor uniform
        GLint lightColorLocation = glGetUniformLocation(buffer->shaderProgram, "lightColor");
        glUniform3f(lightColorLocation, 0.0f,0.0f,1.0f);

        // Set the light ambient uniform 
        GLint lightAmbientLocation = glGetUniformLocation(buffer->shaderProgram, "spotLights[3].ambient");
        glUniform3f(lightAmbientLocation, spotLightEntityFour->lightComponent->ambient.r, spotLightEntityFour->lightComponent->ambient.g, spotLightEntityFour->lightComponent->ambient.b);

        // Set the light diffuse uniform
        GLint lightDiffuseLocation = glGetUniformLocation(buffer->shaderProgram, "spotLights[3].diffuse");
        glUniform3f(lightDiffuseLocation, spotLightEntityFour->lightComponent->diffuse.r, spotLightEntityFour->lightComponent->diffuse.g, spotLightEntityFour->lightComponent->diffuse.b);

        // Set the light specular uniform
        GLint lightSpecularLocation = glGetUniformLocation(buffer->shaderProgram, "spotLights[3].specular");
        glUniform3f(lightSpecularLocation, spotLightEntityFour->lightComponent->specular.r, spotLightEntityFour->lightComponent->specular.g, spotLightEntityFour->lightComponent->specular.b);

        // Set the light position uniform
        GLint lightPositionLocation = glGetUniformLocation(buffer->shaderProgram, "spotLights[3].position");
        glUniform3f(lightPositionLocation, spotLightEntityFour->transformComponent->position[0], spotLightEntityFour->transformComponent->position[1], spotLightEntityFour->transformComponent->position[2]);
      
        // Set the light direction uniform
        GLint lightDirLocation = glGetUniformLocation(buffer->shaderProgram, "spotLights[3].direction");
        glUniform3f(lightDirLocation, spotLightEntityFour->lightComponent->direction[0], spotLightEntityFour->lightComponent->direction[1], spotLightEntityFour->lightComponent->direction[2]);

        // Set the light constant uniform
        GLint lightConstantLocation = glGetUniformLocation(buffer->shaderProgram, "spotLights[3].constant");
        glUniform1f(lightConstantLocation, spotLightEntityFour->lightComponent->constant);

        // Set the light linear uniform
        GLint lightLinearLocation = glGetUniformLocation(buffer->shaderProgram, "spotLights[3].linear");
        glUniform1f(lightLinearLocation, spotLightEntityFour->lightComponent->linear);

        // Set the light quadratic uniform
        GLint lightQuadraticLocation = glGetUniformLocation(buffer->shaderProgram, "spotLights[3].quadratic");
        glUniform1f(lightQuadraticLocation, spotLightEntityFour->lightComponent->quadratic);

        // Set the light cutOff uniform
        GLint lightCutOffLocation = glGetUniformLocation(buffer->shaderProgram, "spotLights[3].cutOff");
        glUniform1f(lightCutOffLocation, spotLightEntityFour->lightComponent->cutOff);

        // Set the light outerCutOff uniform
        GLint lightOuterCutOffLocation = glGetUniformLocation(buffer->shaderProgram, "spotLights[3].outerCutOff");
        glUniform1f(lightOuterCutOffLocation, spotLightEntityFour->lightComponent->outerCutOff);
    }

    // Diretional light
   Entity directionalLightEntity_ = globals.lights[4];
   Entity* directionalLightEntity = &directionalLightEntity_;
    if(directionalLightEntity != NULL && directionalLightEntity->lightComponent != NULL){

         // Set lightColor uniform
        GLint lightColorLocation = glGetUniformLocation(buffer->shaderProgram, "lightColor");
        glUniform3f(lightColorLocation, 1.0f,0.0f,0.0f);

        // Set the light ambient uniform
        GLint lightAmbientLocation = glGetUniformLocation(buffer->shaderProgram, "dirLight.ambient");
        glUniform3f(lightAmbientLocation, directionalLightEntity->lightComponent->ambient.r, directionalLightEntity->lightComponent->ambient.g, directionalLightEntity->lightComponent->ambient.b);

        // Set the light diffuse uniform
        GLint lightDiffuseLocation = glGetUniformLocation(buffer->shaderProgram, "dirLight.diffuse");
        glUniform3f(lightDiffuseLocation, directionalLightEntity->lightComponent->diffuse.r, directionalLightEntity->lightComponent->diffuse.g, directionalLightEntity->lightComponent->diffuse.b);

        // Set the light specular uniform
        GLint lightSpecularLocation = glGetUniformLocation(buffer->shaderProgram, "dirLight.specular");
        glUniform3f(lightSpecularLocation, directionalLightEntity->lightComponent->specular.r, directionalLightEntity->lightComponent->specular.g, directionalLightEntity->lightComponent->specular.b);

        // Set the light direction uniform
        GLint lightDirLocation = glGetUniformLocation(buffer->shaderProgram, "dirLight.direction");
        glUniform3f(lightDirLocation, directionalLightEntity->lightComponent->direction[0], directionalLightEntity->lightComponent->direction[1], directionalLightEntity->lightComponent->direction[2]);
    }

    Entity pointLightEntityOne_ = globals.lights[5];
    Entity* pointLightEntityOne = &pointLightEntityOne_;
    if(pointLightEntityOne != NULL && pointLightEntityOne->lightComponent != NULL){

        // Set lightColor uniform
        GLint lightColorLocation = glGetUniformLocation(buffer->shaderProgram, "lightColor");
        glUniform3f(lightColorLocation, 0.0f,1.0f,0.0f);

        // Set the light ambient uniform
        GLint plAmbientLoc = glGetUniformLocation(buffer->shaderProgram, "pointLights[0].ambient");
        glUniform3f(plAmbientLoc, pointLightEntityOne->lightComponent->ambient.r, pointLightEntityOne->lightComponent->ambient.g, pointLightEntityOne->lightComponent->ambient.b);

        // Set the light diffuse uniform
        GLint plDiffuseLoc = glGetUniformLocation(buffer->shaderProgram, "pointLights[0].diffuse");
        glUniform3f(plDiffuseLoc, pointLightEntityOne->lightComponent->diffuse.r, pointLightEntityOne->lightComponent->diffuse.g, pointLightEntityOne->lightComponent->diffuse.b);

        // Set the light specular uniform
        GLint plSpecularLoc = glGetUniformLocation(buffer->shaderProgram, "pointLights[0].specular");
        glUniform3f(plSpecularLoc, pointLightEntityOne->lightComponent->specular.r, pointLightEntityOne->lightComponent->specular.g, pointLightEntityOne->lightComponent->specular.b);

        // Set the light position uniform
        GLint plPositionLoc = glGetUniformLocation(buffer->shaderProgram, "pointLights[0].position");
        glUniform3f(plPositionLoc, pointLightEntityOne->transformComponent->position[0], pointLightEntityOne->transformComponent->position[1], pointLightEntityOne->transformComponent->position[2]);

        // Set the light constant uniform
        GLint plConstantLoc = glGetUniformLocation(buffer->shaderProgram, "pointLights[0].constant");
        glUniform1f(plConstantLoc, pointLightEntityOne->lightComponent->constant);

        // Set the light linear uniform
        GLint plLinearLoc = glGetUniformLocation(buffer->shaderProgram, "pointLights[0].linear");
        glUniform1f(plLinearLoc, pointLightEntityOne->lightComponent->linear);

        // Set the light quadratic uniform
        GLint plQuadraticLoc = glGetUniformLocation(buffer->shaderProgram, "pointLights[0].quadratic");
        glUniform1f(plQuadraticLoc, pointLightEntityOne->lightComponent->quadratic);
    }
    Entity pointLightEntityTwo_ = globals.lights[6];
    Entity* pointLightEntityTwo = &pointLightEntityTwo_;
    if(pointLightEntityTwo != NULL && pointLightEntityTwo->lightComponent != NULL){

        // Set lightColor uniform
        GLint lightColorLocation = glGetUniformLocation(buffer->shaderProgram, "lightColor");
        glUniform3f(lightColorLocation, 0.0f,1.0f,0.0f);

        // Set the light ambient uniform
        GLint plAmbientLoc = glGetUniformLocation(buffer->shaderProgram, "pointLights[1].ambient");
        glUniform3f(plAmbientLoc, pointLightEntityOne->lightComponent->ambient.r, pointLightEntityOne->lightComponent->ambient.g, pointLightEntityOne->lightComponent->ambient.b);

        // Set the light diffuse uniform
        GLint plDiffuseLoc = glGetUniformLocation(buffer->shaderProgram, "pointLights[1].diffuse");
        glUniform3f(plDiffuseLoc, pointLightEntityOne->lightComponent->diffuse.r, pointLightEntityOne->lightComponent->diffuse.g, pointLightEntityOne->lightComponent->diffuse.b);

        // Set the light specular uniform
        GLint plSpecularLoc = glGetUniformLocation(buffer->shaderProgram, "pointLights[1].specular");
        glUniform3f(plSpecularLoc, pointLightEntityOne->lightComponent->specular.r, pointLightEntityOne->lightComponent->specular.g, pointLightEntityOne->lightComponent->specular.b);

        // Set the light position uniform
        GLint plPositionLoc = glGetUniformLocation(buffer->shaderProgram, "pointLights[1].position");
        glUniform3f(plPositionLoc, pointLightEntityOne->transformComponent->position[0], pointLightEntityOne->transformComponent->position[1], pointLightEntityOne->transformComponent->position[2]);

        // Set the light constant uniform
        GLint plConstantLoc = glGetUniformLocation(buffer->shaderProgram, "pointLights[1].constant");
        glUniform1f(plConstantLoc, pointLightEntityOne->lightComponent->constant);

        // Set the light linear uniform
        GLint plLinearLoc = glGetUniformLocation(buffer->shaderProgram, "pointLights[1].linear");
        glUniform1f(plLinearLoc, pointLightEntityOne->lightComponent->linear);

        // Set the light quadratic uniform
        GLint plQuadraticLoc = glGetUniformLocation(buffer->shaderProgram, "pointLights[1].quadratic");
        glUniform1f(plQuadraticLoc, pointLightEntityOne->lightComponent->quadratic);
    }
    Entity pointLightEntityThree_ = globals.lights[7];
    Entity* pointLightEntityThree = &pointLightEntityThree_;
    if(pointLightEntityThree != NULL && pointLightEntityThree->lightComponent != NULL){

        // Set lightColor uniform
        GLint lightColorLocation = glGetUniformLocation(buffer->shaderProgram, "lightColor");
        glUniform3f(lightColorLocation, 0.0f,1.0f,0.0f);

        // Set the light ambient uniform
        GLint plAmbientLoc = glGetUniformLocation(buffer->shaderProgram, "pointLights[2].ambient");
        glUniform3f(plAmbientLoc, pointLightEntityOne->lightComponent->ambient.r, pointLightEntityOne->lightComponent->ambient.g, pointLightEntityOne->lightComponent->ambient.b);

        // Set the light diffuse uniform
        GLint plDiffuseLoc = glGetUniformLocation(buffer->shaderProgram, "pointLights[2].diffuse");
        glUniform3f(plDiffuseLoc, pointLightEntityOne->lightComponent->diffuse.r, pointLightEntityOne->lightComponent->diffuse.g, pointLightEntityOne->lightComponent->diffuse.b);

        // Set the light specular uniform
        GLint plSpecularLoc = glGetUniformLocation(buffer->shaderProgram, "pointLights[2].specular");
        glUniform3f(plSpecularLoc, pointLightEntityOne->lightComponent->specular.r, pointLightEntityOne->lightComponent->specular.g, pointLightEntityOne->lightComponent->specular.b);

        // Set the light position uniform
        GLint plPositionLoc = glGetUniformLocation(buffer->shaderProgram, "pointLights[2].position");
        glUniform3f(plPositionLoc, pointLightEntityOne->transformComponent->position[0], pointLightEntityOne->transformComponent->position[1], pointLightEntityOne->transformComponent->position[2]);

        // Set the light constant uniform
        GLint plConstantLoc = glGetUniformLocation(buffer->shaderProgram, "pointLights[2].constant");
        glUniform1f(plConstantLoc, pointLightEntityOne->lightComponent->constant);

        // Set the light linear uniform
        GLint plLinearLoc = glGetUniformLocation(buffer->shaderProgram, "pointLights[2].linear");
        glUniform1f(plLinearLoc, pointLightEntityOne->lightComponent->linear);

        // Set the light quadratic uniform
        GLint plQuadraticLoc = glGetUniformLocation(buffer->shaderProgram, "pointLights[2].quadratic");
        glUniform1f(plQuadraticLoc, pointLightEntityOne->lightComponent->quadratic);
    }
    Entity pointLightEntityFour_ = globals.lights[8];
    Entity* pointLightEntityFour = &pointLightEntityFour_;
    if(pointLightEntityFour != NULL && pointLightEntityFour->lightComponent != NULL){

        // Set lightColor uniform
        GLint lightColorLocation = glGetUniformLocation(buffer->shaderProgram, "lightColor");
        glUniform3f(lightColorLocation, 0.0f,1.0f,0.0f);

        // Set the light ambient uniform
        GLint plAmbientLoc = glGetUniformLocation(buffer->shaderProgram, "pointLights[3].ambient");
        glUniform3f(plAmbientLoc, pointLightEntityOne->lightComponent->ambient.r, pointLightEntityOne->lightComponent->ambient.g, pointLightEntityOne->lightComponent->ambient.b);

        // Set the light diffuse uniform
        GLint plDiffuseLoc = glGetUniformLocation(buffer->shaderProgram, "pointLights[3].diffuse");
        glUniform3f(plDiffuseLoc, pointLightEntityOne->lightComponent->diffuse.r, pointLightEntityOne->lightComponent->diffuse.g, pointLightEntityOne->lightComponent->diffuse.b);

        // Set the light specular uniform
        GLint plSpecularLoc = glGetUniformLocation(buffer->shaderProgram, "pointLights[3].specular");
        glUniform3f(plSpecularLoc, pointLightEntityOne->lightComponent->specular.r, pointLightEntityOne->lightComponent->specular.g, pointLightEntityOne->lightComponent->specular.b);

        // Set the light position uniform
        GLint plPositionLoc = glGetUniformLocation(buffer->shaderProgram, "pointLights[3].position");
        glUniform3f(plPositionLoc, pointLightEntityOne->transformComponent->position[0], pointLightEntityOne->transformComponent->position[1], pointLightEntityOne->transformComponent->position[2]);

        // Set the light constant uniform
        GLint plConstantLoc = glGetUniformLocation(buffer->shaderProgram, "pointLights[3].constant");
        glUniform1f(plConstantLoc, pointLightEntityOne->lightComponent->constant);

        // Set the light linear uniform
        GLint plLinearLoc = glGetUniformLocation(buffer->shaderProgram, "pointLights[3].linear");
        glUniform1f(plLinearLoc, pointLightEntityOne->lightComponent->linear);

        // Set the light quadratic uniform
        GLint plQuadraticLoc = glGetUniformLocation(buffer->shaderProgram, "pointLights[3].quadratic");
        glUniform1f(plQuadraticLoc, pointLightEntityOne->lightComponent->quadratic);
    }
      
    // Set viewPos uniform
    GLint viewPosLocation = glGetUniformLocation(buffer->shaderProgram, "viewPos");
    glUniform3f(viewPosLocation, camera->position[0], camera->position[1], camera->position[2]);

    // retrieve the matrix uniform locations
    unsigned int modelLoc = glGetUniformLocation(buffer->shaderProgram, "model");
    unsigned int viewLoc  = glGetUniformLocation(buffer->shaderProgram, "view");

    // pass them to the shaders 
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &transformComponent->transform[0][0]);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &camera->view[0][0]);

    glUniformMatrix4fv(glGetUniformLocation(buffer->shaderProgram, "projection"), 1, GL_FALSE, &camera->projection[0][0]);
      
    glBindVertexArray(buffer->VAO);
   if(buffer->numIndicies != 0) {
        glDrawElements(buffer->drawMode ,buffer->numIndicies,GL_UNSIGNED_INT,0);
    }else {
        //printf("vertexCount %d\n", buffer->vertexCount);
        glDrawArrays(buffer->drawMode, 0, buffer->vertexCount);
   }

   // debug drawcalls
   if(globals.debugDrawCalls){
        captureFramebuffer(globals.views.full.rect.width,globals.views.full.rect.height, globals.drawCallsCounter++);
   }

   glBindVertexArray(0);
   glBindTexture(GL_TEXTURE_2D, 0);
}



GLuint setupTexture(TextureData textureData){
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
    
    // Use tightly packed data , this necessary?
  //   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    

    printf("Texture width: %d\n", textureData.width);
    printf("Texture height: %d\n", textureData.height);
    printf("Texture channels: %d\n", textureData.channels);
    if(textureData.data == NULL){
        printf("Error: Texture data is null\n");
        return 0;
    }
    if(textureData.channels < 3){
        printf("Error: Texture must have at least 3 channels\n");
        return 0;
    }
    if(textureData.channels == 4){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureData.width, textureData.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData.data);
    }
    if(textureData.channels == 3){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureData.width, textureData.height, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData.data);
    }
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(textureData.data);
    return texture;
}

void setupFontMaterial(GpuData* buffer,int width,int height){
    // shader program
    //unsigned int shaderProgramId;
     #ifdef __EMSCRIPTEN__
        char* vertexShaderSource = readFile("shaders/wasm/text_vertex_wasm.glsl");
        char* fragmentShaderSource = readFile("shaders/wasm/text_fragment_wasm.glsl");
    #else
        char* vertexShaderSource = readFile("shaders/text_vertex.glsl");
        char* fragmentShaderSource = readFile("shaders/text_fragment.glsl");
    #endif
    if(fragmentShaderSource == NULL || vertexShaderSource == NULL) {
        printf("Error loading shader source\n");
        return;
    }
    
    printf("OpenGL ES version: %s\n", glGetString(GL_VERSION));

    // Compile shaders
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    if(vertexShader == 0) {
        printf("Error creating vertex shader\n");
        return;
    }
    glShaderSource(vertexShader, 1, (const GLchar* const*)&vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // Check for shader compile errors
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
    }

    // Fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    if(fragmentShader == 0) {
        printf("Error creating fragment shader\n");
        return;
    }
    glShaderSource(fragmentShader, 1, (const GLchar* const*)&fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
 
    // Check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
    }
   
    // free memory of shader sources
    free(vertexShaderSource);
    free(fragmentShaderSource);
    
    // Link shaders
    buffer->shaderProgram = glCreateProgram();

    glAttachShader(buffer->shaderProgram, vertexShader);
    glAttachShader(buffer->shaderProgram, fragmentShader);
    glLinkProgram(buffer->shaderProgram);

   
    printf("Shader program: %d\n", buffer->shaderProgram);

    // Check for linking errors
    glGetProgramiv(buffer->shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(buffer->shaderProgram, 512, NULL, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
    }
}
void setupFontMesh(GpuData *buffer){
    glGenVertexArrays(1, &buffer->VAO);
    glGenBuffers(1, &buffer->VBO);
    glBindVertexArray(buffer->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, buffer->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);  
}
void setFontProjection(GpuData *buffer,View view){
    glUseProgram(buffer->shaderProgram);

    mat4x4 projection;
    mat4x4_ortho(projection, 0.0f, view.rect.width, 0.0f, view.rect.height, -1.0f, 1.0f);
    glUniformMatrix4fv(glGetUniformLocation(buffer->shaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
}
void renderText(GpuData *buffer, char *text, float x, float y, float scale, Color color)
{
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glActiveTexture(GL_TEXTURE0);
    glUniform3f(glGetUniformLocation(buffer->shaderProgram, "textColor"), color.r, color.g, color.b);
    glBindVertexArray(buffer->VAO);

    // iterate through all characters
    for (unsigned char c = 0; c < strlen(text); c++) {
        Character ch = globals.characters[text[c]];
      
        float xpos = x + (float)ch.Bearing[0] * scale;
        float ypos = y - ((float)ch.Size[1] - (float)ch.Bearing[1]) * scale;

        float w = (float)ch.Size[0] * scale;
        float h = (float)ch.Size[1] * scale;    

        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },            
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }           
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, buffer->VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (float)(ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
   }  
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);

    // return opengl state to default
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ZERO);
}
void setupFontTextures(char* fontPath,int fontSize){
     FT_Library ft;
    if(FT_Init_FreeType(&ft)) {
        printf("ERROR::FREETYPE: Could not init FreeType Library\n");
        exit(1);
    }

    // Load font
    FT_Face face;
    if (FT_New_Face(ft, fontPath, 0, &face))
    {
        printf("ERROR::FREETYPE: Failed to load font\n");
        exit(1);
    }

    // Set font size
    // The function sets the font's width and height parameters. 
    // Setting the width to 0 lets the face dynamically calculate the width based on the given height.
    FT_Set_Pixel_Sizes(face, 0, fontSize);

    // A FreeType face hosts a collection of glyphs. 
    // We can set one of those glyphs as the active glyph by calling FT_Load_Char. 
    // Here we choose to load the character glyph 'X': 
    // load first 128 characters of ASCII set
     
      for (unsigned char char_code = 0; char_code < 128; char_code++) {
    
       
    
        
        if (FT_Load_Char(face, char_code, FT_LOAD_RENDER))
        {
            printf("ERROR::FREETYPE: Failed to load Glyph\n");
            exit(1);
        }
    
        //printf("number of glyphs in this font %ld \n", face->num_glyphs);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        globals.characters[char_code] = (Character){texture, {face->glyph->bitmap.width,face->glyph->bitmap.rows}, {face->glyph->bitmap_left,face->glyph->bitmap_top}, face->glyph->advance.x};
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    // Clean up FreeType library
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

}