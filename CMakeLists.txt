cmake_minimum_required(VERSION 2.8)
project(put_chunk)

set(SOURCES producer.cpp version.cpp common.hpp producer.hpp ndnputchunks.cpp version.hpp)

add_executable(pck ${SOURCES})

target_compile_options(pck PUBLIC -O2 -g -pedantic -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -std=c++11 -pthread -pthread -DNDEBUG -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_SYS_BITYPES_H=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_NDN_CXX=1)

target_link_libraries(pck -Wl,-Bstatic -Wl,-Bdynamic -L/usr/local/lib -L/usr/lib/x86_64-linux-gnu -lndn-cxx -lboost_system -lboost_filesystem -lboost_date_time -lboost_iostreams -lboost_regex -lboost_program_options -lboost_chrono -lboost_random -lboost_thread -lboost_log -lboost_log_setup -lcryptopp -lssl -lcrypto -lsqlite3 -lrt -lpthread -lboost_system -lboost_iostreams -lboost_regex )
