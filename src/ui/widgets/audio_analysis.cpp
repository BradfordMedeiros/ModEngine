#include "./audio_analysis.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <complex>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include "../../scene/objtypes/sound/sound.h"
#include "./widgets.h"

std::vector<std::string> listSoundFiles();
ALuint findOrLoadBuffer(std::string filepath);

namespace {

constexpr size_t spectrumSize = 4096;
constexpr size_t envelopeHopSize = 1024;

struct AudioAnalysis {
  std::vector<float> waveform;
  std::vector<float> energyEnvelope;
  std::vector<float> onsetStrength;
  std::vector<float> magnitudes;
  std::vector<std::pair<float, float>> frequencyPeaks;
  std::vector<float> onsetTimes;
  float durationSeconds = 0.f;
  float estimatedBpm = 0.f;
  float frequencyResolution = 0.f;
};

void downsampleForAnalysis(SoundAnalysisAudio& audio) {
  const int factor = std::max(1, (audio.sampleRate + 22049) / 22050);
  if (factor == 1) {
    return;
  }

  std::vector<float> samples;
  samples.reserve((audio.monoSamples.size() + factor - 1) / factor);
  for (size_t start = 0; start < audio.monoSamples.size(); start += factor) {
    const size_t end = std::min(audio.monoSamples.size(), start + factor);
    const float sum = std::accumulate(audio.monoSamples.begin() + start, audio.monoSamples.begin() + end, 0.f);
    samples.push_back(sum / (end - start));
  }
  audio.monoSamples = std::move(samples);
  audio.sampleRate /= factor;
}

std::vector<float> calculateDftSpectrum(const SoundAnalysisAudio& audio, float timeSeconds) {
  std::vector<std::complex<float>> buffer(spectrumSize);
  const size_t maxStart = audio.monoSamples.size() > spectrumSize
    ? audio.monoSamples.size() - spectrumSize
    : 0;
  const size_t start = std::min(
    static_cast<size_t>(std::max(0.f, timeSeconds) * audio.sampleRate), maxStart);
  const float pi = std::acos(-1.f);

  for (size_t i = 0; i < spectrumSize; i++) {
    const float sample = start + i < audio.monoSamples.size() ? audio.monoSamples[start + i] : 0.f;
    const float window = 0.5f - 0.5f * std::cos(2.f * pi * i / (spectrumSize - 1));
    buffer[i] = std::complex<float>(sample * window, 0.f);
  }

  for (size_t i = 1, reversed = 0; i < spectrumSize; i++) {
    size_t bit = spectrumSize >> 1;
    while (reversed & bit) {
      reversed ^= bit;
      bit >>= 1;
    }
    reversed ^= bit;
    if (i < reversed) {
      std::swap(buffer[i], buffer[reversed]);
    }
  }

  for (size_t length = 2; length <= spectrumSize; length <<= 1) {
    const float angle = -2.f * pi / length;
    const std::complex<float> step(std::cos(angle), std::sin(angle));
    for (size_t offset = 0; offset < spectrumSize; offset += length) {
      std::complex<float> rotation(1.f, 0.f);
      for (size_t i = 0; i < length / 2; i++) {
        const auto even = buffer[offset + i];
        const auto odd = buffer[offset + i + length / 2] * rotation;
        buffer[offset + i] = even + odd;
        buffer[offset + i + length / 2] = even - odd;
        rotation *= step;
      }
    }
  }

  std::vector<float> magnitudes(spectrumSize / 2 + 1);
  for (size_t bin = 0; bin < magnitudes.size(); bin++) {
    magnitudes[bin] = std::abs(buffer[bin]) * (2.f / spectrumSize);
  }
  return magnitudes;
}

AudioAnalysis analyzeAudio(const SoundAnalysisAudio& audio) {
  AudioAnalysis analysis;
  analysis.durationSeconds = static_cast<float>(audio.monoSamples.size()) / audio.sampleRate;
  analysis.frequencyResolution = static_cast<float>(audio.sampleRate) / spectrumSize;

  const size_t waveformBins = std::min<size_t>(1000, audio.monoSamples.size());
  analysis.waveform.reserve(waveformBins);
  for (size_t bin = 0; bin < waveformBins; bin++) {
    const size_t start = bin * audio.monoSamples.size() / waveformBins;
    const size_t end = std::max(start + 1, (bin + 1) * audio.monoSamples.size() / waveformBins);
    float sum = 0.f;
    const size_t boundedEnd = std::min(end, audio.monoSamples.size());
    for (size_t i = start; i < boundedEnd; i++) {
      sum += std::abs(audio.monoSamples[i]);
    }
    analysis.waveform.push_back(sum / std::max<size_t>(1, boundedEnd - start));
  }

  for (size_t start = 0; start < audio.monoSamples.size(); start += envelopeHopSize) {
    const size_t end = std::min(audio.monoSamples.size(), start + envelopeHopSize);
    float squareSum = 0.f;
    for (size_t i = start; i < end; i++) {
      squareSum += audio.monoSamples[i] * audio.monoSamples[i];
    }
    analysis.energyEnvelope.push_back(std::sqrt(squareSum / (end - start)));
  }

  analysis.onsetStrength.reserve(analysis.energyEnvelope.size());
  for (size_t i = 1; i < analysis.energyEnvelope.size(); i++) {
    analysis.onsetStrength.push_back(std::max(0.f, analysis.energyEnvelope[i] - analysis.energyEnvelope[i - 1]));
  }

  const float envelopeRate = static_cast<float>(audio.sampleRate) / envelopeHopSize;
  if (analysis.onsetStrength.size() >= static_cast<size_t>(envelopeRate * 3.f)) {
    const size_t minLag = std::max<size_t>(1, static_cast<size_t>(envelopeRate * 60.f / 180.f));
    const size_t maxLag = static_cast<size_t>(envelopeRate * 60.f / 60.f);
    float bestCorrelation = 0.f;
    size_t bestLag = 0;
    for (size_t lag = minLag; lag <= maxLag && lag < analysis.onsetStrength.size(); lag++) {
      float product = 0.f;
      float leftEnergy = 0.f;
      float rightEnergy = 0.f;
      for (size_t i = lag; i < analysis.onsetStrength.size(); i++) {
        const float left = analysis.onsetStrength[i];
        const float right = analysis.onsetStrength[i - lag];
        product += left * right;
        leftEnergy += left * left;
        rightEnergy += right * right;
      }
      const float correlation = product / std::sqrt(std::max(1e-12f, leftEnergy * rightEnergy));
      if (correlation > bestCorrelation) {
        bestCorrelation = correlation;
        bestLag = lag;
      }
    }
    if (bestLag != 0 && bestCorrelation > 0.1f) {
      analysis.estimatedBpm = 60.f * envelopeRate / bestLag;
    }
  }

  if (!analysis.onsetStrength.empty()) {
    const float averageOnset = std::accumulate(
      analysis.onsetStrength.begin(), analysis.onsetStrength.end(), 0.f) / analysis.onsetStrength.size();
    const float threshold = averageOnset * 1.5f;
    const size_t minimumSpacing = std::max<size_t>(1, static_cast<size_t>(envelopeRate * 0.25f));
    size_t lastOnset = 0;
    bool hasLastOnset = false;
    for (size_t i = 1; i + 1 < analysis.onsetStrength.size(); i++) {
      const bool localPeak = analysis.onsetStrength[i] >= analysis.onsetStrength[i - 1] &&
        analysis.onsetStrength[i] > analysis.onsetStrength[i + 1];
      if (localPeak && analysis.onsetStrength[i] > threshold &&
          (!hasLastOnset || i - lastOnset >= minimumSpacing)) {
        analysis.onsetTimes.push_back((i + 1) * static_cast<float>(envelopeHopSize) / audio.sampleRate);
        lastOnset = i;
        hasLastOnset = true;
      }
    }
  }
  return analysis;
}

void findFrequencyPeaks(AudioAnalysis& analysis) {
  analysis.frequencyPeaks.clear();
  for (size_t bin = 2; bin + 1 < analysis.magnitudes.size(); bin++) {
    if (bin * analysis.frequencyResolution < 20.f) {
      continue;
    }
    const float magnitude = analysis.magnitudes[bin];
    if (magnitude > analysis.magnitudes[bin - 1] && magnitude >= analysis.magnitudes[bin + 1]) {
      analysis.frequencyPeaks.emplace_back(bin * analysis.frequencyResolution, magnitude);
    }
  }
  std::sort(analysis.frequencyPeaks.begin(), analysis.frequencyPeaks.end(),
    [](const auto& left, const auto& right) { return left.second > right.second; });
  if (analysis.frequencyPeaks.size() > 8) {
    analysis.frequencyPeaks.resize(8);
  }
}

}

