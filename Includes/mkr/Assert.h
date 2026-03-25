#ifndef ASSERT_HPP
#define ASSERT_HPP

#define ASSERT(expr, msg) \
do { \
    if ((expr)) { \
        printf("\n\n  %s | In file: %s, line: %d\n\n", msg, __FILE__, __LINE__); \
        exit(EXIT_FAILURE); \
    } \
} while (false)

#define SHADER_WARNING(msg) \
do { \
    printf("\n\n  %s | In file: %s, line: %d\n\n", msg, __FILE__, __LINE__); \
    exit(EXIT_FAILURE); \
} while (false)

#define WARNING(expr, msg) \
do { \
    if ((expr)) { \
        printf("\n\n  %s | In file: %s, line: %d\n\n", msg, __FILE__, __LINE__); \
    } \
} while (false)
    
#endif