#include "stdafx.h"
#include "DBCFileInfoProcess.h"
#include "resource.h"


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

	CInvalidArgException   ex;

	/*Analyze from the beginning of the file*/
	m_File.seekg(0);
	m_mapMessages.clear();
	m_mapSignals.clear();
		

	/*Get One Line and if the operation has no problem, then go on processing*/
	for (m_File.getline(buf, sizeof(buf));  m_File.good(); m_File.getline(buf, sizeof(buf)))
	{
		strCurrentLine = buf;
		int iPos = strCurrentLine.Find(_T("BO_"));

		/* 1. BO_ means the message definition, get the message info*/
		if (iPos == 0)
		{			
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
		}// "BO_" END

		 /* 2. SG_ means the signal definition, get the signal info*/
		iPos = strCurrentLine.Find(_T("SG_"));
		if (iPos == 1)
		{
			int				iPos2 = 0;
			CString			strSignalInfo = strCurrentLine;
			CString         strRestToken2;  

			InitSignalInfo(&SignalInfo);
			
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
						SignalInfo.iMultiplexor = _tcstoul(strRestToken2, NULL, 10);	//The Multiplexed Value to which this signal belongs
						m_mapMessages[MsgInfo.u32MsgID].set_Multiplexor.insert(SignalInfo.iMultiplexor);
					}
					else
					{
						SignalInfo.iMultiplexor = -1;		//-1 means this signal is a multiplexor
						m_mapMessages[MsgInfo.u32MsgID].str_MuxSignalName = SignalInfo.strSignalName;
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
					/*2.2 Signal Start Bit*/
					SignalInfo.u32StartBit = _tcstoul(strRestToken2, NULL, 10);					
				}
				else
				{
					continue;
				}
				strRestToken2 = strRestToken.Tokenize(_T("|"), iPos2);
				if (iPos2 != -1 && strRestToken2 != _T("") && _istdigit(strRestToken2[0]) )
				{
					/*2.3 Signal Length*/
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
			{	/*2.4 Byte Order*/
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
					ex.ReportError(MB_OK, IDS_ENDIAN_TYPE_INCORRECT);
					continue;
				}
				/*2.5 Signal Type*/
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
					ex.ReportError(MB_OK, IDS_VALUE_TYPE_INCORRECT);
					continue;
				}
			}
			else
			{
				continue;
			}

			/*2.6 Calculate the segment locations in the message buffer*/
			SegmentInfo_t	SegmentInfo = {0};
			BOOL            b_Flag_FirstByte = TRUE;
			
			//SignalInfo.mvector_SegmentInfo.clear();
			
			if (SignalInfo.bEndian == 1)	//Intel Order
			{
				int i_RestLen = SignalInfo.u8Length;				
				do
				{
					SegmentInfo.u16_ByteOffset = (SignalInfo.u32StartBit+ (SignalInfo.u8Length - i_RestLen)) / 8;
					if (b_Flag_FirstByte)
					{
						SegmentInfo.u8_BitOffset = SignalInfo.u32StartBit % 8;
						b_Flag_FirstByte = FALSE;
					}
					else
					{
						SegmentInfo.u8_BitOffset = 0;
					}

					SegmentInfo.u8_SegmentLen = i_RestLen<(8 - SegmentInfo.u8_BitOffset)? i_RestLen: (8 - SegmentInfo.u8_BitOffset);

					i_RestLen -= SegmentInfo.u8_SegmentLen;

					SignalInfo.mvector_SegmentInfo.push_back(SegmentInfo);

				} while (i_RestLen>0);
			}
			else if(SignalInfo.bEndian == 0) //Motorola Order
			{

			}

			

#if USE_VECTOR
			m_mapSignals.insert(multimap<UINT32, SignalInfo_t>::value_type(m_vecMessages.back().u32MsgID, SignalInfo));
#else
			m_mapSignals.insert(multimap<UINT32, SignalInfo_t>::value_type(MsgInfo.u32MsgID, SignalInfo));
#endif
		
			continue;
		}// "SG_" END


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
	pSignalInfo->mvector_SegmentInfo.clear();
}

void DBCFileInfoProcess::InitMessageInfo(MessageInfo_t* pMessageInfo)
{
	pMessageInfo->strMsgName.Empty();
	pMessageInfo->u32MsgID = 0;
	pMessageInfo->u8DLC = 0;
	pMessageInfo->set_Multiplexor.clear();
	pMessageInfo->str_MuxSignalName.Empty();
}


