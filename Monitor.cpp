#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
extern "C"

BOOL WINAPI ReadDirectoryChangesW(
                      __in     HANDLE hDirectory,
                      __out_bcount_part(nBufferLength, *lpBytesReturned) LPVOID lpBuffer,
                      __in     DWORD nBufferLength,
                      __in     BOOL bWatchSubtree,
                      __in     DWORD dwNotifyFilter,
                      __out    LPDWORD lpBytesReturned,
                      __inout  LPOVERLAPPED lpOverlapped,
                      __in_opt LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );

DWORD WINAPI ThreadProc(LPVOID lpParam)
{
    BOOL bRet = FALSE;
    BYTE Buffer[1024] = { 0 };
	char szLog[] = {0};
    FILE_NOTIFY_INFORMATION *pBuffer = (FILE_NOTIFY_INFORMATION *)Buffer;
	FILE* hFile = fopen("log.txt","a+");
    DWORD BytesReturned = 0;
    HANDLE hDir = CreateFile("C:\\test",
                FILE_LIST_DIRECTORY, 
                FILE_SHARE_READ|FILE_SHARE_DELETE|FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                FILE_FLAG_BACKUP_SEMANTICS,
                NULL);
    if ( INVALID_HANDLE_VALUE == hDir )
    {
        return 1;
    }

    printf("Monitoring... \r\n");

    while ( TRUE )
    {
        ZeroMemory(Buffer, 1024);
        bRet = ReadDirectoryChangesW(hDir,
                        &Buffer,
                        sizeof(Buffer),
                        TRUE,
                        FILE_NOTIFY_CHANGE_FILE_NAME |  // �޸��ļ���
                        FILE_NOTIFY_CHANGE_ATTRIBUTES | // �޸��ļ�����
                        FILE_NOTIFY_CHANGE_LAST_WRITE , // ���һ��д��
                        &BytesReturned,
                        NULL, NULL);
        
        if ( bRet == TRUE )
        {
            char szFileName[MAX_PATH] = { 0 };

            // ���ַ�ת�����ֽ�
            WideCharToMultiByte(CP_ACP,
                                0,
                                pBuffer->FileName,
                                pBuffer->FileNameLength / 2,
                                szFileName,
                                MAX_PATH,
                                NULL,
                                NULL);
			
            switch(pBuffer->Action) 
            { 
                // ���
            case FILE_ACTION_ADDED: 
                {
					fprintf(hFile,"File Create: %s\r\n", szFileName);
					fflush(hFile);
					printf("File Create: %s\r\n", szFileName);
                    
                    break;
                }
                // ɾ��
            case FILE_ACTION_REMOVED: 
                {
					fprintf(hFile,"File Delete: %s\r\n", szFileName);
					fflush(hFile);
					printf("File Delete: %s\r\n", szFileName);
                    
                    break; 
                }
                // �޸�
            case FILE_ACTION_MODIFIED: 
                {
					fprintf(hFile,"File Modify: %s\r\n", szFileName);
					fflush(hFile);
					printf("File Modify: %s\r\n", szFileName);
                    
                    break; 
                }
                // ������
            case FILE_ACTION_RENAMED_OLD_NAME: 
                {
					fprintf(hFile,"File Rename: %s", szFileName);
					fflush(hFile);
					printf("File Rename: %s", szFileName);
                    if ( pBuffer->NextEntryOffset != 0 )
                    {
                        FILE_NOTIFY_INFORMATION *tmpBuffer = (FILE_NOTIFY_INFORMATION *)((DWORD)pBuffer + pBuffer->NextEntryOffset);
                        switch ( tmpBuffer->Action )
                        {
                        case FILE_ACTION_RENAMED_NEW_NAME:
                            {        
                                ZeroMemory(szFileName, MAX_PATH);
                                WideCharToMultiByte(CP_ACP,
                                    0,
                                    tmpBuffer->FileName,
                                    tmpBuffer->FileNameLength / 2,
                                    szFileName,
                                    MAX_PATH,
                                    NULL,
                                    NULL);
                                fprintf(hFile," ->  : %s \r\n", szFileName);
								fflush(hFile);
								printf(" ->  : %s \r\n", szFileName);
                                break;
                            }
                        }
                    }
                    break; 
                }
            case FILE_ACTION_RENAMED_NEW_NAME: 
                {
					fprintf(hFile,"Rename(new) : %s\r\n", szFileName);
					fflush(hFile);
					printf("Rename(new) : %s\r\n", szFileName);
                }
            }
        }
    }

    CloseHandle(hDir);
	fclose(hFile);
    return 0;
}

int main(int argc, char* argv[])
{
    HANDLE hThread = CreateThread(NULL, 0, ThreadProc, NULL, 0, NULL);
    if ( hThread == NULL )
    {
        return -1;
    }

    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);

	return 0;
}
