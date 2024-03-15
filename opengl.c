#include "opengl.h"
#include "types.h"

GLuint sceneShaderProgram, uiShaderProgram;

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

GLuint tempSetupMaterial(){
     #ifdef __EMSCRIPTEN__
        char* vertexShaderSource = loadShaderSource("shaders/wasm/vertex_wasm.glsl");
        char* fragmentShaderSource = loadShaderSource("shaders/wasm/fragment_wasm.glsl");
    #else
        char* vertexShaderSource = loadShaderSource("shaders/vertex.glsl");
        char* fragmentShaderSource = loadShaderSource("shaders/fragment.glsl");
    #endif

    if(fragmentShaderSource == NULL || vertexShaderSource == NULL) {
        printf("Error loading shader source\n");
       // return;
    }

    printf("OpenGL ES version: %s\n", glGetString(GL_VERSION));

    // Compile shaders
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    if(vertexShader == 0) {
        printf("Error creating vertex shader\n");
      //  return;
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
       // return;
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

    GLuint shaderId;
    // Link shaders
    shaderId = glCreateProgram();
    glAttachShader(shaderId, vertexShader);
    glAttachShader(shaderId, fragmentShader);
    glLinkProgram(shaderId);

    printf("Shader program: %d\n", shaderId);

    // Check for linking errors
    glGetProgramiv(shaderId, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderId, 512, NULL, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
    }
    return shaderId;
}

void setupMaterial(GpuData* buffer){
     #ifdef __EMSCRIPTEN__
        char* vertexShaderSource = loadShaderSource("shaders/wasm/vertex_wasm.glsl");
        char* fragmentShaderSource = loadShaderSource("shaders/wasm/fragment_wasm.glsl");
    #else
        char* vertexShaderSource = loadShaderSource("shaders/vertex.glsl");
        char* fragmentShaderSource = loadShaderSource("shaders/fragment.glsl");
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

    glUseProgram(buffer->shaderProgram);
    glUniform1i(glGetUniformLocation(buffer->shaderProgram, "texture1"), 0);
}

void renderMesh(GpuData* buffer, Color* diffuse,Color* ambient, Color* specular,float shininess,GLuint diffuseMap) {
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseMap);
    
    glUseProgram(buffer->shaderProgram);
   // glUniform1i(glGetUniformLocation(buffer->shaderProgram, "texture1"), 0);

    // Set the color uniform
    GLint colorLocation = glGetUniformLocation(buffer->shaderProgram, "color");
    glUniform4f(colorLocation, diffuse->r, diffuse->g, diffuse->b, diffuse->a); 

    glBindVertexArray(buffer->VAO);
    glDrawElements(buffer->drawMode ,buffer->numIndicies,GL_UNSIGNED_INT,0);
    glBindVertexArray(0);
}

GLuint setupTexture(TextureData textureData){
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    printf("Texture width: %d\n", textureData.width);
    printf("Texture height: %d\n", textureData.height);
    printf("Texture channels: %d\n", textureData.channels);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureData.width, textureData.height, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData.data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(textureData.data);

    return texture;
}