
#include <assert.h>
#include <assert.h>

#include "SqCommand.h"
#include "UndoRedoStack.h"

class Cmd : public SqCommand
{
public:
    virtual void execute() override
    {
        ++executeCount;
    }
    virtual void undo() override
    {
        ++undoCount;
    }

    int id;
    int executeCount = 0;
    int undoCount = 0;
};

using Cp = std::shared_ptr<Cmd>;

static void test0()
{
    UndoRedoStack ur;
    assert(!ur.canRedo());
    assert(!ur.canUndo());

    std::shared_ptr<Cmd> cmd(std::make_shared<Cmd>());
    assert(cmd->executeCount == 0);
    assert(cmd->undoCount == 0);
    ur.execute(cmd);

    assert(cmd->executeCount == 1);
    assert(cmd->undoCount == 0);

    assert(ur.canUndo());
    assert(!ur.canRedo());

    ur.undo();
    assert(cmd->undoCount == 1);
    assert(!ur.canUndo());

    assert(ur.canRedo());
    ur.redo();
}

static void test1()
{
    UndoRedoStack ur;
    assert(!ur.canRedo());
    assert(!ur.canUndo());

    std::shared_ptr<Cmd> cmd(std::make_shared<Cmd>());
    assert(cmd->executeCount == 0);
    assert(cmd->undoCount == 0);
    cmd->id = 55;
    ur.execute(cmd);

    assert(cmd->executeCount == 1);
    assert(cmd->undoCount == 0);

    std::shared_ptr<Cmd> cmd2(std::make_shared<Cmd>());
    cmd2->id = 77;
    ur.execute(cmd2);
    assert(cmd2->executeCount == 1);
    assert(cmd->executeCount == 1);
    assert(cmd2->undoCount == 0);

    ur.undo();
    assert(cmd2->undoCount == 1);
    ur.undo();
    assert(cmd->undoCount == 1);
}

void testUndoRedo()
{
    test0();
    test1();
}