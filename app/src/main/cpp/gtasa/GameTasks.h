#pragma once
#include <functional>
namespace WorldRPG::GameTasks {
// Service exclusively from the game hook. Run cancels work that has not started
// after 200 ms; started mutations finish before their caller can return.
void Service();
bool Run(std::function<void()> task);
}
