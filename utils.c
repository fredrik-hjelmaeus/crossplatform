#include "utils.h"
#include "globals.h"
#include <ctype.h>

char* readFile(const char *filename) {

    FILE *fp;

    // open file
    fp = fopen(filename, "r");

    // Check if this file is null
    if (fp == NULL) {
        printf("Could not open file %s", filename);
        return NULL;
    }

    // Find out the size of the file
    fseek(fp, 0, SEEK_END);

    // Store the size of the file
    long filesize = ftell(fp);

    // seek back to the beginning of the file
    fseek(fp, 0, SEEK_SET);

    // allocate memory based on filesize and type
    char* buffer = malloc(sizeof(char) * filesize + 1);
    
    // read the file into the buffer
    size_t result = fread(buffer, sizeof(char), filesize, fp);
    if(result != filesize) {
        printf(TEXT_COLOR_ERROR "Error reading file %s" TEXT_COLOR_RESET, filename);
        return NULL;
    }

    // Null terminate the buffer
    buffer[filesize] = '\0';

    fclose(fp);
    return buffer;
}

// Get a random number from 0 to 255
int randInt(int rmin, int rmax) {
    return rand() % rmax + rmin;
}
float randFloat(float rmin, float rmax) {
    return (float)rand() / (float)RAND_MAX * (rmax - rmin) + rmin;
}

unsigned char* loadImage(const char* filename, int* width, int* height, int* nrChannels){
    stbi_set_flip_vertically_on_load(1); 
    unsigned char* result = stbi_load(filename, width, height, nrChannels, 0);
    if(result == NULL) {
        
        printf(TEXT_COLOR_ERROR "Error loading image %s" TEXT_COLOR_RESET "\n", filename);
        return NULL;
    }
    return result;
}

float deg2rad(float degrees) {
    return degrees * M_PI / 180.0;
}


/**
 * Parses faceline in loadObj.
 * example: "f 1/2/3 4/5/6 7/8/9";
 * If the line is a quad, it will be split into two triangles and add one additional faceLineCount.
 */
