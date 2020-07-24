#include "stdafx.h"
#include "MyMarco.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern GOLBALVALUE m_MainValue ;

VOID printdata(char * data ,int len) {
	printf("databegin\n");
	for (int i = 0; i < len; i++) {
		printf("%02x",data[i]);
	}
	printf("dataend\n");

}

BOOL SendData(SOCKET s, char *data, int len)
{
	//printdata(data, len);
	char * p = data;
	int i = 0;
	int k = len;
	int ret = 0;

	if(len <= 0) return TRUE;
	while(1)
	{
		ret = send(s,p,k,0);
		if(ret == 0 || ret == SOCKET_ERROR 
			||m_MainValue.m_IsMainExit)
		{
			TRACE("SendData OUT,%d\n",WSAGetLastError());
			return FALSE;
		}
		i += ret;
		p += ret;
		k -= ret;
		if(i >= len) break;
	}
	return TRUE;
}

BOOL RecvData(SOCKET s, char *data, int len)
{
	char * p = data;
	int i = 0;
	int k = len;
	int ret = 0;
	printdata(data, len);
	if(len <= 0) return TRUE;
	while(1)
	{
		ret = recv(s,p,k,0);
		if(ret == 0 || ret == SOCKET_ERROR
			|| m_MainValue.m_IsMainExit)
		{
			TRACE("RecvData OUT,%d\n",WSAGetLastError());
			return FALSE;
		}
		i += ret;
		p += ret;
		k -= ret;
		if(i >= len) break;
	}
	return TRUE;
}

int AcceptClientMain(SOCKET s,LPCLIENTITEM pData)
{
	char ch = 0;
	int  nlinelen = 0;
	char slinedata[8192] = {0};
	int  ret = 0;

	//����һ������
	while(1)
	{
		//����һ���ַ�
		ret = recv(s,&ch,1,0);
		if(ret == 0 || ret == SOCKET_ERROR || m_MainValue.m_IsMainExit)
			return -1;

		//��ȡ����
		slinedata[nlinelen] = ch;
		if(nlinelen >= 4 &&
			slinedata[nlinelen] == '\n' &&
			slinedata[nlinelen - 1] == '\r' &&
			slinedata[nlinelen - 2] == '\n' &&
			slinedata[nlinelen - 3] == '\r')
			break;

		if(nlinelen++ > 8000)
			return -1;
	}
	
	TRACE("%s\n",slinedata);
    printf("7---data:%s, len:%d ---\n",slinedata,nlinelen);

	char* pFlag = strchr(slinedata,'/');
	if(pFlag == NULL) return -1;
	if(*(pFlag + 1) == '/')
	{
		pFlag += 2;
		pFlag = strchr(pFlag,'/');
		if(pFlag == NULL) return -1;
	}
	pFlag ++;

	//ȡ��������
	char m_sCommand[10] = {0};
	memcpy(m_sCommand,pFlag,4);
	int m_Command = atoi(m_sCommand);

	//�鿴�����Ƿ�Ϸ�
	if(m_Command > 4999 ||	m_Command < 3000)
		return -1;

	//����login����
	AscToBcd((BYTE*)(pFlag + 4), (BYTE*) &pData->m_SysInfo, sizeof(LOGININFO) * 2);
	return m_Command;
}

void BcdToAsc(unsigned char *BcdStr,unsigned char *AscStr,int BcdStrLen)
{
	int i, j;
	for(i = 0; i < BcdStrLen; i++)
	{
		j = (BcdStr[i] >> 4) & 0x0f;
                
		if(j > 9)
			AscStr[2 * i] = j + 'A' - 10;
		else
			AscStr[2 * i] = j + '0';

		j = BcdStr[i] & 0x0f;

		if(j > 9)
			AscStr[2 * i + 1 ] = j + 'A' - 10;
		else
			AscStr[2 * i + 1] = j + '0';
	}
}

