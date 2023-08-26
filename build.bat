cls
rm -fr build
cmake -S . -B build -G "Unix Makefiles"
cd build
make
cd ..