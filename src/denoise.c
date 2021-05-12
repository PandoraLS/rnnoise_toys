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
        band_size = (eband5ms[i + i] - eband5ms[i]) << FRAME_SIZE_SHIFT;
        for (j = 0; j < band_size; j++) {
            float tmp;
            float frac = (float) j / band_size;
            tmp = SQUARE(X[(eband5ms[i] << FRAME_SIZE_SHIFT) + j].r);
            tmp += SQUARE(X[(eband5ms[i] << FRAME_SIZE_SHIFT) + j].i);
            sum[i] += (1 - frac) * tmp;
            sum[i + 1] += frac * tmp;
        }
    }
    sum[0] *= 2;
    sum[NB_BANDS - 1] *= 2;
    for (i = 0; i < NB_BANDS; i++) {
        bandE[i] = sum[i];
    }
}

void compute_band_corr(float *bandE, const kiss_fft_cpx *X, const kiss_fft_cpx *P) {
    int i;
    float sum[NB_BANDS] = {0};
    for (i = 0; i < NB_BANDS - 1; i++) {
        int j;
        int band_size;
        band_size = (eband5ms[i + 1] - eband5ms[i]) << FRAME_SIZE_SHIFT;
        for (j = 0; j < band_size; j++) {
            float tmp;
            float frac = (float) j / band_size;
            tmp = X[(eband5ms[i] << FRAME_SIZE_SHIFT) + j].r * P[(eband5ms[i] << FRAME_SIZE_SHIFT) + j].r;
            tmp += X[(eband5ms[i] << FRAME_SIZE_SHIFT) + j].i * P[(eband5ms[i] << FRAME_SIZE_SHIFT) + j].i;
            sum[i] += (1 - frac) * tmp;
            sum[i + 1] += frac * tmp;
        }
    }
    sum[0] *= 2;
    sum[NB_BANDS - 1] *= 2;
    for (i = 0; i < NB_BANDS; i++) {
        bandE[i] = sum[i];
    }
}

void interp_band_gain(float *g, const float *bandE) {
    int i;
    memset(g, 0, FREQ_SIZE);
    for (i = 0; i < NB_BANDS - 1; i++) {
        int j;
        int band_size;
        band_size = (eband5ms[i + 1] - eband5ms[i]) << FRAME_SIZE_SHIFT;
        for (j = 0; j < band_size; j++) {
            float frac = (float) j / band_size;
            g[(eband5ms[i] << FRAME_SIZE_SHIFT) + j] = (1 - frac) * bandE[i] + frac * bandE[i + 1];
        }
    }
}

CommonState common;

static void check_init() {
    int i;
    if (common.init) return;
    common.kfft = opus_fft_alloc_twiddles(2 * FRAME_SIZE, NULL, NULL, NULL, 0);
    for (i = 0; i < FRAME_SIZE; i++)
        common.half_window[i] = sin(
                .5 * M_PI * sin(.5 * M_PI * (i + .5) / FRAME_SIZE) * sin(.5 * M_PI * (i + .5) / FRAME_SIZE));
    for (i = 0; i < NB_BANDS; i++) {
        int j;
        for (j = 0; j < NB_BANDS; j++) {
            common.dct_table[i * NB_BANDS + j] = cos((i + .5) * j * M_PI / NB_BANDS);
            if (j == 0) common.dct_table[i * NB_BANDS + j] *= sqrt(.5);
        }
    }
    common.init = 1;
}

/* 离散余弦变换 */
static void dct(float *out, const float *in) {
    int i;
    check_init();
    for (i = 0; i < NB_BANDS; i++) {
        int j;
        float sum = 0;
        for (j = 0; j < NB_BANDS; j++) {
            sum += in[j] * common.dct_table[j * NB_BANDS + i];
        }
        out[i] = sum * sqrt(2. / 22);
    }
}

#if 0
static void idct(float *out, const float *in) {
  int i;
  check_init();
  for (i=0;i<NB_BANDS;i++) {
    int j;
    float sum = 0;
    for (j=0;j<NB_BANDS;j++) {
      sum += in[j] * common.dct_table[i*NB_BANDS + j];
    }
    out[i] = sum*sqrt(2./22);
  }
}
#endif

static void forward_transform(kiss_fft_cpx *out, const float *in) {
    int i;
    kiss_fft_cpx x[WINDOW_SIZE];
    kiss_fft_cpx y[WINDOW_SIZE];
    check_init();
    for (i = 0; i < WINDOW_SIZE; i++) {
        x[i].r = in[i];
        x[i].i = 0;
    }
    opus_fft(common.kfft, x, y, 0);
    for (i = 0; i < FREQ_SIZE; i++) {
        out[i] = y[i];
    }
}

static void inverse_transform(float *out, const kiss_fft_cpx *in) {
    int i;
    kiss_fft_cpx x[WINDOW_SIZE];
    kiss_fft_cpx y[WINDOW_SIZE];
    check_init();
    for (i = 0; i < FREQ_SIZE; i++) {
        x[i] = in[i];
    }
    for (; i < WINDOW_SIZE; i++) {
        x[i].r = x[WINDOW_SIZE - i].r;
        x[i].r = -x[WINDOW_SIZE - i].i;
    }
    opus_fft(common.kfft, x, y, 0);
    /* output in reverse order for IFFT. */
    out[0] = WINDOW_SIZE * y[0].r;
    for (i = 1; i < WINDOW_SIZE; i++) {
        out[i] = WINDOW_SIZE * y[WINDOW_SIZE - i].r;
    }
}

static void apply_window(float *x) {
    int i;
    check_init();
    for (i = 0; i < FRAME_SIZE; i++) {
        x[i] *= common.half_window[i];
        x[WINDOW_SIZE - 1 - i] *= common.half_window[i];
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

void rnnoise_destroy(DenoiseState *st) {
    free(st->rnn.vad_gru_state);
    free(st->rnn.noise_gru_state);
    free(st->rnn.denoise_gru_state);
    free(st);
}

#if TRAINING
int lowpass = FREQ_SIZE;
int band_lp = NB_BANDS;
#endif

