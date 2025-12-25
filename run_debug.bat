cmake -S . -B build -Dgtest_force_shared_crt=ON
cmake --build build --config Debug
cd .\Build\Glitter\Debug\
.\Glitter.exe