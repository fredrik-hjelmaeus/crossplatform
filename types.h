#ifndef TYPES_H
#define TYPES_H

#include <stdio.h>
#include <stdbool.h>
#include "opengl_types.h"
#include <ft2build.h>

#include FT_FREETYPE_H

typedef struct Color {
    GLfloat  r;
    GLfloat  g;
    GLfloat  b;
    GLfloat  a;
} Color;

typedef struct Arena {
    size_t size;  // Total size of the memory block
    size_t used;  // Amount of memory already used
    void* base;   // Pointer to the base of the memory block
} Arena;

typedef enum {
    SPLIT_DEFAULT = 0,
    SPLIT_HORIZONTAL = 1,
    SPLIT_VERTICAL = 2
} SplitDirection;

 typedef struct {
    unsigned int TextureID;  // ID handle of the glyph texture
    int Size[2];             // Size of glyph (width, height)
    int Bearing[2];          // Offset from baseline to left/top of glyph (x, y)
    unsigned int Advance;    // Offset to advance to next glyph
} Character;

typedef struct Camera {
    vec3 position;
    vec3 front;
    vec3 up;
    vec3 target;
    mat4x4 view;
    mat4x4 projection;
    float speed;
    bool viewMatrixNeedsUpdate;
    bool projectionMatrixNeedsUpdate;

    // Perspective
    float fov;
    float near;
    float far;
    float aspectRatio;
    // Orthographic
    float left;   // Left boundary for orthographic projection
    float right;  // Right boundary for orthographic projection
    float bottom; // Bottom boundary for orthographic projection
    float top;    // Top boundary for orthographic projection
    bool isOrthographic; // Flag to indicate if the camera is orthographic
} Camera;

typedef struct Rectangle{
    int x;      // Rectangle left position (top-left or bottom-left corner)
    int y;      // Rectangle top position (top-left or bottom-left corner)
    int width;  // Rectangle width
    int height; // Rectangle height
} Rectangle;

// TODO: introduce concept of Layouts?
typedef struct View {
    Rectangle rect; // x left start corner, y top start corner, x right corner = x + width, y bottom corner = y + height
    Color clearColor;
    Camera* camera; // Pointer to a camera, can be NULL if no camera
    bool isMousePointerWithin; // Flag to indicate if the mouse pointer is within the view
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

typedef struct Vertex {
    vec3 position;
    vec3 color;
    vec2 texcoord;
    vec3 normal;
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

typedef struct Material {
    bool active;
    Color ambient;
    Color diffuse;
    Color specular;
    float shininess;
    GLuint diffuseMap;
    float diffuseMapOpacity;
    GLuint specularMap;
  //  Texture* normalMap;
} Material;

typedef struct GpuData {
    GLuint VBO;
    GLuint VAO;
    GLuint EBO;
    GLuint shaderProgram;
    GLuint numIndicies;
    GLuint vertexCount;
    GLenum drawMode;
} GpuData;

// Data we get from obj-loader/parser.
// Child struct in ObjGroup.
typedef struct ObjData {
    char* name;
   // char* material;
    Vertex* vertexData;
    int num_of_vertices;
} ObjData;

// Data we get from obj-loader/parser. 
// Parent struct for ObjData.
typedef struct ObjGroup {
    char* name;
    int objectCount;
    ObjData* objData;
} ObjGroup;

// Used to instruct createMesh what type of data struct to setup.
typedef enum {
    VERTS_ONEUV = 0,
    VERTS_COLOR_ONEUV = 1,
    VERTS_COLOR_ONEUV_INDICIES = 2
} VertexDataType;

typedef void (*ButtonCallback)();

// ECS

enum Tag {
    UNINITIALIZED = 0,
    MODEL = 1,
    BOUNDING_BOX = 2,
    TEXT = 3,
    LIGHT = 4,
};
typedef struct Vector2{
    float x;
    float y;
} Vector2;

typedef struct TransformComponent {
    bool active;
    bool modelNeedsUpdate;
    vec3 position;
    vec3 rotation;
    vec3 scale;
    mat4x4 transform;
} TransformComponent;

typedef struct GroupComponent {
    bool active;
    mat4x4 transform;
} GroupComponent;

typedef struct MeshComponent {
    bool active;
    bool drawIndexed;
    Vertex* vertices;
    size_t vertexCount;
    unsigned int* indices;
    size_t indexCount;
    GpuData* gpuData;
} MeshComponent;

typedef struct UIComponent {
    bool active;
    bool hovered;
    bool clicked;
    Rectangle boundingBox;
    char* text;
    bool uiNeedsUpdate;
    ButtonCallback onClick;
    // padding?
    // margin?
    // offset?
    // border?
    // font?
    // fontSize?
    // textAlign
    // textOverflow?
    // opacity?
    // visibility?
} UIComponent;

typedef struct LightComponent {
    bool active;
    vec3 direction;
    Color ambient;
    Color diffuse;
    Color specular;
    float intensity;
} LightComponent;

typedef struct MaterialComponent {
    bool active;
    Color ambient;
    Color diffuse;
    Color specular;
    float shininess;
    GLuint diffuseMap;
    float diffuseMapOpacity;
    GLuint specularMap;
  //  Texture* normalMap;
} MaterialComponent;

typedef struct Entity {
    int id;
    bool alive;
    int tag;
    TransformComponent* transformComponent;
    GroupComponent* groupComponent;
    MeshComponent* meshComponent;
    MaterialComponent* materialComponent;
    UIComponent* uiComponent;
    LightComponent* lightComponent;
} Entity;

typedef vec3 Point;







#endif // TYPES_H
