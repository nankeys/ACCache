//
// Created by Alfred on 2022/9/13.
//

#include "MemcachedClient.h"
#include <iostream>
#include <set>
#include <algorithm>
#include "toolbox.h"
#include "config.h"

using namespace std;

MemcachedClient::MemcachedClient(vector<pair<string, int>>& server_info, bool replica) {

    memcached_return rc;
    memcached_server_st *server = nullptr;

    this->memc = memcached_create(nullptr);
    if(memc == nullptr) {
        printf("Error create memcached link.\n");
        exit(-1);
    }

    for(auto & pr : server_info) {
        //printf("Server = %s:%d\n", pr.first.c_str(),  pr.second);
        server = memcached_server_list_append(server, pr.first.c_str(), pr.second, &rc);
        this->server_key.push_back(pr.first + to_string(pr.second));
    }

    rc = memcached_server_push(memc, server);
    if (MEMCACHED_SUCCESS != rc)
        cout <<"memcached_server_push failed! rc: " << rc << endl;

    int server_count = memcached_server_count(memc);

    for(int sn = 0; sn < server_count; sn ++) {
    }

    memcached_server_list_free(server);

    rc = memcached_behavior_set(memc,  MEMCACHED_BEHAVIOR_DISTRIBUTION, MEMCACHED_DISTRIBUTION_CONSISTENT);
    if (MEMCACHED_SUCCESS != rc) {
        printf("Failing to set!\n");
        exit(-1);
    }

    if(replica) {
        rc = memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_NUMBER_OF_REPLICAS, 3);
        if (MEMCACHED_SUCCESS != rc) {
            printf("Setting replications failed!\n");
            exit(-1);
        }

        rc = memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_RANDOMIZE_REPLICA_READ, 1);
        if (MEMCACHED_SUCCESS != rc) {
            printf("Setting replications failed!\n");
            exit(-1);
        }
    }
}

bool MemcachedClient::insert(const char *key, const char *value, time_t expiration) {
    if (nullptr == key || nullptr == value)
        exit(1);
    //cout << strlen(key) << " " << strlen(value) << endl;
    if(strlen(key) == 0 || strlen(value) == 0)
        exit(2);

    uint32_t flags = 0;
    memcached_return rc;

    rc = memcached_set(this->memc, key, strlen(key), value, strlen(value), expiration, flags);
    // insert ok
    if (MEMCACHED_SUCCESS == rc)
        return true;
    else
        return false;
}

bool MemcachedClient::gset(const char *gkey, const char *key, const char *value, time_t expiration) {
    if (nullptr == key || nullptr == value || nullptr == gkey)
        exit(1);
    //cout << strlen(key) << " " << strlen(value) << endl;
    if(strlen(key) == 0 || strlen(value) == 0 || strlen(gkey) == 0)
        exit(1);

    uint32_t flags = 0;
    memcached_return rc;

    rc = memcached_set_by_key(this->memc, gkey, strlen(gkey), key, strlen(key), value, strlen(value), expiration, flags);

    // insert ok
    if (MEMCACHED_SUCCESS == rc)
        return true;
    else
        return false;
}


bool MemcachedClient::get(const char *key, std::string& value) {
    if (nullptr == key)
        exit(1);
    if(strlen(key) == 0)
        exit(1);


    uint32_t flags = 0;
    memcached_return rc, rc2;
    size_t value_length;

    char* v = memcached_get(memc, key, strlen(key), &value_length, &flags, &rc);

    // get ok
    if(rc == MEMCACHED_SUCCESS) {
        value = v;
        free(v);
        if(value.length() == 0) {
            insert(key, "1111");
            v = memcached_get(memc, key, strlen(key), &value_length, &flags, &rc2);
            if(rc2 == MEMCACHED_SUCCESS) {
                value = v;
                free(v);
            }
        }
        return true;
    }
    return false;
}

bool MemcachedClient::gget(const char *gkey, const char *key, std::string& value) {
    if (nullptr == key || nullptr == gkey)
        exit(1);
    if(strlen(key) == 0 || strlen(gkey) == 0)
        exit(1);


    uint32_t flags = 0;
    memcached_return rc;
    size_t value_length;

    char* v = memcached_get_by_key(memc, gkey, strlen(gkey), key, strlen(key), &value_length, &flags, &rc);

    // get ok
    if(rc == MEMCACHED_SUCCESS) {
        value = v;
        free(v);
        return true;
    }
    value = "";
    return false;
}