void DBCFileInfoProcess::InitializeSignal_CLanguage(vector<UINT32> selected_messages)
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
	strInitFunction = _T("/*****************  The following is the message initialization function**********************/\n");

	for (i = 0; i < msg_number; i++)
	{
		int    j = 0;
		size_t signal_number = m_mapSignals.count(selected_messages[i]);  //how many signals in this message

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
			{	/*Search the GenSigStartValue keyword to get initial value for this signal*/
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
		e.ReportError();		
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
				strErr.Format(_T("Sorry, but the software has no ability to process signals that is larger than 64 bits.\nSignalName = %s\nbitlength = %d"), signal_info.strSignalName, signal_info.u8Length);
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
				strErr.Format(_T("Sorry, but the software has no ability to process signals that is larger than 64 bits.\nSignalName = %s\nbitlength = %d"), signal_info.strSignalName, signal_info.u8Length);
				MessageBox(NULL, strErr, _T("error"), MB_OK);
			}
		}

	}

	return strSignalRawType;
	
}



void DBCFileInfoProcess::ReadSignal_CLanguage(vector<UINT32> selected_messages)
{
	int				i = 0;
	CString         str_ReadSignal = _T(""), str_ReadSegment = _T(""), str_Comment = _T(""), str_Function = _T("");
	CString         str_AllSignals = _T("");  
	CString         *pstr_MultiSignals = NULL;
	
	size_t			u_MsgNumber = selected_messages.size();

	str_Function = CONST_STRING_READ_FUNCTION_NAME;
	for (i = 0; i < u_MsgNumber; i++)
	{
		int		j = 0, k=0, n=0, a = 0;;
		uint8_t	u8_Mask = 0;  //the bit mask
		uint8_t u8_Left_Offset = 0;  //the location offset counted from bit0 of the signal, 'Left' means the buffer value should be shifted left
		size_t	u_SignalNumber = m_mapSignals.count(selected_messages[i]);  //how many signals in this message
		size_t  u_MultiplexorNumber = m_mapMessages[selected_messages[i]].set_Multiplexor.size();

		ASSERT(u_SignalNumber > 0);	
		
		
		str_Comment.Format(_T("\t/**********************%s******************************/\n"), m_mapMessages[selected_messages[i]].strMsgName);		
		str_AllSignals += str_Comment;

		/*¡øprocess for each message that has multiplexing signals */
		if (u_MultiplexorNumber>0)
		{
			ASSERT(u_MultiplexorNumber > 1);
			if (pstr_MultiSignals != NULL)
			{
				delete [] pstr_MultiSignals;
			}
			pstr_MultiSignals = new CString[u_MultiplexorNumber];  
			for (a = 0; a < u_MultiplexorNumber; a++)
			{
				pstr_MultiSignals[a].Empty();
			}
		}
		
		/*start to read signals */
		for (auto it = m_mapSignals.find(selected_messages[i]); j < u_SignalNumber && it != m_mapSignals.end(); it++, j++)
		{
			SegmentInfo_t				SegmentInfo = {0};
			vector<SegmentInfo_t>		vector_SegmentInfo = it->second.mvector_SegmentInfo;
			
			size_t		u_Size = vector_SegmentInfo.size();
			CString     str_SignalRawType = GetSignalRawType(it->second);

#ifdef _DEBUG
			if (it->first == 0xC8)
			{
				TRACE("This is a view point for reading signals from buffer");
			}
#endif
			/*start reading this signal from message buffer*/
			if (it->second.bIsMulti && it->second.iMultiplexor != -1)
			{
				str_ReadSignal.Format(_T("\t\t%s.%s = "), m_mapMessages[selected_messages[i]].strMsgName, it->second.strSignalName);
			}
			else
			{
				str_ReadSignal.Format(_T("\t%s.%s = "), m_mapMessages[selected_messages[i]].strMsgName, it->second.strSignalName);
			}
			

			if (it->second.bEndian == 1)  //Intel Order
			{
				u8_Left_Offset = 0;
				
				for (k = 0; k < u_Size; k++)
				{
					/*calculate the bit mask*/
					u8_Mask = 0;
					for (n = 0; n < vector_SegmentInfo[k].u8_SegmentLen; n++)
					{
						u8_Mask |= (1 << n);
					}
					u8_Mask = u8_Mask << vector_SegmentInfo[k].u8_BitOffset;
					/*figure out one segment of the signal*/
					if (k == 0)
					{
						if (str_SignalRawType == _T("BOOL"))
						{
							str_ReadSegment.Format(_T("(%s_buf[%d]&0x%02x)>>%d"), m_mapMessages[selected_messages[i]].strMsgName,
								vector_SegmentInfo[k].u16_ByteOffset, u8_Mask, vector_SegmentInfo[k].u8_BitOffset);
						}
						else
						{
							str_ReadSegment.Format(_T("((%s)(%s_buf[%d]&0x%02x)>>%d)"), str_SignalRawType, m_mapMessages[selected_messages[i]].strMsgName,
								vector_SegmentInfo[k].u16_ByteOffset, u8_Mask, vector_SegmentInfo[k].u8_BitOffset);
							u8_Left_Offset += vector_SegmentInfo[k].u8_SegmentLen;
						}
					}
					else
					{
						ASSERT(str_SignalRawType != _T("BOOL"));
						str_ReadSegment.Format(_T(" | ((%s)(%s_buf[%d]&0x%02x)<<%d)"), str_SignalRawType, m_mapMessages[selected_messages[i]].strMsgName, 
							vector_SegmentInfo[k].u16_ByteOffset,u8_Mask, u8_Left_Offset);
						u8_Left_Offset += vector_SegmentInfo[k].u8_SegmentLen;
					}
					/*combine the segments together*/
					str_ReadSignal += str_ReadSegment;  
				}
				
			}
			else if (it->second.bEndian == 0)	//Motorola Order
			{

			}
			str_ReadSignal += _T(";");

			/*¡øprocess for each message that has multiplexing signals */
			if (it->second.bIsMulti && it->second.iMultiplexor!=-1)
			{							
				a = 0;
				set<INT16>::iterator  it_Multiplexor = m_mapMessages[selected_messages[i]].set_Multiplexor.begin();
				for (;it_Multiplexor != m_mapMessages[selected_messages[i]].set_Multiplexor.end(); it_Multiplexor++)
				{
					if (*it_Multiplexor == it->second.iMultiplexor)
					{
						break;
					}
					a++;					
				}
				ASSERT(a < u_MultiplexorNumber);
				if (pstr_MultiSignals[a].IsEmpty())
				{
					pstr_MultiSignals[a].Format(_T("\tif( %s.%s == %d )\n\t{\n"), m_mapMessages[selected_messages[i]].strMsgName, 
						m_mapMessages[selected_messages[i]].str_MuxSignalName, *it_Multiplexor);
				}
				pstr_MultiSignals[a] += str_ReadSignal + _T("\n");				
			}
			else
			{
				str_AllSignals += str_ReadSignal + _T("\n");
			}
		}
		if (u_MultiplexorNumber > 0)
		{   /* we cache the multiplexing signals in pstr_MultiSignals and 
			do not combine these signal to str_AllSignals untill we process all signals in this message*/
			for (a = 0; a < u_MultiplexorNumber; a++)
			{
				pstr_MultiSignals[a] += _T("\t}\n");
				str_AllSignals += pstr_MultiSignals[a];
			}
		}

		str_AllSignals += _T("\n");
	}
	str_Function += str_AllSignals + _T('}');


	CFile f;
	CFileException e;
	
	size_t u_StringSize = 0;

	ASSERT(!m_strFileName.IsEmpty());
	CString strFileName = m_strFilePathName;

	int iPos = strFileName.Replace(_T(".dbc"), _T("(read).c"));
	ASSERT(iPos == 1);

	if (!f.Open(strFileName, CFile::modeCreate | CFile::modeWrite, &e))
	{
		e.ReportError();
	}

	/*Output codes to construct the signals */
#ifdef _UNICODE
	u_StringSize = str_Function.GetLength() * 2;
#else
	size = str_Function.GetLength();
#endif	
	f.Write(str_Function.GetString(), u_StringSize);

	f.Flush();
	f.Close();

	if (pstr_MultiSignals != NULL)
	{
		delete[] pstr_MultiSignals;
		pstr_MultiSignals = NULL;
	}
}