void handleFaceLine(char* line, int* vf, int* tf, int* vn, int* vfCount, int* tfCount, int* vnCount, int* faceLineCount) {
   
   int num_slashes = 0;
   int spaceCount = 0;
  
   bool isSlash = false;
   bool hasTextureData = true;
   char tempNumber[10];
   int tempNumberIndex = 0;
   int tokenIndex = 0;
   int tokensArr[12] = {0};
   char* linePtr = (line+2); // skip f and space
   while(*linePtr != '\0'){

       //printf("linePtr--------: %c\n", *linePtr);
       //printf("tknIndex-------: %d\n", tokenIndex);
    
       if(*linePtr == ' '){
           spaceCount++;
           tempNumber[tempNumberIndex] = '\0'; // Null-terminate the string
           int num = atoi(tempNumber);
           tokensArr[tokenIndex] = num;
           tokenIndex++;
           //printf("space num: %d\n", num);
           tempNumberIndex = 0;
           isSlash = false;
       }
       else if(*linePtr == '/'){
         num_slashes++;
         if(isSlash){
            //printf("this is a double slash\n");
            hasTextureData = false;
         }
         if(tempNumberIndex > 0){
            tempNumber[tempNumberIndex] = '\0'; // Null-terminate the string
            int num = atoi(tempNumber);
            tokensArr[tokenIndex] = num;
            tokenIndex++;
            tempNumberIndex = 0;
           // printf("slash num: %d\n", num);
         }
         isSlash = true;
       }
       else{
        tempNumber[tempNumberIndex] = *linePtr;
        tempNumberIndex++;
        isSlash = false;
       }
       linePtr++;
   }
   // Pick up the last number
   if(tempNumberIndex > 0){
         tempNumber[tempNumberIndex] = '\0'; // Null-terminate the string
         int num = atoi(tempNumber);
         tokensArr[tokenIndex] = num;
        // printf("last num: %d\n", num);
   }


     // if has texture and is triangle,the token count should be 9
    // tokenIndex 0 , 3 , 6 is v
    // tokenIndex 1 , 4 , 7 is vt
    // tokenIndex 2 , 5 , 8 is vn
   // if has texture and is quad, the token count should be 12
    // tokenIndex 0 , 3 , 6 , 9 is v
    // tokenIndex 1 , 4 , 7 , 10 is vt
    // tokenIndex 2 , 5 , 8 , 11 is vn
   // if has no texture and is triangle, the token count should be 6
    // tokenIndex 0 , 2 , 4 is v
    // tokenIndex 1 , 3 , 5 is vn
   // if has no texture and is quad, the token count should be 8
    // tokenIndex 0 , 2 , 4 , 6 is v
    // tokenIndex 1 , 3 , 5 , 7 is vn
    if(hasTextureData){
            vf[*vfCount] = tokensArr[0];
            (*vfCount)++;
            tf[*tfCount] = tokensArr[1];
            (*tfCount)++;
            vn[*vnCount] = tokensArr[2];
            (*vnCount)++;
            vf[*vfCount] = tokensArr[3];
            (*vfCount)++;
            tf[*tfCount] = tokensArr[4];
            (*tfCount)++;
            vn[*vnCount] = tokensArr[5];
            (*vnCount)++;
            vf[*vfCount] = tokensArr[6];
            (*vfCount)++;
            tf[*tfCount] = tokensArr[7];
            (*tfCount)++;
            vn[*vnCount] = tokensArr[8];
            (*vnCount)++;
        if(spaceCount > 2){

           // printf("Quad with texture(uv) data\n");
            
            //First triangle: v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3
            //Second triangle: v1/vt1/vn1 v3/vt3/vn3 v4/vt4/vn4

            // First vertex
            vf[*vfCount] = tokensArr[0];
            (*vfCount)++;
            tf[*tfCount] = tokensArr[1];
            (*tfCount)++;
            vn[*vnCount] = tokensArr[2];
            (*vnCount)++;

            // Second vertex
            vf[*vfCount] = tokensArr[6];
            (*vfCount)++;
            tf[*tfCount] = tokensArr[7];
            (*tfCount)++;
            vn[*vnCount] = tokensArr[8];
            (*vnCount)++;

            // Third vertex
            vf[*vfCount] = tokensArr[9];
            (*vfCount)++;
            tf[*tfCount] = tokensArr[10];
            (*tfCount)++;
            vn[*vnCount] = tokensArr[11];
            (*vnCount)++;

            // We create additional triangle so we need to increment faceLineCount
            (*faceLineCount)++;

        }else {
          //  printf("Triangle with texture(uv) data\n");
        }
    }else {
        vf[*vfCount] = tokensArr[0];
        (*vfCount)++;
        vn[*vnCount] = tokensArr[1];
        (*vnCount)++;
        vf[*vfCount] = tokensArr[2];
        (*vfCount)++;
        vn[*vnCount] = tokensArr[3];
        (*vnCount)++;
        vf[*vfCount] = tokensArr[4];
        (*vfCount)++;
        vn[*vnCount] = tokensArr[5];
        (*vnCount)++;
        if(spaceCount > 2){
        
            //First triangle: v1//vn1 v2//vn2 v3//vn3
            //Second triangle: v1//vn1 v3//vn3 v4//vn4

            // First vertex
            vf[*vfCount] = tokensArr[0];
            (*vfCount)++;
            vn[*vnCount] = tokensArr[1];
            (*vnCount)++;
            // Second vertex
            vf[*vfCount] = tokensArr[4];
            (*vfCount)++;
            vn[*vnCount] = tokensArr[5];
            (*vnCount)++;
            // Third vertex
            vf[*vfCount] = tokensArr[6];
            (*vfCount)++;
            vn[*vnCount] = tokensArr[7];
            (*vnCount)++;

            // We create additional triangle so we need to increment faceLineCount
            (*faceLineCount)++;
            //printf("Quad without texture(uv) data\n");

        }else {
           // printf("Triangle without texture(uv) data\n");
        }
    }

    (*faceLineCount)++;
}

/**
 * Run tests.
 * Atm no expect/asserts, just printouts you manually have to check.
 */
void runTests()
{
   char line[] =  "f 1/2/3 4/5/6 7/8/9"; // 9
   char line2[] = "f 7//7 8//8 9//9"; // 6
   char line3[] = "f 7//7 8//8 9//9 10//10"; // 8
   char line4[] = "f 7555555//55555557 83333333//833333333 222222229//222222229"; // 6
   char line5[] = "f 1/2/3 4/5/6 7/8/9 3/3/3"; // 12

   int vf[300000] = {0}; 
   int tf[300000] = {0}; 
   int vn[300000] = {0};
  
   int faceLineCount = 0;
   
   int vfCount = 0;
   int tfCount = 0;
   int vnCount = 0; 

   handleFaceLine(line,  vf, tf, vn, &vfCount, &tfCount, &vnCount, &faceLineCount);
   handleFaceLine(line2, vf, tf, vn, &vfCount, &tfCount, &vnCount, &faceLineCount);
   handleFaceLine(line3, vf, tf, vn, &vfCount, &tfCount, &vnCount, &faceLineCount);
   handleFaceLine(line4, vf, tf, vn, &vfCount, &tfCount, &vnCount, &faceLineCount);
   handleFaceLine(line5, vf, tf, vn, &vfCount, &tfCount, &vnCount, &faceLineCount);

   // Exit program after tests
   exit(0);
}