std::vector<std::map<std::string, std::string>> MemcachedClient::get_stats() {
    memcached_stat_st *stats = nullptr;
    memcached_return_t rc;
    char *args = nullptr;

    stats = memcached_stat(memc, args, &rc);

    int server_count = memcached_server_count(memc);

    for(int sn = 0; sn < server_count; sn ++) {

        const char *ip = memcached_server_name(memcached_server_instance_by_position(memc, sn));
        int port = memcached_server_port(memcached_server_instance_by_position(memc, sn));
        rc = memcached_stat_servername(stats, args, ip, port);
        if (rc != MEMCACHED_SUCCESS) {
            printf("Error\n");
            exit(-1);
        }

        char **stats_key;
        stats_key = memcached_stat_get_keys(memc, stats, &rc);
        if (rc != MEMCACHED_SUCCESS) {
            printf("Error\n");
            exit(-1);
        }

        //cout << "stats = " << (stats == nullptr) << endl;

        map<string, string> tmp;
        char *ckey = nullptr;
        ckey = stats_key[0];
        for (int i = 1; ckey != nullptr; i++) {
            //printf("key = %s\n", ckey);
            const char *s = ckey;
            memcached_return mr;
            char *cvalue = memcached_stat_get_value(memc, stats, s, &mr);
            if (mr == MEMCACHED_SUCCESS) {
                //cout << ckey << " = " << cvalue << endl;
                //cout << string(ckey) << " = " << string(cvalue) << endl;
                tmp[ckey] = cvalue;
            }
            ckey = stats_key[i];
        }

        this->server_status.push_back(tmp);

        //free(stats_key);
    }

    /*for(int i = 0; i < server_status.size(); i ++) {
        cout << "Server Node " << i << " : " << endl;
        for(auto &pr: server_status[i]) {
            cout << "\t" << pr.first << " : " << pr.second << endl;
        }
    }*/

    memcached_stat_free(memc, stats);
    //return false;

    return server_status;
}

static memcached_return_t stat_printer(const memcached_instance_st *server,
                                       const char *key, size_t key_length,
                                       const char *value, size_t value_length,
                                       void *context)
{
    (void)server;
    (void)context;
    (void)key;
    (void)key_length;
    (void)value;
    (void)value_length;

    return MEMCACHED_SUCCESS;
}

bool MemcachedClient::flush() {
    memcached_return rc;

    rc = memcached_flush(memc, 0);

    if(rc != MEMCACHED_SUCCESS)
        return false;

    rc = memcached_stat_execute(memc, "reset", stat_printer, NULL);
    if(rc != MEMCACHED_SUCCESS)
        return false;

    return true;
}

MemcachedClient::~MemcachedClient() {
    memcached_free(memc);
}

size_t MemcachedClient::mgget(const char *gkey, char **key, size_t *key_len, const size_t& key_num) {
    memcached_return rc;
    memcached_result_st results_obj;
    memcached_result_st *results;

    results= memcached_result_create(memc, &results_obj);

    //size_t key_len[key_num];
    size_t gkey_len = strlen(gkey);
    size_t value_len = 0;

    //for(int i = 0 ; i < key_num; i ++) {
    //    key_len[i] = strlen(key[i]);
    //}

    rc = memcached_mget_by_key(memc, gkey, gkey_len, key, key_len, key_num);
    if(rc != MEMCACHED_SUCCESS)
        return 0;

    while ((results= memcached_fetch_result(memc, &results_obj, &rc)))
    {
        //if(rc != MEMCACHED_SUCCESS) {
            //value_len = 0;
        //    break;
        //}

        value_len += memcached_result_length(results);
    }

    //cout << "value len = " << value_len << endl;

    memcached_result_free(&results_obj);
    return value_len;
}

vector<string> MemcachedClient::get_server_key(const int& key_len) {
    const memcached_instance_st *mc;
    memcached_return_t rc;

    size_t count = memcached_server_count(memc);
    cout << "# of servers = " << count << endl;

    vector<string> keys;
    set<string> names;

    while(true) {
        string key = makeRandStr(key_len, true);
        mc = memcached_server_by_key(memc, key.c_str(), key.size(), &rc);
        string name = memcached_server_name(mc);
        in_port_t port = memcached_server_port(mc);
        // cout << name << ":" << port << endl;
        string server = name + ":" + to_string(port);
        if(names.count(server) == 0) {
            keys.push_back(key);
            names.insert(server);
        }
        // cout << "key set size = " << keys.size() << endl;
        if(keys.size() == count) break;
    }

    cout << keys.size() << " keys in total" << endl;
    for(auto &pr: names) {
        cout << pr  << endl;
    }

    /*for(auto & key : keys) {
        cout << key << endl;
        mc = memcached_server_by_key(memc, key.c_str(), key.size(), &rc);
        cout << memcached_server_name(mc) << endl;
    }*/

    return keys;
}

