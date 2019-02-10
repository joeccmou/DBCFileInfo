#include "stdafx.h"
#include "DBCFileInfoProcess.h"



TCHAR* g_ColumnNames[3] = {_T("NAME") , _T("ID"),_T("DLC") };





DBCFileInfoProcess::DBCFileInfoProcess()
{


}


DBCFileInfoProcess::~DBCFileInfoProcess()
{


}

bool DBCFileInfoProcess::IsFileOpen()
{
	return m_File.is_open();
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

	if (m_File.is_open())
	{
		m_File.close();
	}

	m_File.open(strFilePathName, ifstream::in);
	if (!m_File.is_open())
	{
		return FILE_ACCESS_FAILURE_NOT_OPEN;
	}

	AnalyzeDBCFileInfo();

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
	m_File.seekg(0);
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
						SignalInfo.iMultiplexor = _tcstoul(strRestToken2, NULL, 10);	//What Multiplexed Value this signal belongs to
					}
					else
					{
						SignalInfo.iMultiplexor = -1;		//This signal gives the infomation for multiplexing
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
	pSignalInfo->iMultiplexor = 0;
	pSignalInfo->u32StartBit = 0;
	pSignalInfo->u8Length = 0;
	pSignalInfo->strSignalName.Empty();

	pSignalInfo->u64RawInitValue = 0;
}

void DBCFileInfoProcess::InitMessageInfo(MessageInfo_t* pMessageInfo)
{
	pMessageInfo->strMsgName.Empty();
	pMessageInfo->u32MsgID = 0;
	pMessageInfo->u8DLC = 0;
}


void DBCFileInfoProcess::OutputInitializationCode_CLanguage(vector<UINT32> selected_messages)
{
	int				i = 0, iPosFile = 0, iPosString = 0;
	char			buf[2048] = { 0 };
	CString         strCurLine = _T(""), strGetSigStartValue = _T(""), strRawInitValue = _T("");
	CString         strMsgDef = _T(""), strSignalRawType = _T(""), strMsgName=_T("");
	CString         strMessageVar = _T(""), strEquation = _T(""), strSignalName = _T(""), strInitFunction = _T("");
	UINT64          u64RawInitValue = 0;
	
	size_t			msg_number = selected_messages.size();

	/*Get the position of starting point of the <BA_ XXX>*/
	m_File.clear();
	m_File.seekg(0);

	do
	{
		int iTempPosFile = m_File.tellg();
		m_File.getline(buf, 2048);
		strCurLine = buf;
		iPosString = strCurLine.Find(_T("BA_ "));
		if (iPosString == 0)
		{		
			iPosFile = iTempPosFile;
			break;
		}

	} while (!m_File.eof() && !m_File.fail());

	ASSERT(!m_File.eof() && !m_File.fail());
	

	strMsgDef = _T("/*****************  The following is the message structure **********************/\n");
	strMessageVar = _T("/*****************  The following is the message variable **********************/\n");
	strInitFunction = _T("/*****************  The following is the message initialization definition**********************/\n");

	for (i = 0; i < msg_number; i++)
	{
		int    j = 0;
		size_t signal_number = m_mapSignals.count(selected_messages[i]);

		ASSERT(signal_number>0);		

		strMsgDef.AppendFormat(CONST_STRING_TYPEDEF_STRUCT);

		for (auto it = m_mapSignals.find(selected_messages[i]); j<signal_number && it != m_mapSignals.end() ; it++,j++)
		{
			
			m_File.clear();
			m_File.seekg(iPosFile);
			
			/*Define the message structure*/
			strSignalRawType = GetSignalRawType(it->second);
			strMsgDef.AppendFormat(_T("%s	%s;\n"), strSignalRawType.GetString(), it->second.strSignalName.GetString());
			
			do
			{	/*Search the GenSigStartValue keyword to get initial value for selected messages*/
				m_File.getline(buf, 2048);
				strCurLine = buf;

				if (strCurLine.Find(_T("BA_ ")) != 0)
				{	
					break;
				}
				
				strGetSigStartValue.Format(_T("BA_ \"GenSigStartValue\" SG_ %d %s "), it->first, it->second.strSignalName.GetString());
				iPosString = strCurLine.Find(strGetSigStartValue);
				if (iPosString == 0)
				{	/*This Signal has User-defined Initial Value*/
					strRawInitValue = strCurLine.Mid(strGetSigStartValue.GetLength());
					strRawInitValue.TrimRight(_T(';'));
					u64RawInitValue = _tcstoui64(strRawInitValue, NULL, 10);
					it->second.u64RawInitValue = u64RawInitValue;
					break;
				}

			} while (!m_File.eof() && !m_File.fail());

		}
		strMsgName = m_mapMessages[selected_messages[i]].strMsgName;
		strMsgDef.AppendFormat(_T("}%s_t;\n\n"), strMsgName.GetString());

		/*Define message variables*/
		strMessageVar.AppendFormat(_T("%s_t\t%s;\n"), strMsgName, strMsgName);
	}
	strMessageVar += _T("\n");
	
	
	strInitFunction += CONST_STRING_INITIAL_FUNCTION_NAME;
	/*Output the initial value*/
	for (i = 0; i < msg_number; i++)
	{
		int    j = 0;
		size_t signal_number = m_mapSignals.count(selected_messages[i]);

		ASSERT(signal_number>0);

		/*1. Get message name*/
		strMsgName = m_mapMessages[selected_messages[i]].strMsgName;
		strInitFunction.AppendFormat(_T("\t/**********%s************/\n"), strMsgName);
		for (auto it = m_mapSignals.find(selected_messages[i]); j<signal_number && it != m_mapSignals.end(); it++, j++)
		{
			/*2. Get signal name and signal initial value*/
			strEquation.Format(_T("\t%s.%s=0x%x;\n"), strMsgName, it->second.strSignalName, it->second.u64RawInitValue);
			strInitFunction += strEquation;
		}	
		strInitFunction += _T("\n");
	}
	strInitFunction.AppendFormat(_T("}\n"));


	
	CFile f;
	CFileException e;

	ASSERT(!m_strFileName.IsEmpty());
	CString strFileName = m_strFilePathName;

	int iPos = strFileName.Replace(_T(".dbc"),_T("(init).c"));
	ASSERT(iPos == 1);

	if (!f.Open(strFileName, CFile::modeCreate | CFile::modeWrite, &e))
	{
		TRACE(_T("File could not be opened %d\n"), e.m_cause);
	}

	/*Output message structure definition*/
#ifdef _UNICODE
	size_t size = strMsgDef.GetLength()*2;
#else
	size_t size = strMsgDef.GetLength();
#endif
	f.Write(strMsgDef.GetString(), size);

	/*Output message variable definition*/
#ifdef _UNICODE
	size = strMessageVar.GetLength() * 2;
#else
	size = strMessageVar.GetLength();
#endif
	f.Write(strMessageVar.GetString(), size);

	/*Output message initialization function definition*/
#ifdef _UNICODE
	size = strInitFunction.GetLength() * 2;
#else
	size = strInitFunction.GetLength();
#endif	
	f.Write(strInitFunction.GetString(), size);
	
	f.Flush();
	f.Close();

}

