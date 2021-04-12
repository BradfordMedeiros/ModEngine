#include <vector>
#include <map>
#include <string>

struct BenchmarkMeasurement {
  float frametime;
  int numObjects;
};

struct Benchmark {
  bool shouldBenchmark;
  std::vector<BenchmarkMeasurement> samples;
};

Benchmark createBenchmark(bool shouldBenchmark);
void logBenchmarkTick(Benchmark& benchmark, float frametime, int numObjects);
std::string benchmarkResult(Benchmark& benchmark);


