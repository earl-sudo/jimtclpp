make clean
cmake -DCMAKE_VERBOSE_MAKEFILE=1 -DCMAKE_BUILD_TYPE=Debug ..
make all
make test
# make install
cmake -DCMAKE_VERBOSE_MAKEFILE=1 -DCMAKE_BUILD_TYPE=Release ..
make all
make test
# make install
# cmake .. -G "Visual Studeo 2019" -A -x64 -T v120_xp
# -T v120_xp/LLVM
# -Thost=x64
# cmake --build . --target myexe --config Release
ls ../bin
