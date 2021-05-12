//
// Created by aone on 2021/5/10.
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "kiss_fft.h"
#include "pitch.h"
#include "rnn.h"
#include "rnnoise.h"
#include "rnn_data.h"

#define FRAME_SIZE_SHIFT 2
#define FRAME_SIZE (120<<FRAME_SIZE_SHIFT) /* FRAME_SIZE = 480 */
#define WINDOW_SIZE (2*FRAME_SIZE)
#define FREQ_SIZE (FRAME_SIZE + 1)

#define PITCH_MIN_PERIOD 60
#define PITCH_MAX_PERIOD 768
#define PITCH_FRAME_SIZE 960
#define PITCH_BUF_SIZE (PITCH_MAX_PERIOD + PITCH_FRAME_SIZE)

#define SQUARE(x) ((x)*(x))

#define NB_BANDS 22

#define CEPS_MEM 8
#define NB_DELTA_CEPS 6

/*!
 * 22个Bark尺度的频带gains
 * 跨帧的前6个系数的一阶导数
 * 跨帧的前6个系数的二阶导数
 * 6个频段的pitch增益(发声强度)
 * 1个pitch周期
 * 1个特殊的非平稳值, 用于检测语音
 * 共 22 + 6 + 6 + 6 + 1 + 1 = 42个特征
 */
#define NB_FEATURES (NB_BANDS + 3 * NB_DELTA_CEPS + 2)

#ifndef TRAINING
#define TRAINING 0
#endif

/* 内置模型，在没有文件作为输入时使用 */
extern const struct RNNModel rnnoise_model_orig;

static const opus_int16 eband5ms[] = {
        /*0 200 400 600 800 1k 1.2 1.4 1.6 2k  2.4  2.8  3.2  4k  4.8 5.6  6.8  8k  9.6  12k  15.6  20k*/
          0, 1,  2,  3,  4, 5,  6,  7,  8, 10, 12,  14,  16,  20, 24, 28,  34,  40, 48,  60,  78,   100
};

typedef struct {
    int init;
    kiss_fft_state *kfft;
    float half_window[FRAME_SIZE];
    float dct_table[NB_BANDS * NB_BANDS];
} CommonState;

struct DenoiseState {
    float analysis_mem[FRAME_SIZE];
    float cepstral_mem[CEPS_MEM][NB_BANDS];
    int memid;
    float synthesis_mem[FRAME_SIZE];
    float pitch_buf[PITCH_BUF_SIZE];
    float pitch_enh_buf[PITCH_BUF_SIZE];
    float last_gain;
    int last_period;
    float mem_hp_x[2];
    float lastg[NB_BANDS];
    RNNState rnn;
};

void compute_band_energy(float *bandE, const kiss_fft_cpx *X) {
    int i;
    float sum[NB_BANDS] = {0};;
    for (i = 0; i < NB_BANDS - 1; i++) {
        int j;
        int band_size;

    }
}

int rnnoise_get_size() {
    return sizeof(DenoiseState);
}

int rnnoise_get_frame_size() {
    return FRAME_SIZE;
}

int rnnoise_init(DenoiseState *st, RNNModel *model) {
    memset(st, 0, sizeof(*st));
    if (model)
        st->rnn.model = model;
    else
        st->rnn.model = &rnnoise_model_orig;
    st->rnn.vad_gru_state = calloc(sizeof(float), st->rnn.model->vad_gru_size);
    st->rnn.noise_gru_state = calloc(sizeof(float), st->rnn.model->noise_gru_size);
    st->rnn.denoise_gru_state = calloc(sizeof(float), st->rnn.model->denoise_gru_size);
    return 0;
}

DenoiseState *rnnoise_create(RNNModel *model) {
    DenoiseState *st;
    st = malloc(rnnoise_get_size());
    rnnoise_init(st, model);
    return st;
}