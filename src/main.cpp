//#include "Random.h"
//#include "eccache.h"
#include <random>
#include <algorithm>
#include "SPCache.h"
#include "MemcachedClient.h"
#include "OurScheme.h"
#include "Random.h"
#include "eccache.h"
#include "parameter.h"
#include "fmt/core.h"

int main() {
    ConfigParameter gcp1(twitter, 9);

    // Exp#1 (Tail latency and access throughput) & Exp#4 (Impact of number of caching nodes)
    for(int k = 0; k < 3; k ++) { // number of runs
        for(int i =  0; i < 10; i ++) { // traceno
            for(int j = 0; j < 5; j ++) { // Server number
                ofstream fout(gcp1.PATH_PREFIX + "/result", ios::out|ios::app);
                fout << variation[j] << endl;
                fout.close();

                cout << "Server num = " << SERVER_NUM[j] << endl;
                ConfigParameter gcp(meta, traceno[i], hotlim[i][2], SERVER_NUM[j]);
                MemcachedClient mca(gcp.SERVER_INFO);
                mca.flush();

                //AC-Cache
                OurScheme os(meta, traceno[i], hotlim[i][2], SERVER_NUM[j]);
                os.distribute(gcp.PATH_PREFIX + "/" + fmt::format("graph{:02d}_{}_agg", gcp.TRACE_NO, groupnum[i][j]), groupnum[i][j], SERVER_NUM[j]);
                os.query();
                mca.flush();

                //EC-Cache
                eccache::init(gcp, twitter);
                eccache::distribution();
                eccache::test(gcp, SERVER_NUM[j]);
                mca.flush();

                //SP-Cache
                SPCache::initial(twitter, SERVER_NUM[j]);
                SPCache::distribution();
                SPCache::test(SERVER_NUM[j]);
                mca.flush();

                random_read_file(twitter, SERVER_NUM[j]);
                random_init();
                random_test(twitter, SERVER_NUM[j]);
                mca.flush();
            }
        }
    }

    // Exp#2 (Percent imbalance)
    // You need to change the parameter in config.h
    for(int k = 0; k < 3; k ++) { // number of runs
        for(int i =  0; i < 10; i ++) { // traceno
            for(int j = 0; j < 7; j ++) { // day number
                ofstream fout(gcp1.PATH_PREFIX + "/result", ios::out|ios::app);
                fout << variation[j] << endl;
                fout.close();

                cout << "Server num = " << day_num[j] << endl;
                ConfigParameter gcp(meta, traceno[i], hotlim[i][2], day_num[j]);
                MemcachedClient mca(gcp.SERVER_INFO);
                mca.flush();

                //AC-Cache
                OurScheme os(meta, traceno[i], hotlim[i][2], day_num[j]);
                os.distribute(gcp.PATH_PREFIX + "/" + fmt::format("graph{:02d}_{}_agg", gcp.TRACE_NO, groupnum[i][2]), groupnum[i][2], day_num[j]);
                os.query();

                auto stats = mca.get_stats();
                ofstream fout2(gcp.PATH_PREFIX + "/ACCache_" + to_string(day_num[j]) + "n" + to_string(i));
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
                eccache::test(gcp, day_num[j]);
                stats = mca.get_stats();
                fout2 = ofstream(gcp.PATH_PREFIX + "/ECCache_" + to_string(day_num[j]) + "n" + to_string(i));
                for(int i = 0; i < stats.size(); i ++) {
                    fout2 << "Server No. " << i << " : " << endl;
                    for(auto &pr: stats[i]) {
                        fout2 << "\t" << pr.first << " : " << pr.second << endl;
                    }
                }
                fout2.close();
                mca.flush();

                //SP-Cache
                SPCache::initial(twitter, day_num[j]);
                SPCache::distribution();
                SPCache::test(day_num[j]);
                stats = mca.get_stats();
                fout2 = ofstream(gcp.PATH_PREFIX + "/SPCache_" + to_string(day_num[j]) + "n" + to_string(i));
                for(int i = 0; i < stats.size(); i ++) {
                    fout2 << "Server No. " << i << " : " << endl;
                    for(auto &pr: stats[i]) {
                        fout2 << "\t" << pr.first << " : " << pr.second << endl;
                    }
                }
                fout2.close();
                mca.flush();

                random_read_file(twitter, day_num[j]);
                random_init();
                random_test(twitter, day_num[j]);
                stats = mca.get_stats();
                fout2 = ofstream(gcp.PATH_PREFIX + "/Random_" + to_string(day_num[j]) + "n" + to_string(i));
                for(int i = 0; i < stats.size(); i ++) {
                    fout2 << "Server No. " << i << " : " << endl;
                    for(auto &pr: stats[i]) {
                        fout2 << "\t" << pr.first << " : " << pr.second << endl;
                    }
                }
                fout2.close();
                mca.flush();
            }
        }
    }

    /*for(int i = 0; i < 3; i ++) {
        for (int snum = 4; snum < 20; snum += 3) {
            ofstream fout(gcp.PATH_PREFIX + "/result", ios::out|ios::app);
            fout << snum << endl;

            cout << "Server num = " << snum << endl;
            ConfigParameter cp(twitter, snum);
            MemcachedClient mc(cp.SERVER_INFO);

            OurScheme os(twitter, snum);
            cout << cp.PATH_PREFIX + "/newgroup34" << endl;
            os.distribute(cp.PATH_PREFIX + "/newgroup34", 246, snum);
            os.query(snum);

            //MemcachedClient mc(cp.SERVER_INFO);
            auto stats = mc.get_stats();

            ofstream fout2(cp.PATH_PREFIX + "/Clime_" + to_string(snum) + "n" + to_string(i));
            for(int i = 0; i < stats.size(); i ++) {
                fout2 << "Server Num " << i << " : " << endl;
                for(auto &pr: stats[i]) {
                    fout2 << "\t" << pr.first << " : " << pr.second << endl;
                }
            }
            fout.close();
            fout2.close();
            mc.flush();*/

            /*eccache::init(cp, twitter);
            eccache::distribution();
            eccache::test(cp, snum);
            MemcachedClient mc(cp.SERVER_INFO);
            mc.flush();*/

            /*SPCache::initial(twitter, snum);
            SPCache::distribution();
            SPCache::test(snum);
            mc.flush();

            random_read_file(twitter, snum);
            random_init();
            random_test(twitter, snum);
            mc.flush();*/
        //}
        //ConfigParameter cp(twitter, 6);
    //}

    /*for(int i = 0; i < 3; i ++) {
        eccache::init(gcp, twitter);
        eccache::distribution();
        cout << "The " << i << "th time" << endl;
        for (int snum = 0; snum < 7; snum++) {
            cout << "Server num = " << snum << endl;
            ConfigParameter cp(twitter, snum);

            eccache::test(cp, snum);
            //SPCache::initial(twitter, snum);
            //SPCache::distribution();
            //SPCache::test(snum);
            //random_test(twitter, snum);
            MemcachedClient mc(cp.SERVER_INFO);
            auto stats = mc.get_stats();

            ofstream fout(cp.PATH_PREFIX + "/ECCache_day" + to_string(snum) + "n" + to_string(i));
            for(int i = 0; i < stats.size(); i ++) {
                fout << "Server Node " << i << " : " << endl;
                for(auto &pr: stats[i]) {
                    fout << "\t" << pr.first << " : " << pr.second << endl;
                }
            }
            fout.close();

            //mc.get_stats();
            //
        }
        MemcachedClient mcc(gcp.SERVER_INFO);
        mcc.flush();
    }*/

    /*for(int i = 0; i < 3; i ++) {
        random_read_file(twitter, 0);
        random_init();
        cout << "The " << i << "th time" << endl;
        for (int snum = 0; snum < 7; snum++) {
            cout << "Server num = " << snum << endl;
            ConfigParameter cp(twitter, snum);

            random_test(twitter, snum);
            //SPCache::initial(twitter, snum);
            //SPCache::distribution();
            //SPCache::test(snum);
            //random_test(twitter, snum);
            MemcachedClient mc(cp.SERVER_INFO);
            auto stats = mc.get_stats();

            ofstream fout(cp.PATH_PREFIX + "/Random_day" + to_string(snum) + "n" + to_string(i));
            for(int i = 0; i < stats.size(); i ++) {
                fout << "Server Node " << i << " : " << endl;
                for(auto &pr: stats[i]) {
                    fout << "\t" << pr.first << " : " << pr.second << endl;
                }
            }
            fout.close();

            //mc.get_stats();
            //
        }
        MemcachedClient mcc(gcp.SERVER_INFO);
        mcc.flush();
    }

    for(int i = 0; i < 3; i ++) {
        SPCache::initial(twitter, 0);
        SPCache::distribution();
        cout << "The " << i << "th time" << endl;
        for (int snum = 0; snum < 7; snum++) {
            cout << "Server num = " << snum << endl;
            ConfigParameter cp(twitter, snum);

            //random_test(twitter, snum);
            //SPCache::initial(twitter, snum);
            //SPCache::distribution();
            SPCache::test(snum);
            //random_test(twitter, snum);
            MemcachedClient mc(cp.SERVER_INFO);
            auto stats = mc.get_stats();

            ofstream fout(cp.PATH_PREFIX + "/SPCache_day" + to_string(snum) + "n" + to_string(i));
            for(int i = 0; i < stats.size(); i ++) {
                fout << "Server Node " << i << " : " << endl;
                for(auto &pr: stats[i]) {
                    fout << "\t" << pr.first << " : " << pr.second << endl;
                }
            }
            fout.close();

            //mc.get_stats();
            //
        }
        MemcachedClient mcc(gcp.SERVER_INFO);
        mcc.flush();
    }*/

/*    for(int i = 0; i < 2; i ++) {
        OurScheme os(twitter, 0);
        os.distribute(gcp.PATH_PREFIX + "/newgroup34", 246, gcp.SERVER_NUM);
        cout << "The " << i << "th time" << endl;
        for (int snum = 0; snum < 7; snum++) {
            cout << "Server num = " << snum << endl;
            ConfigParameter cp(twitter, snum);

            //random_test(twitter, snum);
            os.query(snum);
            //SPCache::initial(twitter, snum);
            //SPCache::distribution();
            //SPCache::test(snum);
            //random_test(twitter, snum);
            MemcachedClient mc(cp.SERVER_INFO);
            auto stats = mc.get_stats();

            ofstream fout(cp.PATH_PREFIX + "/Clime_day" + to_string(snum) + "n" + to_string(i));
            for(int i = 0; i < stats.size(); i ++) {
                fout << "Server Node " << i << " : " << endl;
                for(auto &pr: stats[i]) {
                    fout << "\t" << pr.first << " : " << pr.second << endl;
                }
            }
            fout.close();

            //mc.get_stats();
            //
        }
        MemcachedClient mcc(gcp.SERVER_INFO);
        mcc.flush();
    }
*/

    return 0;
}