void AscToBcd(unsigned char *AscStr,unsigned char *BcdStr,int AscStrLen)
{
	
	int i, j, k;
	for(i = 0; i < AscStrLen; i++)
	{
        
		if( ( AscStr[i] >= '0' ) && ( AscStr[i] <= '9' ) )

			j = AscStr[i] - '0';

		else if ( ( AscStr[i] >= 'a' ) && ( AscStr[i] <= 'z' ) )

			j = AscStr[i] - 'a' + 0x0a;

		else 

			j = AscStr[i] - 'A' + 0x0a;

		i++;

		if( ( AscStr[i] >= '0' ) && ( AscStr[i] <= '9' ) )

			k = AscStr[i] - '0';

		else if ( ( AscStr[i] >= 'a' ) && ( AscStr[i] <= 'z' ) )

			k = AscStr[i] - 'a' + 0x0a;

		else

			k = AscStr[i] - 'A' + 0x0a;

		BcdStr[i / 2] = (j << 4) | k;

	}

} 

void TransParentDC(CRect rect,CDC * pDC)
{
	CDC m_MemDC;
	m_MemDC.CreateCompatibleDC(pDC);
	CBitmap m_Bitmap;
	m_Bitmap.CreateCompatibleBitmap
		(pDC,rect.Width(),rect.Height());
	CBitmap *pOldBitmap = 
		m_MemDC.SelectObject(&m_Bitmap);
	m_MemDC.FillSolidRect(0,0,rect.Width(),
		rect.Height(),GetSysColor(COLOR_MENU));

	COLORREF cor =  pDC->GetPixel(0,0);
	for(int y = 0; y < rect.Height();y++)
	{
		for(int x = 0; x < rect.Width(); x++)
		{
			COLORREF ch = pDC->GetPixel(x,y);
			if(ch != cor)
				m_MemDC.SetPixelV(x,y,ch);
		}
	}
	pDC->BitBlt(0,0,rect.Width(),
		rect.Height(),&m_MemDC,0,0,SRCCOPY);
	m_MemDC.SelectObject(pOldBitmap);
	m_Bitmap.DeleteObject();
}

