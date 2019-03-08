#pragma once

#include <memory>
#include <list>

class SqCommand;

class UndoRedoStack
{
public:
    bool canUndo() const;
    bool canRedo() const;

    // execute the command, make undo record
    void execute(std::shared_ptr<SqCommand>);
    void undo();
    void redo();

private:



    std::list<std::shared_ptr<SqCommand>> undoList;
    std::list<std::shared_ptr<SqCommand>> redoList;

};

using UndoRedoStackPtr = std::shared_ptr<UndoRedoStack>;
