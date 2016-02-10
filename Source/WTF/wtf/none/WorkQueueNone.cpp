#include "config.h"
#include "WorkQueue.h"

void WorkQueue::platformInitialize(const char*, Type, QOS)
{
}

void WorkQueue::platformInvalidate()
{
}

void WorkQueue::dispatch(std::function<void()>)
{
}

void WorkQueue::dispatchAfter(std::chrono::nanoseconds, std::function<void()>)
{
}
