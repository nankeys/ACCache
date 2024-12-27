//#include "Random.h"
//#include "eccache.h"
#include <random>
#include <algorithm>
#include "SPCache.h"
#include "MemcachedClient.h"
#include "OurScheme.h"
#include "Random.h"
#include "eccache.h"
#include "fmt/core.h"

int main() {
    ConfigParameter gcp1(twitter);

    for(int i = 0; i < 3; i ++) { // number of runs
        ofstream fout(gcp1.PATH_PREFIX + "/result", ios::out|ios::app);
        fout << "result:" << endl;
        fout.close();

        ConfigParameter gcp(twitter);
        MemcachedClient mca(gcp.SERVER_INFO);
        mca.flush();

        cout << "Server num = " << gcp.DAY << endl;

        //AC-Cache
        OurScheme os(twitter);
        os.distribute(fmt::format("{}/graph{:02d}_agg", gcp.PATH_PREFIX, gcp.TRACE_NO), gcp.GROUP_NUM, gcp.SERVER_NUM);
        os.query();

        auto stats = mca.get_stats();
        ofstream fout2(gcp.PATH_PREFIX + "/ACCache_" + to_string(gcp.DAY) + "n" + to_string(i));
        for(int i = 0; i < stats.size(); i ++) {
            fout2 << "Server No. " << i << " : " << endl;
            for(auto &pr: stats[i]) {
                fout2 << "\t" << pr.first << " : " << pr.second << endl;
            }
        }
        fout2.close();
        mca.flush();

        //EC-Cache
        eccache::init(gcp, twitter);
        eccache::distribution();
        eccache::test(gcp, gcp.DAY);
        stats = mca.get_stats();
        fout2 = ofstream(gcp.PATH_PREFIX + "/ECCache_" + to_string(gcp.DAY) + "n" + to_string(i));
        for(int i = 0; i < stats.size(); i ++) {
            fout2 << "Server No. " << i << " : " << endl;
            for(auto &pr: stats[i]) {
                fout2 << "\t" << pr.first << " : " << pr.second << endl;
            }
        }
        fout2.close();
        mca.flush();

        //SP-Cache
        SPCache::initial(twitter, gcp.DAY);
        SPCache::distribution();
        SPCache::test(gcp.DAY);
        stats = mca.get_stats();
        fout2 = ofstream(gcp.PATH_PREFIX + "/SPCache_" + to_string(gcp.DAY) + "n" + to_string(i));
        for(int i = 0; i < stats.size(); i ++) {
            fout2 << "Server No. " << i << " : " << endl;
            for(auto &pr: stats[i]) {
                fout2 << "\t" << pr.first << " : " << pr.second << endl;
            }
        }
        fout2.close();
        mca.flush();

        random_read_file(twitter, gcp.DAY);
        random_init();
        random_test(twitter, gcp.DAY);
        stats = mca.get_stats();
        fout2 = ofstream(gcp.PATH_PREFIX + "/Random_" + to_string(gcp.DAY) + "n" + to_string(i));
        for(int i = 0; i < stats.size(); i ++) {
            fout2 << "Server No. " << i << " : " << endl;
            for(auto &pr: stats[i]) {
                fout2 << "\t" << pr.first << " : " << pr.second << endl;
            }
        }
        fout2.close();
        mca.flush();
    }
    return 0;
}
