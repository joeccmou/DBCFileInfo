
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

#include "DBCFileInfoProcess.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDBCFileInfoView

IMPLEMENT_DYNCREATE(CDBCFileInfoView, CView)

BEGIN_MESSAGE_MAP(CDBCFileInfoView, CView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_VIEW_REPORTVIEW, &CDBCFileInfoView::OnViewReportview)
	ON_COMMAND(ID_VIEW_ICONVIEW, &CDBCFileInfoView::OnViewIcon)
	ON_UPDATE_COMMAND_UI(ID_VIEW_REPORTVIEW, &CDBCFileInfoView::OnUpdateViewReportview)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ICONVIEW, &CDBCFileInfoView::OnUpdateViewIcon)
	ON_COMMAND(ID_FILE_OPEN, &CDBCFileInfoView::OnFileOpen)
	ON_COMMAND(ID_READ_SIGNAL_CLANGUAGE, &CDBCFileInfoView::OnReadSignal_Clanguage)
	ON_COMMAND(ID_WRITE_SIGNAL_CLANGUAGE, &CDBCFileInfoView::OnWriteSignal_Clanguage)
	ON_COMMAND(ID_INITIALIZE_SIGNAL_CLANGUAGE, &CDBCFileInfoView::OnInitializeSignal_Clanguage)
	//ON_COMMAND(ID_UTILITIES_SAECODETODTCCODE, &CDBCFileInfoView::OnUtilitiesSaecodetodtccode)
END_MESSAGE_MAP()

// CDBCFileInfoView construction/destruction

CDBCFileInfoView::CDBCFileInfoView()
{
	// TODO: add construction code here
	m_szSelectedItems = 0;

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

int CDBCFileInfoView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here
	

	return 0;
}


void CDBCFileInfoView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	// TODO: Add your specialized code here and/or call the base class

	CRect		client_rect;
	HANDLE      hIconLarge = NULL;
	HANDLE      hIconSmall = NULL;
	BOOL		bSuccess = FALSE;
	int         result = -1;
	DWORD       dwOldExtStyle = 0;

	this->GetWindowRect(&client_rect);
	this->ScreenToClient(&client_rect);
	m_ClientRect = client_rect;

	bSuccess = m_ListCtrl.Create(WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS ,
									client_rect, this, IDD_VIEW_CTRLLIST);
	if (!bSuccess)
	{
		ASSERT(0);
	}
	dwOldExtStyle = m_ListCtrl.SetExtendedStyle(/*LVS_EX_ONECLICKACTIVATE | */LVS_EX_FULLROWSELECT |  LVS_EX_AUTOCHECKSELECT);
	m_dwViewType = LVS_REPORT;
	
	bSuccess = m_StateImageList.Create(IDB_STATE_ICON, 16, 1, RGB(255, 0, 0));
	if (!bSuccess)
	{
		ASSERT(0);
	}
	bSuccess = m_LargeImageList.Create(ICON_SIZE_LARGE, ICON_SIZE_LARGE, ILC_COLOR32, 0, 1);
	if (!bSuccess)
	{
		ASSERT(0);
	}
	bSuccess = m_SmallImageList.Create(ICON_SIZE_SMALL, ICON_SIZE_SMALL, ILC_COLOR32, 0, 1);
	if (!bSuccess)
	{
		ASSERT(0);
	}

	hIconLarge = LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_ICON_MESSAGE_CLOSE), IMAGE_ICON, ICON_SIZE_LARGE, ICON_SIZE_LARGE, 0);
	if (hIconLarge ==NULL)
	{
		ASSERT(0);
	}
	
	result = m_LargeImageList.Add((HICON)hIconLarge);
	if (result == -1)
	{
		ASSERT(0);
	}

	hIconSmall = LoadImage(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDI_ICON_MESSAGE_CLOSE), IMAGE_ICON, ICON_SIZE_SMALL, ICON_SIZE_SMALL, 0);
	if (hIconSmall == NULL)
	{
		ASSERT(0);
	}

	result = m_SmallImageList.Add((HICON)hIconSmall);
	if (result == -1)
	{
		ASSERT(0);
	}

	m_ListCtrl.SetImageList(&m_LargeImageList, LVSIL_NORMAL);
	m_ListCtrl.SetImageList(&m_SmallImageList, LVSIL_SMALL);
	m_ListCtrl.SetImageList(&m_StateImageList, LVSIL_STATE);

	// insert columns
	LV_COLUMN lvc;
	CString strText;

	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

	lvc.iSubItem = 0;
	lvc.pszText = g_ColumnNames[0];
	lvc.cx = 300;
	lvc.fmt = LVCFMT_LEFT;
	m_ListCtrl.InsertColumn(0, &lvc);
	
	lvc.iSubItem = 1;
	lvc.pszText = g_ColumnNames[1];
	lvc.cx = 200;
	lvc.fmt = LVCFMT_LEFT;
	m_ListCtrl.InsertColumn(1, &lvc);

	lvc.iSubItem = 2;
	lvc.pszText = g_ColumnNames[2];
	lvc.cx = 150;
	lvc.fmt = LVCFMT_LEFT;
	m_ListCtrl.InsertColumn(2, &lvc);

}


void CDBCFileInfoView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	if (m_ListCtrl.GetSafeHwnd() != NULL)
	{
		CRect		client_rect;

		this->GetWindowRect(&client_rect);
		this->ScreenToClient(&client_rect);
		m_ClientRect = client_rect;

		m_ListCtrl.SetWindowPos(NULL, client_rect.left, client_rect.top, client_rect.Width(), client_rect.Height(), SWP_SHOWWINDOW);
	}
}


