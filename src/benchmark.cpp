#include "./benchmark.h"

Benchmark createBenchmark(bool shouldBenchmark){
  Benchmark result{
    .shouldBenchmark = shouldBenchmark,
  };
  return result;
}

void logBenchmarkTick(Benchmark& benchmark, float frametime, int numObjects){
  if (!benchmark.shouldBenchmark){
    return;
  }
  benchmark.samples.push_back(
    BenchmarkMeasurement{
      .frametime = frametime,
      .numObjects = numObjects,
    }
  );
}

double averageFrametime(std::vector<BenchmarkMeasurement> samples){
  double frames = 0;
  for (auto sample : samples){
    frames = frames + sample.frametime;
  } 
  auto averageFrametime = frames / samples.size();
  return averageFrametime;
}

std::map<int, double> numObjectsToFrametime(Benchmark& benchmark){
  std::map<int, double> objectsToFrametime;

  std::map<int, std::vector<BenchmarkMeasurement>> measurements;
  for (auto sample : benchmark.samples){
    if (measurements.find(sample.numObjects) == measurements.end()){
      measurements[sample.numObjects] = {};
    }
    measurements.at(sample.numObjects).push_back(sample);
  } 
  for (auto &[numObjects, measurement] : measurements){
    objectsToFrametime[numObjects] = averageFrametime(measurement);
  }
  return objectsToFrametime;
}

std::string benchmarkResult(Benchmark& benchmark){
  auto benchmarkResult  = std::string("average frametime: ") + std::to_string(averageFrametime(benchmark.samples)); 
  auto objectsToFrame = numObjectsToFrametime(benchmark);
  for (auto &[numObj, frametime] : objectsToFrame){
    benchmarkResult = benchmarkResult + std::to_string(numObj) + "-" + std::to_string(frametime) + "\n";
  }
  return benchmarkResult;
}