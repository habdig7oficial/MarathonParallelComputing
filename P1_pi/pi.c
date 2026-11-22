#include <stdio.h>
#include <omp.h>


double pi(long long n, unsigned int threads){
    double step = 1.0 / (double)n;
    double sum = 0;

    omp_set_num_threads(threads);
    
    #pragma omp parallel for reduction(+:sum) schedule(static)
    for(long long i = 0; i < n; i++){
        double x = (i + 0.5) * step;
        sum += 4.0 / (1.0 + x * x);
    }

    return sum * step;
}
