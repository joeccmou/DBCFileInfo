#pragma once

#include <fstream>
#include <map>
#include <vector>

using namespace std;

#define USE_VECTOR 0

#define CONST_STRING_TYPEDEF_STRUCT		_T("typedef struct {\n") 

#define CONST_STRING_INITIAL_FUNCTION_NAME   _T("void InitializeMessages(void)\n{\n")

enum enum_FILE_ACCESS_FAILURE_t
{
	FILE_ACCESS_FAILURE_NOT_OPEN	=	1,
	FILE_ACCESS_FAILURE_USER_CANCEL	=	2
};



typedef struct {

	UINT32			u32MsgID;
	CString         strMsgName;
	UINT8           u8DLC;

}MessageInfo_t;


typedef struct {

	CString         strSignalName;
	BOOL 			bIsMulti;
	INT8            iMultiplexor;	//Only valid when bIsMulti=TRUE	
	UINT32          u32StartBit;
	UINT8           u8Length;
	BOOL            bEndian;
	BOOL            bIsSigned;

	float           fScale;
	float           fOffset;
	float           fMinValue;
	float           fMaxValue;

	UINT64          u64RawInitValue;


}SignalInfo_t;


extern TCHAR* g_ColumnNames[3];



class DBCFileInfoProcess
{
public:
	ifstream     m_File;
	CString      m_strFileName;
	CString      m_strFilePathName;

#if USE_VECTOR
	vector<MessageInfo_t>				m_vecMessages;
#else
	map<UINT32, MessageInfo_t>			m_mapMessages;
#endif
	multimap<UINT32, SignalInfo_t>		m_mapSignals;

public:
	DBCFileInfoProcess();
	virtual ~DBCFileInfoProcess();

	UINT  OpenFile();
	bool  IsFileOpen();
	void  AnalyzeDBCFileInfo();
	
	CString GetSignalRawType(SignalInfo_t signal_info);

	void	InitSignalInfo(SignalInfo_t* pSignalInfo);
	void	InitMessageInfo(MessageInfo_t* pMessageInfo);


	void	OutputInitializationCode_CLanguage(vector<UINT32> SelectedMessages);

};

