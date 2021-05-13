//
// Created by aone on 2021/5/10.
//

#ifndef RNNOISE_TOYS_COMMON_H
#define RNNOISE_TOYS_COMMON_H

#include "stdlib.h"
#include "string.h"

#define RNN_INLINE inline
#define OPUS_INLINE inline

/*!
 * RNNoise wrapper for malloc(). 如果要进行自己的动态分配, 您需要做的就是替换此函数和rnnoise_free
 */
#ifndef OVERRIDE_RNNOISE_ALLOC
static RNN_INLINE void *rnnoise_alloc(size_t size) {
    return malloc(size);
}
#endif

/*!
 * RNNoise wrapper for free(). 如果要进行自己的动态分配, 您需要做的就是替换此函数和rnnoise_alloc
 */
#ifndef OVERRIDE_RNNOISE_FREE
static RNN_INLINE void rnnoise_free(void *ptr) {
    free(ptr);
}
#endif

/** 复制src到dst的n个元素, 0* 提供编译时类型检查  */
#ifndef OVERRIDE_RNN_COPY
#define RNN_COPY(dst, src, n) (memcpy((dst), (src), (n)*sizeof(*(dst)) + 0*((dst)-(src)) ))
#endif

/** 复制src到dst的n个元素, allowing overlapping regions. 0* 提供编译时类型检查 */
#ifndef OVERRIDE_RNN_MOVE
#define RNN_MOVE(dst, src, n) (memmove((dst), (src), (n)*sizeof(*(dst)) + 0*((dst)-(src)) ))
#endif

/** 将dst的n个元素置0 */
#ifndef OVERRIDE_RNN_CLEAR
#define RNN_CLEAR(dst, n) (memset((dst), 0, (n)*sizeof(*(dst))))
#endif


#endif //RNNOISE_TOYS_COMMON_H
