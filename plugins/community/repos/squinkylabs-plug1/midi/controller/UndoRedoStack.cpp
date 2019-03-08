
#include <assert.h>

#include "SqCommand.h"
#include "UndoRedoStack.h"

#if 0

std::shared_ptr<SqCommand> UndoRedoStack::popUndo()
{
    assert(canUndo());

    auto returnValue = undoList.front();
    undoList.pop_front();
    return returnValue;
}

std::shared_ptr<SqCommand> UndoRedoStack::popRedo()
{
    assert(canRedo());

    auto returnValue = redoList.front();
    redoList.pop_front();
    return returnValue;
}

void UndoRedoStack::pushUndo(std::shared_ptr<SqCommand> cmd)
{
    undoList.push_front(cmd);
}

void UndoRedoStack::pushRedo(std::shared_ptr<SqCommand> cmd)
{
    redoList.push_front(cmd);
}
#endif

bool UndoRedoStack::canUndo() const
{
    return !undoList.empty();
}

bool UndoRedoStack::canRedo() const
{
    return !redoList.empty();
}

void UndoRedoStack::execute(std::shared_ptr<SqCommand> cmd)
{
    cmd->execute();
    undoList.push_front(cmd);
    redoList.clear();   
}

void UndoRedoStack::undo()
{
    assert(canUndo());
    CommandPtr cmd = undoList.front();
    cmd->undo();
    undoList.pop_front();

    redoList.push_front(cmd);
}

void UndoRedoStack::redo()
{
    assert(canRedo());
    CommandPtr cmd = redoList.front();
    cmd->execute();
    redoList.pop_front();

    undoList.push_front(cmd);
}