# rnnoise_toys

ref: [xiph/rnnoise](https://github.com/xiph/rnnoise)

RNNoise is a noise suppression library based on a recurrent neural network.

## compile and make (Autotools 工作方式)
To compile, 只需输入以下命令
```shell script
./autogen.sh
./configure
make
```

Optionally
```shell script
make install
```


While it is meant to be used as a library, a simple command-line tool is
provided as an example. It operates on RAW 16-bit (machine endian) mono
PCM files sampled at 48 kHz. It can be used as:

./examples/rnnoise_demo <noisy speech> <output denoised>

The output is also a 16-bit raw PCM file.

The latest version of the source is available from
https://gitlab.xiph.org/xiph/rnnoise .  The github repository
is a convenience copy.


# 基于cmake进行debug
使用 clion 软件 载入 CMakeLists.txt 对进行debug

D:\code\rnnoise\pcmfiles\19-198-0002_db20_babble.pcm D:\code\rnnoise\pcmfiles\19-198-0002_db20_babble_denoise.pcm

## TRAINING
使用python要将开始的 `#!/usr/bin/python` 删了

首先准备 clean.raw 和 noise.raw, 如果没有raw文件,则根据wav生成 clean.wav和noise.wav可以长度不一致

辅助操作 使用 `sox` 将wav 变成 raw
```shell script
sox joinedFile_clean.wav --bits 16 --encoding signed-integer --endian little clean.raw
sox joinedFile_noisy.wav --bits 16 --encoding signed-integer --endian little noisy.raw
```

```shell script
cd src # 进入src/文件夹
./compile.sh # 在src/下生成 denoise_training 可执行文件

./denoise_training signal.raw noise.raw count > training.f32 # 根据原始信号生成training.f32, 这里count为500000或50000
# signal.raw是干净语音  noise.war是噪声 如果要使用很多数据,则signal.raw是很多短音频拼接而成 noise.raw同理
# (note the matrix size and replace 500000 87 below)
# count 表示的是生成多少训练样本 50000可以作为小样本实验 500000够用
# 不过这样并不能确定信噪比

cd ../training # 进入training/ 文件夹
python bin2hdf5.py ../src/training.f32 500000 87 training.h5 # 将training.f32 转换为 training.h5

python rnn_train.py # 训练模型, 训练好的模型保存到 training/weights.hf5 

python dump_rnn.py weights.hdf5 ../src/rnn_data.c ../src/rnn_data.rnnn orig
# 将模型参数写入到rnn_data.c和rnn_data.rnnn中 最后一个参数 orig 是rnn_data.c 最后的结构体名字
# rnn_data.rnnn 中存储的是纯参数 rnn_data.c 中的是封装为数组的 参数
# rnn_data.rnnn 这种格式参考 https://github.com/GregorR/rnnoise-models/blob/master/beguiling-drafter-2018-08-30/bd.rnnn
```

## utils
utils/中包含了数据处理的代码
wavutil: https://github.com/smallmuou/wavutils



## Datasets


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