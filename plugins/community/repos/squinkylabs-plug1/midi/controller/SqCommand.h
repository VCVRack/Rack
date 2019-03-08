#pragma once

#include <memory>

class SqCommand
{
public:
    virtual void execute() = 0;
    virtual void undo() = 0;
};

using CommandPtr = std::shared_ptr<SqCommand>;