#include "stdafx.h"
#include "DBCFileInfoProcess.h"



TCHAR* g_ColumnNames[3] = {_T("NAME") , _T("ID"),_T("DLC") };





DBCFileInfoProcess::DBCFileInfoProcess()
{


}


DBCFileInfoProcess::~DBCFileInfoProcess()
{


}


UINT   DBCFileInfoProcess::OpenFile()
{
	CFileDialog*  p_FileDlg = new CFileDialog(TRUE, NULL, 0, OFN_FILEMUSTEXIST | OFN_ENABLESIZING, _T("CAN Network Database(*.dbc)|*.dbc|"), NULL);

	if (p_FileDlg->DoModal() != IDOK)
	{
		return FILE_ACCESS_FAILURE_USER_CANCEL;
	}

	CString strFileName = p_FileDlg->GetFileName();
	CString strExtName = p_FileDlg->GetFileExt();
	CString strFolderPath = p_FileDlg->GetFolderPath();
	CString strFilePathName = p_FileDlg->GetPathName();

	m_strFileName = strFileName;
	m_strFilePathName = strFilePathName;

	m_File.open(strFilePathName, ifstream::in);
	if (!m_File.is_open())
	{
		return FILE_ACCESS_FAILURE_NOT_OPEN;
	}

	AnalyzeDBCFileInfo();

	m_File.close();

	return 0;

}


