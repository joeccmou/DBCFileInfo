
// DBCFileInfoView.h : interface of the CDBCFileInfoView class
//

#pragma once

#include "DBCFileInfoProcess.h"

#define  ICON_SIZE_LARGE    64
#define  ICON_SIZE_SMALL    32


class CDBCFileInfoView : public CView
{
protected: // create from serialization only
	CDBCFileInfoView();
	DECLARE_DYNCREATE(CDBCFileInfoView)

// Attributes
public:
	CDBCFileInfoDoc* GetDocument() const;
protected:
	CImageList				m_LargeImageList;
	CImageList				m_SmallImageList;
	CImageList				m_StateImageList;

	CRect					m_ClientRect;
	CListCtrl				m_ListCtrl;

	DWORD                   m_dwViewType;
	DBCFileInfoProcess      m_DBCFileProcess;

	vector<UINT32>           m_SelectedMsgIDs;		//the selected messages by the user
// Variables
public:
	size_t                  m_szSelectedItems;

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementation
public:
	virtual ~CDBCFileInfoView();

	void GetSeletedMessage();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void OnInitialUpdate();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnViewReportview();
	afx_msg void OnViewIcon();
	afx_msg void OnUpdateViewReportview(CCmdUI *pCmdUI);
	afx_msg void OnUpdateViewIcon(CCmdUI *pCmdUI);
	afx_msg void OnFileOpen();
	afx_msg void OnReadSignal_Clanguage();
	afx_msg void OnWriteSignal_Clanguage();
	afx_msg void OnInitializeSignal_Clanguage();
	//afx_msg void OnUtilitiesSaecodetodtccode();
};

#ifndef _DEBUG  // debug version in DBCFileInfoView.cpp
inline CDBCFileInfoDoc* CDBCFileInfoView::GetDocument() const
   { return reinterpret_cast<CDBCFileInfoDoc*>(m_pDocument); }
#endif