void CDBCFileInfoView::OnViewReportview()
{
	// TODO: Add your command handler code here
	BOOL  bSuccess = FALSE;
	bSuccess = m_ListCtrl.ModifyStyle(LVS_TYPEMASK, LVS_REPORT & LVS_TYPEMASK);
	if (!bSuccess)
	{
		ASSERT(0);
	}
	m_dwViewType = m_ListCtrl.GetStyle()&LVS_TYPEMASK;
	m_ListCtrl.UpdateWindow();
}


void CDBCFileInfoView::OnViewIcon()
{
	// TODO: Add your command handler code here
	BOOL  bSuccess = FALSE;
	bSuccess = m_ListCtrl.ModifyStyle(LVS_TYPEMASK, LVS_ICON & LVS_TYPEMASK);
	if (!bSuccess)
	{
		ASSERT(0);
	}
	m_dwViewType = m_ListCtrl.GetStyle()&LVS_TYPEMASK;
	m_ListCtrl.UpdateWindow();
}


void CDBCFileInfoView::OnUpdateViewReportview(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_dwViewType == LVS_REPORT);
	if (m_dwViewType == LVS_REPORT)
	{
		pCmdUI->Enable(FALSE);
	}
	else
	{
		pCmdUI->Enable(TRUE);
	}
}


void CDBCFileInfoView::OnUpdateViewIcon(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_dwViewType == LVS_ICON);
	if (m_dwViewType == LVS_ICON)
	{
		pCmdUI->Enable(FALSE);
	}
	else
	{
		pCmdUI->Enable(TRUE);
	}
}




void CDBCFileInfoView::OnFileOpen(void)
{
	UINT uRet = m_DBCFileProcess.OpenFile();

	/*file open failure*/
	if (uRet != 0)
	{
		return;
	}

	/*Show Message Items*/
	LV_ITEM lvi;

	CString strText;
	size_t message_num = m_DBCFileProcess.m_mapMessages.size();
	int    i = 0, j = 0;

	m_ListCtrl.DeleteAllItems();

	for (auto iter= m_DBCFileProcess.m_mapMessages.begin(); iter != m_DBCFileProcess.m_mapMessages.end(); iter++)
	{
		ASSERT(iter->first == iter->second.u32MsgID);		

		lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
		lvi.iItem = i;
		lvi.iSubItem = 0;
		lvi.pszText = iter->second.strMsgName.GetBuffer();
		lvi.iImage = 0;
		lvi.stateMask = LVIS_STATEIMAGEMASK;
		lvi.state = LVIS_SELECTED;
		//Message Name
		m_ListCtrl.InsertItem(&lvi);

		//ID
		strText.Format(_T("0x%03X"), iter->first);
		m_ListCtrl.SetItemText(i, 1, strText);
		//DLC
		strText.Format(_T("%u"), iter->second.u8DLC);
		m_ListCtrl.SetItemText(i, 2, strText);

		i++;

	}
	
	m_szSelectedItems = message_num;
	AfxGetMainWnd()->UpdateWindow();

}

void CDBCFileInfoView::GetSeletedMessage()
{
	int i = 0;
	int size = m_ListCtrl.GetItemCount();

	m_SelectedMsgIDs.clear();

	if (size == 0)
	{
		return;
	}
		
	for (i = 0; i < size; i++)
	{
		UINT state = m_ListCtrl.GetItemState(i, LVIS_SELECTED);

		if (state)
		{
			CString strID = m_ListCtrl.GetItemText(i, 1);

			m_SelectedMsgIDs.push_back(_tcstoul(strID, NULL, 16));
		}

	}
}

void CDBCFileInfoView::OnWriteSignal_Clanguage()
{
	// TODO: Add your command handler code here
	if (!m_DBCFileProcess.IsFileOpen())
	{
		MessageBox(_T("Please Open a dbc file at first"), _T("error"));
		return;
	}

	GetSeletedMessage();

	if (!m_SelectedMsgIDs.size())
	{
		MessageBox(_T("Please select the items!"), _T("error"));
		return;
	}


	m_DBCFileProcess.WriteSignal_CLanguage(m_SelectedMsgIDs);

}

void CDBCFileInfoView::OnReadSignal_Clanguage()
{
	// TODO: Add your command handler code here
	if (!m_DBCFileProcess.IsFileOpen())
	{
		MessageBox(_T("Please Open a dbc file at first"), _T("error"));
		return;
	}

	GetSeletedMessage();

	if (!m_SelectedMsgIDs.size())
	{
		MessageBox(_T("Please select the items!"),_T("error"));
		return;
	}


	m_DBCFileProcess.ReadSignal_CLanguage(m_SelectedMsgIDs);

}

void CDBCFileInfoView::OnInitializeSignal_Clanguage()
{
	// TODO: Add your command handler code here
	if (!m_DBCFileProcess.IsFileOpen())
	{
		MessageBox(_T("Please Open a dbc file at first"),_T("error"));
		return;
	}

	GetSeletedMessage();

	if (!m_SelectedMsgIDs.size())
	{
		MessageBox(_T("Please select the items!"), _T("error"));
		return;
	}

	m_DBCFileProcess.InitializeSignal_CLanguage(m_SelectedMsgIDs);

}