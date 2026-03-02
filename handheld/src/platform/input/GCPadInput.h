#ifndef GCPADINPUT_H__
#define GCPADINPUT_H__

#include "Keyboard.h"
#include "Controller.h"
#include "../../client/Options.h"

#ifdef GEKKO
#include <ogc/pad.h>
#endif

class GCPadInput {
public:
    static void init() {
#ifdef GEKKO
        PAD_Init();
        _lastHeldRef() = 0;
#endif
    }

    static void poll(const Options& options) {
#ifdef GEKKO
        PAD_ScanPads();

        unsigned int held = PAD_ButtonsHeld(0);
        unsigned int changed = held ^ _lastHeldRef();

        struct ButtonBinding { int keyCode; unsigned int padMask; };
        const ButtonBinding bindings[] = {
            { options.keyUp.key, PAD_BUTTON_UP },
            { options.keyDown.key, PAD_BUTTON_DOWN },
            { options.keyLeft.key, PAD_BUTTON_LEFT },
            { options.keyRight.key, PAD_BUTTON_RIGHT },
            { options.keyJump.key, PAD_BUTTON_A },
            { options.keyDestroy.key, PAD_BUTTON_B },
            { options.keyUse.key, PAD_BUTTON_X },
            { options.keyBuild.key, PAD_BUTTON_Y },
            { Keyboard::KEY_ESCAPE, PAD_BUTTON_START },
            { options.keyMenuPrevious.key, PAD_BUTTON_UP },
            { options.keyMenuNext.key, PAD_BUTTON_DOWN },
            { options.keyMenuOk.key, PAD_BUTTON_A },
            { options.keyMenuCancel.key, PAD_BUTTON_B },
            { options.keySneak.key, PAD_TRIGGER_L },
            { options.keyUse.key, PAD_TRIGGER_R }
        };

        for (unsigned int i = 0; i < sizeof(bindings)/sizeof(bindings[0]); ++i) {
            unsigned int mask = bindings[i].padMask;
            if (!(changed & mask)) continue;
            Keyboard::feed((unsigned char)bindings[i].keyCode, (held & mask) ? KeyboardAction::KEYDOWN : KeyboardAction::KEYUP);
        }

        float deadZone = options.gamepadDeadZone;
        if (deadZone < 0.0f) deadZone = 0.0f;
        if (deadZone > 0.5f) deadZone = 0.5f;

        float lx = _applyDeadZone(_clampUnit(PAD_StickX(0) / 72.0f), deadZone);
        float ly = _applyDeadZone(_clampUnit(PAD_StickY(0) / 72.0f), deadZone);
        float rx = _applyDeadZone(_clampUnit(PAD_SubStickX(0) / 59.0f), deadZone);
        float ry = _applyDeadZone(_clampUnit(PAD_SubStickY(0) / 59.0f), deadZone);

        float lookScale = options.gamepadSensitivity;
        if (lookScale < 0.1f) lookScale = 0.1f;
        if (lookScale > 2.0f) lookScale = 2.0f;

        rx *= lookScale * (options.gamepadInvertX ? -1.0f : 1.0f);
        ry *= lookScale * (options.gamepadInvertY ? -1.0f : 1.0f);

        Controller::feed(1, Controller::STATE_TOUCH, lx, ly);
        Controller::feed(2, Controller::STATE_TOUCH, _clampUnit(rx), _clampUnit(ry));

        _lastHeldRef() = held;
#endif
    }

private:
    static float _clampUnit(float value) {
        if (value < -1.0f) return -1.0f;
        if (value > 1.0f) return 1.0f;
        return value;
    }

    static float _applyDeadZone(float value, float deadZone) {
        float sign = value < 0 ? -1.0f : 1.0f;
        float abs = value < 0 ? -value : value;
        if (abs <= deadZone) return 0.0f;
        float out = (abs - deadZone) / (1.0f - deadZone);
        return _clampUnit(sign * out);
    }

#ifdef GEKKO
    static unsigned int& _lastHeldRef() {
        static unsigned int s_lastHeld = 0;
        return s_lastHeld;
    }
#endif
};


#endif
