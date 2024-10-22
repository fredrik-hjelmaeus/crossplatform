#include "utils.h"
#include "globals.h"

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
   
   int vertexCount = 3;
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
   // printf("size i want to allocate: %zu \n",size);
   // printf("used size: %zu \n",arena->used);
    //printf("arena size: %zu \n",arena->size);
    if (arena->used + size > arena->size) {
        fprintf(stderr, "Arena out of memory\n");
        exit(1);
    }
    void* ptr = (char*)arena->base + arena->used;
    arena->used += size;
    
    return ptr;
}

// Limit set to 100 o objects !. No indicies created.
// Collects vertices position(vArr),uv/texcoords(tArr) , vertex indices(vf) ,texture indices(tf), normal indices(vn) ,material and object.
// Then uses these and creates a new vertex data 
// final attribute looking like this: x, y ,z, u ,v, nx, ny, nz
// We convert quad to two triangles (handleLineFace) and add one to faceLineCount.
// Support to handle facelines with and without texture data. Example: f 1/2/3 4/5/6 7/8/9 or f 7//7 8//8 9//9
// obj specification: https://paulbourke.net/dataformats/obj/
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
       
    printf("what\n");
    objGroup->objectCount = 0;
   // ObjData objData;
  //  objGroup.objData = objData;
    
    
   

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
           
            //ObjData newObjData;
            //newObjData.material = (char*)arenaAlloc(&globals.assetArena, 100 * sizeof(char));
           // newObjData.material = "default";

            //objGroup.objData[objectCount] = newObjData;

            // Object name
           objGroup->objData[objGroup->objectCount].name = (char*)arenaAlloc(&globals.assetArena, lineLength * sizeof(char));
           for(int i = 0; i < lineLength; i++){
                objGroup->objData[objGroup->objectCount].name[i] = line[i];
            }  
            objGroup->objData[objGroup->objectCount].name[lineLength-1] = '\0'; // Null terminate the string
            //printf("object name %s \n", objGroup.objData[objectCount].name);
            
           // printf("faceLineCount %d \n", faceLineCount);
          //  printf("objectCount %d \n", objectCount);
            faceLineCountStart[objGroup->objectCount] = faceLineCount; //0,2
            if(objGroup->objectCount > 0){
                ASSERT(faceLineCount > 0, "Error: faceLineCount is 0 or less");
                ASSERT(faceLineCount < 100000, "Error: faceLineCount is too large");
                int pi = (int)(objGroup->objectCount-1);
                faceLineCountEnd[pi] = faceLineCount; // {0: 2}, 
                int num_faceLineCountCurrentObject = (int)faceLineCountEnd[pi] - (int)faceLineCountStart[pi];
                objGroup->objData[pi].num_of_vertices =  (num_faceLineCountCurrentObject * 3);
              //  printf("pi %d \n", pi);
                printf("object name %s \n", objGroup->objData[pi].name);
                
                printf("faceLineCountStart[objectCount-1] %d \n", faceLineCountStart[pi]);
                printf("faceLineCountEnd[objectCount-1] %d \n", faceLineCountEnd[pi]);
                printf("num of vertices: %d \n \n", objGroup->objData[pi].num_of_vertices);
               
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
                printf("ignoring material%s \n",line);
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

    // Input Last object
    //printf("objectCount %d \n", objectCount);
    printf("last object name %s \n", objGroup->objData[objGroup->objectCount-1].name);
    faceLineCountEnd[objGroup->objectCount-1] = faceLineCount;
    objGroup->objData[objGroup->objectCount-1].num_of_vertices = (faceLineCountEnd[objGroup->objectCount-1] - faceLineCountStart[objGroup->objectCount-1]) * 3;
    
    printf("last object num of vertices: %d \n \n", objGroup->objData[objGroup->objectCount-1].num_of_vertices); 

    fclose(fp);
        
    printf("faceline count: %d \n",faceLineCount);
    //printf("objectCount: %d \n",objectCount);
    //printf("group: %s \n",group);
    //printf("usemtl: %s \n",usemtl);
  //  printf("vIndex %d \n", vIndex);
    //printf("num_of_vertex %d \n", vfCount);
    
   // printf("uvCount %d \n", uvCount);
   // printf("normalCount %d \n", normalCount); 
   //printf("vnCount %d \n", vnCount);
   for(int i = 0; i < vfCount; i++){
        if(i % 3 == 0){
            printf(" \n");
        }
        printf("vf %d \n", vf[i]);
   }

  // objGroup.objectCount = objectCount;
 //  printf("objectCount %d \n", objGroup.objectCount);
   printf("--------------------\n");
   //faceLineCountEnd[0] = 2;
  // faceLineCountEnd[1] = 4;

/*    for(int i = 0; i < objectCount; i++){
   printf("faceLineCountStart %d , ", faceLineCountStart[i]);
   printf("faceLineCountEnd %d \n", faceLineCountEnd[i]);
  
    ASSERT(faceLineCountEnd[i] > 0, "Error: faceLineCountEnd is 0 or less");
    ASSERT(faceLineCountEnd[i] < 5000, "Error: faceLineCountEnd is too large");
   } */
   
  
   // for(int j = 0; j < faceLineCount; j+=1){
      //  printf("j %d \n", j);
      //  printf("faceLineCountEnd[objectIndex] %d \n", faceLineCountEnd[objectIndex]);

        // Start on a new object 
        //if(faceLineCountEnd[objectIndex] > j){ // wrong here?!?!?! <---
          //  objectIndex++;
           // if(objectIndex >= objGroup.objectCount){
              //  printf("objectIndex %d \n", objectIndex);
              //  printf("objectCount %d \n", objGroup.objectCount);
                //printf("Error: objectIndex exceeded objectCount. Exiting..\n");
              //  break;
              //  exit(1);
           // }
            //printf("size of one vertex %d \n", sizeof(Vertex));
          //  printf("object name %s \n", objGroup.objData[objectIndex].name);
          //  printf("vertex count %d \n", objGroup.objData[objectIndex].num_of_vertices);
          //  printf("objectIndex %d \n \n", objectIndex);
           // ASSERT(objGroup.objData[objectIndex].num_of_vertices > 0, "Error: num_of_vertices is 0");
          //  ASSERT(objGroup.objData[objectIndex].num_of_vertices < 100000, "Error: num_of_vertices is too large");
            
         //   size_t memorySizeToAllocate = objGroup.objData[objectIndex].num_of_vertices * sizeof(Vertex);
           // printf("memorySizeToAllocate %zu \n", memorySizeToAllocate);
            // Allocate memory
          //  objGroup.objData[objectIndex].vertexData = (Vertex*)arenaAlloc(&globals.assetArena, memorySizeToAllocate);
           
      //  }
        //objGroup.objData[0].num_of_vertices = (faceLineCount * 3);
    int objectIndex = 0;
    int vertexIndex = 0;
    
    for(int i = 0; i < objGroup->objectCount; i++){
        objGroup->objData[i].vertexData = (Vertex*)arenaAlloc(&globals.assetArena, objGroup->objData[i].num_of_vertices * sizeof(Vertex));
    }
    
    
    printf("---------filling object %s \n", objGroup->objData[0].name);
    for(int j = 0; j < faceLineCount; j+=1){ // almost correct, prints area light on index 0, nothing on index 1?!, back wall on index 2?,nothing index 3,ceiling index 4, nothing index 5?
        printf("faceLineCount (j) %d \n", j);
        //printf("faceLineCountEnd[objectIndex] >= j %d \n", j >= faceLineCountEnd[objectIndex] );
        if(j >= faceLineCountEnd[objectIndex]){
        printf("faceLineCountEnd[objectIndex]  %d \n", faceLineCountEnd[objectIndex] );
          //  objectIndex++;
          //  j--;


           
            
            printf("---------filling object %s \n", objGroup->objData[objectIndex].name);
        }
        for(int i = 0; i < 3; i++){
            
  printf("objectIndex %d ,vertexindex: %d \n", objectIndex,vertexIndex);
            // x y z
            objGroup->objData[objectIndex].vertexData[vertexIndex].position[0] = vArr[(vf[vertexIndex]-1)*3];
            objGroup->objData[objectIndex].vertexData[vertexIndex].position[1] = vArr[(vf[vertexIndex]-1)*3+1];
            objGroup->objData[objectIndex].vertexData[vertexIndex].position[2] = vArr[(vf[vertexIndex]-1)*3+2];
            printf("vertexIndex %d, v %d , vArr %f \n",vertexIndex, (vf[vertexIndex]), vArr[(vf[vertexIndex]-1)*3]);
            printf("vertexIndex %d, v %d , vArr %f \n",vertexIndex, (vf[vertexIndex]), vArr[(vf[vertexIndex]-1)*3+1]);
            printf("vertexIndex %d, v %d , vArr %f \n",vertexIndex, (vf[vertexIndex]), vArr[(vf[vertexIndex]-1)*3+2]);
           
            // color, default to black atm
            objGroup->objData[objectIndex].vertexData[vertexIndex].color[0] = 1.0; 
            objGroup->objData[objectIndex].vertexData[vertexIndex].color[1] = 1.0;
            objGroup->objData[objectIndex].vertexData[vertexIndex].color[2] = 1.0;
           
            // uv
            if(uvCount == 0){
                objGroup->objData[objectIndex].vertexData[vertexIndex].texcoord[0] = 0.0;
                objGroup->objData[objectIndex].vertexData[vertexIndex].texcoord[1] = 0.0;
            }else {
                objGroup->objData[objectIndex].vertexData[vertexIndex].texcoord[0] = tArr[(tf[vertexIndex]-1)*2];
                objGroup->objData[objectIndex].vertexData[vertexIndex].texcoord[1] = tArr[(tf[vertexIndex]-1)*2+1];
            }

            // normals
            if(vnCount == 0){
                objGroup->objData[objectIndex].vertexData[vertexIndex].normal[0] = 0.0;
                objGroup->objData[objectIndex].vertexData[vertexIndex].normal[1] = 0.0;
                objGroup->objData[objectIndex].vertexData[vertexIndex].normal[2] = 0.0;
            }else {
                objGroup->objData[objectIndex].vertexData[vertexIndex].normal[0] = nArr[(vn[vertexIndex]-1)*3];
                objGroup->objData[objectIndex].vertexData[vertexIndex].normal[1] = nArr[(vn[vertexIndex]-1)*3+1];
                objGroup->objData[objectIndex].vertexData[vertexIndex].normal[2] = nArr[(vn[vertexIndex]-1)*3+2]; 
            }
            vertexIndex++;
        }
         // if(objectIndex >= objGroup->objectCount){
              //  break;
                //exit(1);
            //}
    }
//    }
 // for(int i = 0; i < objData.objectCount; i++){
       // printf("object name in cornell_box: %s\n", objGroup.objData[0].name);
  //  } 
  printf("vertexIndex %d \n", vertexIndex);
  printf("vfCount %d \n", vfCount); 
  printf("faceLineCount %d \n", faceLineCount);
  printf("objGroup->objData objectcount %d \n", objGroup->objectCount);
  for(int i=0; i < objGroup->objectCount; i++){
    for(int j=0; j < objGroup->objData[i].num_of_vertices; j++){
        printf(
        "object %s, vertex %d, x %f, y %f, z %f \n", 
        objGroup->objData[i].name, j, 
        objGroup->objData[i].vertexData[j].position[0], 
        objGroup->objData[i].vertexData[j].position[1], 
        objGroup->objData[i].vertexData[j].position[2]
        );
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
