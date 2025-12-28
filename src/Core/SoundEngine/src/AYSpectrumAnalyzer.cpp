#include "AYSpectrumAnalyzer.h"

#define PI 3.1415926535f
namespace ayt::engine::resource
{
    AYSpectrumAnalyzer::AYSpectrumAnalyzer(size_t fftSize) : _fftSize(fftSize) {
        _input = fftwf_alloc_real(_fftSize);
        _output = fftwf_alloc_complex(_fftSize / 2 + 1);
        _plan = fftwf_plan_dft_r2c_1d(_fftSize, _input, _output, FFTW_ESTIMATE);

        // 初始化汉宁窗
        _window.resize(_fftSize);
        for (size_t i = 0; i < _fftSize; ++i) {
            _window[i] = 0.5f * (1 - cos(2 * PI * i / (_fftSize - 1)));
        }
    }

    AYSpectrumAnalyzer::~AYSpectrumAnalyzer()
    {
        fftwf_destroy_plan(_plan);
        fftwf_free(_input);
        fftwf_free(_output);
    }

    void AYSpectrumAnalyzer::analyze(const std::vector<float>& samples)
    {
        // 应用窗函数
        for (size_t i = 0; i < _fftSize; ++i) {
            _input[i] = (i < samples.size()) ? samples[i] * _window[i] : 0;
        }

        fftwf_execute(_plan);
    }

    std::vector<float> AYSpectrumAnalyzer::getBandEnergies(int numBands)
    {
        std::vector<float> bands(numBands, 0.0f);
        float maxEnergy = 0.0f;

        // 计算频带能量
        for (int i = 0; i < numBands; ++i) {
            int startBin = i * (_fftSize / 2) / numBands;
            int endBin = (i + 1) * (_fftSize / 2) / numBands;

            for (int j = startBin; j < endBin; ++j) {
                float power = sqrt(_output[j][0] * _output[j][0] + _output[j][1] * _output[j][1]);
                bands[i] += power;
            }

            bands[i] /= (endBin - startBin);
            maxEnergy = std::max(maxEnergy, bands[i]);
        }

        // 归一化
        if (maxEnergy > 0) {
            for (auto& band : bands) {
                band /= maxEnergy;
            }
        }

        return bands;
    }
}