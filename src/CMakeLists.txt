cmake_minimum_required(VERSION 3.22)
project(CorAna)

# option(LOCAL "Local test" ON)

set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CORRELATION
        ./ListNode.cpp ./ListNode.h
        ./FreqList.cpp ./FreqList.h
        ./FreqTable.h ./FreqTable.cpp
        #./CMSketch.cpp
        #./CMSketch.h
        )
set(OTHERS
        ./config.h
        ./toolbox.h
        ./ErasureCode/ErasureCode.cpp
        ./ErasureCode/ErasureCode.h
        ./MemcachedClient.cpp
        ./MemcachedClient.h
        )
set(OUR_SCHEME
        OurScheme.cpp
        OurScheme.h
        )
set(SCHEMES
        ./eccache.h ./eccache.cpp
        Random.h Random.cpp
        SPCache.cpp SPCache.h)

#add_executable(CorAna main.cpp ListNode.cpp ListNode.h FreqList.cpp FreqList.h FreqTable.cpp FreqTable.h toolbox.h twitter_trace.h config.h eccache.cpp eccache.h ErasureCode/ErasureCode.cpp ErasureCode/ErasureCode.h)
#add_executable(CorAna main.cpp ErasureCode/ErasureCode.cpp ErasureCode/ErasureCode.h toolbox.h eccache.h eccache.cpp replicas.cpp replicas.h MemcachedClient.cpp MemcachedClient.h CMSketch.cpp CMSketch.h OurScheme.cpp OurScheme.h)
add_executable(CorAna main.cpp
        ${CORRELATION}
        ${OUR_SCHEME}
        ${SCHEMES}
        ${OTHERS}
        )
add_executable(correlation main_correlation.cpp
        ${CORRELATION}
        ${OUR_SCHEME}
        ${OTHERS}
)

target_link_libraries(CorAna isal memcached jsoncpp pthread python3.10 fmt)
target_include_directories(CorAna
        PRIVATE
        /usr/include/python3.10
)

target_link_libraries(correlation isal memcached jsoncpp pthread python3.10 fmt)
target_include_directories(correlation
        PRIVATE
        /usr/include/python3.10
)
