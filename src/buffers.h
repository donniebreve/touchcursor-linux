#ifndef buffers_h
#define buffers_h

#define log(...)                  \
    fprintf(stdout, __VA_ARGS__); \
    fflush(stdout);
#define warn(...)                 \
    fprintf(stdout, __VA_ARGS__); \
    fflush(stdout);
#define error(...)                \
    fprintf(stderr, __VA_ARGS__); \
    fflush(stderr);

#endif