void DBCFileInfoProcess::WriteSignal_CLanguage(vector<UINT32> selected_messages)
{
	int				i = 0;
	CString         str_WriteSignal = _T(""), str_WriteSegment = _T(""), str_Comment = _T(""), str_Function = _T("");
	CString         str_AllSignals = _T("");
	CString         *pstr_MultiSignals = NULL;
	BOOL			*pb_Flag_FirstMultiSegment = NULL;

	size_t			u_MsgNumber = selected_messages.size();

	str_Function = CONST_STRING_WRITE_FUNCTION_NAME;
	for (i = 0; i < u_MsgNumber; i++)
	{
		int		j = 0, k = 0, n = 0, m=0, DLC=0, a = 0, b = -1;
		int     segment_count = 0;
		uint8_t	u8_Mask = 0;  //the bit mask
		uint8_t u8_Right_Offset = 0;	//the location offset counted from bit0 of the signal, 'Right' means the signal value should be shifted right
		uint8_t u8_Left_Offset = 0;
		size_t	u_SignalNumber = m_mapSignals.count(selected_messages[i]);  //how many signals in this message
		size_t  u_MultiplexorNumber = m_mapMessages[selected_messages[i]].set_Multiplexor.size();
		auto    it_Multiplexor = m_mapMessages[selected_messages[i]].set_Multiplexor.begin();
		BOOL    b_Flag_HasMulti = FALSE;  // message_buf[m] contains multiplexing signal
		

		ASSERT(u_SignalNumber > 0);		

		/*¡øprocess for each message that has multiplexing signals */
		if (u_MultiplexorNumber>0)
		{
			ASSERT(u_MultiplexorNumber > 1);
			if (pstr_MultiSignals != NULL)
			{
				delete[] pstr_MultiSignals;
			}
			if (pb_Flag_FirstMultiSegment != NULL)
			{
				delete[] pb_Flag_FirstMultiSegment;
			}

			/*create a string buffer for each multiplexing group*/
			pstr_MultiSignals = new CString[u_MultiplexorNumber];
			for (a=0; it_Multiplexor != m_mapMessages[selected_messages[i]].set_Multiplexor.end(); it_Multiplexor++,a++)
			{
				pstr_MultiSignals[a].Format(_T("\tif( %s.%s == %d )\n\t{\n"), m_mapMessages[selected_messages[i]].strMsgName,
					m_mapMessages[selected_messages[i]].str_MuxSignalName, *it_Multiplexor);
			}

			pb_Flag_FirstMultiSegment = new BOOL[u_MultiplexorNumber];
			for (a = 0; a < u_MultiplexorNumber; a++)
			{
				pb_Flag_FirstMultiSegment[a] = TRUE;
			}
		}

		str_Comment.Format(_T("\t/**********************%s******************************/\n"), m_mapMessages[selected_messages[i]].strMsgName);
		str_AllSignals += str_Comment;

		DLC = m_mapMessages[selected_messages[i]].u8DLC;
		for (m = 0; m < DLC; m++)  
		{			
			b_Flag_HasMulti = FALSE;
			/*Construct message_buf[m]*/
			str_WriteSignal.Format(_T("\t%s_buf[%d] = "), m_mapMessages[selected_messages[i]].strMsgName, m);

			/*find out a signal that locates in message_buf[m]*/
			j = 0;
			segment_count = 0;
			for (auto it = m_mapSignals.find(selected_messages[i]); j < u_SignalNumber && it != m_mapSignals.end(); it++, j++)
			{
				vector<SegmentInfo_t>		vector_SegmentInfo = it->second.mvector_SegmentInfo;

				size_t		u_Size = vector_SegmentInfo.size();
				CString     str_SignalRawType = GetSignalRawType(it->second);


				u8_Right_Offset = 0;

#ifdef _DEBUG
				if (it->first == 0xC8)
				{
					TRACE("This is a view point for write signals into a buffer");
				}
#endif
				for (k = 0; k < u_Size; k++)
				{
					if (k > 0)
					{
						/*calculate the start-bit from which this signal locates to message_buf[m]*/
						if (it->second.bEndian == 1) //Intel Order
						{
							u8_Right_Offset += vector_SegmentInfo[k - 1].u8_SegmentLen;
						}
						else if (it->second.bEndian == 0)	//Motorola Order
						{

						}
					}
					if (vector_SegmentInfo[k].u16_ByteOffset == m)
					{	/*this signal has one segment located in message_buf[m]*/
						break;
					}
				}
				if (k == u_Size)
				{	/*this signal has no segment which locates in message_buf[m]*/
					continue;
				}

				/*construct a segment of the buffer*/
				if (it->second.bEndian == 1)  //Intel Order
				{
					/*calculate the bit mask*/
					u8_Mask = 0;
					for (n = 0; n < vector_SegmentInfo[k].u8_SegmentLen; n++)
					{
						u8_Mask |= (1 << n);
					}
					u8_Mask = u8_Mask << vector_SegmentInfo[k].u8_BitOffset;
						
					if (segment_count == 0)
					{
						if (str_SignalRawType == _T("BOOL"))
						{
							str_WriteSegment.Format(_T("(((uint8_t)%s.%s<<%d)&0x%02x)"), m_mapMessages[selected_messages[i]].strMsgName, it->second.strSignalName,
								vector_SegmentInfo[k].u8_BitOffset, u8_Mask);
						}
						else
						{
							str_WriteSegment.Format(_T("(((uint8_t)(%s.%s>>%d)<<%d)&0x%02x)"), m_mapMessages[selected_messages[i]].strMsgName,
								it->second.strSignalName, u8_Right_Offset, vector_SegmentInfo[k].u8_BitOffset, u8_Mask);
						}
					}
					else
					{
						if (str_SignalRawType == _T("BOOL"))
						{
							str_WriteSegment.Format(_T(" | (((uint8_t)%s.%s<<%d)&0x%02x)"), m_mapMessages[selected_messages[i]].strMsgName, it->second.strSignalName,
								vector_SegmentInfo[k].u8_BitOffset, u8_Mask);
						}
						else
						{
							str_WriteSegment.Format(_T(" | (((uint8_t)(%s.%s>>%d)<<%d)&0x%02x)"), m_mapMessages[selected_messages[i]].strMsgName,
								it->second.strSignalName, u8_Right_Offset, vector_SegmentInfo[k].u8_BitOffset, u8_Mask);
						}

					}

				}
				else if (it->second.bEndian == 0)	//Motorola Order
				{

				}
					
				/*combine segment together*/
				/*a.if it is a multiplexing signal, cache it into proper group*/
				if ( (it->second.bIsMulti && it->second.iMultiplexor != -1) || b_Flag_HasMulti)
				{						
					a = 0;
					it_Multiplexor = m_mapMessages[selected_messages[i]].set_Multiplexor.begin();
					for (; it_Multiplexor != m_mapMessages[selected_messages[i]].set_Multiplexor.end(); it_Multiplexor++)
					{
						if (*it_Multiplexor == it->second.iMultiplexor)
						{
							break;
						}
						a++;
					}
					ASSERT(a < u_MultiplexorNumber);
					if (pb_Flag_FirstMultiSegment[a] == TRUE  )
					{					
						pstr_MultiSignals[a] += _T("\t") + str_WriteSignal + str_WriteSegment;
						pb_Flag_FirstMultiSegment[a] = FALSE;
						
					}
					else
					{
						pstr_MultiSignals[a] += str_WriteSegment;
					}					

					b_Flag_HasMulti = TRUE;
				}
				else
				{
					/*b. it is not a multiplexing signal, combine segments directly*/
					str_WriteSignal += str_WriteSegment;
				}
				segment_count++;

			}
			/*reset flag */
			for (a = 0; a < u_MultiplexorNumber; a++)
			{
				pb_Flag_FirstMultiSegment[a] = TRUE;
			}

			if (segment_count == 0)
			{
				str_WriteSignal += _T("0;");
				str_AllSignals += str_WriteSignal + _T("\n");
			}
			else
			{
				if (b_Flag_HasMulti)
				{	/*if message_buf[m] contains one or more multiplexing signal, don't add 
					message_buf[m] to str_AllSignals untill all multiplexing signals have been found*/
					for (a = 0; a < u_MultiplexorNumber; a++)
					{
						pstr_MultiSignals[a] += _T(";\n");						
					}
				}
				else
				{
					str_WriteSignal += _T(";");
					str_AllSignals += str_WriteSignal + _T("\n");
				}
				
			}				
		
		}
		if (u_MultiplexorNumber > 0)
		{
			for (a = 0; a < u_MultiplexorNumber; a++)
			{
				pstr_MultiSignals[a] += _T("\t}");
				str_AllSignals += pstr_MultiSignals[a] +_T("\n");
			}
		}

		str_AllSignals += _T("\n");
	}
	str_Function += str_AllSignals + _T('}');


	CFile f;
	CFileException e;

	size_t u_StringSize = 0;

	ASSERT(!m_strFileName.IsEmpty());
	CString strFileName = m_strFilePathName;

	int iPos = strFileName.Replace(_T(".dbc"), _T("(write).c"));
	ASSERT(iPos == 1);

	if (!f.Open(strFileName, CFile::modeCreate | CFile::modeWrite, &e))
	{
		e.ReportError();
	}

	/*Output codes to construct the signals */
#ifdef _UNICODE
	u_StringSize = str_Function.GetLength() * 2;
#else
	size = str_Function.GetLength();
#endif	
	f.Write(str_Function.GetString(), u_StringSize);

	f.Flush();
	f.Close();

	if (pstr_MultiSignals != NULL)
	{
		delete[] pstr_MultiSignals;
		pstr_MultiSignals = NULL;
	}
	if (pb_Flag_FirstMultiSegment != NULL)
	{
		delete[] pb_Flag_FirstMultiSegment;
		pb_Flag_FirstMultiSegment = NULL;
	}
}