typedef struct ObjObject {
    char name[100];
    int start;
    int end;
} ObjObject;

void initMemoryArena(Arena* arena, size_t size) {
    arena->size = size;
    arena->base = malloc(size);
    if (arena->base == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);  // Exit or handle the error as appropriate
    }
    arena->used = 0;
}

void* arenaAlloc(Arena* arena, size_t size) {
    // Debug output
  /*   printf("size i want to allocate: %zu \n",size);
    printf("used size: %zu \n",arena->used);
    printf("arena size: %zu \n",arena->size); */
    if (arena->used + size > arena->size) {
        fprintf(stderr, "Arena out of memory\n");
        exit(1);
    }
    void* ptr = (char*)arena->base + arena->used;
    arena->used += size;
    
    return ptr;
}
// Not evaluated yet, do this before using
void resetArena(Arena* arena) {
    arena->used = 0;
}
// Not evaluated yet, do this before using 
void freeArena(Arena* arena) {
    free(arena->base);
    arena->base = NULL;
    arena->used = 0;
}

/**
 * Analyse filepath and make it work on different platforms.
 * For ex. on Linux, convert to \\ to to /, add /mnt/ , & change D: to d  ,
 * Input:  D:\\OneDrive\\viz\\texturer\\Renderingking texturepack V3\\old bricks.jpg
 * result: /mnt/d/OneDrive/viz/texturer/Renderingking texturepack V3/old bricks.jpg
 */
char* handleFilePath(const char* filepath){
    char* newPath = (char*)arenaAlloc(&globals.assetArena, (strlen(filepath) + 1) * sizeof(char));
    if(isupper(filepath[0]) && filepath[1] == ':'){
        strcpy(newPath,"/mnt/");
        char* lower = tolower(filepath[0]);
        newPath[5] = lower;
        int index = 5;
       
        index++;
       for(int i = 2; i < strlen(filepath); i++){
           // printf("char: %c\n", filepath[i]);
            if(filepath[i] == '\\'){
                newPath[index++] = '/';
                i++;
                continue;
            }
            newPath[index++] = filepath[i];
        } 
   
        newPath[index] = '\0'; // Null-terminate the new path
        //printf("newpath %s \n ",newPath);
    }else{
        printf(TEXT_COLOR_ERROR "handleFilePath failed %s" TEXT_COLOR_RESET "\n", filepath);
        exit(1);
    }

    return newPath;
}


/**
 * Parse .mtl file.
 * spec: https://en.wikipedia.org/wiki/Wavefront_.obj_file
 * https://paulbourke.net/dataformats/mtl/
 */
