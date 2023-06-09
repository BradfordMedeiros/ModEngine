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

bool firstLessThanSecond(const BenchmarkMeasurement& measurement1, const BenchmarkMeasurement& measurement2){
  return measurement1.frametime < measurement2.frametime;
}
double medianFrametime(std::vector<BenchmarkMeasurement> samples){
  if (samples.size() == 0){
    return 0;
  }
  sort(samples.begin(), samples.end(), &firstLessThanSecond);
  auto middleIndex = samples.size() / 2;
  return (!(samples.size() % 2)) ?  ((samples.at(middleIndex).frametime + samples.at(middleIndex + 1).frametime) / 2.f) : samples.at(middleIndex).frametime;
}

enum BENCHMARKING_TYPE { BENCHMARK_AVERAGE, BENCHMARK_MEDIAN };

std::map<int, double> sampleValueToFrametime(Benchmark& benchmark, std::function<int(BenchmarkMeasurement&)> getSample, BENCHMARKING_TYPE type){
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
    sampleValueToFrametime[sampleValue] = (type == BENCHMARK_AVERAGE) ? averageFrametime(measurement) : medianFrametime(measurement);
  }
  return sampleValueToFrametime;
}

std::string writeBenchmarkResult(const char* label, std::map<int, double> sampleResults){
  std::string result = std::string(label) + "\n";
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
      [](BenchmarkMeasurement& sample) -> int { return sample.numObjects; },
      BENCHMARK_AVERAGE
    )
  );
 benchmarkResult = benchmarkResult + writeBenchmarkResult(
    "object-count to median frametime", 
    sampleValueToFrametime(
      benchmark, 
      [](BenchmarkMeasurement& sample) -> int { return sample.numObjects; },
      BENCHMARK_MEDIAN
    )
  );

  benchmarkResult = benchmarkResult + writeBenchmarkResult(
    "triangle-count to frametime", 
    sampleValueToFrametime(
      benchmark, 
      [](BenchmarkMeasurement& sample) -> int { return sample.numTriangles; },
      BENCHMARK_AVERAGE
    )
  );
  benchmarkResult = benchmarkResult + writeBenchmarkResult(
    "triangle-count to median frametime", 
    sampleValueToFrametime(
      benchmark, 
      [](BenchmarkMeasurement& sample) -> int { return sample.numTriangles; },
      BENCHMARK_MEDIAN
    )
  );

  return benchmarkResult;
}


// mock for now, needs implementation
FrameInfo getFrameInfo(){
  auto time = timeSeconds(true);
  double totalFrameTime = 0.015;  // ~60fps 
  return FrameInfo {
    .currentTime = time,
    .totalFrameTime = totalFrameTime,
    .time = { 0.01, 0.005},
  };
}