#ifndef CBIND_H_
#define CBIND_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char byte_t;

typedef struct encoder_t encoder_t;

encoder_t* encoder_create(int width, int height, int fps);
void encoder_destroy(encoder_t* enc);
void encoder_frame(encoder_t* enc, byte_t* data);
const byte_t* encoder_data(encoder_t* enc, int* size);

#ifdef __cplusplus
}
#endif

#endif