void parseObjMaterial(const char *filepath){
    
    // Temporary allocate new materials on the stack. 
    // We will copy them to the assetArena after we 
    // know how many materials we have and before this fn ends.
    Material materials[100]; 
    int materialIndex = -1; // for every material
    
    // Check if .mtl file with same name exists, if so load&parse it.
    char* dest = (char*)arenaAlloc(&globals.assetArena, 99 * sizeof(char));
    char* dot = (char*)arenaAlloc(&globals.assetArena, 99 * sizeof(char));
    strcpy(dot, "."); // Initialize dot with a period
    char *filepath_copy = strcpy(dest,filepath);
    dest = strtok(dest, ".");
    dest = strcat(dot, dest);
    dest = strcat(dest, ".mtl");
    printf("dest: %s\n", dest);
    
    // Open .mtl file
    FILE* mtlfp;
    char mtlLine[1024];

    mtlfp = fopen(dest, "r");
    if(mtlfp == NULL) {
        printf(TEXT_COLOR_ERROR "Error opening file %s" TEXT_COLOR_RESET "\n", dest);
        exit(1);
    }

    // Parse .mtl file
    while (fgets(mtlLine, sizeof mtlLine, mtlfp) != NULL){
    // printf(TEXT_COLOR_ERROR "mtl line %s" TEXT_COLOR_RESET "\n", mtlLine);
        // comment
        if((int)mtlLine[0] == 35){
            continue;
        }
        // newline
        if((int)mtlLine[0] == 10){
            continue;
        }
        //newmtl, new material
        if(strncmp(mtlLine, "newmtl", 6) == 0){
           char* token = strtok(mtlLine, " ");
           token = strtok(NULL, " ");
           materialIndex++;
           materials[materialIndex].active = true;
           materials[materialIndex].name = (char*)arenaAlloc(&globals.assetArena, (strlen(token) + 1) * sizeof(char));
           strcpy(materials[materialIndex].name, token);
              
           printf("new material %s \n",materials[materialIndex].name);
        }
        //Ka, ambient color
        if(strncmp(mtlLine, "Ka", 2) == 0){
            sscanf(mtlLine, "Ka %f %f %f", &materials[materialIndex].ambient.r, &materials[materialIndex].ambient.g, &materials[materialIndex].ambient.b);
            printf("ambient color %f %f %f \n",materials[materialIndex].ambient.r, materials[materialIndex].ambient.g, materials[materialIndex].ambient.b);
        }
        //Kd, diffuse color
        if(strncmp(mtlLine, "Kd", 2) == 0){
            sscanf(mtlLine, "Kd %f %f %f", &materials[materialIndex].diffuse.r, &materials[materialIndex].diffuse.g, &materials[materialIndex].diffuse.b);
            printf("diffuse color %f %f %f \n",materials[materialIndex].diffuse.r, materials[materialIndex].diffuse.g, materials[materialIndex].diffuse.b);
        }
        //Ks, specular color
        if(strncmp(mtlLine, "Ks", 2) == 0){
            sscanf(mtlLine, "Ks %f %f %f", &materials[materialIndex].specular.r, &materials[materialIndex].specular.g, &materials[materialIndex].specular.b);
            printf("specular color %f %f %f \n",materials[materialIndex].specular.r, materials[materialIndex].specular.g, materials[materialIndex].specular.b);
        }
        //Ke, emissive color
      /*   if(strncmp(mtlLine, "Ke", 2) == 0){
            sscanf(mtlLine, "Ke %f %f %f", &materials[materialIndex].emissive.r, &materials[materialIndex].emissive.g, &materials[materialIndex].emissive.b);
            printf("emissive color %f %f %f \n",materials[materialIndex].emissive.r, materials[materialIndex].emissive.g, materials[materialIndex].emissive.b);
        } */
        //Ns, shininess/specular exponent
        //d,  dissolve(transparency) & Tr, transparency
       /*  if(strncmp(mtlLine, "d", 1) == 0){
            sscanf(mtlLine, "d %f", &materials[materialIndex].transparency);
            printf("transparency %f \n",materials[materialIndex].transparency);
        } */
        //     # some implementations use 'd'
        //d 0.9
        //     # others use 'Tr' (inverted: Tr = 1 - d)
        //Tr 0.1
        //     Transparent materials can additionally have a 
        //     Transmission Filter Color, specified with "Tf".
        //     # Transmission Filter Color (using R G B)
        //     Transparent materials can additionally have a Transmission Filter Color, specified with "Tf".
        //Tf 1.0 0.5 0.5 
        //     # Transmission Filter Color (using CIEXYZ) - y and z values are optional and assumed to be equal to x if omitted
        //Tf xyz 1.0 0.5 0.5 
        //     # Transmission Filter Color from spectral curve file (not commonly used)
        //     Tf spectral <filename>.rfl <optional factor>
        //     A material can also have an optical density for its surface. 
        //     This is also known as index of refraction. (IOR)
        //Ni 1.45000
       /*  if(strncmp(mtlLine, "Ni", 2) == 0){
            sscanf(mtlLine, "Ni %f", &materials[materialIndex].ior);
            printf("optical density(ior) %f \n",materials[materialIndex].ior);
        } */
        //     # Illumination model (see below) https://en.wikipedia.org/wiki/List_of_common_shading_algorithms
        //illum 2  (0-10), 2 is Color on and Ambient on.
                    /* 
                    0. Color on and Ambient off
                    1. Color on and Ambient on
                    2. Highlight on
                    3. Reflection on and Ray trace on
                    4. Transparency: Glass on, Reflection: Ray trace on
                    5. Reflection: Fresnel on and Ray trace on
                    6. Transparency: Refraction on, Reflection: Fresnel off and Ray trace on
                    7. Transparency: Refraction on, Reflection: Fresnel on and Ray trace on
                    8. Reflection on and Ray trace off
                    9. Transparency: Glass on, Reflection: Ray trace off
                    10. Casts shadows onto invisible surfaces */
        //map_Ka   lemur.tga  can have optional params: map_Ka -o 1 1 1 ambient.tga
        //  The options and their arguments are inserted between the 
        //  keyword and the "filename". 
        //  map_Ka -options args filename
        if(strncmp(mtlLine, "map_Ka", 6) == 0){
            char* token = strtok(mtlLine, " ");
            token = strtok(NULL, "");
            // null terminate token
            token[strlen(token) - 1] = '\0';
            printf("loading texture %s \n", token);

            char* filepath = handleFilePath(token);
            // /mnt/ d D: \\OneDrive\\viz\\texturer\\Renderingking texturepack V3\\old bricks.jpg
            // D:\\OneDrive\\viz\\texturer\\Renderingking texturepack V3\\old bricks.jpg
            TextureData texture = loadTexture(filepath);
            GLuint ambientMap = setupTexture(texture);
             materials[materialIndex].diffuseMap = ambientMap;
        }
        //map_Kd   lemur.tga
        if(strncmp(mtlLine, "map_Kd", 6) == 0){
            char* token = strtok(mtlLine, " ");
            token = strtok(NULL, "");
            // null terminate token
            token[strlen(token) - 1] = '\0';
            printf("loading texture %s \n", token);

            char* filepath = handleFilePath(token);
            // /mnt/ d D: \\OneDrive\\viz\\texturer\\Renderingking texturepack V3\\old bricks.jpg
            // D:\\OneDrive\\viz\\texturer\\Renderingking texturepack V3\\old bricks.jpg
            TextureData texture = loadTexture(filepath);
            GLuint diffuseMap = setupTexture(texture);
            materials[materialIndex].diffuseMap = diffuseMap;
        }
        //map_Ks   lemur.tga
        //map_Ns   lemur_spec.tga
        //map_d    lemur_alpha.tga (alpha)
        //          # some implementations use 'map_bump' instead of 'bump' below
        //map_bump  lemur_bump.tga
        //bump      # bump map (which by default uses luminance channel of the image)
        //disp      lemur_disp.tga
        //          # stencil decal texture (defaults to 'matte' channel of the image)
        //decal     lemur_stencil.tga
        //          # spherical reflection map
        //refl      -type sphere clouds.tga
        //          TEXTURES CAN HAVE OPTIONS, 
        //          SEE Texture options here: https://en.wikipedia.org/wiki/Wavefront_.obj_file
        //        Physically-based rendering (PBR) parameters
        /*          The creators of the online 3D editing and modeling tool, 
                    Clara.io, proposed extending the MTL format to enable specifying physically-based rendering 
                    (PBR) maps and parameters. This extension has been subsequently adopted by Blender and TinyObjLoader. 
                    The extension PBR maps and parameters are */
        //Pr/map_Pr  #roughness
        //Pm/map_Pm  #metallic
        //Ps/map_Ps  #sheen
        //Pc         #clearcoat thickness
        //Pcr        #clearcoat roughness
        //Ke/map_Ke  #emissive
        //aniso      #anisotropy
        //anisor     #anisotropy rotation
        //norm       #normal map (RGB components represent XYZ components of the surface normal)
        //           RMA
        //map_RMA
        //map_ORM

      
        // v: v & space(32), read vertices
       /*  if((int)mtlLine[0] == 118 && (int)mtlLine[1] == 32){
          //  printf(TEXT_COLOR_ERROR "v %s" TEXT_COLOR_RESET "\n", line);
          sscanf(mtlLine, "v %f %f %f", &KaArr[mIndex], &KaArr[mIndex+1], &KaArr[mIndex+2]);
          mIndex+=3;
          continue;
        } */
       
    
    }
    fclose(mtlfp);
    
}

