//
// Created by aone on 2021/5/10.
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <math.h>
#include "common.h"
#include "arch.h"
#include "tansig_table.h"
#include "rnn.h"
#include "rnn_data.h"

/*
    tansig(matlab的称呼)就是tanh(pytorch称呼)： 双曲正切S型传输函数
    这里提前将tansig的值保存下来, 然后直接查表近似, 而不是每次都去算

    e^x - e^(-x)
    ------------
    e^x + e^(-x)

*/
static OPUS_INLINE float tansig_approx(float x) {
    int i;
    float y, dy;
    float sign = 1;
    /* Tests are reversed to catch NaNs */
    if (!(x < 8)) return 1;
    if (!(x > -8)) return -1;
#ifndef FIXED_POINT
    /* Another check in case of -ffast-math*/ // 浮点优化选项 -ffast-math：极大地提高浮点运算速度
    if (celt_isnan(x)) return 0;
#endif
    if (x < 0) {
        x = -x;
        sign = -1;
    }
    i = (int) floor(.5f + 25 * x);
    x -= .04f * i;
    y = tangsig_table[i];  // 提前将激活函数
    dy = 1 - y * y;
    y = y + x * dy * (1 - y * x);
    return sign * y;
}


/*
    sigmoid activation function
                1           e^(x)
    S(x) = ----------- = -----------
            1 + e^(-x)    e^(x) + 1
*/
static OPUS_INLINE float sigmoid_approx(float x) {
    return .5 + .5 * tansig_approx(.5 * x);
}

static OPUS_INLINE float relu(float x) {
    return x < 0 ? 0 : x;
}

