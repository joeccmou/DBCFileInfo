
// DBCFileInfoView.h : interface of the CDBCFileInfoView class
//

#pragma once


class CDBCFileInfoView : public CView
{
protected: // create from serialization only
	CDBCFileInfoView();
	DECLARE_DYNCREATE(CDBCFileInfoView)

// Attributes
public:
	CDBCFileInfoDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementation
public:
	virtual ~CDBCFileInfoView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in DBCFileInfoView.cpp
inline CDBCFileInfoDoc* CDBCFileInfoView::GetDocument() const
   { return reinterpret_cast<CDBCFileInfoDoc*>(m_pDocument); }
#endif