SOCKET StartTcp(WORD Port)
{
	SOCKET		Sck;	
	sockaddr_in addr;
	int			optval =  600 * 1000;

	memset(&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(Port);
	addr.sin_addr.s_addr = inet_addr("192.168.1.189");
	//bzero(&(addr.sin_zero), 8);

	Sck = socket(AF_INET, SOCK_STREAM, 0);
	if(Sck==INVALID_SOCKET)
		return NULL;

	if(bind(Sck, (sockaddr*)&addr, sizeof(addr)) 
		== SOCKET_ERROR)
	{
		closesocket(Sck);
		return NULL;
	}

	if(setsockopt(Sck,SOL_SOCKET,SO_SNDTIMEO,
		(char * )&optval,sizeof(optval)) 
		== SOCKET_ERROR)
	{
		closesocket(Sck);
		return NULL;
	}

	if(setsockopt(Sck,SOL_SOCKET,SO_RCVTIMEO,
		(char * )&optval,sizeof(optval)) 
		== SOCKET_ERROR)
	{
		closesocket(Sck);
		return NULL;
	}

	if(listen(Sck, SOMAXCONN) == SOCKET_ERROR)
	{
		closesocket(Sck);
		return NULL;
	}

	return Sck;
}

BOOL ExecCmd(SOCKET s, UINT Command,int len)
{
	CMDINFO m_CmdInfo = {0};

	//��ʼ��
	m_CmdInfo.m_Command = Command;
	m_CmdInfo.m_DataLen = len;

	//�ر��¼�֪ͨ
	WSAAsyncSelect(s,m_MainValue.m_MainhWnd, 0, 0);
	ULONG icmd = 0;   
    if(ioctlsocket(s,FIONBIO,&icmd))
	{
		closesocket(s);printf("117 closesocket \n");
		return FALSE;
	}
	
	//���Ͱ�ͷ
	if(!SendData(s,(char*) &m_CmdInfo,sizeof(CMDINFO)))
	{
		closesocket(s);printf("117 closesocket \n");
		return FALSE;
	}

	//����¼�֪ͨ
	if(WSAAsyncSelect(s,m_MainValue.m_MainhWnd,
		WM_CLOSEITEM,FD_CLOSE) == SOCKET_ERROR)
	{
		closesocket(s);printf("117 closesocket \n");
		return FALSE;
	}
	return TRUE;
}

BOOL GetValue(char * SrcStr,char * DesStr,char *Value)
{
	char * pStart = strstr(SrcStr,DesStr);
	if(pStart == NULL)
	{
		return FALSE;
	}
	pStart += strlen(DesStr);
	char * pEnd = strchr(pStart,'\r');
	if(pEnd == NULL)
	{
		return FALSE;
	}
	memcpy(Value,pStart,pEnd - pStart);
	return TRUE;
}

BOOL MakeFilePath(char *pathName)
{
	char m_TmpStr[100] = {0};		
	char *pr = NULL;							
	char *ps = NULL;
	int  nLenth = 0;
	
	char m_Path[256] = {0};
	strcpy(m_Path, pathName);

	//�鿴�Ƿ�Ϊ�ļ�
	char* pFind = strchr(m_Path, '.');
	if(pFind != NULL)
	{
		pFind = strrchr(m_Path, '\\');
		if(pFind != NULL)
		{
			*pFind = 0;
		}
	}

	//Ŀ¼�Ѿ�����
	CFileStatus m_FileStatus;
	if(CFile::GetStatus(m_Path,m_FileStatus))
		return TRUE;

	//��ʼ��ָ��
	ps = m_Path;
	nLenth = strlen(m_Path);
	while(1)
	{
		pr = strchr(ps,'\\');
		if(pr == NULL)
		{
			//ֻ��һ��Ŀ¼
			pr = ps + strlen(ps) ;
		}
		int len = pr - m_Path;
		memcpy(m_TmpStr,m_Path,len);
		if(len>3)  
		{
			//������ǰĿ¼
			m_TmpStr[len] = '\0';
			CreateDirectory(m_TmpStr,NULL);
		}
		ps = pr+1;
		if(ps - m_Path >= nLenth)
		{
			//Ŀ¼�����걸
			break;
		}
	}
	//ȷ��������Ŀ¼�Ƿ����
	return CFile::GetStatus(m_Path,m_FileStatus);
}

void GetMyFilePath(char* FileName)
{
	char m_Path[512] = {0};
	GetModuleFileName(NULL,m_Path,500);
	char* pFind = strrchr(m_Path,'\\');
	if(pFind != NULL)
	{
		*pFind = 0;
		strcat(m_Path,"\\");
		strcat(m_Path,FileName);
		strcpy(FileName,m_Path);
	}
}

void GetIniFileName(char* pFile)
{
	char m_IniFileName[256] = {0};

	//ȷ�������ļ�����
	GetModuleFileName(NULL,m_IniFileName,250);
	char *p = strrchr(m_IniFileName,'.');
	*p = 0;
	strcat(m_IniFileName,".ini");

	strcpy(pFile, m_IniFileName);
}

bool CopySocket(HWND hWnd, SOCKET s, DWORD nInfo)
{
	//�����׽���
	DWORD pId = 0;
	GetWindowThreadProcessId(hWnd , &pId);
	WSAPROTOCOL_INFO m_SocketInfo = {0};
	if(WSADuplicateSocket(s, pId , &m_SocketInfo))
	{
		closesocket(s);printf("117 closesocket \n");
		return FALSE;
	}
	
	//��������
	COPYDATASTRUCT ct = {0};
	ct.lpData = &m_SocketInfo;
	ct.cbData = sizeof(WSAPROTOCOL_INFO);
	ct.dwData = nInfo;
	SendMessage(hWnd,WM_COPYDATA,0,(LPARAM) &ct);
	closesocket(s);printf("117 closesocket \n");
	return TRUE;
}