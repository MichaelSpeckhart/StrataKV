#include <cstdlib>
#include <vector>

constexpr size_t VECTOR_SIZE = 1024;

typedef float float32;
typedef double float64;


template <class floating_point_precision>
std::vector<floating_point_precision> vector_dot(
    const std::vector<floating_point_precision> &a, const std::vector<floating_point_precision> &b)
{
    size_t vector_length = std::max(a.size(), b.size());
    std::vector<floating_point_precision> c;
    c.reserve(vector_length);


}
