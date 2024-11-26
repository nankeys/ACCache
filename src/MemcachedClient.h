//
// Created by Alfred on 2022/9/13.
//

#ifndef CORANA_MEMCACHEDCLIENT_H
#define CORANA_MEMCACHEDCLIENT_H

#include <libmemcached/memcached.hpp>
#include <vector>
#include <utility>
#include <string>
#include <map>

class MemcachedClient {
private:
    std::vector<std::string> server_key;
    std::vector<std::map<std::string, std::string>> server_status;

public:
    memcached_st *memc;
    MemcachedClient(std::vector<std::pair<std::string, int>>& server_info, bool replica = false);
    bool insert(const char* key, const char* value, time_t expiration = 0);
    bool gset(const char * gkey, const char* key, const char* value, time_t expiration = 0);
    bool get(const char* key, std::string& value);
    bool gget(const char * gkey, const char* key, std::string& value);
    size_t mgget(const char * gkey, char* key[], size_t *key_len, const size_t& key_num);
    std::vector<std::string> get_server_key(const int& key_len);
    std::vector<std::map<std::string, std::string>> get_stats();
    bool flush();
    ~MemcachedClient();

};


#endif //CORANA_MEMCACHEDCLIENT_H
