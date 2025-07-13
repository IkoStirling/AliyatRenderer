#include "AYEngineCore.h"
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h> 
#pragma comment(lib, "winmm.lib") 

#include "AYRendererManager.h"
#include "AYInputSystem.h"
#include "AYInputBinding.h"
#include "AYGameObject.h"
#include "Component/AYTransformComponent.h"
#include "Component/AYSpriteRenderComponent.h"
#include "Preset/Orc.h"
#include "Preset/Orc_scene.h"

AYEngineCore& AYEngineCore::getInstance()
{
	static AYEngineCore mInstance;
	return mInstance;
}

void AYEngineCore::init()
{
    _lastFrameTime = std::chrono::steady_clock::now();

    GET_MODULE("MemoryPool")->init();
    GET_MODULE("EventSystem")->init();
    GET_MODULE("ResourceManager")->init();
    GET_MODULE("Renderer")->init();
    GET_MODULE("InputSystem")->init();
    GET_MODULE("SceneManager")->init();
    GET_MODULE("PhysicsSystem")->init();

    GET_CAST_MODULE(Mod_Renderer, "Renderer")->setWindowCloseCallback([this]() { 
        close(); 
        });

    auto binding = std::make_shared<AYInputBinding>();
    binding->addAction("double_click", AYInputAction::Type::LongPress, KeyboardInput{GLFW_KEY_W});
    binding->addAction("Mouse_Left", AYInputAction::Type::LongPress, MouseButtonInput{GLFW_MOUSE_BUTTON_LEFT});
    binding->addAction("GamePad_X", AYInputAction::Type::LongPress, GamepadButtonInput{GLFW_GAMEPAD_BUTTON_X});
    auto inputSystem = GET_CAST_MODULE(Mod_InputSystem, "InputSystem");
    inputSystem->addInputMapping("default", binding);

    auto sm = GET_CAST_MODULE(AYSceneManager, "SceneManager");
    sm->addScene<Orc_scene>("level0");
    sm->loadScene("level0");
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
    _timeScale = std::max(0.0f, scale);
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
    }
}

void AYEngineCore::update()
{

    GET_MODULE("InputSystem")->update(_invTargetFPS);

    // ֻ�е��ۻ�ʱ��ﵽһ֡ʱ�Ÿ���
    _accumulatedTime += _unscaledDeltaTime * _timeScale;
    if (_accumulatedTime >= _invTargetFPS) {
        float delta = _invTargetFPS; // ʹ�ù̶�֡���ȷ���ȶ���
        _accumulatedTime -= delta;

        {
            GET_MODULE("EventSystem")->update(delta);
            GET_MODULE("ResourceManager")->update(delta);
            //GET_MODULE("Network")->update(delta);
            GET_MODULE("SceneManager")->update(delta);
            GET_MODULE("PhysicsSystem")->update(delta);
        }
    }

    GET_MODULE("Renderer")->update(_invTargetFPS);

}

void AYEngineCore::start()
{

#ifdef _WIN32
    timeBeginPeriod(1);
#endif

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
#endif

    while (!_shouldClosed)
    {
        const auto frameStartTime = clock::now();

        // ����֡ʱ��ͳ��
        _unscaledDeltaTime = std::chrono::duration_cast<sec>(frameStartTime - _lastFrameTime).count();
        _lastFrameTime = frameStartTime;

        update();

        // ֡�ʿ��� - ��ȷ˯��
        _regulateFrameRate(frameStartTime);

        // FPS����
        frameCount++;
        _updateFPSStats(frameCount, lastFpsUpdate);
    }

#ifdef _WIN32
    timeEndPeriod(1);
#endif
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