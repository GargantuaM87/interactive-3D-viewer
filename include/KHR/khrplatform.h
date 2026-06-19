/* Minimal KHR/khrplatform.h to satisfy glad includes in this project.
   This mirrors the basic typedefs used by GLAD and Khronos headers.
*/

#ifndef KHR_KHRPLATFORM_H
#define KHR_KHRPLATFORM_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int8_t khronos_int8_t;
typedef uint8_t khronos_uint8_t;
typedef int16_t khronos_int16_t;
typedef uint16_t khronos_uint16_t;
typedef int32_t khronos_int32_t;
typedef uint32_t khronos_uint32_t;
typedef int64_t khronos_int64_t;
typedef uint64_t khronos_uint64_t;
typedef intptr_t khronos_intptr_t;
typedef ptrdiff_t khronos_ssize_t;
typedef float khronos_float_t;

#ifdef __cplusplus
}
#endif

#endif /* KHR_KHRPLATFORM_H */
