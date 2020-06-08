//--------------------------------------------------------------------------------------
// File: ThreadPool.h
//
// Worker thread pooling
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged
#pragma once

#include <process.h>
#include <windows.h>

#define MAXQUEUE 15
#define MAXTHREADS 4

//--------------------------------------------------------------------------------------
// A worker thread object that is passed to the thread pool.  This class can be
// inherited in order to submit different jobs to the thread pool
//--------------------------------------------------------------------------------------
class WorkerThread
{
public:
	unsigned virtual ThreadExecute(){
		return 0;	
	};
};



//--------------------------------------------------------------------------------------
// A pool of worker threads.  When a job is submitted, it is given its own thread.
// If no thread is available the pool waits until a thread finishes
//--------------------------------------------------------------------------------------
class ThreadPool  
{
public:
	ThreadPool();
	
	BOOL SubmitJob(WorkerThread* cWork);
	BOOL GetWork(WorkerThread** cWork);

	virtual ~ThreadPool();
	void DoWork();
	void DestroyPool();


private:
	long nWorkInProgress;
	int m_nMaxNumThreads;
	unsigned int m_Thrdaddr;

	HANDLE m_threadhandles[MAXQUEUE];
	WorkerThread* m_pQueue[MAXQUEUE];
	void *m_pParamArray[MAXQUEUE];

	static unsigned _stdcall ThreadExecute(void *Param);

	int m_nTopIndex;
	int m_nBottomIndex;
	
	enum TH_TYPE
	{
		TH_WORK=0,
		TH_EXIT=1,
		TH_EMPTY=2,
	};
	HANDLE m_Handles[3];

	CRITICAL_SECTION CriticalSection;
};

extern ThreadPool g_ThreadPool;