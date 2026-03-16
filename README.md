# Ink: Efficient Incremental k-Critical Path Generation

## Prerequisites

### macOS (Apple Clang)

```bash
brew install tbb libomp cmake
```

### Linux (GCC / Clang)

```bash
# Ubuntu / Debian
sudo apt install libtbb-dev cmake build-essential
```

## Clone

```bash
git clone --recurse-submodules https://github.com/Randy1005/Ink.git
cd Ink
```

If you forgot `--recurse-submodules`:

```bash
git submodule update --init --recursive
```

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

Binaries land in `examples/cpathgen/` and `build/bin/`.

## Benchmarks

### Generate golden files (ground-truth top-K paths, k=20000)

```bash
for bm in tv80 aes_core des_perf vga_lcd; do
  examples/cpathgen/gen-golden 20000 benchmarks/${bm}.edges golden/${bm}.golden
done
```

### Run accuracy comparison (pathgen vs cpathgen vs sequential golden)

```bash
rm -f big-table.csv
for bm in tv80 aes_core des_perf vga_lcd; do
  examples/cpathgen/big-table 20000 benchmarks/${bm}.edges golden/${bm}.golden
done
cat big-table.csv
```

## macOS Notes

Apple Clang does not ship with OpenMP or parallel STL by default.
The CMake build handles both automatically:

- **OpenMP**: uses Homebrew `libomp` (`brew install libomp`)
- **Parallel STL** (`std::execution::par_unseq`): enabled via
  `-D_LIBCPP_ENABLE_EXPERIMENTAL` and linked against `libc++experimental.a`
  from the active SDK — no extra install needed
- **TBB**: uses Homebrew `tbb` instead of the submodule (`brew install tbb`)
