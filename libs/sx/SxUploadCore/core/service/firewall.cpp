
#include "firewall.h"
#include <netfw.h>
#include <oleauto.h>

#pragma warning(disable: 4996) //解决GetVersionEx 过期问题
void WriteFireWallXP(LPCTSTR ruleName, LPCTSTR appPath,bool NoopIfExist)
{
    HRESULT hr = S_OK;
    HRESULT comInit = E_FAIL;
    INetFwProfile* fwProfile = NULL;
    INetFwMgr* fwMgr = NULL;
    INetFwPolicy* fwPolicy = NULL;
    INetFwAuthorizedApplication* fwApp = NULL;
    INetFwAuthorizedApplications* fwApps = NULL;
    BSTR bstrRuleName = SysAllocString((OLECHAR*)ruleName);
    BSTR bstrAppName = SysAllocString((OLECHAR*)appPath);

    // Initialize COM.
    comInit = CoInitializeEx(
        0,
        COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE
        );

    // Ignore RPC_E_CHANGED_MODE; this just means that COM has already been
    // initialized with a different mode. Since we don't care what the mode is,
    // we'll just use the existing mode.
    if (comInit != RPC_E_CHANGED_MODE)
    {
        hr = comInit;
        if (FAILED(hr))
        {
            printf("CoInitializeEx failed: 0x%08lx\n", hr);
            goto error;
        }
    }

    // Create an instance of the firewall settings manager.
    hr = CoCreateInstance(
        __uuidof(NetFwMgr),
        NULL,
        CLSCTX_INPROC_SERVER,
        __uuidof(INetFwMgr),
        (void**)&fwMgr
        );
    if (FAILED(hr))
    {
        printf("CoCreateInstance failed: 0x%08lx\n", hr);
        goto error;
    }
    // Retrieve the local firewall policy.
    hr = fwMgr->get_LocalPolicy(&fwPolicy);
    if (FAILED(hr))
    {
        printf("get_LocalPolicy failed: 0x%08lx\n", hr);
        goto error;
    }

    // Retrieve the firewall profile currently in effect.
    hr = fwPolicy->get_CurrentProfile(&fwProfile);
    if (FAILED(hr))
    {
        printf("get_CurrentProfile failed: 0x%08lx\n", hr);
        goto error;
    }




    // Retrieve the authorized application collection.
    hr = fwProfile->get_AuthorizedApplications(&fwApps);
    if (FAILED(hr))
    {
        printf("get_AuthorizedApplications failed: 0x%08lx\n", hr);
        goto error;
    }

    //check if exist
    if (NoopIfExist)
    {
        hr = fwApps->Item(bstrRuleName, &fwApp);
        if (hr == S_OK)
        {
            printf("item is exist");
            goto error;
        }
    }


    // Create an instance of an authorized application.
    hr = CoCreateInstance(
        __uuidof(NetFwAuthorizedApplication),
        NULL,
        CLSCTX_INPROC_SERVER,
        __uuidof(INetFwAuthorizedApplication),
        (void**)&fwApp
        );
    if (FAILED(hr))
    {
        printf("CoCreateInstance failed: 0x%08lx\n", hr);
        goto error;
    }
    // Set the process image file name.
    hr = fwApp->put_ProcessImageFileName(bstrAppName);
    if (FAILED(hr))
    {
        printf("put_ProcessImageFileName failed: 0x%08lx\n", hr);
        goto error;
    }

    // Set the application friendly name.
    hr = fwApp->put_Name(bstrRuleName);
    if (FAILED(hr))
    {
        printf("put_Name failed: 0x%08lx\n", hr);
        goto error;
    }

    // Add the application to the collection.
    hr = fwApps->Add(fwApp);
    if (FAILED(hr))
    {
        printf("Add failed: 0x%08lx\n", hr);
        goto error;
    }

error:
    // Release the local firewall policy.
    if (fwPolicy != NULL)
    {
        fwPolicy->Release();
    }

    // Release the firewall settings manager.
    if (fwMgr != NULL)
    {
        fwMgr->Release();
    }
    SysFreeString(bstrRuleName);
    SysFreeString(bstrAppName);
    if (fwApp != NULL)
    {
        fwApp->Release();
    }

    // Release the authorized application collection.
    if (fwApps != NULL)
    {
        fwApps->Release();
    }

    if (fwProfile != NULL)
    {
        fwProfile->Release();
    }

    // Uninitialize COM.
    if (SUCCEEDED(comInit))
    {
        CoUninitialize();
    }
}

