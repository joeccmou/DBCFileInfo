#pragma once

#include <fstream>
#include <map>
#include <vector>
#include <set>

using namespace std;

#define USE_VECTOR 0

#define CONST_STRING_TYPEDEF_STRUCT		_T("typedef struct {\n") 

#define CONST_STRING_INITIAL_FUNCTION_NAME   _T("void InitializeMessages(void)\n{\n")
#define CONST_STRING_READ_FUNCTION_NAME   _T("void ReadMessages(void)\n{\n")
#define CONST_STRING_WRITE_FUNCTION_NAME   _T("void WriteMessages(void)\n{\n")

enum enum_FILE_ACCESS_FAILURE_t
{
	FILE_ACCESS_FAILURE_NOT_OPEN	=	1,
	FILE_ACCESS_FAILURE_USER_CANCEL	=	2
};

/*we devide a signal into segments, SegmentInfo_t contains the location of each segment */
typedef struct {

	UINT16			u16_ByteOffset;
	UINT8           u8_BitOffset;
	UINT8           u8_SegmentLen;

}SegmentInfo_t;


typedef struct {

	UINT32			u32MsgID;
	CString         strMsgName;
	UINT8           u8DLC;
	         
	std::set<INT16>	set_Multiplexor;
	CString         str_MuxSignalName;
}MessageInfo_t;


typedef struct {

	CString         strSignalName;
	BOOL 			bIsMulti;
	INT16           iMultiplexor;	//Only valid when bIsMulti=TRUE	
	UINT32          u32StartBit;
	UINT8           u8Length;
	BOOL            bEndian;   //0: Motorola  1:Intel
	BOOL            bIsSigned;

	float           fScale;
	float           fOffset;
	float           fMinValue;
	float           fMaxValue;

	UINT64          u64RawInitValue;

	std::vector<SegmentInfo_t>  mvector_SegmentInfo;

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
	map<UINT32, MessageInfo_t>			m_mapMessages;  //MsgID-->MsgInfo
#endif
	multimap<UINT32, SignalInfo_t>		m_mapSignals;   //MsgID-->SignalInfo

public:
	DBCFileInfoProcess();
	virtual ~DBCFileInfoProcess();

	UINT  OpenFile();
	bool  IsFileOpen();
	void  AnalyzeDBCFileInfo();
	
	CString GetSignalRawType(SignalInfo_t signal_info);

	void	InitSignalInfo(SignalInfo_t* pSignalInfo);
	void	InitMessageInfo(MessageInfo_t* pMessageInfo);


	void	InitializeSignal_CLanguage(vector<UINT32> SelectedMessages);   //the initialization code in C language
	void    ReadSignal_CLanguage(vector<UINT32> selected_messages);  //the code that reads memory buffer into signals in C language
	void    WriteSignal_CLanguage(vector<UINT32> selected_messages);

};