// Limit set to 100 o objects !. No indicies created.
// Collects vertices position(vArr),uv/texcoords(tArr) , vertex indices(vf) ,texture indices(tf), normal indices(vn) ,material and object.
// Then uses these and creates a new vertex data 
// final attribute looking like this: x, y ,z, u ,v, nx, ny, nz
// We convert quad to two triangles (handleLineFace) and add one to faceLineCount.
// Support to handle facelines with and without texture data. Example: f 1/2/3 4/5/6 7/8/9 or f 7//7 8//8 9//9
// obj specification: https://paulbourke.net/dataformats/obj/ 
// mtl specification: https://paulbourke.net/dataformats/mtl/
// & https://en.wikipedia.org/wiki/Wavefront_.obj_file
ObjGroup* loadObjFile(const char *filepath)
{
    // How many vertices can we store in the obj file
    #define OBJDATA_MAX 300001

    #define MAX_NUM_OBJECTS = 100
    
    // Allocate memory for the vertex data parsing
    int* vf = (int*)arenaAlloc(&globals.assetArena, OBJDATA_MAX * sizeof(int));
    int* tf = (int*)arenaAlloc(&globals.assetArena, OBJDATA_MAX * sizeof(int));
    int* vn = (int*)arenaAlloc(&globals.assetArena, OBJDATA_MAX * sizeof(int));
    float* vArr = (float*)arenaAlloc(&globals.assetArena, OBJDATA_MAX * sizeof(float));
    float* tArr = (float*)arenaAlloc(&globals.assetArena, OBJDATA_MAX * sizeof(float));
    float* nArr = (float*)arenaAlloc(&globals.assetArena, OBJDATA_MAX * sizeof(float));

    // An obj can have multiple objects. Every object gets placed in objData. All objData gets placed in a objGroup.
    // This objGroup is what is returned.
    ObjGroup* objGroup = (ObjGroup*)arenaAlloc(&globals.assetArena, sizeof(ObjGroup));
    objGroup->name = (char*)arenaAlloc(&globals.assetArena, 100 * sizeof(char)); // TODO: unneccessary?
    objGroup->name = filepath;
    objGroup->objData = (ObjData*)arenaAlloc(&globals.assetArena, 10 * sizeof(ObjData));
    objGroup->objectCount = 0;
   
    int vIndex = 0;
    int uvCount = 0;
    int normalCount = 0;
    int vfCount = 0;
    int tfCount = 0;
    int vnCount = 0;

    int faceLineCount = 0;

    // Keeps track of where objects faceLineCount.
    int faceLineCountStart[100];// = {0};// 100 is max num of object
    int faceLineCountEnd[100];// = {0};   // 100 is max num of object
    char material[100];
    char group[100];
    char usemtl[100];


    
    parseObjMaterial(filepath);
    
    

    FILE* fp;
    char line[1024];

    fp = fopen(filepath, "r");
    if(fp == NULL) {
        printf(TEXT_COLOR_ERROR "Error opening file %s" TEXT_COLOR_RESET "\n", filepath);
        exit(1);
    }
    
    while (fgets(line, sizeof line, fp) != NULL){
    // printf(TEXT_COLOR_ERROR "line %s" TEXT_COLOR_RESET "\n", line);
        // comment
        if((int)line[0] == 35){
            continue;
        }
        // newline
        if((int)line[0] == 10){
            continue;
        }
        // m, check for mtllib
        if((int)line[0] == 109){
            if(strncmp(line, "mtllib", 6) == 0){
                for(int i = 7; i < strlen(line); i++){
                    material[i-7] = line[i];
                }
            }
            continue;
        }
        // o, object
        if((int)line[0] == 111){
           // printf(TEXT_COLOR_BLUE "new object: %s" TEXT_COLOR_RESET , line);
            int lineLength = strlen(line);
            if(lineLength >= 100){
                printf("Error: Object name too long. Exiting..");
                exit(1);
            }
        
            // Object name
           objGroup->objData[objGroup->objectCount].name = (char*)arenaAlloc(&globals.assetArena, lineLength * sizeof(char));
           for(int i = 0; i < lineLength; i++){
                objGroup->objData[objGroup->objectCount].name[i] = line[i];
            }  
            objGroup->objData[objGroup->objectCount].name[lineLength-1] = '\0'; // Null terminate the string
            
            faceLineCountStart[objGroup->objectCount] = faceLineCount; 
            if(objGroup->objectCount > 0){
                ASSERT(faceLineCount > 0, "Error: faceLineCount is 0 or less");
                ASSERT(faceLineCount < 100000, "Error: faceLineCount is too large");
                int pi = (int)(objGroup->objectCount-1);
                faceLineCountEnd[pi] = faceLineCount; 
                int num_faceLineCountCurrentObject = (int)faceLineCountEnd[pi] - (int)faceLineCountStart[pi];
                objGroup->objData[pi].num_of_vertices =  (num_faceLineCountCurrentObject * 3);  
            }
            objGroup->objectCount++;
            if(objGroup->objectCount > 100){
                printf("Error: Too many objects. Exiting..");
                exit(1);
            }
        }
        // g, grouping, read in the groups
        if((int)line[0] == 103){
            for(int i = 2; i < strlen(line); i++){
                group[i-2] = line[i];
            }
            continue;
        }
        // u, check for usemtl
        if((int)line[0] == 117){
            if(strncmp(line, "usemtl", 6) == 0){
                printf("ignoring material %s \n",line);
             //   objGroup.objData[objectCount].material = (char*)arenaAlloc(&globals.assetArena, 100 * sizeof(char));
             //   objGroup.objData[objectCount].material = strdup(usemtl);
             /*    for(int j = 7; j < strlen(line); j++){
                    usemtl[j-7] = line[j];
                } */
               // usemtl[strlen(line)-7] = '\0'; // Null terminate usemtl
            }
            continue;
        }
        // v: v & space(32), read vertices
        if((int)line[0] == 118 && (int)line[1] == 32){
          //  printf(TEXT_COLOR_ERROR "v %s" TEXT_COLOR_RESET "\n", line);
          sscanf(line, "v %f %f %f", &vArr[vIndex], &vArr[vIndex+1], &vArr[vIndex+2]);
          vIndex+=3;
          continue;
        }
        // vt: v & t, read texcoords(uv)
        if((int)line[0] == 118 && (int)line[1] == 116){
            char *token = strtok(line, " ");
            token = strtok(NULL, " ");
            tArr[uvCount] = strtof(token, NULL);
            token = strtok(NULL, " ");
            tArr[uvCount+1] = strtof(token, NULL);
            uvCount+=2;
            continue;
        }
        // vn: v & n, read normals
        if((int)line[0] == 118 && (int)line[1] == 110){
            char *token = strtok(line, " ");
            token = strtok(NULL, " ");
            nArr[normalCount] = strtof(token, NULL);
            token = strtok(NULL, " ");
            nArr[normalCount+1] = strtof(token, NULL);
            token = strtok(NULL, " ");
            nArr[normalCount+2] = strtof(token, NULL);
            normalCount+=3;
            continue;
        }
        // f: face indicies, 
        // Can be : f 1/1/1 2/2/2 3/3/3
        // or       f 1//1 2//2 3//3
        // or       f 1//1 2//2 3//3 4//4  <- quad
        if((int)line[0] == 102){
           handleFaceLine(line, vf, tf, vn, &vfCount, &tfCount, &vnCount, &faceLineCount);
           if(vfCount >= OBJDATA_MAX || tfCount >= OBJDATA_MAX){
                printf("Error: Too many vertices in obj file. OBJDATA_MAX exceeded. Exiting..");
                printf("vfCount: %d\n", vfCount);
                printf("tfCount: %d\n", tfCount);
                exit(1);
           }
        }
  
    }
    fclose(fp);

    if(objGroup->objectCount > 0){
        // Input Last object
        faceLineCountEnd[objGroup->objectCount-1] = faceLineCount;
        objGroup->objData[objGroup->objectCount-1].num_of_vertices = (faceLineCountEnd[objGroup->objectCount-1] - faceLineCountStart[objGroup->objectCount-1]) * 3;
      
    }else {
        // If file contain o object, this was specified there. But if not, we need to create a default object here.
        objGroup->objData[0].num_of_vertices = (faceLineCount * 3);
        objGroup->objData[0].vertexData = (Vertex*)arenaAlloc(&globals.assetArena, objGroup->objData[0].num_of_vertices * sizeof(Vertex));
    }
    
    // Allocate memory for the vertex data in objData
    if(objGroup->objectCount > 0){
        for(int i = 0; i < objGroup->objectCount; i++){
            objGroup->objData[i].vertexData = (Vertex*)arenaAlloc(&globals.assetArena, objGroup->objData[i].num_of_vertices * sizeof(Vertex));
        }
    }

    // TODO: use this structure instead for filling vertex data?. The problem is that it is not working.
    //for (int i = 0; i < objGroup->objectCount; i++) {                                    <---  loop over objects
    //    for (int j = faceLineCountStart[i]; j < faceLineCountEnd[i]; j++) {              <---  loop over faces
    //  for(int k = 0; k < 3; k++){                                                        <---  loop over vertices 
    // NOTE: Atm we fill vertexData in only objData[0] in objGroup. Somehow this works and we can access and use objData[1], objData[2] etc. 
    // when creatObject is called. I presume this is because of the arenaAlloc and the memory is allocated in the same memory block,so it is accessible.
    // But why can't we directly fill objData[1], objData[2] etc. with vertexData?. This is a unsolved mystery.
        
    // Fill vertex data
    int vertexIndex = 0;
    for(int j = 0; j < faceLineCount; j+=1){ 

        for(int i = 0; i < 3; i++){
        
            // x y z
            objGroup->objData->vertexData[vertexIndex].position[0] = vArr[(vf[vertexIndex]-1)*3];
            objGroup->objData->vertexData[vertexIndex].position[1] = vArr[(vf[vertexIndex]-1)*3+1];
            objGroup->objData[0].vertexData[vertexIndex].position[2] = vArr[(vf[vertexIndex]-1)*3+2];
           
            // color, default to black atm
            objGroup->objData[0].vertexData[vertexIndex].color[0] = 1.0; 
            objGroup->objData[0].vertexData[vertexIndex].color[1] = 1.0;
            objGroup->objData[0].vertexData[vertexIndex].color[2] = 1.0;
           
            // uv
            if(uvCount == 0){
                objGroup->objData[0].vertexData[vertexIndex].texcoord[0] = 0.0;
                objGroup->objData[0].vertexData[vertexIndex].texcoord[1] = 0.0;
            }else {
                objGroup->objData[0].vertexData[vertexIndex].texcoord[0] = tArr[(tf[vertexIndex]-1)*2];
                objGroup->objData[0].vertexData[vertexIndex].texcoord[1] = tArr[(tf[vertexIndex]-1)*2+1];
            }

            // normals
            if(vnCount == 0){
                objGroup->objData[0].vertexData[vertexIndex].normal[0] = 0.0;
                objGroup->objData[0].vertexData[vertexIndex].normal[1] = 0.0;
                objGroup->objData[0].vertexData[vertexIndex].normal[2] = 0.0;
            }else {
                objGroup->objData[0].vertexData[vertexIndex].normal[0] = nArr[(vn[vertexIndex]-1)*3];
                objGroup->objData[0].vertexData[vertexIndex].normal[1] = nArr[(vn[vertexIndex]-1)*3+1];
                objGroup->objData[0].vertexData[vertexIndex].normal[2] = nArr[(vn[vertexIndex]-1)*3+2]; 
            }
            vertexIndex++;
        }
    }

  return objGroup;
}

