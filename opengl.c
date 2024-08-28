#include "types.h"
#include "linmath.h"
#include "globals.h"
#include "opengl.h"



void setupMesh(Vertex* vertices, int vertexCount, unsigned int* indices, int indexCount, GpuData* buffer) {

    if(buffer->drawMode == NULL){
        buffer->drawMode = GL_TRIANGLES;
    }
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
void renderMesh(GpuData* buffer,TransformComponent* transformComponent, Color* diffuse,Color* ambient, Color* specular,float shininess,GLuint diffuseMap,Camera* camera,bool useDiffuseMap) {

    // Check if camera is NULL
    if (camera == NULL) {
        fprintf(stderr, "Error: camera is NULL\n");
        return;
    }
     
    // Set shader
    glUseProgram(buffer->shaderProgram);

    // Assign diffuseMap to texture1 slot
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseMap);
    glUniform1i(glGetUniformLocation(buffer->shaderProgram, "texture1"), 0);

    // Enable or disable the diffuse map
    //bool useDiffuseMap = true; // or false to disable
    glUniform1i(glGetUniformLocation(buffer->shaderProgram, "useDiffuseMap"), useDiffuseMap);

    // Set the color uniform
    GLint colorLocation = glGetUniformLocation(buffer->shaderProgram, "color");
    glUniform4f(colorLocation, diffuse->r, diffuse->g, diffuse->b, diffuse->a);

    // Set the ambient light uniform
    GLint ambientLocation = glGetUniformLocation(buffer->shaderProgram, "ambient");
    glUniform4f(ambientLocation, ambient->r, ambient->g, ambient->b, ambient->a);

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
GLuint setupFontTexture(FT_Face face){
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

    glBindTexture(GL_TEXTURE_2D, 0);
    return texture;
}

 void setupFontMaterial(GpuData* buffer,int width,int height){
     #ifdef __EMSCRIPTEN__
        #error "This function is not implemented for Emscripten."
      //  char* vertexShaderSource = readFile("shaders/wasm/vertex_wasm.glsl");
      //  char* fragmentShaderSource = readFile("shaders/wasm/fragment_wasm.glsl");
    #else
        char* vertexShaderSource = readFile("shaders/vert_text.glsl");
        char* fragmentShaderSource = readFile("shaders/frag_text.glsl");
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
    printf("width: %d\n", width);
    printf("height: %d\n", height); 
    

    // free memory of shader sources
    free(vertexShaderSource);
    free(fragmentShaderSource);

    // Link shaders
    buffer->shaderProgram = glCreateProgram();
    printf("setupFontMaterial\n");
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
 
    /* glUseProgram(buffer->shaderProgram);
    mat4x4 projection;
    mat4x4_ortho(projection, 0.0f, (float)width, 0.0f, (float)height, -1.0f, 1.0f);
    glUniformMatrix4fv(glGetUniformLocation(buffer->shaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);  */
}
 
 void renderText(GpuData *buffer, char *text, TransformComponent *transformComponent, Color *diffuse)
{
  
    // OpenGL state
    // ------------
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set shader
    glUseProgram(buffer->shaderProgram);
    mat4x4 projection;
    mat4x4_ortho(projection, 0.0f, 800.0f, 0.0f, 600.0f, -1.0f, 1.0f);
    GLint projLoc = glGetUniformLocation(buffer->shaderProgram, "projection");
    if (projLoc == -1) {
        printf("Could not find uniform location for 'projection'\n");
    }
    //glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]); 
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, (const GLfloat*)projection); 

    glActiveTexture(GL_TEXTURE0);
    glUniform3f(glGetUniformLocation(buffer->shaderProgram, "textColor"), 1.0f, 1.0f, 1.0f);
    glBindVertexArray(buffer->VAO);

    // iterate through all characters
   // std::string::const_iterator c;
  //  for (c = text.begin(); c != text.end(); c++) 
  //  {
        Character ch = globals.characters[0];
        // print all things about ch:
        printf("ch.TextureID: %d\n", ch.TextureID);
        printf("ch.Size[0]: %d\n", ch.Size[0]);
        printf("ch.Size[1]: %d\n", ch.Size[1]);
        printf("ch.Bearing[0]: %d\n", ch.Bearing[0]);
        printf("ch.Bearing[1]: %d\n", ch.Bearing[1]);
        printf("ch.Advance: %d\n", ch.Advance);
        
        float scale = 1.0f;
        float x = 1.0f;
        float y = 1.0f;

        float xpos = x + ch.Bearing[0] * scale;
        float ypos = y - (ch.Size[1] - ch.Bearing[1]) * scale;
        printf("xpos: %f\n", xpos);
        printf("ypos: %f\n", ypos);

        float w = ch.Size[0] * scale;
        float h = ch.Size[1] * scale;
        printf("w: %f\n", w);
        printf("h: %f\n", h);
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
       // x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
   // }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
     
}
   
void setupFontMesh(GpuData *buffer){
    glGenVertexArrays(1, &buffer->VAO);
    glGenBuffers(1, &buffer->VBO);
    glBindVertexArray(&buffer->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, &buffer->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}