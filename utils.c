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
//    vec3 vertex;
    char* objFile = readFile(filepath);
    int skip = 0;
    int vertexRead = 0;
    int i;
    for(i = 0; i < strlen(objFile); i++) {

        // Skip looks for the end of the line \n (10)
        if(skip == 1){
            if((int)objFile[i] == 10){
                skip = 0;
            }
            continue;
        }

        // Comment starts with # (35), activates skip.
        if((int)objFile[i] == 35){
            skip = 1;
            continue;
        }

        // VERTEX
        // A vertex is specified via a line starting with the letter v. 
        // That is followed by (x,y,z[,w]) coordinates. W is optional and defaults to 1.0. 
        // A right-hand coordinate system is used to specify the coordinate locations. 
        // Some applications support vertex colors, by putting red, green and 
        // blue values after x y and z (this precludes specifying w). 
        // The color values range from 0 to 1.
        if((int)objFile[i] == 118){
            vertexRead = 1;
            continue;
        }
        if(vertexRead > 0){
            if((int)objFile[i] == 10){
                vertexRead = 0;
                continue;
            }
            if(vertexRead == 1){
                
             //   vertex.position.x = atof(&objFile[i]);
            }
        }

        printf("%c ", objFile[i]);
    }
    printf("\n");
}
