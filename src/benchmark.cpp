#include "./benchmark.h"

Benchmark createBenchmark(bool shouldBenchmark){
  Benchmark result{
    .shouldBenchmark = shouldBenchmark,
  };
  return result;
}

void logBenchmarkTick(Benchmark& benchmark, float frametime){
  if (!benchmark.shouldBenchmark){
    return;
  }
  benchmark.samples.push_back(
    BenchmarkMeasurement{
      .frametime = frametime,
    }
  );
}

std::string benchmarkResult(Benchmark& benchmark){
  double frames = 0;
  for (auto sample : benchmark.samples){
    frames = frames + sample.frametime;
  }
  auto averageFrametime = frames / benchmark.samples.size();
  return std::string("average frametime: ") + std::to_string(averageFrametime); 
}