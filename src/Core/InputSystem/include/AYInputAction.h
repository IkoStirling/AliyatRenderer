#pragma once
#include "AYInputType.h"
namespace ayt::engine::input
{
    class AYInputSystem;

    class AYInputAction {
    public:
        enum class Type
        {
            Press,
            Release,
            Hold,
            DoublePress,
            LongPress
        };

        AYInputAction() = delete;

        AYInputAction(Type type, UniversalInput input, float long_pressDuration = 0.3f, float double_pressDuration = 0.3f);

        bool evaluate(const AYInputSystem& input) const;

        void rebind(UniversalInput input);

        const UniversalInput& getInput() const;

    private:
        Type _type;
        UniversalInput _input;
        float _durationThreshold;
        float _doublePressThreshold;
    };

}