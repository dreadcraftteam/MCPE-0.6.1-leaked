#pragma once

class Platform
{
public:
    virtual ~Platform() {}

    virtual void pointerDown(int pointerId, int x, int y) = 0;
    virtual void pointerUp(int pointerId, int x, int y) = 0;
    virtual void pointerMove(int pointerId, int x, int y) = 0;
};