void  DBCFileInfoProcess::AnalyzeDBCFileInfo()
{
	char			buf[2048] = {0};
	CString			strCurrentLine;
	CString			strRestToken;

	MessageInfo_t	MsgInfo = {0};
	SignalInfo_t	SignalInfo = {0};

	/*Analyze from the beginning of the file*/
	m_File.seekg(ifstream::beg);
	m_mapMessages.clear();
	m_mapSignals.clear();
		

	/*Get One Line and if the operation has no problem, then go on processing*/
	for (m_File.getline(buf, sizeof(buf));  m_File.good(); m_File.getline(buf, sizeof(buf)))
	{
		strCurrentLine = buf;
		int iPos = strCurrentLine.Find(_T("BO_"));

		if (iPos == 0)
		{
			/* 1. BO_ means the message definition, get the message info*/
			CString strMessageInfo = strCurrentLine;

			InitMessageInfo(&MsgInfo);
							
			strRestToken = strMessageInfo.Tokenize(_T(" "), iPos);
			if (strRestToken != _T(""))
			{
				/*First token contains nothing useful, do nothing*/
			}

			strRestToken = strMessageInfo.Tokenize(_T(" "), iPos);
			if (iPos != -1 && strRestToken != _T("") && _istdigit(strRestToken[0]))
			{
				/*1.1 Message ID*/
				MsgInfo.u32MsgID = _tcstoul(strRestToken, NULL, 10);				
			}
			else
			{	/*Information is not enough, go on to the next line*/
				continue;
			}
			

			strRestToken = strMessageInfo.Tokenize(_T(" "), iPos);
			if (iPos != -1 && strRestToken != _T(""))
			{
				/*1.2 Message Name*/
				MsgInfo.strMsgName = strRestToken.Trim(_T(": "));
			}
			else
			{
				continue;
			}

			strRestToken = strMessageInfo.Tokenize(_T(" "), iPos);
			if (iPos != -1 && strRestToken != _T("") && _istdigit(strRestToken[0]))
			{
				/*1.3 Message Data Length Code*/
				MsgInfo.u8DLC = _tcstoul(strRestToken, NULL, 10);
			}
			else
			{
				continue;
			}
#if USE_VECTOR
			m_vecMessages.push_back(MsgInfo);
#else
			m_mapMessages[MsgInfo.u32MsgID] = MsgInfo;
#endif

			/*Get next line in the DBC file*/
			continue;
		}

		
		iPos = strCurrentLine.Find(_T("SG_"));
		if (iPos == 1)
		{
			int				iPos2 = 0;
			CString			strSignalInfo = strCurrentLine;
			CString         strRestToken2;  

			InitSignalInfo(&SignalInfo);

			/* 2. SG_ means the signal definition, get the signal info*/
			strRestToken = strSignalInfo.Tokenize(_T(" "), iPos);
			if (strRestToken != _T(""))
			{
				/*First token contains nothing useful, do nothing*/
			}

			strRestToken = strSignalInfo.Tokenize(_T(":"), iPos);
			strRestToken.Trim();
			if (iPos != -1 && strRestToken != _T("") )
			{
				/*2.1 Signal Name*/
				iPos2 = 0;
				strRestToken2 = strRestToken.Tokenize(_T(" "), iPos2);
				if (iPos2 != -1 && strRestToken2 != _T(""))
				{
					SignalInfo.strSignalName = strRestToken2;
				}
				else
				{
					continue;
				}

				/*(2.2) Multiplexed Message Flag*/
				strRestToken2 = strRestToken.Tokenize(_T(" "), iPos2);
				if (iPos2 != -1 && strRestToken2 != _T(""))
				{
					SignalInfo.bIsMulti = TRUE;
					strRestToken2 = strRestToken2.Mid(1);
					if ( !strRestToken2.IsEmpty() && _istdigit(strRestToken2[0]) )
					{
						SignalInfo.iMultiValue = _tcstoul(strRestToken2, NULL, 10);	//What Multiplexed Value this signal belongs to
					}
					else
					{
						SignalInfo.iMultiValue = -1;		//This signal is the Multiplexed Message Flag signal
					}
				}

			}
			else
			{	/*Information is not enough, go on to the next line*/
				continue;
			}


			strRestToken = strSignalInfo.Tokenize(_T("@"), iPos);
			strRestToken.Trim();
			if (iPos != -1 && strRestToken != _T(""))
			{				
				iPos2 = 0;
				strRestToken2 = strRestToken.Tokenize(_T("|"), iPos2);				
				if (iPos2 != -1 && strRestToken2 != _T("") && _istdigit(strRestToken2[0]) )
				{
					/*2.2 Signal Start Bit and Signal Length*/
					SignalInfo.u32StartBit = _tcstoul(strRestToken2, NULL, 10);					
				}
				else
				{
					continue;
				}
				strRestToken2 = strRestToken.Tokenize(_T("|"), iPos2);
				if (iPos2 != -1 && strRestToken2 != _T("") && _istdigit(strRestToken2[0]) )
				{
					/*2.3 Signal Start Bit and Signal Length*/
					SignalInfo.u8Length = _tcstoul(strRestToken2, NULL, 10);
				}
				else
				{
					continue;
				}
					
			}
			else
			{
				continue;
			}

			strRestToken = strSignalInfo.Tokenize(_T(" "), iPos);
			if (iPos != -1 && strRestToken != _T(""))
			{
				if (strRestToken[0] == '0')	//motorola order
				{
					SignalInfo.bEndian = 0;
				}
				else if (strRestToken[0] == '1')		//intel order
				{
					SignalInfo.bEndian = 1;
				}
				else
				{
					continue;
				}

				if (strRestToken[1] == '+')	//unsigned number
				{
					SignalInfo.bIsSigned = FALSE;
				}
				else if (strRestToken[1] == '-')		//signed number
				{
					SignalInfo.bIsSigned = TRUE;
				}
				else
				{
					continue;
				}
			}
			else
			{
				continue;
			}
#if USE_VECTOR
			m_mapSignals.insert(multimap<UINT32, SignalInfo_t>::value_type(m_vecMessages.back().u32MsgID, SignalInfo));
#else
			m_mapSignals.insert(multimap<UINT32, SignalInfo_t>::value_type(MsgInfo.u32MsgID, SignalInfo));
#endif

			continue;
		}


	}

#if USE_VECTOR
	//ASSERT(!m_File.fail() && !m_File.bad());
	BOOL bFlag = 0;
	for (int i = 0; i < m_vecMessages.size()-1; i++)
	{
		for (int j = i + 1; j < m_vecMessages.size(); j++)
		{
			if (m_vecMessages[i].u32MsgID == m_vecMessages[j].u32MsgID)
			{
				bFlag = 1;
				break;
			}
		}
		if (bFlag == 1)	break;
	}
#endif

	return;
}


void DBCFileInfoProcess::InitSignalInfo(SignalInfo_t* pSignalInfo)
{
	pSignalInfo->bEndian = 0;
	pSignalInfo->bIsMulti = 0;
	pSignalInfo->bIsSigned = 0;
	pSignalInfo->fMaxValue = 0;
	pSignalInfo->fMinValue = 0;
	pSignalInfo->fOffset = 0;
	pSignalInfo->fScale = 0;
	pSignalInfo->iMultiValue = 0;
	pSignalInfo->u32StartBit = 0;
	pSignalInfo->u8Length = 0;
	pSignalInfo->strSignalName.Empty();
}

void DBCFileInfoProcess::InitMessageInfo(MessageInfo_t* pMessageInfo)
{
	pMessageInfo->strMsgName.Empty();
	pMessageInfo->u32MsgID = 0;
	pMessageInfo->u8DLC = 0;
}