#include "base/threadpool.h"
#include "base/Logger.h"

namespace vodeox
{

static const char* component = "Threadpool";

Worker::Worker(vodeox::concurrent_queue<WorkItem>& wi_queue)
               : m_items_queue(wi_queue), m_bIsRunning(false)
{
    LOG_INFO(component, "Worker created.");
}

Worker::~Worker()
{
    LOG_INFO(component, "Worker destroyed.");
}

void Worker::run()
{
    LOG_INFO(component, "Worker run.");

    while (m_bIsRunning)
    {
        std::vector<WorkItem> items;
        m_items_queue.wait_and_pop(items);

        for (int i = 0; i < items.size(); i++)
            items[i].execute();
    }
}

void Worker::shutdown()
{
    m_bIsRunning = true;
}

Threadpool::Threadpool(int numThreads)
{
    LOG_INFO(component, "Threadpool created");

    for (int i = 0; i < numThreads; i++)
    {
        std::tr1::shared_ptr<Worker> w(new Worker(m_witems));
        m_workers.push_back(w);
    }
}

Threadpool::~Threadpool()
{
    LOG_INFO(component, "Threadpool destroyed");
    stop();
}

void Threadpool::start()
{
    LOG_INFO(component, "Threadpool start");

    for (int i = 0; i < m_workers.size(); i++)
        m_workers[i]->start(m_workers[i].get());
}

void Threadpool::stop()
{
    LOG_INFO(component, "Threadpool stop");

    for (int i = 0; i < m_workers.size(); i++)
        m_workers[i]->shutdown();
}

void Threadpool::add(const WorkItem& wi)
{
    LOG_INFO(component, "Threadpool got new item");

    m_witems.push(wi);
}

} //namespace vodeox
