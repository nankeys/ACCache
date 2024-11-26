//
// Created by Alfred on 2022/9/12.
//

#include "ErasureCode.h"
#include <algorithm>
#include <cstring>
#include <cassert>

prealloc_encode::prealloc_encode(int n, int k) : encode_matrix(n * k), table(32 * k * (n - k))
{
}

prealloc_recover::prealloc_recover(int n, int k, size_t errors_count, size_t len)
        : errors_matrix(n * k), invert_matrix(n * k), decode_matrix(n * k), table(32 * k * (n - k))
{
    decoding = (uint8_t**)malloc(errors_count * sizeof(uint8_t*));
    for (int i = 0; i < errors_count; ++i)
    {
        decoding[i] = (uint8_t*)malloc(len * sizeof(uint8_t));
    }
    //printf("decoding len = %d\n", len);
}

ErasureCode::ErasureCode(const int& n, const int& k)
        :k(k),n(n)
{
    /*//this->preallocEncode = new prealloc_encode(this->n, this->k);
    size_t line_size = source.length() % this->k == 0 ? source.length() / this->k : size_t(source.length() / this->k)+1;
    //printf("linesize = %d\n", line_size);
    this->len = line_size * k;
    //printf("len = %d\n", this->len);
    this->filling_len = this->len - source.length();

    this->source = ErasureCode::string2array(source, k, n);*/
    //this->set_source(source);
}


uint8_t** ErasureCode::string2array(const string& source, int* filling_len) {
    size_t line_size;
    string s;
    uint8_t** data;
    if(source.length() % k == 0) {
        line_size = source.length() / k;
        s = source;
        *filling_len = 0;
        //cout << "source len = " << source.length() <<endl;
        //cout << "k = " << k << endl;
        //cout << "66666666666666666666666" << endl;
    } else {
        line_size = size_t(source.length() / k ) + 1;
        *filling_len = line_size * k - source.length();
        string bu = string(*filling_len, 0x00);
        cout << "BU len = " << bu.length() << endl;
        s = source + bu;
    }

    data = new uint8_t* [n];
    for(int i = 0; i < n; i ++) {
        data[i] = new uint8_t [line_size];
    }

    for(int i = 0; i < k; i ++) {
        for(size_t j = 0; j < line_size; j ++) {
            data[i][j] = s[i * line_size + j];
        }
    }

    for(int i = k; i < n; i ++) {
        for(size_t j = 0; j < line_size; j ++) {
            data[i][j] = 0;
        }
    }

    return data;
}


void ErasureCode::encode_data(prealloc_encode& preallocEncode, uint8_t **source, const int & len) {
    //uint8_t **source = string2array(str);
    // Generate encode matrix
    gf_gen_cauchy1_matrix(preallocEncode.encode_matrix.data(), this->n, this->k);

    // Generates the expanded tables needed for fast encoding
    ec_init_tables(this->k, this->n - this->k, &preallocEncode.encode_matrix[this->k * this->k], preallocEncode.table.data());

    // Actually generated the error correction codes
    ec_encode_data(len, this->k, this->n - this->k, preallocEncode.table.data(), source, source+k);

    //return this->preallocEncode->encode_matrix;
    /*for(int i = 0; i < n; i ++) {
        for(size_t j = 0; j < this->len/this->k; j ++) {
            printf("data[%d][%d] = %d\t", i,j,this->source[i][j]);
        }
        cout << endl;
    }*/
}

