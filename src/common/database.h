#pragma once

class ThreadOperation
{
public:
    virtual void RunThreadPart() = 0;
    virtual void CancelThinkPart() = 0;
    virtual void RunThinkPart() = 0;
private:
};