#pragma once
#include "pch.h"

#include <mutex>

//////////////////////////////////////////////////////
//
// Class : CImageItem
//
//////////////////////////////////////////////////////

class CImageItem
{
public:
  
   cv::Mat     m_cvMatImage;        
   double      m_dfNormalPlayTime;

   CImageItem()
   {
      m_dfNormalPlayTime = 0.0;
   }
};

//////////////////////////////////////////////////////
//
// Class : CImageQueue
//
//////////////////////////////////////////////////////

class CImageQueue : public std::deque<CImageItem*>
{
public:
   int              m_dwCountPush;
   DWORD            m_dwlastPush;

   std::recursive_mutex m_csQueue;

public:
   CImageQueue  (void);
   ~CImageQueue (void);

public:
   CImageItem* operator[] (uint i);

public:
   void                      Lock   (  );
   void                      Unlock (  );
   int                       Size   (  );
   void                      Push   (CImageItem* pItem);
   CImageItem* Pop    (  );
   CImageItem* Front  (  );
   void                      Clear  (  );
   DWORD                     GetLastPushTime (   );
   void                      ResetLastPushTime (   );
}; 
