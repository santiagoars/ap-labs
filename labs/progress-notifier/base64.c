#include <stdio.h>
#include "logger.h"
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

int progress;
int percentage;
int encode_decode;

int base64encode(const void* data_buf, size_t dataLength, char* result, size_t resultSize) {
   const char base64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
   const uint8_t *data = (const uint8_t *)data_buf;
   size_t resultIndex = 0;
   size_t x;
   uint32_t n = 0;
   int padCount = dataLength % 3;
   uint8_t n0, n1, n2, n3;

   for (x = 0; x < dataLength; x += 3) {
        percentage = x * 100 / dataLength;
        n = ((uint32_t)data[x]) << 16; 
        
        if((x+1) < dataLength)
            n += ((uint32_t)data[x+1]) << 8;
        
        if((x+2) < dataLength)
            n += data[x+2];

        n0 = (uint8_t)(n >> 18) & 63;
        n1 = (uint8_t)(n >> 12) & 63;
        n2 = (uint8_t)(n >> 6) & 63;
        n3 = (uint8_t)n & 63;
            
        if(resultIndex >= resultSize) return 1; 
        result[resultIndex++] = base64chars[n0];
        if(resultIndex >= resultSize) return 1;  
        result[resultIndex++] = base64chars[n1];

        if((x+1) < dataLength) {
            if(resultIndex >= resultSize) return 1;  
            result[resultIndex++] = base64chars[n2];
        }

        if((x+2) < dataLength) {
            if(resultIndex >= resultSize) return 1;
            result[resultIndex++] = base64chars[n3];
        }
   }

   if (padCount > 0) { 
        for (; padCount < 3; padCount++) 
        { 
            if(resultIndex >= resultSize) return 1;
            result[resultIndex++] = '=';
        } 
   }
   return 0;
}
#define WHITESPACE 64
#define EQUALS     65
#define INVALID    66
static const unsigned char d[] = {
    66,66,66,66,66,66,66,66,66,66,64,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,62,66,66,66,63,52,53,
    54,55,56,57,58,59,60,61,66,66,66,65,66,66,66, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,66,66,66,66,66,66,26,27,28,
    29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66
};

int base64decode (char *in, size_t inLen, unsigned char *out, size_t *outLen) { 
    char *end = in + inLen;
    char iter = 0;
    uint32_t buf = 0;
    size_t len = 0;
    while (in < end) {
        percentage = len * 100 / *outLen;
        unsigned char c = d[*in++];
        
        switch (c) {
        case WHITESPACE: continue;   
        case INVALID:    return 1;   
        case EQUALS:                 
            in = end;
            continue;
        default:
            buf = buf << 6 | c;
            iter++; 
            
            if (iter == 4) {
                if ((len += 3) > *outLen) return 1; 
                *(out++) = (buf >> 16) & 255;
                *(out++) = (buf >> 8) & 255;
                *(out++) = buf & 255;
                buf = 0; iter = 0;

            }   
        }
    }
   
    if (iter == 3) {
        if ((len += 2) > *outLen) return 1;
        *(out++) = (buf >> 10) & 255;
        *(out++) = (buf >> 2) & 255;
    }
    else if (iter == 2) {
        if (++len > *outLen) return 1; 
        *(out++) = (buf >> 4) & 255;
    }

    *outLen = len; 
    return 0;
}

char *readFile(char fileName[], size_t *finalSize){
    FILE *file = fopen(fileName, "r");
    if(file == NULL){
        errorf("Error opening file %s\n", fileName);
        return NULL;
    }
    int max = 1000;
    size_t curr = 0;
    char *buffer = calloc(max, sizeof(char));
    int n;
    while((n = fread(buffer + curr, sizeof(char), max, file)) == max){
        curr += max;
        buffer = realloc(buffer, curr + max);
    }
    fclose(file);
    curr += n;
    *finalSize = curr;
    return buffer;
}

void handle_sigint(int sig) {
    infof("Current action: ");
    switch (progress) {
    case 0:
        infof("Reading file\n");
        break;
    case 1:
        if (encode_decode) {
            infof("Decoding %d%%\n", percentage);
        } else {
            infof("Encoding %d%%\n", percentage);
        }
        break;
    case 2:
        infof("Writing file\n");
        break;
    default:
        warnf("Problem with progress variable\n");
        break;
    }
} 

void writeFile(char newFileName[], char buffer[], size_t size){
    FILE *file = fopen(newFileName, "w");
    if(file == NULL){
        errorf("Error opening file %s\n", newFileName);
        return;
    }
    fwrite(buffer, sizeof(char), size, file);
    fclose(file);
}

int main(int argc, char *argv[]){
    if(argc != 3){
        errorf("Error in parameters, usage:\n");
        errorf("%s [--encode | --decode] <filePath>\n", argv[0]);
        return -1;
    }
    progress = 0;
    percentage = 0;
    signal(SIGINT, handle_sigint);
    signal(SIGUSR1, handle_sigint);
    if(strcmp(argv[1], "--encode") == 0){ 
        encode_decode = 0;
        size_t oSize = 0;
        char *oBuffer = readFile(argv[2], &oSize);
        size_t newSize = 4 * ((int)oSize / 3) + (oSize % 3 > 0) * 4;
        char *newBuffer = calloc(newSize, sizeof(char));
        progress = 1;
        int err = base64encode(oBuffer, oSize, newBuffer, newSize);
        if(err){
            errorf("Error encoding\n");
            return -1;
        }

        progress = 2;
        writeFile("encoded.txt", newBuffer, newSize);
    } else if(strcmp(argv[1], "--decode") == 0){ 
        encode_decode = 1;
        size_t oSize = 0;
        char *oBuffer = readFile(argv[2], &oSize);
        size_t newSize = 3 * (oSize / 4);
        char *newBuffer = calloc(newSize, sizeof(char));
        progress = 1;
        int err = base64decode(oBuffer, oSize, newBuffer, &newSize);
        if(err){
            errorf("Error decoding\n");
            return -1;
        }
        // Write
        progress = 2;
        writeFile("decoded.txt", newBuffer, newSize);
    } else {
        errorf("Error, choose one of the two options: [--encode | --decode]\n");
        return -1;
    }
    return 0;
}