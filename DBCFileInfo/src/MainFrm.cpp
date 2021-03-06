
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "DBCFileInfo.h"

#include "MainFrm.h"
#include "DBCFileInfoDoc.h"
#include "DBCFileInfoView.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_NUM, &CMainFrame::OnUpdateNum)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndStatusBar.Create(this))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));
	m_wndStatusBar.SetPaneInfo(2, ID_INDICATOR_NUM, SBPS_NORMAL, 200);
	
	
	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	int xSize = ::GetSystemMetrics(SM_CXSCREEN);
	int ySize = ::GetSystemMetrics(SM_CYSCREEN);
	cs.cx = xSize * 6 / 10;
	cs.cy = ySize * 6 / 10;
	cs.x = (xSize - cs.cx) / 2;
	cs.y = (ySize - cs.cy) / 2;

	cs.style = WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU | WS_THICKFRAME;

	//cs.style |= FWS_ADDTOTITLE;	//将文档名显示到窗口标题

	return TRUE;
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}
#endif //_DEBUG


// CMainFrame message handlers

void CMainFrame::OnUpdateFrameTitle(BOOL bAddToTitle)
{
	// TODO: Add your specialized code here and/or call the base class
	CString strTitle;

	strTitle = _T("CAN Network Database File");
	SetTitle(strTitle);

	CFrameWnd::OnUpdateFrameTitle(bAddToTitle);
}

void CMainFrame::OnUpdateNum(CCmdUI *pCmdUI)
{
	pCmdUI->Enable();
	CString strNum;
	
	CDBCFileInfoView* pView = dynamic_cast<CDBCFileInfoView*>(GetActiveView());
	if (pView != NULL)
	{
		strNum.Format(_T("%d item selected"), pView->m_szSelectedItems);
	}
	else
	{
		strNum.Empty();
	}
	pCmdUI->SetText(strNum);
}