#ifndef TYPES_H
#define TYPES_H

#include <stdio.h>
#include "opengl_types.h"

typedef struct View {
    int x;
    int y;
    int width;
    int height;
} View;

typedef struct Views {
    View main;
    View ui;
    View full;
} Views;

enum Viewport {
    VIEWPORT_MAIN = 0,
    VIEWPORT_UI = 1
};

typedef struct Color {
    GLfloat  r;
    GLfloat  g;
    GLfloat  b;
    GLfloat  a;
} Color;

typedef struct Vertex {
    vec3 position;
    vec3 color;
    vec2 texcoord;
    //vec3 normal;
} Vertex;

typedef struct TextureData {
    unsigned char* data;
    int width;
    int height;
    int channels;
} TextureData;

typedef struct Texture {
    GLuint id;
    char* type;
} Texture;

typedef struct GpuData {
    GLuint VBO;
    GLuint VAO;
    GLuint EBO;
    GLuint shaderProgram;
    GLuint numIndicies;
    GLuint vertexCount;
    GLenum drawMode;
} GpuData;

typedef struct Material {
    Color ambient;
    Color diffuse;
    Color specular;
    GLfloat shininess;
    GLuint diffuseMap;
   /* Texture* specularMap;
    Texture* normalMap; */
} Material;

// Data we get from obj-loader/parser.
typedef struct ObjData {
    Vertex* vertexData;
    int num_of_vertices;
} ObjData;

// Used to instruct createMesh what type of data struct to setup.
typedef enum {
    VERTS_ONEUV = 0,
    VERTS_COLOR_ONEUV = 1,
    VERTS_COLOR_ONEUV_INDICIES = 2
} VertexDataType;

// ECS

enum Tag {
    UNINITIALIZED = 0,
    MODEL = 1
};

typedef struct TransformComponent {
    int active;
    int isDirty;
    vec3 position;
    vec3 rotation;
    vec3 scale;
    mat4x4 transform;
} TransformComponent;

typedef struct GroupComponent {
    int active;
    mat4x4 transform;
} GroupComponent;

typedef struct MeshComponent {
    int active;
    int drawIndexed;
    Vertex* vertices;
    size_t vertexCount;
    unsigned int* indices;
    size_t indexCount;
    GpuData* gpuData;
} MeshComponent;

typedef struct UIComponent {
    int active;
} UIComponent;

typedef struct MaterialComponent {
    int active;
    Color ambient;
    Color diffuse;
    Color specular;
    float shininess;
    GLuint diffuseMap;
  //  Texture* specularMap;
  //  Texture* normalMap;
} MaterialComponent;

typedef struct Entity {
    int id;
    int alive;
    int tag;
    TransformComponent* transformComponent;
    GroupComponent* groupComponent;
    MeshComponent* meshComponent;
    MaterialComponent* materialComponent;
    UIComponent* uiComponent;
} Entity;

typedef struct Camera {
    vec3 position;
    vec3 front;
    vec3 up;
    vec3 target;
    mat4x4 view;
    mat4x4 projection;
    float speed;
} Camera;








#endif // TYPES_H
