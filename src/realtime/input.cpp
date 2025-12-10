#include "input.h"
#include "../realtime.h"
#include <iostream>
#include <cmath>
#include <glm/glm.hpp>

namespace InputHandler {
    
    void handleKeyPress(Realtime* realtime, QKeyEvent* event) {
        Qt::Key key = Qt::Key(event->key());
        realtime->m_keyMap[key] = true;
        
        if (key == Qt::Key_P && !realtime->m_pKeyPressed) {
            realtime->m_pKeyPressed = true;
            realtime->m_fpsMode = !realtime->m_fpsMode;
            realtime->setFpsMode(realtime->m_fpsMode);
            emit realtime->fpsModeToggled(realtime->m_fpsMode);
        }
        
        if (key == Qt::Key_F && !realtime->m_flashlightFKeyPressed) {
            realtime->m_flashlightFKeyPressed = true;
            realtime->setFlashlightEnabled(!realtime->m_flashlightEnabled);
            emit realtime->flashlightChargeChanged(realtime->m_flashlightCharge, realtime->m_flashlightPenaltyTimer > 0.0f);
        }
        
        if (key == Qt::Key_N) {
            realtime->m_useNormalMapping = !realtime->m_useNormalMapping;
            std::cout << "Normal mapping: " << (realtime->m_useNormalMapping ? "ON" : "OFF") << std::endl;
            realtime->update();
        }
        
        if (key == Qt::Key_B) {
            realtime->m_useBumpMapping = !realtime->m_useBumpMapping;
            std::cout << "Bump mapping: " << (realtime->m_useBumpMapping ? "ON" : "OFF") << std::endl;
            realtime->update();
        }
        
        if (key == Qt::Key_Plus || key == Qt::Key_Equal) {
            realtime->m_bumpStrength += 2.0f;
            std::cout << "Bump strength: " << realtime->m_bumpStrength << std::endl;
            realtime->update();
        }
        if (key == Qt::Key_Minus) {
            realtime->m_bumpStrength = std::max(0.0f, realtime->m_bumpStrength - 2.0f);
            std::cout << "Bump strength: " << realtime->m_bumpStrength << std::endl;
            realtime->update();
        }
    }
    
    void handleKeyRelease(Realtime* realtime, QKeyEvent* event) {
        Qt::Key key = Qt::Key(event->key());
        realtime->m_keyMap[key] = false;
        
        if (key == Qt::Key_P) {
            realtime->m_pKeyPressed = false;
        }
        
        if (key == Qt::Key_F) {
            realtime->m_flashlightFKeyPressed = false;
        }
    }
    
    void handleMousePress(Realtime* realtime, QMouseEvent* event) {
        if (event->buttons().testFlag(Qt::LeftButton)) {
            realtime->m_mouseDown = true;
            realtime->m_prev_mouse_pos = glm::vec2(event->position().x(), event->position().y());
        }
    }
    
    void handleMouseRelease(Realtime* realtime, QMouseEvent* event) {
        if (!event->buttons().testFlag(Qt::LeftButton)) {
            realtime->m_mouseDown = false;
        }
    }
    
    void handleMouseMove(Realtime* realtime, QMouseEvent* event) {
        if (realtime->m_cameraPath.isPlaying()) {
            return;
        }
        
        int posX = event->position().x();
        int posY = event->position().y();
        
        if (realtime->m_fpsMode) {
            if (realtime->m_ignoreNextMouseMove) {
                realtime->m_ignoreNextMouseMove = false;
                int centerX = realtime->width() / 2;
                int centerY = realtime->height() / 2;
                realtime->m_prev_mouse_pos = glm::vec2(centerX, centerY);
                return;
            }
            
            int deltaX = posX - realtime->m_prev_mouse_pos.x;
            int deltaY = posY - realtime->m_prev_mouse_pos.y;
            realtime->m_prev_mouse_pos = glm::vec2(posX, posY);
            
            float sensitivity = 0.006f;
            if (std::abs(deltaX) > 0 || std::abs(deltaY) > 0) {
                realtime->m_pendingRotation.x += -deltaX * sensitivity;
                realtime->m_pendingRotation.y += -deltaY * sensitivity;
            }
        } else if (realtime->m_mouseDown) {
            int deltaX = posX - realtime->m_prev_mouse_pos.x;
            int deltaY = posY - realtime->m_prev_mouse_pos.y;
            realtime->m_prev_mouse_pos = glm::vec2(posX, posY);
            realtime->m_pendingRotation.x += deltaX * 0.003f;
            realtime->m_pendingRotation.y += deltaY * 0.003f;
        }
    }
}

