#include <cstddef>
#include <arm_neon.h>
#include <iostream>


#include <chrono>

float dot_simd(const float* a, const float* b, const size_t length)
{
    // Use 4 independent accumulators to break latency dependency chains
    float32x4_t acc0 = vdupq_n_f32(0.0f);
    float32x4_t acc1 = vdupq_n_f32(0.0f);
    float32x4_t acc2 = vdupq_n_f32(0.0f);
    float32x4_t acc3 = vdupq_n_f32(0.0f);

    size_t i = 0;

    // Process 16 floats (64 bytes) per iteration
    for (; i + 16 <= length; i += 16)
    {
        // Interleaved loads & fused multiply-add
        acc0 = vfmaq_f32(acc0, vld1q_f32(a + i + 0),  vld1q_f32(b + i + 0));
        acc1 = vfmaq_f32(acc1, vld1q_f32(a + i + 4),  vld1q_f32(b + i + 4));
        acc2 = vfmaq_f32(acc2, vld1q_f32(a + i + 8),  vld1q_f32(b + i + 8));
        acc3 = vfmaq_f32(acc3, vld1q_f32(a + i + 12), vld1q_f32(b + i + 12));
    }

    // Combine the 4 accumulators into 1
    float32x4_t acc = vaddq_f32(vaddq_f32(acc0, acc1), vaddq_f32(acc2, acc3));

    // Efficient ARM64 horizontal add reduction
    #if defined(__aarch64__)
    float sum = vaddvq_f32(acc);
    #else
    // Fallback for ARMv7
    const float32x2_t s = vadd_f32(vget_low_f32(acc), vget_high_f32(acc));
    float sum = vget_lane_f32(vpadd_f32(s, s), 0);
    #endif

    // Handle remaining elements (tail loop)
    for (; i < length; ++i)
    {
        sum += a[i] * b[i];
    }

    return sum;
}



float dot(const float* a, const float*b, const size_t length)
{
    float result = 0.0f;
    for (int i = 0; i < length; ++i)
    {
        result += a[i] * b[i];
    }

    return result;
}

int main()
{
    constexpr size_t length = 1 << 19;
    float a[length];
    float b[length];
    for (size_t i = 0; i < length; ++i)
    {
        a[i] = static_cast<float>(i);
        b[i] = static_cast<float>(i * 2);
    }

    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

    float result = dot(a, b, length);

    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

    std::cout << "Dot Product: " << result << std::endl;

    std::chrono::duration<double, std::milli> duration = end - start;
    std::cout << "Time taken by dot: " << duration.count() << " ms" << std::endl;

    start = std::chrono::high_resolution_clock::now();

    float simd_result = dot_simd(a, b, length);
    end = std::chrono::high_resolution_clock::now();

    std::cout << "SIMD Dot Product: " << simd_result << std::endl;

    duration = end - start;
    std::cout << "Time taken by dot_simd: " << duration.count() <<
                    " ms" << std::endl;

    std::cout << "Reg Result: " << result << std::endl;
    std::cout << "SIMD Result: " << simd_result << std::endl;

    // Compare results and make sure they are the same within error rate
    if (std::abs(result - simd_result) > 1e-1)
    {
        std::cerr << "Results do not match!" << std::endl;

        return 1;
    }





    return 0;
}