//写入防火墙，最低支持版本Windows Vista
void WriteFireWallOverXP(LPCTSTR ruleName, LPCTSTR appPath, bool NoopIfExist)
{
    HRESULT hrComInit = S_OK;
    HRESULT hr = S_OK;

    INetFwPolicy2 *pNetFwPolicy2 = NULL;
    INetFwRules *pFwRules = NULL;
    INetFwRule *pFwRule = NULL;


    BSTR bstrRuleName = SysAllocString((OLECHAR*)ruleName);
    BSTR bstrAppName = SysAllocString((OLECHAR*)appPath);

    // Initialize COM.
    hrComInit = CoInitializeEx(
        0,
        COINIT_APARTMENTTHREADED
        );

    // Ignore RPC_E_CHANGED_MODE; this just means that COM has already been
    // initialized with a different mode. Since we don't care what the mode is,
    // we'll just use the existing mode.
    if (hrComInit != RPC_E_CHANGED_MODE)
    {
        if (FAILED(hrComInit))
        {
            printf("CoInitializeEx failed: 0x%08lx\n", hrComInit);
            goto Cleanup;
        }
    }

    // Retrieve INetFwPolicy2
    hr = CoCreateInstance(
        __uuidof(NetFwPolicy2),
        NULL,
        CLSCTX_INPROC_SERVER,
        __uuidof(INetFwPolicy2),
        (void**)&pNetFwPolicy2);

    if (FAILED(hr))
    {
        printf("CoCreateInstance for INetFwPolicy2 failed: 0x%08lx\n", hr);
        goto Cleanup;
    }

    // Retrieve INetFwRules
    hr = pNetFwPolicy2->get_Rules(&pFwRules);
    if (FAILED(hr))
    {
        printf("get_Rules failed: 0x%08lx\n", hr);
        goto Cleanup;
    }

    //see if existed
    if (NoopIfExist)
    {
        hr = pFwRules->Item(bstrRuleName, &pFwRule);
        if (hr == S_OK)
        {
            printf("Item existed", hr);
            goto Cleanup;
        }
    }

    // Create a new Firewall Rule object.
    hr = CoCreateInstance(
        __uuidof(NetFwRule),
        NULL,
        CLSCTX_INPROC_SERVER,
        __uuidof(INetFwRule),
        (void**)&pFwRule);
    if (FAILED(hr))
    {
        printf("CoCreateInstance for Firewall Rule failed: 0x%08lx\n", hr);
        goto Cleanup;
    }

    // Populate the Firewall Rule object


    pFwRule->put_Name(bstrRuleName);
    pFwRule->put_ApplicationName(bstrAppName);
    pFwRule->put_Action(NET_FW_ACTION_ALLOW);
    pFwRule->put_Enabled(VARIANT_TRUE);

    // Add the Firewall Rule
    hr = pFwRules->Add(pFwRule);
    if (FAILED(hr))
    {
        printf("Firewall Rule Add failed: 0x%08lx\n", hr);
        goto Cleanup;
    }

Cleanup:

    // Free BSTR's
    SysFreeString(bstrRuleName);
    SysFreeString(bstrAppName);


    // Release the INetFwRule object
    if (pFwRule != NULL)
    {
        pFwRule->Release();
    }

    // Release the INetFwRules object
    if (pFwRules != NULL)
    {
        pFwRules->Release();
    }

    // Release the INetFwPolicy2 object
    if (pNetFwPolicy2 != NULL)
    {
        pNetFwPolicy2->Release();
    }

    // Uninitialize COM.
    if (SUCCEEDED(hrComInit))
    {
        CoUninitialize();
    }
}


namespace h7 {

void WriteFireWall(LPCTSTR ruleName, LPCTSTR appPath,bool NoopIfExist)
{
    //check windows version
    OSVERSIONINFO ovi = { sizeof(OSVERSIONINFO) };
    if (!::GetVersionEx(&ovi))
        return;
    int version = ovi.dwMajorVersion * 100 + ovi.dwMinorVersion;
    if (version < 600)  //600为vista
    {
        WriteFireWallXP(ruleName, appPath, NoopIfExist);
    }
    else
    {
        WriteFireWallOverXP(ruleName, appPath, NoopIfExist);
    }
}
}

//int _tmain(int argc, _TCHAR* argv[])
//{

//    WriteFireWall(L"testfirewall", L"d:\\test.text",true);
//    getchar();
//    return 0;

//}

