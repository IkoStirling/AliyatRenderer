#include "AYEngineCore.h"
#include "AYEngineCore.h"
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h> 
#pragma comment(lib, "winmm.lib") 

#include "AYRendererManager.h"
#include "AYInputSystem.h"
#include "AYInputBinding.h"
#include "AYGameObject.h"

#include "Preset/Orc.h"
#include "Preset/Orc_scene.h"

namespace ayt::engine
{
    Core& Core::getInstance()
    {
        static Core mInstance;
        return mInstance;
    }

    void Core::init()
    {
        setLogger(std::make_unique<log::Logger>());
        GET_MODULE("MemoryPool")->init();
        GET_MODULE("EventSystem")->init();
        GET_MODULE("MaterialManager")->init();
        GET_MODULE("ResourceManager")->init();
        GET_MODULE("AVEngine")->init();
        GET_MODULE("Renderer")->init();
        GET_MODULE("InputSystem")->init();
        GET_MODULE("SceneManager")->init();
        GET_MODULE("PhysicsSystem")->init();

        GET_CAST_MODULE(Mod_Renderer, "Renderer")->setWindowCloseCallback([this]() {
            close();
            });

        auto binding = std::make_shared<InputBinding>();
        binding->addAction("double_click", InputAction::Type::LongPress, KeyboardInput{ GLFW_KEY_W });
        binding->addAction("Mouse_Left", InputAction::Type::LongPress, MouseButtonInput{ GLFW_MOUSE_BUTTON_LEFT });
        binding->addAction("GamePad_X", InputAction::Type::LongPress, GamepadButtonInput{ GLFW_GAMEPAD_BUTTON_X });
        auto inputSystem = GET_CAST_MODULE(Mod_InputSystem, "InputSystem");
        inputSystem->addInputMapping("default", binding);

        auto sm = GET_CAST_MODULE(SceneManager, "SceneManager");
        sm->addScene<Orc_scene>("level0");
        sm->loadScene("level0");

        _lastFrameTime = std::chrono::steady_clock::now();
    }


    Core::~Core()
    {
    }

    void Core::setTargetFPS(float fps)
    {
        _targetFPS = std::clamp(fps, 1.0f, 10000.0f); // 设置合理上限
        _invTargetFPS = 1.0f / _targetFPS; // 预计算倒数
        _accumulatedTime = 0.0f; // 重置累积时间
    }

    void Core::setTimeScale(float scale)
    {
        _timeScale = std::max(0.0f, scale);
    }

    float Core::getCurrentFPS() const
    {
        return _currentFPS;
    }

    float Core::getDeltaTime() const
    {
        return _unscaledDeltaTime * _timeScale;
    }

    float Core::getUnscaledDeltaTime() const
    {
        return _unscaledDeltaTime;
    }

    void Core::_updateFPSStats(int& frameCount, std::chrono::steady_clock::time_point& lastFpsUpdate) {
        using namespace std::chrono;
        auto now = steady_clock::now();
        float elapsed = duration<float>(now - lastFpsUpdate).count();

        if (elapsed >= 1.0f) {
            _currentFPS = frameCount / elapsed;
            frameCount = 0;
            lastFpsUpdate = now;
        }
    }

    void Core::update()
    {


        // 只有当累积时间达到一帧时才更新
        _accumulatedTime += _unscaledDeltaTime * _timeScale;
        if (_accumulatedTime >= _invTargetFPS) {
            float delta = _invTargetFPS; // 使用固定帧间隔确保稳定性
            _accumulatedTime -= delta;

            {
                GET_MODULE("InputSystem")->update(delta); //涉及持续按下状态，因此也放在逻辑块中
                GET_MODULE("EventSystem")->update(delta);
                GET_MODULE("SceneManager")->update(delta);
                //GET_MODULE("Network")->update(delta);
                GET_MODULE("PhysicsSystem")->update(delta);
                GET_MODULE("CombatSystem")->update(delta);
                GET_MODULE("ResourceManager")->update(delta);
                GET_MODULE("AVEngine")->update(delta);
            }
        }

        GET_MODULE("Renderer")->update(_invTargetFPS);

    }

    void Core::start()
    {

#ifdef _WIN32
        timeBeginPeriod(1);
#endif

        using clock = std::chrono::steady_clock; // 改用steady_clock
        using sec = std::chrono::duration<float>;
        using ms = std::chrono::milliseconds;

        _lastFrameTime = clock::now();
        auto lastFpsUpdate = _lastFrameTime;
        int frameCount = 0;

        // 精确计算目标间隔时间
        const auto targetFrameDuration = ms(static_cast<int>(1000.0f / _targetFPS));

#ifdef _WIN32
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
#endif

        while (!_shouldClosed)
        {
            const auto frameStartTime = clock::now();

            // 更新帧时间统计
            _unscaledDeltaTime = std::chrono::duration_cast<sec>(frameStartTime - _lastFrameTime).count();
            _lastFrameTime = frameStartTime;

            update();

            // 帧率控制 - 精确睡眠
            _regulateFrameRate(frameStartTime);

            // FPS计算
            frameCount++;
            _updateFPSStats(frameCount, lastFpsUpdate);
        }

#ifdef _WIN32
        timeEndPeriod(1);
#endif
    }

    void Core::close()
    {
        _shouldClosed = true;
        modules::ModuleManager::getInstance().allModuleShutdown();
    }

    void Core::setLogger(std::unique_ptr<log::Logger>&& logger)
    {
        if (_logger)
            _logger->close();
        _logger = std::move(logger);
        _logger->init();
    }

    void Core::_regulateFrameRate(std::chrono::high_resolution_clock::time_point frameStartTime)
    {
        if (_noLimitFPS)
            return;
        using namespace std::chrono;
        auto targetDuration = microseconds(static_cast<int64_t>(1000000.0f * _invTargetFPS));

        auto now = steady_clock::now();
        auto elapsed = now - frameStartTime;

        if (elapsed < targetDuration) {
            auto remaining = targetDuration - elapsed;

            // 三阶段等待策略
            if (remaining > 2ms) {
                std::this_thread::sleep_for(remaining - 1ms);
                // 忙等待剩余时间
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
                // 短时间直接忙等待
                while (steady_clock::now() < frameStartTime + targetDuration) {
                    _mm_pause();
                }
            }
        }
    }
}