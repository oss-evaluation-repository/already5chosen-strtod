#ifdef _MSC_VER
 #define _CRT_SECURE_NO_WARNINGS
#endif
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cfloat>
#include <random>
#include <vector>
#include <chrono>

static uint64_t mulu(uint64_t x, uint64_t y) {
#ifndef _MSC_VER
  return uint64_t(((unsigned __int128)x * y) >> 64);
#else
  uint64_t ret;
  _umul128(x, y, &ret);
  return ret;
#endif
}

static uint64_t d2u(double x) {
  uint64_t y;
  memcpy(&y, &x, sizeof(y));
  return y;
}

static double u2d(uint64_t x) {
  double y;
  memcpy(&y, &x, sizeof(y));
  return y;
}

int main(int argz, char** argv)
{
  if (argz < 2) {
    fprintf(stderr, "Usage:tst3 inp-file-name [nRep]\n");
    return 1;
  }

  long nRep = 1;
  if (argz > 2) {
    long v = strtol(argv[2], NULL, 0);
    if (v > 0 && v < 1000000)
      nRep = v;
  }

  FILE* fp = fopen(argv[1], "r");
  if (!fp) {
    perror(argv[1]);
    return 1;
  }

  // read input
  std::vector<char*> inpv;
  char buf[4096];
  while (fgets(buf, sizeof(buf), fp)) {
    size_t len = strlen(buf);
    if (len > 9) {
      char* p = new char[len+1];
      memcpy(p, buf, len+1);
      inpv.push_back(p);
    }
  }
  fclose(fp);

  // correctness test
  int nErrors = 0;
  for (auto it = inpv.begin(); it != inpv.end(); ++it) {
    char* str = *it;
    uint64_t u = strtoull(&str[0], NULL, 16);
    char* endp;
    double d   = strtod(&str[16], &endp);
    if (endp==&str[16] || d2u(d) != u) {
      if (nErrors < 1000)
        fprintf(stderr,
        "Test fail at #%zu\n%s"
        "%016llx %.17e %.17e %016llx\n"
        , it-inpv.begin()
        , str
        , u
        , u2d(u)
        , d
        , d2u(d)
        );
      ++nErrors;
    }
  }
  if (nErrors > 0) {
    printf("%d errors\n", nErrors);
    return 1;
  }
  printf("ok.\n"); fflush(stdout);

  // prepare plan of timing test;
  std::mt19937_64 gen;
  gen.seed(1);
  size_t inplen = inpv.size();
  std::vector<char*> rndinp(inplen*nRep);
  for (long i = 0; i < nRep; ++i) {
    for (size_t k = 0; k < inplen; ++k) {
      rndinp[i*inplen+k] = &inpv.data()[mulu(gen(), inplen)][16];
    }
  }

  auto t0 = std::chrono::steady_clock::now();
  uint64_t dummy = 0;
  for (size_t k = 0; k < inplen*nRep; ++k)
    dummy += d2u(strtod(rndinp[k], NULL));
  auto t1 = std::chrono::steady_clock::now();
  auto dt = t1 - t0;

  for (auto it = inpv.begin(); it != inpv.end(); ++it)
    delete [] *it;

  printf("%.3f msec. %.2f nsec/iter\n", dt.count()*1e-6, double(dt.count())/(inplen*nRep));
  return dummy==42? 42 :0;
}
