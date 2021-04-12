#include <vector>
#include <string>

struct BenchmarkMeasurement {
  float frametime;
};

struct Benchmark {
  bool shouldBenchmark;
  std::vector<BenchmarkMeasurement> samples;
};

Benchmark createBenchmark(bool shouldBenchmark);
void logBenchmarkTick(Benchmark& benchmark, float frametime);
std::string benchmarkResult(Benchmark& benchmark);


