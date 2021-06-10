# 清理 compile 和 make 过程中的 dirty files
# rm 命令末尾的 2> /dev/null 可以忽略掉删除不存在文件时输出的错误信息

# 清理./autogen.sh 产生的 files
rm -r autom4te.cache/ 2> /dev/null
rm m4/libtool.m4 m4/ltoptions.m4 m4/ltsugar.m4 m4/ltversion.m4 m4/lt~obsolete.m4 2> /dev/null
rm aclocal.m4 compile config.guess config.h.in config.sub depcomp install-sh ltmain.sh Makefile.in missing package_version 2> /dev/null

# 清理 ./configure 产生的 files
rm doc/Doxyfile 2> /dev/null
rm -r examples/.deps/ 2> /dev/null
rm -r src/.deps/ 2> /dev/null
rm config.h config.log config.status libtool Makefile rnnoise.pc rnnoise-uninstalled.pc stamp-h1 2> /dev/null

# 清理 make 产生的 files
rm -r .libs/ 2> /dev/null
rm -r examples/.libs/ 2> /dev/null
rm examples/.dirstamp 2> /dev/null
rm examples/rnnoise_demo 2> /dev/null
rm examples/rnnoise_demo.o 2> /dev/null
rm -r src/.libs/ 2> /dev/null
rm src/.dirstamp 2> /dev/null
rm src/celt_lpc.lo src/celt_lpc.o src/denoise.lo src/denoise.o src/kiss_fft.lo src/kiss_fft.o src/pitch.lo src/pitch.o 2> /dev/null
rm src/rnn.lo src/rnn.o src/rnn_data.lo src/rnn_data.o src/rnn_reader.lo src/rnn_reader.o 2> /dev/null
rm librnnoise.la config.h.in~ 2> /dev/null

echo "Cleanup completed!"
