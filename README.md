# Standard UTAU Library

C++ utility library for Ameya/UTAU, providing foundational support for C++ applications to process UTAU data.

## Features

+ Read and write UTAU files(`*.ust`, `oto.ini`, `prefix.map`)

+ Read and write plugin temporary file for UTAU plugins(`*.tmp`)

+ Convert UTAU project to synthesis arguments

## Requirements

+ CMake 3.16
+ C++ 17
+ Boost.Test, for the test cases only

## Building the tests

```
cmake -B build -DSTDUTAU_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --no-tests=error
```

## References

+ [LibUtau 2.16](https://w.atwiki.jp/libutau/)
    + Copyright 2014 SHINTA(翔星 P)
    + 「Lib UTAU」は、「UTAU プラグイン開発支援用 C++ クラスライブラリ」です。

+ [utau-zh-docs](https://suibianp.github.io/utau-zh-docs/)
    + Copyright SuibianP

+ [UTAU Wiki](https://w.atwiki.jp/utaou/)

## Notes

+ Each declaration carries its own documentation, so the headers are the API reference.

+ Strings are raw bytes. None of these formats records its own encoding, so converting is the caller's job.

## License

This library is released under the Apache 2.0 License.

Copyright (C) 2020-present SineStriker.
