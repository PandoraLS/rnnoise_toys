# rnnoise_toys

RNNoise is a noise suppression library based on a recurrent neural network.

To compile, just type:
% ./autogen.sh
% ./configure
% make

Optionally:
% make install

While it is meant to be used as a library, a simple command-line tool is
provided as an example. It operates on RAW 16-bit (machine endian) mono
PCM files sampled at 48 kHz. It can be used as:

./examples/rnnoise_demo <noisy speech> <output denoised>

The output is also a 16-bit raw PCM file.

The latest version of the source is available from
https://gitlab.xiph.org/xiph/rnnoise .  The github repository
is a convenience copy.


# debug parameters (clion IDE)
D:\code\rnnoise\pcmfiles\19-198-0002_db20_babble.pcm D:\code\rnnoise\pcmfiles\19-198-0002_db20_babble_denoise.pcm

## TRAINING
使用python要将开始的 `#!/usr/bin/python` 删了
```shell script
cd src # 进入src/文件夹
./compile.sh # 在src/下生成 denoise_training 可执行文件

./denoise_training signal.raw noise.raw count > training.f32 # 根据原始信号生成training.f32, 这里count为500000
# (note the matrix size and replace 500000 87 below)

cd ../training # 进入training/ 文件夹
python bin2hdf5.py ../src/training.f32 500000 87 training.h5 # 将training.f32 转换为 training.h5

python rnn_train.py # 训练模型, 训练好的模型保存到 training/weights.hf5 

python dump_rnn.py weights.hdf5 ../src/rnn_data.c ../src/rnn_data_tmp.h orig
# 将模型参数写入到rnn_data.c和rnn_data.h中 最后一个参数 orig 是rnn_data.c 最后的结构体名字
```



## 学习rnnoise

根据rnnoise来学习rnnoise的一些技术

开发只在dev上开发，测试通过则可以合并到master上

只需要抄神经网络相关的代码即可

调试命令
```shell
rnnoise_toys C:\Education\code\rnnoise_toys\denoise_examples\61-70968-0001_db20_babble-48k.pcm C:\Education\code\rnnoise_toys\denoise_examples\61-70968-0001_db20_babble-48k-denoise.pcm
```
## TODO
当训练模型很糟糕的时候, 算法倾向于抹去所有的音频, 基本听不到声音的那种状态,目前仍在探索这方面的内容