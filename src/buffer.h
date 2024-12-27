#ifndef __BUFFER_H_
#define __BUFFER_H_

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

class BufferedWriter {
public:
    BufferedWriter(const std::string& filePath, size_t bufferSize = 1024, bool truncate = true)
        : filePath(filePath), bufferSize(bufferSize), currentSize(0), firstWrite(true), truncate(truncate) {}

    ~BufferedWriter() {
        flush(); // 在析构时自动将缓冲区数据写入文件
    }

    void write(const std::string& data) {
        size_t dataSize = data.size();
        if (currentSize + dataSize > bufferSize) {
            flush(); // 如果缓冲区已满，写入文件
        }
        buffer.push_back(data);
        currentSize += dataSize;
    }

    void flush() {
        if (!buffer.empty()) {
            std::ofstream outFile;

            // 首次写入时根据 truncate 参数决定是否清空文件内容
            if (firstWrite) {
                if (truncate) {
                    outFile.open(filePath, std::ios::out | std::ios::trunc); // 清空文件
                } else {
                    outFile.open(filePath, std::ios::out | std::ios::app);  // 追加模式
                }
                firstWrite = false;
            } else {
                outFile.open(filePath, std::ios::out | std::ios::app);
            }

            if (!outFile) {
                throw std::ios_base::failure("Failed to open file for writing.");
            }

            for (const auto& data : buffer) {
                outFile << data;
            }

            buffer.clear();
            currentSize = 0;
        }
    }

private:
    std::string filePath;            // 文件路径
    size_t bufferSize;               // 缓冲区大小
    size_t currentSize;              // 当前缓冲区占用大小
    std::vector<std::string> buffer; // 缓冲区
    bool firstWrite;                 // 是否首次写入文件
    bool truncate;                   // 是否在首次写入时清空文件
};

#endif //__BUFFER_H_