#include "iostream"
#include "vector"
#include "algorithm"
#include "random"
#include <cmath>

#include "includes/common.h"

struct sample_point {
    double x;
    double val;
};

int main() {
    /*
     * Generate random variable P from uniform distribution U
     * where variable P has pdf(x)=exp(-x/2pi)*sin(x)^2
     */
    int N = 100000;
    double sum = 0.0;
    std::vector<sample_point> samples;
    for (int i = 0; i != N; ++i) {
        double x = utilities::random_double(0, 2*utilities::pi);  // x range = [0, 2pi]
        double sin_x = sin(x);
        double val = exp(-x / (2*utilities::pi)) * sin_x * sin_x;        // function f(x)=exp(-x/2pi)*sin(x)^2
        sum += val;                                                         // for integration
        sample_point p{x, val};
        samples.emplace_back(p);                                            // for inv-function calculation
    }
    double average = sum / N;                                               // unit-length average area
    double area = 2 * utilities::pi * sum / N;                              // area = (2pi - 0) * avg_area
    std::cout << "pdf unit-length average area: " << average << std::endl;
    std::cout << "pdf total area under curve: " << area << std::endl;

    std::sort(samples.begin(), samples.end(),
              [](const sample_point& a, const sample_point& b)->bool{ return a.x < b.x; });

    double uniform_u = 0.3;                                                 // assume U=0.3
    double integral_target = uniform_u * sum;                               // integral until uniform_u
    double accumulation = 0.0;
    double result_generated_x = 0.0;
    for (auto &sample: samples) {
        accumulation += sample.val;
        if (accumulation >= integral_target) {
            result_generated_x = sample.x;
            break;
        }
    }
    std::cout << "result_generated_x: " << result_generated_x << std::endl;
    return 0;
}