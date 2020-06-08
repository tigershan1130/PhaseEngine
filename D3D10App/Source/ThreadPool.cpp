//--------------------------------------------------------------------------------------
// File: ThreadPool.cpp
//
// Worker thread pooling
//
// Coded by Nate Orr
//--------------------------------------------------------------------------------------
#pragma unmanaged
#include "ThreadPool.h"

ThreadPool g_ThreadPool;

ThreadPool::ThreadPool()
{
	m_nMaxNumThreads = MAXTHREADS;

	m_Handles[TH_EMPTY] = CreateSemaphore(NULL,MAXQUEUE,MAXQUEUE,L"EmptySlot");
	m_Handles[TH_WORK] = CreateSemaphore(NULL,0,MAXQUEUE,L"WorkToDo");
	m_Handles[TH_EXIT] = CreateEvent(NULL,TRUE,FALSE,L"Exit");
	InitializeCriticalSection(&CriticalSection); 

	for(int i=0;i<m_nMaxNumThreads;i++)
	{
		m_threadhandles[i] = (HANDLE) _beginthreadex( NULL, 0, ThreadExecute, this, 0, &m_Thrdaddr);
	}

	m_nTopIndex = 0;
	m_nBottomIndex = 0;
	nWorkInProgress=0;
}


ThreadPool::~ThreadPool()
{
	
}

unsigned _stdcall ThreadPool::ThreadExecute(void *Param){

	((ThreadPool*)Param)->DoWork();
	return(0);
}

void ThreadPool::DoWork()
{
	WorkerThread* cWork;
	while(GetWork(&cWork))
	{
		cWork->ThreadExecute();
		InterlockedDecrement(&nWorkInProgress);
	}
	
}


//Queues up another to work
BOOL ThreadPool::SubmitJob(WorkerThread* cWork)
{
	InterlockedIncrement(&nWorkInProgress);
	if(WaitForSingleObject(m_Handles[TH_EMPTY],INFINITE) != WAIT_OBJECT_0){
		return(0);
	}
	
	EnterCriticalSection(&CriticalSection); 
	m_pQueue[m_nTopIndex] = cWork;
	m_nTopIndex = (m_nTopIndex++) % (MAXQUEUE -1);
	ReleaseSemaphore(m_Handles[TH_WORK],1,NULL);
	LeaveCriticalSection(&CriticalSection);

	return(1);	
}



BOOL ThreadPool::GetWork(WorkerThread** cWork)
{
	if((WaitForMultipleObjects(2,m_Handles,FALSE,INFINITE) - WAIT_OBJECT_0) == 1)
		return 0;
	
	EnterCriticalSection(&CriticalSection); 
	*cWork = m_pQueue[m_nBottomIndex];
	m_nBottomIndex = (m_nBottomIndex++) % (MAXQUEUE -1);
	ReleaseSemaphore(m_Handles[TH_EMPTY],1,NULL);
	LeaveCriticalSection(&CriticalSection);

	return 1;	
}


void ThreadPool::DestroyPool(){
	while(nWorkInProgress > 0){
		Sleep(10);
	}
	SetEvent(m_Handles[TH_EXIT]);
	DeleteCriticalSection(&CriticalSection);

}
