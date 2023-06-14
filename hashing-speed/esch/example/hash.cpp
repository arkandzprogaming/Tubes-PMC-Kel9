#include "esch.hpp"
#include <iostream>
#include <time.h>

// Compile it with
//
// g++ -std=c++20 -Wall -O3 -I ./include example/hash.cpp
int
main()
{
  constexpr size_t d_len = 32ul; // message length in bytes

  uint8_t data[d_len];
  uint8_t dig0[esch256::DIGEST_LEN];
  uint8_t dig1[esch384::DIGEST_LEN];
  clock_t start0, end0, start1, end1;
  double cpu_time_used;

  // random message bytes
  // sparkle_utils::random_data(data, d_len);

  // fixed message bytes
  std::memset(data, 0, sizeof(data));

  std::memset(dig0, 0, sizeof(dig0));
  std::memset(dig1, 0, sizeof(dig1));

  // compute Esch256 digest
  start0 = clock();
  esch256::hash(data, d_len, dig0);
  end0 = clock();
  // compute Esch384 digest
  start1 = clock();
  esch384::hash(data, d_len, dig1);
  end1 = clock();

  using namespace sparkle_utils;
  std::cout << "esch256( " << to_hex(data, d_len)
            << " ) = " << to_hex(dig0, sizeof(dig0)) << std::endl;

  cpu_time_used = ((double)(end0 - start0)) / CLOCKS_PER_SEC;
  std::cout << "CPU time used for esch256: " << cpu_time_used << std::endl;

  std::cout << "esch384( " << to_hex(data, d_len)
            << " ) = " << to_hex(dig1, sizeof(dig1)) << std::endl;

  cpu_time_used = ((double)(end1 - start1)) / CLOCKS_PER_SEC;
  std::cout << "CPU time used for esch384: " << cpu_time_used << std::endl;

  return EXIT_SUCCESS;
}
