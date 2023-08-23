#include "ImageQueue.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif



//////////////////////////////////////////////////////
//
// Class : CImageQueue
//
//////////////////////////////////////////////////////

CImageQueue::CImageQueue()
{
	m_dwCountPush = 0;
	m_dwlastPush = 0;
}

CImageQueue::~CImageQueue()
{
	Clear();
}

CImageItem* CImageQueue::operator[] (uint i)
{
	Lock();
	CImageItem* pItem = ((std::deque<CImageItem*>)(*this))[i];
	Unlock();
	return pItem;
}

int CImageQueue::Size()
{
	Lock();
	int nSize = (int)size();
	Unlock();
	return nSize;
}

void CImageQueue::Lock()
{
	m_csQueue.lock();
}

void CImageQueue::Unlock()
{
	m_csQueue.unlock();
}

void CImageQueue::Push(CImageItem* pItem)
{
	int nSize = (int)size();
	if (nSize > 1000)
	{
		Clear();
		return;
	}

	m_csQueue.lock();
	
	push_back(pItem);
	m_dwCountPush++;
	m_dwlastPush = GetTickCount64();
	m_csQueue.unlock();

}

CImageItem* CImageQueue::Pop()
{
	CImageItem* pItem = NULL;

	Lock();
	int nSize = (int)size();
	if (nSize)
	{
		pItem = front();
		pop_front();
	}
	Unlock();

	return pItem;
}

CImageItem* CImageQueue::Front()
{
	Lock();
	CImageItem* pItem = front();
	Unlock();
	return pItem;
}

void CImageQueue::Clear()
{
	Lock();
	int nItem = (int)size();
	if (nItem) {
		std::deque<CImageItem*>::iterator iter = begin();
		std::deque<CImageItem*>::iterator iterEnd = end();
		while (iter != iterEnd) {
			delete* iter;
			iter++;
		}
	}
	clear();
	m_dwCountPush = 0;
	Unlock();
}


DWORD CImageQueue::GetLastPushTime()
{
	DWORD dwLastPush;
	Lock();
	dwLastPush = m_dwlastPush;
	Unlock();
	return dwLastPush;
}

void CImageQueue::ResetLastPushTime()
{
	Lock();
	m_dwlastPush = 0;
	Unlock();
}
