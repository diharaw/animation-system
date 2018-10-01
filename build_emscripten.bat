cd build_em
call emsdk activate latest
call vcvarsamd64_x86
cmake .. -DBUILD_SAMPLES=OFF -DASSIMP_BUILD_ASSIMP_TOOLS=OFF -DASSIMP_BUILD_TESTS=OFF -DASSIMP_NO_EXPORT=ON -DBUILD_SHARED_LIBS=OFF -DCMAKE_TOOLCHAIN_FILE=C:\emsdk\emscripten\1.37.28\cmake\Modules\Platform\Emscripten.cmake -G "NMake Makefiles"
timeout /t -1