void renderAudioAnalysisWidget(bool includePanel) {
  if (includePanel) {
    ImGui::Begin("Audio Analysis");
  }

  static int selectedFile = 0;
  static std::optional<SoundAnalysisAudio> loadedAudio;
  static std::optional<AudioAnalysis> analysis;
  static std::string loadedFile;
  static std::string error;
  static float spectrumTime = 0.f;

  const auto audioFiles = listSoundFiles();
  if (audioFiles.empty()) {
    ImGui::TextDisabled("No WAV or Ogg sound files are available.");
  } else {
    selectedFile = std::clamp(selectedFile, 0, static_cast<int>(audioFiles.size()) - 1);
    if (ImGui::BeginCombo("Audio file", audioFiles[selectedFile].c_str())) {
      for (int i = 0; i < static_cast<int>(audioFiles.size()); i++) {
        const bool selected = selectedFile == i;
        if (ImGui::Selectable(audioFiles[i].c_str(), selected)) {
          selectedFile = i;
        }
        if (selected) {
          ImGui::SetItemDefaultFocus();
        }
      }
      ImGui::EndCombo();
    }
    ImGui::SameLine();
    if (ImGui::Button("Play One-shot")) {
      playSourceOneshot(
        findOrLoadBuffer(audioFiles[selectedFile]),
        std::nullopt,
        std::nullopt,
        std::nullopt,
        false,
        true,
        -1);
    }

    if (ImGui::Button("Load and Analyze")) {
      SoundAnalysisAudio decoded;
      std::string loadError;
      if (decodeSoundFileForAnalysis(audioFiles[selectedFile], decoded, loadError)) {
        downsampleForAnalysis(decoded);
        if (decoded.sampleRate > 0 && !decoded.monoSamples.empty()) {
          loadedAudio = std::move(decoded);
          analysis = analyzeAudio(*loadedAudio);
          loadedFile = audioFiles[selectedFile];
          spectrumTime = 0.f;
          analysis->magnitudes = calculateDftSpectrum(*loadedAudio, spectrumTime);
          findFrequencyPeaks(*analysis);
          error.clear();
        } else {
          error = "The selected sound file contains no usable audio samples.";
          loadedAudio.reset();
          analysis.reset();
        }
      } else {
        error = std::move(loadError);
        loadedAudio.reset();
        analysis.reset();
      }
    }

    if (!error.empty()) {
      ImGui::TextWrapped("Audio analysis failed: %s", error.c_str());
    }
    if (analysis.has_value() && loadedAudio.has_value()) {
      ImGui::TextWrapped("File: %s", loadedFile.c_str());
      ImGui::Text("Duration: %.2f s  |  Sample rate: %d Hz",
        analysis->durationSeconds, loadedAudio->sampleRate);
      ImGui::PlotLines("Waveform (absolute amplitude)", analysis->waveform.data(),
        static_cast<int>(analysis->waveform.size()), 0, nullptr, 0.f, 1.f, ImVec2(0.f, 70.f));

      if (analysis->estimatedBpm > 0.f) {
        ImGui::Text("Estimated tempo: %.1f BPM (rough energy-envelope estimate)", analysis->estimatedBpm);
      } else {
        ImGui::Text("Estimated tempo: no stable pulse found (rough energy-envelope estimate)");
      }
      ImGui::PlotLines("Beat energy (50 ms frames)", analysis->energyEnvelope.data(),
        static_cast<int>(analysis->energyEnvelope.size()), 0, nullptr, FLT_MAX, FLT_MAX, ImVec2(0.f, 70.f));

      ImGui::Text("Transient candidates: %d", static_cast<int>(analysis->onsetTimes.size()));
      if (!analysis->onsetTimes.empty()) {
        std::string onsetList;
        const size_t shownOnsets = std::min<size_t>(12, analysis->onsetTimes.size());
        for (size_t i = 0; i < shownOnsets; i++) {
          if (!onsetList.empty()) {
            onsetList += ", ";
          }
          onsetList += std::to_string(analysis->onsetTimes[i]);
        }
        ImGui::TextWrapped("Onset times (s): %s%s", onsetList.c_str(),
          analysis->onsetTimes.size() > shownOnsets ? ", ..." : "");
      }

      const float maxSpectrumTime = std::max(0.f,
        analysis->durationSeconds - static_cast<float>(spectrumSize) / loadedAudio->sampleRate);
      if (maxSpectrumTime > 0.f &&
          ImGui::SliderFloat("Spectrum time (s)", &spectrumTime, 0.f, maxSpectrumTime)) {
        analysis->magnitudes = calculateDftSpectrum(*loadedAudio, spectrumTime);
        findFrequencyPeaks(*analysis);
      }
      ImGui::PlotLines("Frequency spectrum (0 - Nyquist)", analysis->magnitudes.data(),
        static_cast<int>(analysis->magnitudes.size()), 0, nullptr, FLT_MAX, FLT_MAX, ImVec2(0.f, 90.f));
      ImGui::Text("Resolution: %.2f Hz/bin", analysis->frequencyResolution);
      for (size_t i = 0; i < analysis->frequencyPeaks.size(); i++) {
        ImGui::Text("Peak %d: %.1f Hz (magnitude %.3f)", static_cast<int>(i + 1),
          analysis->frequencyPeaks[i].first, analysis->frequencyPeaks[i].second);
      }
      ImGui::TextWrapped("Tempo and transient times are rough estimates; use them as starting beat markers.");
    }
  }

  if (includePanel) {
    ImGui::End();
  }
}
