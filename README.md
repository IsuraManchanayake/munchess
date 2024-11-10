# Munchess

## Description

A UCI compatible chess engine written in C from scratch. Refer to statistics.md for progress and statistics.

## Installation

### macOS & Linux

Pre-requisites - CMake >= 3.5. A C compiler that supports C17 standard or above. C23 is recommended (Haven't tried on C99).

```bash
mkdir build
cd build
cmake ..
make
./tests  # For tests
./munchess  # For engine
```

### Windows

Pre-requisites - CMake >= 3.5, Visual Studio (MSVC or Clang-CL).

To generate the Visual Studio project,
```
mkdir build
cd build
cmake ..
make
```

Open `chess.sln` in the `build` folder using Visual Studio and run either `Tests` or `Munchess` project by selecting the corresponding project as the startup project (right-click on the project and select set as startup project).

## Usage

### macOS or Linux

```bash
cd build
echo 'uci\nisready\nposition startpos moves e2e4 d7d5 \ngo\quit' | ./munchess
```

### Windows

```ps
cd build
echo 'uci\nisready\nposition startpos moves e2e4 d7d5 \ngo\quit' | ./munchess.exe
```

Refer to [UCI Documentation](https://gist.github.com/DOBRO/2592c6dad754ba67e6dcaec8c90165bf) for more details

## Benchmarks

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
