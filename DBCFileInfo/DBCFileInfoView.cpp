
// DBCFileInfoView.cpp : implementation of the CDBCFileInfoView class
//

#include "stdafx.h"
// SHARED_HANDLERS can be defined in an ATL project implementing preview, thumbnail
// and search filter handlers and allows sharing of document code with that project.
#ifndef SHARED_HANDLERS
#include "DBCFileInfo.h"
#endif

#include "DBCFileInfoDoc.h"
#include "DBCFileInfoView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDBCFileInfoView

IMPLEMENT_DYNCREATE(CDBCFileInfoView, CView)

BEGIN_MESSAGE_MAP(CDBCFileInfoView, CView)
END_MESSAGE_MAP()

// CDBCFileInfoView construction/destruction

CDBCFileInfoView::CDBCFileInfoView()
{
	// TODO: add construction code here

}

CDBCFileInfoView::~CDBCFileInfoView()
{
}

BOOL CDBCFileInfoView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CDBCFileInfoView drawing

void CDBCFileInfoView::OnDraw(CDC* /*pDC*/)
{
	CDBCFileInfoDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// CDBCFileInfoView diagnostics

#ifdef _DEBUG
void CDBCFileInfoView::AssertValid() const
{
	CView::AssertValid();
}

void CDBCFileInfoView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CDBCFileInfoDoc* CDBCFileInfoView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDBCFileInfoDoc)));
	return (CDBCFileInfoDoc*)m_pDocument;
}
#endif //_DEBUG


// CDBCFileInfoView message handlers
