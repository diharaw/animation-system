cd build_em
cmake .. -DBUILD_SAMPLES=OFF -DASSIMP_BUILD_ASSIMP_TOOLS=OFF -DASSIMP_BUILD_TESTS=OFF -DASSIMP_NO_EXPORT=ON -DBUILD_SHARED_LIBS=OFF -DCMAKE_TOOLCHAIN_FILE=C:\emsdk\emscripten\1.38.12\cmake\Modules\Platform\Emscripten.cmake -G "NMake Makefiles"
timeout /t -1