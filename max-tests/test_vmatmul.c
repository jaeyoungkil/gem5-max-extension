#include <stdio.h>
#include <stdint.h>

#define MAT_SIZE 4

// Simple 4x4 matrix multiply in C for reference
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

int main() {
    // Test matrices (4x4, row-major order)
    int32_t A[16] = {
        1, 2, 3, 4,
        5, 6, 7, 8,
        1, 2, 3, 4,
        5, 6, 7, 8
    };

    int32_t B[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };

    int32_t C_scalar[16];  // Result from scalar computation
    int32_t C_vector[16];  // Result from vector instruction

    // Compute reference result using scalar code
    matmul_scalar(A, B, C_scalar);

    printf("Scalar result:\n");
    for (int i = 0; i < MAT_SIZE; i++) {
        for (int j = 0; j < MAT_SIZE; j++) {
            printf("%4d ", C_scalar[i * MAT_SIZE + j]);
        }
        printf("\n");
    }

    // Initialize vector length with LMUL=1
    // With VLEN=512, VLMAX = (1 * 512) / 32 = 16 elements
    // This fits the entire 4x4 matrix in a single register (no micro-op splitting!)
    asm volatile("vsetvli zero, %0, e32, m1, ta, ma" :: "r"(16));

    // Load matrices into vector registers
    // v1 = A, v2 = B, v3 = C (result)
    asm volatile("vle32.v v1, (%0)" :: "r"(A));
    asm volatile("vle32.v v2, (%0)" :: "r"(B));

    // Execute matrix multiply: v3 = v1 × v2
    // vmatmul.vv v3, v1, v2
    // Encoding: funct6=0x2C, vm=1, vs2=2, vs1=1, funct3=0x2, vd=3, opcode=0x57
    // Hex: 0xB220A1D7
    asm volatile(".word 0xB220A1D7");

    // Store result
    asm volatile("vse32.v v3, (%0)" :: "r"(C_vector));

    printf("\nVector instruction result:\n");
    for (int i = 0; i < MAT_SIZE; i++) {
        for (int j = 0; j < MAT_SIZE; j++) {
            printf("%4d ", C_vector[i * MAT_SIZE + j]);
        }
        printf("\n");
    }

    // Compare results
    int errors = 0;
    for (int i = 0; i < 16; i++) {
        if (C_scalar[i] != C_vector[i]) {
            printf("Mismatch at index %d: scalar=%d, vector=%d\n",
                   i, C_scalar[i], C_vector[i]);
            errors++;
        }
    }

    if (errors == 0) {
        printf("\nTEST PASSED: Matrix multiply matches reference!\n");
        return 0;
    } else {
        printf("\nTEST FAILED: %d mismatches found\n", errors);
        return 1;
    }
}
