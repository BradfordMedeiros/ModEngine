#ifndef MOD_BENCHMARK
#define MOD_BENCHMARK

#include <vector>
#include <map>
#include <functional>
#include <string>

struct BenchmarkMeasurement {
  float frametime;
  int numObjects;
  int numTriangles;
};

struct Benchmark {
  bool shouldBenchmark;
  std::function<void(float)> drawScreenspaceLinePoint;
  std::vector<BenchmarkMeasurement> samples;
};

Benchmark createBenchmark(bool shouldBenchmark);
void logBenchmarkTick(Benchmark& benchmark, float frametime, int numObjects, int numTriangles);
std::string benchmarkResult(Benchmark& benchmark);


#endif
