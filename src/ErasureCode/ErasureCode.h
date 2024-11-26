//
// Created by Alfred on 2022/9/12.
//

#ifndef CORANA_ERASURECODE_H
#define CORANA_ERASURECODE_H

#include <iostream>
#include <vector>
#include <isa-l.h>
using namespace std;

#define GET_ARRAY_LEN(arr,len) {len = (sizeof(arr) / sizeof(arr[0]));}

struct prealloc_encode
{
    prealloc_encode(int n, int k);

    prealloc_encode(const prealloc_encode&) = delete;
    prealloc_encode(prealloc_encode&&)      = default;

    vector<uint8_t> encode_matrix;
    vector<uint8_t> table;
};

struct prealloc_recover
{
    prealloc_recover(int n, int k, size_t errors_count, size_t len);

    prealloc_recover(const prealloc_recover&) = delete;
    prealloc_recover(prealloc_recover&&)      = default;

    vector<uint8_t> errors_matrix;
    vector<uint8_t> invert_matrix;
    vector<uint8_t> decode_matrix;
    vector<uint8_t> table;
    uint8_t**            decoding;
};

class ErasureCode {
private:
    int k;
    int n;

public:
    ErasureCode(const int& n, const int& k);
    void encode_data(prealloc_encode& preallocEncode, uint8_t **source, const int & len);
    void recover_data(const vector<int> &errors, const int & len, uint8_t **dest, uint8_t **erroneous_data, prealloc_encode& preallocEncode, prealloc_recover &preallocRecover);
    uint8_t** create_erroneous_data(uint8_t** source_data, std::vector<int> errors);
    string get_source(uint8_t **source, const int& len);
    string get_line(uint8_t **source, const int& len, const int& line_num);
    //string* get_parity();
    int getK() const;
    int getN() const;
    uint8_t** string2array(const string& source, int* filling_len);
    ~ErasureCode();

};


#endif //CORANA_ERASURECODE_H
