#include "types.h"
#include "linmath.h"
#include "globals.h"
#include "opengl.h"



void setupMesh(Vertex* vertices, int vertexCount, unsigned int* indices, int indexCount, GpuData* buffer) {

    buffer->drawMode = GL_TRIANGLES;
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    

    // Texture attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    // Unbind VBO/buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Unbind VAO/vertex array
    glBindVertexArray(0);

}

void setupMaterial(GpuData* buffer){
     #ifdef __EMSCRIPTEN__
        char* vertexShaderSource = readFile("shaders/wasm/vertex_wasm.glsl");
        char* fragmentShaderSource = readFile("shaders/wasm/fragment_wasm.glsl");
    #else
        char* vertexShaderSource = readFile("shaders/vertex.glsl");
        char* fragmentShaderSource = readFile("shaders/fragment.glsl");
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

void renderMesh(GpuData* buffer,TransformComponent* transformComponent, Color* diffuse,Color* ambient, Color* specular,float shininess,GLuint diffuseMap) {
    
    // Set shader
    glUseProgram(buffer->shaderProgram);

    // Assign diffuseMap to texture1 slot
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseMap);
    glUniform1i(glGetUniformLocation(buffer->shaderProgram, "texture1"), 0);

    // Set the color uniform
    GLint colorLocation = glGetUniformLocation(buffer->shaderProgram, "color");
    glUniform4f(colorLocation, diffuse->r, diffuse->g, diffuse->b, diffuse->a);

    // Set the ambient light uniform
    GLint ambientLocation = glGetUniformLocation(buffer->shaderProgram, "ambient");
    glUniform4f(ambientLocation, ambient->r, ambient->g, ambient->b, ambient->a);

    // create view/camera transformation
    mat4x4 view;
    mat4x4_identity(view);
 /*    float radius = 3.0f; // camera rotation radius
    float camX = (float)sin(globals.delta_time) * radius; // camera rotation x
    float camY = 0.0f;
    float camZ = (float)cos(globals.delta_time) * radius; */
   // globals.camera.position = (vec3){0.0f, 0.0f, 3.0f};
  //const float cameraSpeed = 0.05f; // adjust accordingly
    //if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)

    // multiply camera speed float by camera front vector
    
/* globals.camera.position[0] += globals.camera.speed * globals.camera.front[0];
globals.camera.position[1] += globals.camera.speed * globals.camera.front[1];
globals.camera.position[2] += globals.camera.speed * globals.camera.front[2]; */



        
    // eye / position
    //vec3 cameraPosition = {0.0f, 0.0f, 3.0f};
    // target / center
    //vec3 cameraFront = {0.0f, 0.0f, -1.0f};
    globals.camera.target[0] = globals.camera.position[0] + globals.camera.front[0];
    globals.camera.target[1] = globals.camera.position[1] + globals.camera.front[1];
    globals.camera.target[2] = globals.camera.position[2] + globals.camera.front[2];
 /*    target[0] = cameraPosition[0] + cameraFront[0];
    target[1] = cameraPosition[1] + cameraFront[1];
    target[2] = cameraPosition[2] + cameraFront[2]; */
    // camera up
    //vec3 cameraUp = {0.0f, 1.0f, 0.0f};
    mat4x4_look_at(view, globals.camera.position, globals.camera.target, globals.camera.up);
    
    // assign to globals
   /*  globals.camera.position[0] = cameraPosition[0];
    globals.camera.position[1] = cameraPosition[1];
    globals.camera.position[2] = cameraPosition[2]; */
    
      
    // create transformations
    mat4x4 model;
    mat4x4_identity(model);
    mat4x4 projection;
    mat4x4_identity(projection);

    // set model position
    float x = 0.0f, y = 0.0f, z = 0.0f; // replace with your desired position
    mat4x4_translate(model, transformComponent->position[0],transformComponent->position[1],transformComponent->position[2]);

    // rotate model
    float degrees = 0.0f * globals.delta_time;
    float radians = degrees * M_PI / 180.0f;
    mat4x4 rotatedModel;
    // The cast on model tells the compiler that you're aware of the 
    // const requirement and that you're promising not to modify the model matrix.
    mat4x4_rotate(rotatedModel, (const float (*)[4])model, 0.0f,1.0f,0.0f, radians);
    // translate view
   // mat4x4_translate(view, 0.0f, 0.0f, -300.0f);
    // perspective projection
    mat4x4_perspective(projection, 45.0f, globals.views.main.width / globals.views.main.height, 0.1f, 100.0f);
    
    // retrieve the matrix uniform locations
    unsigned int modelLoc = glGetUniformLocation(buffer->shaderProgram, "model");
    unsigned int viewLoc  = glGetUniformLocation(buffer->shaderProgram, "view");

    // pass them to the shaders 
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &rotatedModel[0][0]);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);

    // note: currently we set the projection matrix each frame,
    // but since the projection matrix rarely changes it's often best practice
    // to set it outside the main loop only once.
    glUniformMatrix4fv(glGetUniformLocation(buffer->shaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
        
    glBindVertexArray(buffer->VAO);
   if(buffer->numIndicies != 0) {
        glDrawElements(buffer->drawMode ,buffer->numIndicies,GL_UNSIGNED_INT,0);
    }else {
        glDrawArrays(buffer->drawMode, 0, buffer->vertexCount);
   }
    glBindVertexArray(0);
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
