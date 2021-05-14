//
// Created by aone on 2021/5/10.
//

#include <stdio.h>
#include "rnnoise.h"
#include "utils.h"

#define FRAME_SIZE 480

#define SEENLI_DEBUG

int main(int argc, char **argv) {

#ifdef SEENLI_DEBUG
    // 由于origin_feature.txt保存的时候是 追加模式，所以如果事先有 origin_feature.txt 就先删除了
    if (cfileexists("origin_feature.txt")) {
//        printf("origin_feature.txt file exists. \n");
        int del = remove("origin_feature.txt");
        if (del) { // 删除失败
            printf("origin_feature.txt is not Deleted \n");
            *(int *) 0 = 0; /* 向地址0000处写入一个0，从而触发一个访问违例异常 */
        }
    }
#endif
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






//    FILE *fp_feature;
//    fp_feature = fopen("origin_feature.txt","a+");
//    if (fp_feature == NULL) {
//        printf("origin_feature.txt failed to open. \n");
//    } else {
//        fprintf(fp_feature, "rnnoise demo \n");
//    }

    rnnoise_destroy(st);
    fclose(f1);
    fclose(fout);
    return 0;
}