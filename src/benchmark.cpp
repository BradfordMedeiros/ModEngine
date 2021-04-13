#include "./benchmark.h"

Benchmark createBenchmark(bool shouldBenchmark){
  Benchmark result{
    .shouldBenchmark = shouldBenchmark,
  };
  return result;
}

void logBenchmarkTick(Benchmark& benchmark, float frametime, int numObjects, int numTriangles){
  if (!benchmark.shouldBenchmark){
    return;
  }
  benchmark.samples.push_back(
    BenchmarkMeasurement{
      .frametime = frametime,
      .numObjects = numObjects,
      .numTriangles = numTriangles,
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

std::map<int, double> sampleValueToFrametime(Benchmark& benchmark, std::function<int(BenchmarkMeasurement&)> getSample){
  std::map<int, double> sampleValueToFrametime;

  std::map<int, std::vector<BenchmarkMeasurement>> measurements;
  for (auto sample : benchmark.samples){
    auto sampleValue = getSample(sample);
    if (measurements.find(sampleValue) == measurements.end()){
      measurements[sampleValue] = {};
    }
    measurements.at(sampleValue).push_back(sample);
  } 
  for (auto &[sampleValue, measurement] : measurements){
    sampleValueToFrametime[sampleValue] = averageFrametime(measurement);
  }
  return sampleValueToFrametime;
}

std::string writeBenchmarkResult(std::string label, std::map<int, double> sampleResults){
  std::string result = label + "\n";
  for (auto &[sampleValue, frametime] : sampleResults){
    result = result + std::to_string(sampleValue) + " " + std::to_string(frametime) + "\n";
  }
  return result + "\n";
}

std::string benchmarkResult(Benchmark& benchmark){
  auto benchmarkResult  = std::string("average frametime: ") + std::to_string(averageFrametime(benchmark.samples)) + "\n\n"; 
  
  benchmarkResult = benchmarkResult + writeBenchmarkResult(
    "object-count to frametime", 
    sampleValueToFrametime(
      benchmark, 
      [](BenchmarkMeasurement& sample) -> int { return sample.numObjects; }
    )
  );

  benchmarkResult = benchmarkResult + writeBenchmarkResult(
    "triangle-count to frametime", 
    sampleValueToFrametime(
      benchmark, 
      [](BenchmarkMeasurement& sample) -> int { return sample.numTriangles; }
    )
  );

  return benchmarkResult;
}