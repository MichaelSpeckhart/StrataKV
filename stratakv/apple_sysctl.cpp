#include <cstdint>
#include <sys/sysctl.h>
#include <stdio.h>
#include <stdint.h>

void probe_strata_hardware() {
    int64_t l1i_size = 0;
    int64_t l1d_size = 0;
    int64_t l2_size = 0;
    int32_t p_core_count = 0;
    int64_t cache_line = 0;
    size_t len;

    // 1. Get the Cache Size for the L1 cache
    len = sizeof(l1i_size);
    sysctlbyname("hw.perflevel0.l1icachesize", &l1i_size, &len, NULL, 0);

    len = sizeof(l1d_size);
    sysctlbyname("hw.perflevel0.l1dcachesize", &l1d_size, &len, NULL, 0);

    // 1. Get Cache Line Size (Should be 128 on M5)
    len = sizeof(cache_line);
    sysctlbyname("hw.cachelinesize", &cache_line, &len, NULL, 0);

    // 2. Get L2 Cache Size for the Performance Level (Cluster)
    len = sizeof(l2_size);
    sysctlbyname("hw.perflevel0.l2cachesize", &l2_size, &len, NULL, 0);

    // 3. Get Physical P-Core count
    int32_t p_cores = 0;
    len = sizeof(p_cores);
    sysctlbyname("hw.perflevel0.physicalcpu", &p_cores, &len, NULL, 0);
    
    

    printf("--- StrataKV Hardware Probe ---\n");
    printf("P-Core L1i (Instruction): %lld KB\n", l1i_size / 1024);
    printf("P-Core L1d (Data):        %lld KB\n", l1d_size / 1024);
    printf("Cache Line Size:          %lld bytes\n", cache_line);
    printf("P-Core L2 Cache:          %lld MB\n", l2_size / 1024 / 1024);
    printf("P-Cores available:        %d\n", p_cores);
}

int main() {
    probe_strata_hardware();
    return 0;
}
