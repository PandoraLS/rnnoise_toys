//
// Created by aone on 2021/5/10.
//

// TODO include未完待续
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "rnnoise.h"
#include "rnn.h"
#include "rnn_data.h"

#define FRAME_SIZE_SHIFT 2
#define FRAME_SIZE (120<<FRAME_SIZE_SHIFT)

// 内置模型，在没有文件作为输入时使用
extern const struct RNNModel rnnoise_model_orig;

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