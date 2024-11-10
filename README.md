# Munchess

## Description

A UCI compatible chess engine written in C from scratch. Refer to statistics.md for progress and statistics.

## Building

### macOS & Linux

Pre-requisites - CMake >= 3.5. A C compiler that supports C17 standard or above. C23 is recommended (Haven't tried with C99).

```bash
mkdir build
cd build
cmake ..
make
./tests  # Run tests
./munchess  # Run engine
```

### Windows

Pre-requisites - CMake >= 3.5, Visual Studio (MSVC or Clang-CL (MSVC CLI)).

(I personally use VSCode's CMake extension to configure and generate Visual Studio project. But, will paste the cmake command it generates.)

```ps
mkdir build
cd build
cmake.EXE -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE --no-warn-unused-cli -S.. -B. -G "Visual Studio 17 2022" -T host=x64 -A x64  # Change parameters according to your system
```

Open `chess.sln` in the `build` folder using Visual Studio and run either `Tests` or `Munchess` project by selecting the corresponding project as the startup project (right-click on the project and select `Set as Startup Project`).

## Usage

### macOS or Linux

```bash
cd build
echo 'uci\nisready\nposition startpos moves e2e4 d7d5 \ngo\nquit' | ./munchess
```

### Windows

```ps
cd build\Debug  # or build\Release
(echo uci && echo isready && echo position startpos moves e2e4 d7d5 && echo go && echo quit) | .\munchess
```

Refer to [UCI Documentation](https://gist.github.com/DOBRO/2592c6dad754ba67e6dcaec8c90165bf) for more details

## Benchmarks

Current benchmark results are in the statistics.md. To run benchmarks yourself,

```bash
python benchmark.py --help
```

For example, to compare the latest engine with the latest engine using 10 games,
```bash
python benchmark.py latest latest 10
```

## License

MIT License

## Contact (Shameless Disclaimer)

This is a project to demonstrate my abilities in C programming language. Some key usages of the language features are, arena allocation, bit fields in structs for compacting memory usage, branch-less programming where the performance is critical, cross-platform programming using pre-processor directives, parsing strings, test-driven programming. I am looking for C Software Engineering positions. Please hire me. Thanks.