// Function to convert an integer to a string and append ".png"
void intToPngFilename(int number, char* buffer, size_t bufferSize) {
    if (bufferSize < 10) {
        fprintf(stderr, "Buffer size is too small\n");
        return;
    }
    snprintf(buffer, bufferSize, "%d.png", number);
}


//----------------------------------------
//---COLLISION DETECTION------------------
//----------------------------------------

/**
 *  Function to check if a point is inside the rectangle.
 * NOTE: x,y,width,height is expected to be in SDL coordinates, Where x,y is the bottom left corner of the rectangle.
 * use convertViewRectangleToSDLCoordinates to convert view coordinates to SDL coordinates if needed.
 * @param rect Rectangle to check against, x,y,width,height is expected to be in SDL coordinates
 */ 
int isPointInsideRect(Rectangle rect, vec2 point) {
    return (point[0] >= rect.x && point[0] <= rect.x + rect.width &&
            point[1] >= rect.y && point[1] <= rect.y + rect.height);
}

/**
 * Function to convert a view rectangle to SDL coordinates from view coordinates(opengl viewport coords).
 */
Rectangle convertViewRectangleToSDLCoordinates(View view,int windowHeight) {
    view.rect.y = (float)windowHeight - (float)view.rect.y - (float)view.rect.height;
    return view.rect;
}

