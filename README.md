# lab_device

![CI](https://github.com/annanas-dev/lab_device/actions/workflows/ci.yml/badge.svg)
![coverage](https://raw.githubusercontent.com/annanas-dev/lab_device/main/badges/coverage.svg)

## Как запустить тесты локально
```bat
cmake -S . -B build -A x64
cmake --build build --config Debug --target device_tests
ctest --test-dir build -C Debug --output-on-failure
