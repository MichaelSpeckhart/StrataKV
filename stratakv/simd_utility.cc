#include <cstddef>
#include <arm_neon.h>
#include <iostream>


#include <chrono>


float dot_simd(const float* a, const float *b, size_t length)
{
    float32x4_t acc = vdupq_n_f32(0.0f);

    size_t i = 0;

    for (; i + 4 <= length; i += 4)
    {
        // Load 4 floats from each array starting at index i
        float32x4_t va = vld1q_f32(a + i);
        float32x4_t vb = vld1q_f32(b + i);

        // Multiply the vectors
        float32x4_t vm = vmulq_f32(va, vb);

        // Accumulate the results
        acc = vmlaq_f32(acc, va, vb);
    }

    // Horizontal add to get the final sum
    float32x2_t s = vadd_f32(vget_low_f32(acc), vget_high_f32(acc));
    float sum = vget_lane_f32(vpadd_f32(s, s), 0);

    // Handle remaining elements
    for (; i < length; ++i)
    {
        sum += a[i] * b[i];
    }

    return sum;
}


float dot(const float* a, const float*b, size_t length)
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

    const size_t length = 1 << 19;
    float a[length];
    float b[length];
    for (size_t i = 0; i < length; ++i)
    {
        a[i] = static_cast<float>(i);
        b[i] = static_cast<float>(i * 2);
    }

    std::chrono::high_resolution_clock::time_point start, end;

    start = std::chrono::high_resolution_clock::now();

    float result = dot(a, b, length);

    end = std::chrono::high_resolution_clock::now();

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

    // Compare results and make sure they are the same within error rate
    if (std::abs(result - simd_result) > 1e-1)
    {
        std::cerr << "Results do not match!" << std::endl;
        return 1;
    }



    return 0;
}