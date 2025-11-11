#include <stdio.h>
#include <stdint.h>

#define MAT_SIZE 4
#define NUM_ITERATIONS 1000  // Run 1000 matrix multiplies

// Scalar 4x4 matrix multiply
void matmul_scalar(int32_t *A, int32_t *B, int32_t *C) {
    for (int i = 0; i < MAT_SIZE; i++) {
        for (int j = 0; j < MAT_SIZE; j++) {
            int32_t sum = 0;
            for (int k = 0; k < MAT_SIZE; k++) {
                sum += A[i * MAT_SIZE + k] * B[k * MAT_SIZE + j];
            }
            C[i * MAT_SIZE + j] = sum;
        }
    }
}

// Vector 4x4 matrix multiply using vmatmul instruction
void matmul_vector(int32_t *A, int32_t *B, int32_t *C) {
    // Configure vector unit (LMUL=1, SEW=32, VL=16)
    asm volatile("vsetvli zero, %0, e32, m1, ta, ma" :: "r"(16));

    // Load matrices into vector registers
    asm volatile("vle32.v v1, (%0)" :: "r"(A));
    asm volatile("vle32.v v2, (%0)" :: "r"(B));

    // Execute matrix multiply: v3 = v1 × v2
    asm volatile(".word 0xB220A1D7");  // vmatmul.vv v3, v1, v2

    // Store result
    asm volatile("vse32.v v3, (%0)" :: "r"(C));
}

// Read cycle counter (using RISC-V rdcycle instruction)
static inline uint64_t read_cycles(void) {
    uint64_t cycles;
    asm volatile("rdcycle %0" : "=r"(cycles));
    return cycles;
}

// Read instruction counter (using RISC-V rdinstret instruction)
static inline uint64_t read_instructions(void) {
    uint64_t instret;
    asm volatile("rdinstret %0" : "=r"(instret));
    return instret;
}

int main() {
    // Test matrices
    int32_t A[16] = {
        1, 2, 3, 4,
        5, 6, 7, 8,
        2, 3, 4, 5,
        6, 7, 8, 9
    };

    int32_t B[16] = {
        9, 8, 7, 6,
        5, 4, 3, 2,
        1, 2, 3, 4,
        5, 6, 7, 8
    };

    int32_t C_scalar[16];
    int32_t C_vector[16];

    printf("=== Matrix Multiply Performance Benchmark ===\n");
    printf("Matrix size: %dx%d\n", MAT_SIZE, MAT_SIZE);
    printf("Iterations: %d\n\n", NUM_ITERATIONS);

    // ========================================
    // SCALAR IMPLEMENTATION BENCHMARK
    // ========================================
    printf("--- Scalar Implementation ---\n");

    uint64_t scalar_start_cycles = read_cycles();
    uint64_t scalar_start_instret = read_instructions();

    for (int iter = 0; iter < NUM_ITERATIONS; iter++) {
        matmul_scalar(A, B, C_scalar);
    }

    uint64_t scalar_end_cycles = read_cycles();
    uint64_t scalar_end_instret = read_instructions();

    uint64_t scalar_cycles = scalar_end_cycles - scalar_start_cycles;
    uint64_t scalar_instructions = scalar_end_instret - scalar_start_instret;

    printf("Total cycles:       %lu\n", scalar_cycles);
    printf("Total instructions: %lu\n", scalar_instructions);
    printf("Cycles per iter:    %.2f\n", (double)scalar_cycles / NUM_ITERATIONS);
    printf("Instr per iter:     %.2f\n", (double)scalar_instructions / NUM_ITERATIONS);
    printf("CPI (avg):          %.3f\n\n", (double)scalar_cycles / scalar_instructions);

    // ========================================
    // VECTOR IMPLEMENTATION BENCHMARK
    // ========================================
    printf("--- Vector Implementation (vmatmul) ---\n");

    uint64_t vector_start_cycles = read_cycles();
    uint64_t vector_start_instret = read_instructions();

    for (int iter = 0; iter < NUM_ITERATIONS; iter++) {
        matmul_vector(A, B, C_vector);
    }

    uint64_t vector_end_cycles = read_cycles();
    uint64_t vector_end_instret = read_instructions();

    uint64_t vector_cycles = vector_end_cycles - vector_start_cycles;
    uint64_t vector_instructions = vector_end_instret - vector_start_instret;

    printf("Total cycles:       %lu\n", vector_cycles);
    printf("Total instructions: %lu\n", vector_instructions);
    printf("Cycles per iter:    %.2f\n", (double)vector_cycles / NUM_ITERATIONS);
    printf("Instr per iter:     %.2f\n", (double)vector_instructions / NUM_ITERATIONS);
    printf("CPI (avg):          %.3f\n\n", (double)vector_cycles / vector_instructions);

    // ========================================
    // COMPARISON
    // ========================================
    printf("--- Performance Comparison ---\n");
    double speedup = (double)scalar_cycles / vector_cycles;
    double instr_reduction = (1.0 - (double)vector_instructions / scalar_instructions) * 100.0;

    printf("Speedup:                 %.2fx\n", speedup);
    printf("Instruction reduction:   %.1f%%\n", instr_reduction);
    printf("Cycles saved per iter:   %.0f\n",
           (double)(scalar_cycles - vector_cycles) / NUM_ITERATIONS);
    printf("Instructions saved/iter: %.0f\n\n",
           (double)(scalar_instructions - vector_instructions) / NUM_ITERATIONS);

    // ========================================
    // VALIDATE CORRECTNESS
    // ========================================
    int errors = 0;
    for (int i = 0; i < 16; i++) {
        if (C_scalar[i] != C_vector[i]) {
            printf("Mismatch at index %d: scalar=%d, vector=%d\n",
                   i, C_scalar[i], C_vector[i]);
            errors++;
        }
    }

    if (errors == 0) {
        printf("✓ Correctness validation: PASSED\n");
        return 0;
    } else {
        printf("✗ Correctness validation: FAILED (%d errors)\n", errors);
        return 1;
    }
}
