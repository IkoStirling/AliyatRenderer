#include "AYEngineCore.h"
#include <windows.h>
#include <mmsystem.h>  // ����������ͷ�ļ�
#pragma comment(lib, "winmm.lib") 

AYEngineCore& AYEngineCore::getInstance()
{
	static AYEngineCore mInstance;
	return mInstance;
}

void AYEngineCore::init()
{
    _lastFrameTime = std::chrono::steady_clock::now();
	AYModuleManager::getInstance().allModuleInit();
}


AYEngineCore::~AYEngineCore()
{
}

void AYEngineCore::setTargetFPS(float fps)
{
    _targetFPS = std::clamp(fps, 1.0f, 10000.0f); // ���ú�������
    _invTargetFPS = 1.0f / _targetFPS; // Ԥ���㵹��
    _accumulatedTime = 0.0f; // �����ۻ�ʱ��
}

void AYEngineCore::setTimeScale(float scale)
{
    _timeScale = max(0.0f, scale);
}

float AYEngineCore::getCurrentFPS() const
{
	return _currentFPS;
}

float AYEngineCore::getDeltaTime() const
{
    return _unscaledDeltaTime * _timeScale;
}

float AYEngineCore::getUnscaledDeltaTime() const
{
    return _unscaledDeltaTime;
}

void AYEngineCore::_updateFPSStats(int& frameCount, std::chrono::steady_clock::time_point& lastFpsUpdate) {
    using namespace std::chrono;
    auto now = steady_clock::now();
    float elapsed = duration<float>(now - lastFpsUpdate).count();

    if (elapsed >= 1.0f) {
        _currentFPS = frameCount / elapsed;
        frameCount = 0;
        lastFpsUpdate = now;

        std::cout << "Current FPS: " << _currentFPS
            << " | Frame time: " << _unscaledDeltaTime * 1000.0f << "ms"
            << " | Time scale: " << _timeScale
            << std::endl;
    }
}

bool AYEngineCore::update()
{
    _accumulatedTime += _unscaledDeltaTime * _timeScale;

    // ֻ�е��ۻ�ʱ��ﵽһ֡ʱ�Ÿ���
    if (_accumulatedTime >= _invTargetFPS) {
        float delta = _invTargetFPS; // ʹ�ù̶�֡���ȷ���ȶ���
        _accumulatedTime -= delta;

        {
            /*
                ���ڽ�������Ⱦģ��ĸ��µ����������ʱ�����������Ⱦ���ܱ��ppt
                 
            */
            AYModuleManager::getInstance().allModuleUpdate(delta);
        }

    }
    return true;
}

void AYEngineCore::start()
{

#ifdef _WIN32
    timeBeginPeriod(1);
#endif // _WIN32

    

    using clock = std::chrono::steady_clock; // ����steady_clock
    using sec = std::chrono::duration<float>;
    using ms = std::chrono::milliseconds;

    _lastFrameTime = clock::now();
    auto lastFpsUpdate = _lastFrameTime;
    int frameCount = 0;

    // ��ȷ����Ŀ����ʱ��
    const auto targetFrameDuration = ms(static_cast<int>(1000.0f / _targetFPS));

#ifdef _WIN32
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
#endif // _WIN32

    while (!_shouldClosed)
    {
        const auto frameStartTime = clock::now();

        // ����֡ʱ��ͳ��
        _unscaledDeltaTime = std::chrono::duration_cast<sec>(frameStartTime - _lastFrameTime).count();
        _lastFrameTime = frameStartTime;

        if (!update())
            break;

        // ֡�ʿ��� - ��ȷ˯��
        _regulateFrameRate(frameStartTime);

        // FPS����
        frameCount++;
        _updateFPSStats(frameCount, lastFpsUpdate);
    }
#ifdef _WIN32
    timeEndPeriod(1);
#endif // _WIN32
}

void AYEngineCore::close()
{
    _shouldClosed = true;
}

void AYEngineCore::_regulateFrameRate(std::chrono::high_resolution_clock::time_point frameStartTime)
{
    if (_noLimitFPS)
        return;
    using namespace std::chrono;
    auto targetDuration = microseconds(static_cast<int64_t>(1000000.0f * _invTargetFPS));

    auto now = steady_clock::now();
    auto elapsed = now - frameStartTime;

    if (elapsed < targetDuration) {
        auto remaining = targetDuration - elapsed;

        // ���׶εȴ�����
        if (remaining > 2ms) {
            std::this_thread::sleep_for(remaining - 1ms);
            // æ�ȴ�ʣ��ʱ��
            while (steady_clock::now() < frameStartTime + targetDuration) {
                _mm_pause();
            }
        }
        else if (remaining > 500us) {
            std::this_thread::sleep_for(remaining - 100us);
            while (steady_clock::now() < frameStartTime + targetDuration) {
                _mm_pause();
            }
        }
        else {
            // ��ʱ��ֱ��æ�ȴ�
            while (steady_clock::now() < frameStartTime + targetDuration) {
                _mm_pause();
            }
        }
    }
}