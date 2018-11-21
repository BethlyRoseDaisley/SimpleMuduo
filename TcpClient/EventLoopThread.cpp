#include <assert.h>

#include "Logger.hh"
#include "EventLoop.hh"
#include "EventLoopThread.hh"

EventLoopThread::EventLoopThread()
  :p_loop(NULL),
  m_exiting(false),
  m_thread(std::bind(&EventLoopThread::threadFunc, this)),
  m_mutex(),
  m_cond(m_mutex)
{

}

EventLoopThread::~EventLoopThread()
{
  m_exiting = true;
  if(p_loop !=NULL)
  {
    p_loop->quit();
    m_thread.join();
  }
}


EventLoop* EventLoopThread::startLoop()
{
  assert(!m_thread.isStarted());
  m_thread.start();

  {
    MutexLockGuard lock(m_mutex);
    while(p_loop == NULL)
    {
      LOG_TRACE << "EventLoopThread::startLoop() wait()";
      m_cond.wait();
    }
  }

  LOG_TRACE << "EventLoopThread::startLoop() wakeup";

  return p_loop;
}


void EventLoopThread::threadFunc()
{
  EventLoop loop;

  if(m_threadInitCallBack)
  {
    m_threadInitCallBack(&loop);
  }

  {
    MutexLockGuard lock(m_mutex);
    p_loop = &loop;
    m_cond.notify();
    LOG_TRACE << "EventLoopThread::threadFunc() notify()";
  }

  loop.loop();

  p_loop = NULL;

}