/**
 * Converts ui coordinates(x,y) to SDL coordinates for the provided view.
 * So if view has x,y = 0,200 and width,height = 800,200 , 
 * zero will be upper left corner: 0,400 and bottom right corner: 800,600
 */
void convertUIcoordinateToWindowcoordinates(View view, TransformComponent* transformComponent, int windowHeight,int windowWidth,vec2 convertedPoint) {
    float scaleFactorX = transformComponent->scale[0] / 100.0;  
    float scaleFactorY = transformComponent->scale[1] / 100.0; 
    transformComponent->scale[0] / 2.0 + transformComponent->position[0];
    convertedPoint[0] = (float)windowWidth / 2.0 + (transformComponent->position[0] - scaleFactorX * 100 / 2.0); 
    convertedPoint[1] = ((float)view.rect.height / 2.0) + transformComponent->position[1];
}
float absValue(float value) {
    return value < 0 ? -value : value;
}



// DEBUG



/**
 * Function to capture the framebuffer and save it to a png file.
 * Used to write every drawcall to a png file, for debugging purposes.
 */
void captureFramebuffer(int width, int height, int drawCallsCounter) {
    char filename[20];
    intToPngFilename(drawCallsCounter, filename, sizeof(filename));

    GLubyte* pixels = (GLubyte*)malloc(3 * width * height);
    if (!pixels) {
        fprintf(stderr, "Failed to allocate memory for pixels\n");
        return;
    }
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    // Flip y
    stbi_flip_vertically_on_write(1);

    // Save the pixels to an image file (e.g., using stb_image_write)
    stbi_write_png(filename, width, height, 3, pixels, width * 3);

    free(pixels);
}

/**
 * @brief Load a texture
 * Load a texture from file
*/
TextureData loadTexture(char* path) {
    int width, height, nrChannels;
    unsigned char *data = loadImage(path, &width, &height, &nrChannels); 
    if(data == NULL) {
        printf(TEXT_COLOR_ERROR "Failed to load texture\n" TEXT_COLOR_RESET);
        exit(1);
    }
    TextureData textureData = {data, width, height, nrChannels};
    return textureData;
}