#include <time.h>
#include <stdio.h>
#include <math.h>
#define STB_IMAGE_IMPLEMENTATION
#include "./headers/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "./headers/stb_image_write.h"

float object_sqrt(unsigned char *object, int object_width, int object_height) {
    float sum = 0;
    for (int i = 0; i < object_width * object_height; i++) {
        sum += object[i] * object[i];
    }
    return sqrt(sum);
}

void bwConverter(unsigned char *input, unsigned char *output, int width, int height, int channel) {
    for (int i = 0; i < width ; i++) {
        for (int j=0; j< height; j++) {
            int index = j * width * channel + i * channel;
            int index_out = j * width + i;
            unsigned char red = input[index];
            unsigned char green =input[index + 1];
            unsigned char blue = input[index + 2];
            output[index_out] = (red*red + green*green + blue*blue+1)/(red+green+blue+1);
        }
    }
}

int objectDetector(unsigned char *image, int width, int height, unsigned char *object, int object_width, int object_height) {
    float max, similarity;
    int best_x, best_y,img_element, obj_element;
    float sum, sqrt1, sqrt2;
    similarity = 0;
    max = 0.1;
    sqrt2 = object_sqrt(object, object_width, object_height);
    for (int i = 0; i < width - object_width; i++) {
        //sqrt1 dau tien
        for (int j = 0; j < height - object_height; j++) {
            sum = 0;
            sqrt1 = 0;
            
            for (int a = 0; a < object_width ; a++){
                for (int b = 0; b < object_height; b++){
                    img_element = image[(b+j) * width + (a+i)];
                    obj_element = object[b * object_width + a];
                    sum += img_element * obj_element;   
                    sqrt1 += img_element * img_element;
                }
            }
            similarity = sum / (sqrt(sqrt1) * sqrt2);
                
            if (similarity > max) {
                max = similarity;
                best_x = i;
                best_y  = j;
            }
        }
    }
    return best_y * width + best_x;
}

void drawRectandUpdateTemplate(unsigned char *image, int width, int height, int channel, int new_index, int object_width, int object_height, unsigned char* object) {
    int best_y, best_x;
    best_y = new_index / (width);
    best_x = new_index % (width);
    printf("Best x: %d, Best y: %d\n", best_x, best_y);
    int index, index_object, i, j;
    for (i = 0; i<object_height; i++){
        for (j = 0; j<object_width; j++){
            for (int k = 0; k<3; k++){
                index = (best_y + i) * width * channel + (best_x +  j) * channel + k;
                index_object =  i * object_width * channel + j * channel + k;
                object[index_object] = image[index]; 
            }
        }
    }
    for (i = 0; i <object_width; i++) {
        for (int k = 0; k< 3; k++){
            index = (best_y ) * width * channel + (best_x+i) * channel + k;
            image[index] = 0;
            index = index  + (object_height-1) * width * channel;
            image[index] = 0;
        }
    }
    for (i = 1; i <object_height -1; i++) {
        for (int k = 0; k< 3; k++){
            index = (best_y + i) * width * channel + (best_x) * channel + k;
            image[index] = 0;
            index = index + (object_width-1) * channel + k;
            image[index] = 0;
        }
    }
}

void updateTemplate(unsigned char* template, unsigned char* img, int index){

}

int main() {
    clock_t begin = clock();

    int width, height, channel;
    int width2, height2, channel2;
    char image_path[50];
    char object_path[50] = "./data/template.jpg";
    char save_path[50];
    unsigned char *image_grey, *object_grey;
    unsigned char *image, *object;

    for (int count = 0; count < 63; count++){
        clock_t tic = clock();

        sprintf(image_path, "./data/images/img%d.jpg", count);
        sprintf(save_path, "./results/result_%d.jpg", count);

        image = stbi_load(image_path, &width, &height, &channel, 0);
        if(image == NULL) {
            printf("Error: image not found!\n");
            exit(1);
        }
        if (count == 0){
          object = stbi_load(object_path, &width2, &height2, &channel2, 0);
          if(object == NULL) {
              printf("Error: object not found!\n");
              exit(1);
          }
        }

        image_grey = (unsigned char *)calloc(width*height, sizeof(unsigned char));
        bwConverter(image, image_grey, width, height, channel);
        object_grey = (unsigned char *)calloc(width2*height2, sizeof(unsigned char));
        bwConverter(object, object_grey, 160, 214, 3);

        // //detect object
        printf("Detecting object...\n");
        int index = objectDetector(image_grey, width, height, object_grey, width2, height2);
        drawRectandUpdateTemplate(image, width, height, channel, index, width2, height2, object);
        
        //save image
        stbi_write_jpg(save_path, width, height, channel, image, width * channel);
        stbi_write_jpg(object_path, width2, height2, channel2, object, width2* channel2);
        printf("New image successfully saved to %s\n", save_path);
        
        clock_t toc = clock();
        double time = (double)(toc - tic) / CLOCKS_PER_SEC;
        printf("Processing time %f - image: %d\n", time, count);
    }
    free(image);
    free(object);
    free(image_grey);
    free(object_grey);
    
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Processing time: %f", time_spent);
    return 0;
}