#include <random>
#include <algorithm>
#include "MemcachedClient.h"
#include "OurScheme.h"
#include "parameter.h"

int main() {
    for(int i =  8; i < 10; i ++) {
        //if (i == 1) continue; // ignore trace 02 for now
        for(int j = 0; j < 5; j ++) {
            ConfigParameter gcp(meta, traceno[i], hotlim[i][2], 6);
            OurScheme os(meta, traceno[i], hotlim[i][2], variation[j]);
            os.CorrelationAnalysis(0);
        }
    }
    
    return 0;
}
