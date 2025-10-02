#pragma once
#include <fftw3.h>
#include <vector>

class AYSpectrumAnalyzer 
{
public:
    AYSpectrumAnalyzer(size_t fftSize = 2048); 

    ~AYSpectrumAnalyzer();

    void analyze(const std::vector<float>& samples);

    // 获取频带能量（0~1归一化）
    std::vector<float> getBandEnergies(int numBands);

    size_t getFFTSize() { return _fftSize; }
private:
    fftwf_plan _plan;
    float* _input;
    fftwf_complex* _output;
    size_t _fftSize;
    std::vector<float> _window;
};