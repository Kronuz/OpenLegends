
#pragma once

class CToolBarBox : 
   public CWindowImpl<CToolBarBox, CToolBarCtrl>
{
public:
   DECLARE_WND_SUPERCLASS(_T("CToolBarBox"), GetWndClassName())

   BEGIN_MSG_MAP(CToolBarBox)
      DEFAULT_REFLECTION_HANDLER()
   END_MSG_MAP()
};
