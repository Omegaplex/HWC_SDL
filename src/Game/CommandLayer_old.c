/*=============================================================================
    Name    : CommandLayer_old.c
    Purpose : Reference-only dead CommandLayer helpers kept out of the build.
=============================================================================*/
#include "types.h"
#include "debug.h"
#include "universe.h"
#include "dock.h"
#include "commandlayer.h"

void clCancelAllLaunchOrdersFromPlayer(struct Player *player)
{
    Node *nextnode;
    Node *curnode = universe.mainCommandLayer.todolist.head;
    CommandToDo *command;
    sdword i;

    dbgAssert(singlePlayerGame);

    while (curnode != NULL)
    {
        command = (CommandToDo *)listGetStructOfNode(curnode);
        nextnode = curnode->next;

        if (command->ordertype.order == COMMAND_LAUNCHSHIP)
        {
            for (i = 0; i < command->selection->numShips; i++)
            {
                if (command->selection->ShipPtr[i]->playerowner == player)
                {
                    if (!ShipCanHyperspace(command->selection->ShipPtr[i]))
                    {
                        RemoveShipFromLaunching(command->selection->ShipPtr[i]);
                        if (clRemoveShipFromSelection(command->selection, command->selection->ShipPtr[i]))
                        {
                            if (command->selection->numShips == 0)
                            {
                                FreeCommandToDoContents(command);
                                listDeleteNode(curnode);
                                break;
                            }
                        }
                    }
                }
            }
        }

        curnode = nextnode;
    }
}