void ErasureCode::recover_data(const vector<int> &errors, const int & len, uint8_t **dest, uint8_t **erroneous_data, prealloc_encode& preallocEncode, prealloc_recover &preallocRecover) {
    //cout << "recover:1111111111111111111111111111111111" << endl;
    //this->preallocRecover = new prealloc_recover(this->n, this->k, errors.size(), this->len/this->k);
    //cout << "recover:2222222222222222222222222222222222" << endl;
    for (int i = 0, r = 0; i < this->k; ++i, ++r)  {
        while (find(errors.cbegin(), errors.cend(), r) != errors.cend())
            ++r;
        for (int j = 0; j < this->k; j++) {
            preallocRecover.errors_matrix[k * i + j] = preallocEncode.encode_matrix[this->k * r + j];
        }
    }

    gf_invert_matrix(preallocRecover.errors_matrix.data(), preallocRecover.invert_matrix.data(), this->k);

    for (int e = 0; e < errors.size(); ++e) {
        int idx = errors[e];

        // We lost one of the buffers containing the data
        if (idx < this->k) {
            for (int j = 0; j < this->k; j++) {
                preallocRecover.decode_matrix[this->k * e + j] = preallocRecover.invert_matrix[this->k * idx + j];
            }
        } else {
            // We lost one of the buffer containing the error correction codes
            for (int i = 0; i < this->k; i++) {
                uint8_t s = 0;
                for (int j = 0; j < this->k; j++)
                    s ^= gf_mul(preallocRecover.invert_matrix[j * this->k + i], preallocEncode.encode_matrix[this->k * idx + j]);
                preallocRecover.decode_matrix[this->k * e + i] = s;
            }
        }
    }

    ec_init_tables(this->k, this->n - this->k, preallocRecover.decode_matrix.data(), preallocRecover.table.data());
    ec_encode_data(len, this->k, (this->n - this->k), preallocRecover.table.data(), erroneous_data, preallocRecover.decoding);


    bool success = false;

    for (int i = 0; i < errors.size(); ++i) {
        int ret = memcmp(dest[errors[i]], preallocRecover.decoding[i], len);
        success = (ret == 0);
        assert((success == true));
        //cout << i << " " <<"3:555555555555555555555555555555555555" << endl;
    }
}


string ErasureCode::get_source(uint8_t **source, const int& len) {
    //int len;
    //GET_ARRAY_LEN(source[0], len);
    //cout << "len  = " << len << endl;
    char tmp[len + 1];
    string rst;
    for(int i = 0; i < k; i ++) {
        for(size_t j = 0; j < len; j ++) {
            tmp[j] = source[i][j];
        }
        tmp[len] = '\0';
        rst += tmp;
        //cout << strlen(tmp) << endl;
    }
    //rst = rst.substr(0, this->len - this->filling_len);
    return rst;
}

string ErasureCode::get_line(uint8_t **source, const int& len, const int& line_num)
{
    //assert(line_num >= n);
    char tmp[len + 1];
    string rst;

    for(int i = 0; i < len; i ++) {
        tmp[i] = source[line_num][i];
    }
    tmp[len] = '\0';
    rst = tmp;

    return rst;
}

/*string* ErasureCode::get_parity() {
    char tmp[this->len / k + 1];
    auto *rst = new string[this->n - this->k];

    cout << "444444444444444444444" << endl;
    for(int i = this->k; i < this->n; i ++) {
        for(size_t j = 0; j < this->len / this->k; j ++) {
            tmp[i] = this->source[i][j];
            cout << "test tmp[i] = " << tmp[i] << endl;
        }
        tmp[this->len / k] = '\0';
        cout << "55555555555555555" << endl;
        cout << "tmp = " << tmp << endl;
        cout << "77777777777777777" << endl;
        rst[i] = tmp;
        cout << "666666666666666666" << endl;
    }

    return rst;
}*/

ErasureCode::~ErasureCode() {
    //delete this->preallocEncode;
    //delete this->preallocRecover;
    /*for(int i = 0 ; i < this->k; i ++) {
        delete [] this->source[i];
    }
    delete [] this->source;*/
}

uint8_t **ErasureCode::create_erroneous_data(uint8_t **source_data, std::vector<int> errors) {
    uint8_t** erroneous_data;
    erroneous_data = (uint8_t**)malloc(this->k * sizeof(uint8_t*));

    for (int i = 0, r = 0; i < this->k; ++i, ++r)
    {
        while (std::find(errors.cbegin(), errors.cend(), r) != errors.cend())
            ++r;
        for (int j = 0; j < this->k; j++)
        {
            erroneous_data[i] = source_data[r];
        }
    }
    return erroneous_data;
}

int ErasureCode::getK() const {
    return k;
}

int ErasureCode::getN() const {
    return n;
}