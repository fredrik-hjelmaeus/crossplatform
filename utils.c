#include "utils.h"

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

void loadObjFile(const char *filepath)
{
    //Vertex* vertices = NULL;
    int vertexRead = 0;
    char material[100];

    FILE* fp;
    char line[1024];

    fp = fopen(filepath, "r");
    if(fp == NULL) {
        printf(TEXT_COLOR_ERROR "Error opening file %s" TEXT_COLOR_RESET "\n", filepath);
        return;
    }
    
    while (fgets(line, sizeof line, fp) != NULL){

        // comment
        if((int)line[0] == 35){
           // printf("found comment \n");
            continue;
        }
        // newline
        if((int)line[0] == 10){
           // printf("found new line \n");
            continue;
        }
        // m, check for mtllib
        if((int)line[0] == 109){
            if(strncmp(line, "mtllib", 6) == 0){
               // printf("found material \n");
                for(int i = 7; i < strlen(line); i++){
                   // printf("%c", line[i] , "\n");
                    material[i-7] = line[i];
                }
            }
            continue;
        }
      
        printf("%d %c \n", (int)line[0], line[0]);
      //  printf(TEXT_COLOR_WARNING "unparsed lines: %s",TEXT_COLOR_RESET , line);
    }

   

        

       

        // VERTEX
        // A vertex is specified via a line starting with the letter v. 
        // That is followed by (x,y,z[,w]) coordinates. W is optional and defaults to 1.0. 
        // A right-hand coordinate system is used to specify the coordinate locations. 
        // Some applications support vertex colors, by putting red, green and 
        // blue values after x y and z (this precludes specifying w). 
        // The color values range from 0 to 1.
        /* if((int)objFile[i] == 118){
            vertexRead = 1;
            continue;
        }
        if(vertexRead > 0){
            if((int)objFile[i] == 10){
                vertexRead = 0;
                continue;
            }
            if(vertexRead == 1){
                Vertex vertex {
                    .position = {0.0f, 0.0f, 0.0f},
                    .color = {0.0f, 0.0f, 0.0f},
                    .texcoord = {0.0f, 0.0f}
                }
            }
        } */
printf("material: %s \n",material);
    printf("\n");
}
