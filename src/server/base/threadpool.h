#ifndef __THREADPOOL_H
#define __THREADPOOL_H

#include <vector>
#include <tr1/memory>

#include "base/scoped_lock.h"
#include "base/concurrent_queue.h"

namespace vodeox
{

class WorkItem
{
 public:
    virtual void execute() {}
};

typedef vodeox::concurrent_queue<std::tr1::shared_ptr<WorkItem> >  WorkQueue;

class Worker : public vodeox::thread
{
 protected:
    WorkQueue&                  m_items_queue;
    bool                        m_bIsRunning;
 public:
    Worker(WorkQueue& wi_queue);
    virtual ~Worker();

    void run();
    void shutdown();
};

class Threadpool
{
 protected:
    vodeox::mutex                               mutex;
    std::vector<std::tr1::shared_ptr<Worker> >  m_workers;
    WorkQueue                                   m_witems;

 public:
    Threadpool(int numThreads=10);
    virtual ~Threadpool();

    void start();
    void stop();

    void add(std::tr1::shared_ptr<WorkItem>& wi); 
};

} //namespace 
#endif
