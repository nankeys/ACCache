#include <random>
#include <algorithm>
#include "MemcachedClient.h"
#include "OurScheme.h"

int main() {
    ConfigParameter gcp(twitter);
    OurScheme os(twitter);
    os.CorrelationAnalysis();
    return 0;
}
