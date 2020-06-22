#include <cstdio>
#include <api.h>
// TODO: Put this include inside project root directory.
#include <../yevano.cpp/format.h>
#include <filesystem>

constexpr auto build_mode = "static";
// constexpr auto build_mode = "dynamic";

constexpr auto src_dir = "src";
constexpr auto build_dir = "build";
constexpr auto vendor_dir = "vendor";

constexpr auto executable = "stella";
constexpr auto dylib = "stella.dll";

extern "C" bool build() {
    using string = std::string;
    using namespace std::filesystem;

    int status = 0;
    string defines = "-DSTBI_NO_THREAD_LOCALS";
    
    string cxxflags =
        "-std=c++17 "
        "-gstabs "
        "-nostdlib "
        "-fno-builtin -fno-rtti -fno-exceptions -fno-stack-protector "
        "-m64 -mwindows";
    
    string ldflags = "-lgcc -msse4.1";
    
    string platform_ldflags =
        "-lopengl32 -lgdi32 -luser32 -lshell32 -lkernel32 "
        "-Wl,-ewin32_main";
    
    string game_ldflags = formatv("-L%/imgui/lib -l:imgui.a", vendor_dir);

    // TODO: I should find a better way to do this.
    string includes = formatv(
        "-I% "
        "-I%/imgui "
        "-I%/stb "
        "-I%/rnd "
        "-I%/pt_math "
        "-I%/GL",
        src_dir, vendor_dir, vendor_dir, vendor_dir, vendor_dir, vendor_dir);
    
    string platform_sources = formatv("%/win32_platform.cpp", src_dir);
    string game_sources = formatv("%/stella.cpp", src_dir);

    // TODO: Read from environment variable.
    string cc = "g++";

    create_directory(build_dir);

    if(build_mode == "static") {
        status = run(formatv(
            "% % % % % % -DSTELLA_STATIC -o %/% % % %",
            cc,
            cxxflags,
            defines,
            includes,
            platform_sources,
            game_sources,
            build_dir,
            executable,
            ldflags,
            game_ldflags,
            platform_ldflags
        ));
        printv("Status code: %", status);
    } else if(build_mode == "dynamic") {
        // TODO
    } else {
        run(formatv("echo Invalid build mode: %", build_mode));
        status = 1;
    }

    // TODO: Rethink return value of build function.
    return status ? false : true;
}