#pragma once

#include <QKeyEvent>
#include <QMouseEvent>
#include "../realtime.h"

namespace InputHandler {
    void handleKeyPress(Realtime* realtime, QKeyEvent* event);
    void handleKeyRelease(Realtime* realtime, QKeyEvent* event);
    void handleMousePress(Realtime* realtime, QMouseEvent* event);
    void handleMouseRelease(Realtime* realtime, QMouseEvent* event);
    void handleMouseMove(Realtime* realtime, QMouseEvent* event);
}

