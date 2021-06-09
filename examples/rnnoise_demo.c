//
// Created by aone on 2021/5/10.
//

#include <stdio.h>
#include "rnnoise.h"

#define FRAME_SIZE 480

int main(int argc, char **argv) {
    int i;
    int first = 1; // 标记是否是第一帧pcm
    float x[FRAME_SIZE];
    FILE *f1, *fout;
    DenoiseState *st;
    st = rnnoise_create(NULL);
    if (argc != 3) {
        fprintf(stderr, "usage: %s <noisy speech> <output denoised>\n", argv[0]);
        return 1;
    }
    f1 = fopen(argv[1], "rb");
    fout = fopen(argv[2], "wb");
    while (1) {
        short tmp[FRAME_SIZE];
        fread(tmp, sizeof(short), FRAME_SIZE, f1); // 从文件流中读数据
        if (feof(f1)) break; // 达到文件结尾
        for (i = 0; i < FRAME_SIZE; i++) x[i] = tmp[i];
        rnnoise_process_frame(st, x, x);
        for (i = 0; i < FRAME_SIZE; i++) tmp[i] = x[i];
        if (!first) fwrite(tmp, sizeof(short), FRAME_SIZE, fout);
        // 第一帧不处理不保存, 从第二帧开始保存, out文件自然会少一帧, 但不影响PESQ的测试
        // 一帧480个采样点,每个采样点short类型的2字节，少的一帧就是960字节
        first = 0;
    }
    rnnoise_destroy(st);
    fclose(f1);
    fclose(fout);
    return 0;
}
