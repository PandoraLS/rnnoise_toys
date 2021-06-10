# rnnoise_toys

RNNoise is a noise suppression library based on a recurrent neural network.

ref: [xiph/rnnoise](https://github.com/xiph/rnnoise)

工作平台: ubuntu 16.04, IDE: Clion (注意配置complier: Files -> Settings -> Build,Execution,Deployment -> Toolchains)

## Debug (CMake)
PS: 也可以在windows平台进行debug

基于已经训练好的rnn调试C语言代码,rnn的参数已经写入到了`src/rnn_data.c`中
1) 载入CMakeLists.txt: 打开项目自动载入项目 (如果没有载入项目,则 File -> Reload CMake Project)
2) 项目Debug的 Program arguments 中填入 带噪语音路径和增强后语音路径, 就可以断点调试了
```shell script
/home/xxx/examples/noisy.pcm /home/xxx/examples/noisy-denoised.pcm 
```

在linux下会在`cmake-build-debug/`文件夹下有一个`rnnoise_toys`可执行文件, 在windows下会在`cmake-build-debug/`文件夹下有一个`rnnoise_toys.exe`可执行文件, 

## 训练并使用自己的模型 (demo例子)
自行安装keras环境(cpu或gpu版本均可), 可以安装cpu版本的keras将此demo迅速实现

### 准备训练数据
speech: [McGill TSP speech database](http://www-mmsp.ece.mcgill.ca/Documents/Data/) 下载`48k.zip`

noise: [noisex92](http://spib.linse.ufsc.br/noise.html) 这里使用Cockpit Noise 3 (F-16) 

干净语音数据集使用TSP的 FA(female A) 其中前50条作为训练集 后10条作为测试用 (demo样例), 并将前50条音频拼接起来作为训练集. 这是由于在训练过程中[只能使用一个speech.raw和一个noise.raw来产生训练集](https://github.com/xiph/rnnoise/issues/18#issuecomment-377708183)
如果你想用更多种类更多数量的语音和噪声,只需多拼接一些数据就ok.

由于噪声数据f16默认是16k采样率,所以需要预先升采样到48k
```shell script
sox f16.wav -r 48000 f16-48k.wav
```

将 `*.wav` 格式的音频变为 `*.raw` 格式的原始音频文件
```shell script
sox joinedFile_clean.wav --bits 16 --encoding signed-integer --endian little clean.raw
sox joinedFile_noise.wav --bits 16 --encoding signed-integer --endian little noise.raw
```

### training
```shell script
cd src # 进入src/文件夹
./compile.sh # 在src/下生成 denoise_training 可执行文件(用于根据clean.raw和noise.raw生成训练集)

./denoise_training signal.raw noise.raw 500000 > training.f32  # (note the matrix size and replace 500000 87 below)
# 根据原始信号生成training.f32, signal.raw是干净语音  noise.raw是噪声,
# 第三个参数表示的是生成多少训练样本, 500000够用, 当然也可以设置小一些(50000)或大一些(5000000)

cd ../training # 进入training/ 文件夹
python bin2hdf5.py ../src/training.f32 500000 87 training.h5 # 将training.f32 转换为 training.h5

python rnn_train.py # 训练模型, 训练好的模型会被保存到 training/weights.hf5
```

### 将训练好的参数打包到src/rnn_data.c
```shell script
python dump_rnn.py weights.hdf5 ../src/rnn_data.c ../src/rnn_data.rnnn orig # 读取模型并将参数打包处理
# 将模型参数写入到rnn_data.c和rnn_data.rnnn中 最后一个参数orig是当前训练模型的名称, 对应rnn_data.c最后的结构体名字(可以随意起名)
# rnn_data.c的是封装为数组的模型参数, rnn_data.rnnn中存储的是纯参数(rnn_data.rnnn暂时用不到) 
# rnn_data.rnnn 这种格式参考 https://github.com/GregorR/rnnoise-models/blob/master/beguiling-drafter-2018-08-30/bd.rnnn
```

接下来就可以使用自己训练出的模型参数了, 已经训练好的FA+f16的模型在`training_model/TSP-FA+f16/`文件夹下,如需使用则需用`training_model/TSP-FA+f16/rnn_data.c`替换掉`src/rnn_data.c`即可

## easy compile and make (Autotools)
以下是比较简单的 compile 和 make 方法 , 会产生一些 dirty files (原README). 
```shell script
./autogen.sh
./configure
make
```

Optionally
```shell script
make install
```

虽然它应该用作一个库,但这里提供了一个简单的命令行工具作为示例.它对48k Hz采样的 RAW 16-bit(mechine endian)单声道PCM进行操作, 用法如下:
```shell script
./examples/rnnoise_demo <noisy speech> <output denoised>
```

The output is also a 16-bit raw PCM file.

The latest version of the source is available from
https://gitlab.xiph.org/xiph/rnnoise .  The github repository
is a convenience copy.

## 参考链接
- [音频格式转换工具](https://github.com/smallmuou/wavutils)
- [16k采样率的rnnoise](https://github.com/YongyuG/rnnoise_16k)
- [windows下训练编译运行](https://github.com/jagger2048/rnnoise-windows)
- [已经训练好的rnnoise模型](https://github.com/GregorR/rnnoise-models)