CString DBCFileInfoProcess::GetSignalRawType(SignalInfo_t signal_info)
{
	CString strSignalRawType = _T("");

	if (signal_info.u8Length == 1)
	{
		strSignalRawType = _T("BOOL");
	}
	else if (1 < signal_info.u8Length )
	{
		if (signal_info.bIsSigned)
		{
			if (signal_info.u8Length <=8 )
			{
				strSignalRawType = _T("int8_t");
			}
			else if (signal_info.u8Length <= 16)
			{
				strSignalRawType = _T("int16_t");
			}
			else if (signal_info.u8Length <= 32)
			{
				strSignalRawType = _T("int32_t");
			}
			else if (signal_info.u8Length <= 64)
			{
				strSignalRawType = _T("int64_t");
			}
			else
			{
				CString   strErr;
				strErr.Format(_T("Sorry, but it has no ability to process signals that is larger than 64 bits.\nSignalName = %s\nbitlength = %d"), signal_info.strSignalName, signal_info.u8Length);
				MessageBox(NULL,strErr,_T("error"), MB_OK);
			}
		}
		else
		{
			if (signal_info.u8Length <= 8)
			{
				strSignalRawType = _T("uint8_t");
			}
			else if (signal_info.u8Length <= 16)
			{
				strSignalRawType = _T("uint16_t");
			}
			else if (signal_info.u8Length <= 32)
			{
				strSignalRawType = _T("uint32_t");
			}
			else if (signal_info.u8Length <= 64)
			{
				strSignalRawType = _T("uint64_t");
			}
			else
			{
				CString   strErr;
				strErr.Format(_T("Sorry, but it has no ability to process signals that is larger than 64 bits.\nSignalName = %s\nbitlength = %d"), signal_info.strSignalName, signal_info.u8Length);
				MessageBox(NULL, strErr, _T("error"), MB_OK);
			}
		}

	}

	return strSignalRawType;
	
}