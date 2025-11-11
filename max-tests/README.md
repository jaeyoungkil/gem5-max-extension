# MAX Extension Tests

Test programs for the Matrix Acceleration Extension (MAX) for RISC-V.

## Files

- `test_vmatmul.c` - Basic functional test for vmatmul.vv instruction
- `Makefile` - Build system for compiling tests

## Building

```bash
make
```

This produces:
- `test_vmatmul` - RISC-V binary
- `test_vmatmul.dump` - Disassembly listing

## Running in gem5

```bash
cd ..
./build/RISCV/gem5.opt configs/example/se.py \
    --cmd=max-tests/test_vmatmul \
    --cpu-type=MinorCPU
```

## Test Description

**test_vmatmul**: Tests the vmatmul.vv instruction (opcode 0x2c)
- Multiplies two 4x4 matrices
- Matrix A × Identity Matrix B = A (easy verification)
- Compares vector result against scalar reference
- Reports PASS/FAIL

## Instruction Details

- **Instruction**: vmatmul.vv v3, v1, v2
- **Encoding**: 0xB2420E57
- **Operation**: 4×4 matrix multiply (row-major)
- **Element type**: int32_t (32-bit signed integers)
