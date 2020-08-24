gggggggggggggggg
	
	
	bbbbaaaa

	if(a)
	{
	}
		a = b & c ? d :
//**************************************************************************
//
//  Copyright © 2001, by eMation, Inc.  All rights reserved
//
//**************************************************************************
//
//  Filename   :  Enterprise.cpp
//  
//  Subsystem  :  Kernel
//
//  DesDescription:  Interface for Communication with Enterprise
//
//  Class      :  
//              
//  Author     :  Brian Dodge
//
//**************************************************************************
#include "pch.h"

EEnterprise    *g_pEnterprise = NULL;
const TCHAR* ENTERPRISE_COMPONENT_NAME = _T("Enterprise");

/*************************************************************************************/
extern "C"
#ifdef WIN32
__declspec(dllexport)
#endif
IKernelService* EEnterpriseInitService(IKernelBase* pKernelBase, const TCHAR* pszDllParams)
{
    g_pEnterprise = new EEnterprise(pKernelBase);
    
    if (!g_pEnterprise)
        return NULL;

    return (IKernelService*)g_pEnterprise;EEntRequest::EEntRequest(EEnterprise* pOwner, bool bPretty, bool bVerbose)
}

/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
//
// EEntRequest Object
//
//**************************************************************************
EEntRequest::EEntRequest(EEnterprise* pOwner, bool bPretty, bool bVerbose)  :
                        m_pOwner(pOwner),
                        m_pToken(NULL),
                        m_bAddedProlog(false),
                        m_pretty(bPretty),
                        m_verbose(bVerbose),
                        m_msgID(0)
{
    ETime now  = ETime::GetCurrentTime();
    m_CreateTime = now.GetTime();
    m_AppendTime = m_CreateTime;
}

//**************************************************************************
EEntRequest::~EEntRequest()
{
    if(m_pToken) DestroyXmlToken(m_pToken);
}

//**************************************************************************
KERESULT EEntRequest::StartRequest(
                                   TCHAR* request,
                                   DWORD id,
                                   FILETIME* pTimeStamp,
                                   TCHAR* pszDevice,
                                   TCHAR* pszSerial
                                   )
{
    // initialize building of XML block to send to enterprise
    //
    KERESULT kerr;


    ETime now  = ETime::GetCurrentTime();
    m_AppendTime = now.GetTime();

    m_StrReq         = request;
    m_DeviceName     = pszDevice ? pszDevice : _T("nil-device");
    m_SerialNumber   = pszSerial ? pszSerial : _T("nil-serial");

    // put in the start of the emessage if first req for this req
    //
    if(! m_bAddedProlog)
    {
        m_bAddedProlog = true;

        if(m_pToken) DestroyXmlToken(m_pToken);
        kerr = CreateStringXmlToken(&m_pToken, NULL);
        if(kerr != KE_OK)   return kerr;
        if(! m_pToken)      return KE_FAILED;
        
        m_pToken->XmlSetPretty(m_pretty); // dont waste bandwidth for aesthetics
        
        
        // put in boiler plate
        
        // <?xml version="1.0" encoding="UTF-8" ?>
        if(m_verbose)
        {
            kerr = m_pToken->XmlOpenSection(_T("?xml"));
            kerr = m_pToken->XmlPutAttribute(_T("version"), _T("1.0"));
            kerr = m_pToken->XmlPutAttribute(_T("encoding"), _T("UTF-8"));
            kerr = m_pToken->XmlPutAttribute(_T("?"), NULL);
            kerr = m_pToken->XmlAbortSection();
            
            // <!DOCTYPE eMessage SYSTEM "DRM.dtd">
            kerr = m_pToken->XmlOpenSection(_T("!--DOCTYPE")); // commented until working
            kerr = m_pToken->XmlPutAttribute(_T("eMessage"), NULL);
            kerr = m_pToken->XmlPutAttribute(_T("SYSTEM"), NULL);
            kerr = m_pToken->XmlPutAttribute(_T("\"DRM.dtd\"--"), NULL);
            kerr = m_pToken->XmlAbortSection();
            if(kerr != KE_OK) return kerr;
        }
        else
        {
            kerr = m_pToken->XmlOpenSection(_T("?xml"));
            kerr = m_pToken->XmlPutAttribute(_T("?"), NULL);
            kerr = m_pToken->XmlAbortSection();
        }
        // <eMessage msgid="n" retries="n" version="1.0">
        kerr = m_pToken->XmlOpenSection(_T("eMessage"));
        if(kerr != KE_OK) return kerr;
        
        // msgid=
        if(id <= 0)
            id = m_pOwner->BumpPostSequence();
        if(id >= 0)
        {
            
            TCHAR mtxt[32];
            _sntprintf(mtxt, 32, _T("%d"), id);
            kerr = m_pToken->XmlPutAttribute(m_verbose? _T("msgid") : _T("id"), mtxt);
        }
        m_msgID = id;

        kerr = m_pToken->XmlPutAttribute(m_verbose? _T("retries") : _T("rc"), _T("0"));
        
        if(kerr != KE_OK) return kerr;
        
        // version=
        if(ENT_XML_VERSION_MAJOR > 0 || ENT_XML_VERSION_MINOR > 0)
        {
            TCHAR vtxt[16];
            int   vmaj;
            
            vmaj = ENT_XML_VERSION_MAJOR;
            if(!m_verbose)
                vmaj++;
            _sntprintf(vtxt, 16, _T("%d.%d"), vmaj, ENT_XML_VERSION_MINOR);
            kerr = m_pToken->XmlPutAttribute(m_verbose?_T("version"):_T("v"), vtxt);
        }
        
        // device stuff
        kerr = AddDeviceInfo(pszDevice, pszSerial);
        if(kerr != KE_OK) return kerr;
    }
    // <req>
    kerr = m_pToken->XmlOpenSection(request);
    if(kerr != KE_OK) return kerr;

    kerr = AddTimeStampAttribute(pTimeStamp);
    return kerr;
}

//**************************************************************************
KERESULT EEntRequest::FinishRequest(void)
{
    if(m_StrReq.GetLength() > 0)
    {
        // close the sectioned opened by StartRequest
        //
        if(m_pToken->XmlCloseSection(m_StrReq.GetBuffer(0)) != KE_OK)
            return KE_FAILED;
        m_StrReq.Empty();
    }
    return KE_OK;
}

//**************************************************************************
KERESULT EEntRequest::AddDeviceInfo(TCHAR* pszDevice, TCHAR* pszSerial)
{
    KERESULT kerr;

    //<Device model="m" sn="n" owner="w" version="x.y.z" />
    ASSERT(m_pToken != NULL);
    kerr = m_pToken->XmlOpenSection(m_verbose?_T("Device"):_T("De"));        
    if(kerr != KE_OK) return kerr;
    kerr = m_pToken->XmlPutAttribute(m_verbose?_T("model"):_T("mn"), 
        pszDevice ? pszDevice : _T("nil-device"));
    if(kerr != KE_OK) return kerr;
    kerr = m_pToken->XmlPutAttribute(_T("sn"),
        pszSerial ? pszSerial : _T("nil-serial"));
    if(kerr != KE_OK) return kerr;
    kerr = m_pToken->XmlPutAttribute(m_verbose?_T("owner"):_T("ow"), m_pOwner->GetOwner().GetBuffer(0));
    if(kerr != KE_OK) return kerr;
    if(m_verbose)
    {
        TCHAR vbuf[64];
        _sntprintf(vbuf, 64, _T("%d.%d.%.1f"), KERNEL_VERSION_MAJOR, KERNEL_VERSION_MINOR, KERNEL_VERSION_BUILD);
        kerr = m_pToken->XmlPutAttribute(_T("version"), vbuf);
        if(kerr != KE_OK) return kerr;
    }
    kerr = m_pToken->XmlCloseSection(NULL);
    return kerr;
}

//**************************************************************************
KERESULT EEntRequest::AddStatusInfo(unsigned long status)
{
    KERESULT kerr;
    TCHAR stat[32];

    if(! m_pToken) return KE_FAILED;
    _sntprintf(stat, 32, _T("%d"), status);
    kerr = m_pToken->XmlPutValue(m_verbose?_T("Status"):_T("St"), stat);
    return kerr;
}

//**************************************************************************
KERESULT EEntRequest::AddTimeStampAttribute(FILETIME* pTimeStamp)
{
    KERESULT kerr;
    FILETIME timestamp;
    EString  strTime;

    if(! pTimeStamp)
    {
        GetSystemTimeAsFileTime(&timestamp);
    }
    else
    {
        timestamp = *pTimeStamp;
    }
    ETime* pTime = new ETime(timestamp);
    pTime->GetISO8601Time(strTime);
    delete pTime;

    kerr = m_pToken->XmlPutAttribute(m_verbose?_T("utc"):_T("t"), strTime.GetBuffer(0));
    return kerr;
}

//**************************************************************************
KERESULT EEntRequest::AddDataItemInfo(TCHAR* type, TCHAR* name, TCHAR* value, TCHAR* quality)
{
    KERESULT kerr;

    if(! m_pToken) return KE_FAILED;
    if(! name || ! value) return KE_FAILED;
    kerr =  m_pToken->XmlOpenSection(m_verbose?_T("DataItem"):_T("Di"));
    if(kerr != KE_OK) return kerr;
    if(name && *name != _T('\0'))
    {
        kerr = m_pToken->XmlPutAttribute(m_verbose?_T("name"):_T("n"), name);
        if(kerr != KE_OK) return kerr;
    }
    if(type && *type != _T('\0'))
    {
        kerr = m_pToken->XmlPutAttribute(m_verbose?_T("type"):_T("y"), type);
        if(kerr != KE_OK) return kerr;
    }
    if(quality && *quality != _T('\0'))
    {
        kerr = m_pToken->XmlPutAttribute(m_verbose?_T("quality"):_T("q"), quality);
        if(kerr != KE_OK) return kerr;
    }
    kerr = m_pToken->XmlPutValue(NULL, value);
    if(kerr != KE_OK) return kerr;
    kerr = m_pToken->XmlCloseSection(m_verbose?_T("DataItem"):_T("Di"));
    return kerr;
}

//**************************************************************************
KERESULT EEntRequest::AddAlarmInfo(
                                        int     severity,
                                        TCHAR*  description,
                                        TCHAR*  type,
                                        TCHAR*  name,
                                        bool    active,
                                        bool    ack
                                        )
{
    KERESULT kerr;
    TCHAR    sev[32];

    if(! m_pToken) return KE_FAILED;
    kerr = m_pToken->XmlPutAttribute(m_verbose?_T("name"):_T("n"), name);
    if(kerr != KE_OK) return kerr;
    _sntprintf(sev, 32, _T("%d"), severity);
    kerr = m_pToken->XmlPutAttribute(m_verbose?_T("severity"):_T("sv"), sev);
    if(kerr != KE_OK) return kerr;
    if(description)
    {
        kerr = m_pToken->XmlPutAttribute(m_verbose?_T("description"):_T("de"), description);
        if(kerr != KE_OK) return kerr;
    }
    if(type)
    {
        kerr = m_pToken->XmlPutAttribute(m_verbose?_T("type"):_T("y"), type);
        if(kerr != KE_OK) return kerr;
    }
    kerr = m_pToken->XmlPutAttribute(m_verbose?_T("active"):_T("at"), active ? _T("y") : _T("n"));
    if(kerr != KE_OK) return kerr;
    kerr = m_pToken->XmlPutAttribute(m_verbose?_T("ack"):_T("ak"), ack ? _T("y") : _T("n"));
    return kerr;
}

//**************************************************************************
KERESULT EEntRequest::AddStrInfo(TCHAR* element, TCHAR* value, TCHAR* attr, ...)
{
    KERESULT kerr;
    TCHAR* attrval;
    
    va_list vargs;
    va_start(vargs, attr);

    if(! m_pToken)  return KE_FAILED;

    if(element != NULL)
    {
        kerr = m_pToken->XmlOpenSection(element);
        if(kerr != KE_OK) return kerr;
    }
    while(attr)
    {
        attrval = va_arg(vargs, TCHAR*);
        if(! attrval) attrval = _T("");
        kerr = m_pToken->XmlPutAttribute(attr, attrval);
        if(kerr != KE_OK) return kerr;
        attr = va_arg(vargs, TCHAR*);
    }
    if(element)
    {
        if(value)
        {
            // section has value, so need <s>value</s> format
            kerr = m_pToken->XmlPutValue(NULL, value);
            if(kerr != KE_OK) return kerr;
            kerr = m_pToken->XmlCloseSection(element);
        }
        else
        {
            // section has NO value, so use <s att=att /> format
            kerr = m_pToken->XmlCloseSection(NULL);
        }
    }
    va_end(vargs);
    return kerr;
}

//**************************************************************************
KERESULT EEntRequest::AddIntInfo(TCHAR* element, int ival, TCHAR* attr, ...)
{
    KERESULT kerr;
    TCHAR*   attrval;
    TCHAR    value[16];

    if(! m_pToken)  return KE_FAILED;
    if(! element)   return KE_FAILED;

    _sntprintf(value, 16, _T("%d"), ival);
    
    va_list vargs;
    va_start(vargs, attr);

    if(! m_pToken)  return KE_FAILED;
    if(! element)   return KE_FAILED;

    kerr = m_pToken->XmlOpenSection(element);
    if(kerr != KE_OK) return kerr;
    while(attr)
    {
        attrval = va_arg(vargs, TCHAR*);
        if(! attrval) attrval = _T("");
        kerr = m_pToken->XmlPutAttribute(attr, attrval);
        if(kerr != KE_OK) return kerr;
        attr = va_arg(vargs, TCHAR*);
    }
    if(value)
    {
        // section has value, so need <s>value</s> format
        kerr = m_pToken->XmlPutValue(NULL, value);
        if(kerr != KE_OK) return kerr;
        kerr = m_pToken->XmlCloseSection(element);
    }
    else
    {
        // section has NO value, so use <s att=att /> format
        kerr = m_pToken->XmlCloseSection(NULL);
    }
    va_end(vargs);
    return kerr;
}

//**************************************************************************
KERESULT EEntRequest::AddRawXML(TCHAR* pXML)
{
    if(! m_pToken)  return KE_FAILED;
    if(! pXML)      return KE_FAILED;

    return m_pToken->XmlPutRaw(pXML);
}

//**************************************************************************
KERESULT EEntRequest::GetRequest(EString& strReq)
{
    EString strXml;

    if(! m_pToken)
        return KE_FAILED;
    
    if(FinishRequest() != KE_OK)
        return KE_FAILED;

    if(m_pToken->XmlCloseSection(_T("eMessage")) != KE_OK)
        return KE_FAILED;

    // wrap request in xmldata block
    //
    if(m_pToken->XmlGetStreamBuffer(strXml) != KE_OK)
        return KE_FAILED;

    strReq.Empty();

#ifdef NEED_HEADER
    if(m_verbose)
        strReq = _T("xmldata=");
    else
        strReq = _T("x=");
#endif

    strReq += strXml;
    return KE_OK;
}

//**************************************************************************
KERESULT EEntRequest::GetSize(int& size)
{
    EString strReq;

    // [ISSUE] bdd - there should be a XmlGetStreamBufferSize()
    // this is WAY too inefficient
    //
    m_pToken->XmlGetStreamBuffer(strReq);
    size = strReq.GetLength();
    return KE_OK;
}

//**************************************************************************
KERESULT EEntRequest::GetCreationTime(time_t& createtime)
{
    createtime = m_CreateTime;
    return KE_OK;
}

//**************************************************************************
KERESULT EEntRequest::GetAppendTime(time_t& appendtime)
{
    appendtime = m_AppendTime;
    return KE_OK;
}
/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
//
// EEnterprise Object
//
EEnterprise::EEnterprise(IKernelBase* pParent) : EKernelService(pParent)
{
    mx_iRegRetryCount   = 0;    // max count of retry attempts, <= 0 = infinite
    mx_iRegRetryPeriod  = 10;   // seconds between registration attempts
    mx_iPort            = 80;
    mx_Strategy         = (int)IEnterprise::InitiateConnection;
    m_NoSoapRequired    = 0;
    mx_MaxQueueSize     = MAX_ENTERPRISE_QUEUE_SIZE;
    mx_bDebugPost       = 0;
    mx_bDebugPostQueue  = 0;
    mx_PrettyXML        = 0;
    mx_VerboseXML       = 0;
    mx_QueueCoalesce    = 0;    // no
    mx_MaxRequestSize   = 0;    // infinite
    mx_MinRequestAge    = 5;    // seconds
    mx_MinRequestAgeQD  = 60;   // seconds
    mx_MaxRequestAge    = 10;   // seconds
    mx_QueueHighThreshold   = mx_MaxQueueSize / 2;    // when QueueHighThreshold is signaled
    mx_QueueLowThreshold    = 0;                      // when QueueLowThreshold is signaled
    m_pIWebClient       = NULL;
    m_bShutdown         = false;
    m_pISoapDispatcher  = NULL;
    m_pISoapParser      = NULL;
    m_pIEventLog        = NULL;
    m_pIWebClient       = NULL;
    m_bShutdown         = false;
    m_pQueue            = NULL;
    m_PostSequence      = 0;
    m_PostRetryCount    = 0;
    m_pCurrentReq       = NULL;
    m_bUTF8encode       = true;   // always utf-8 encode unicode messages to enterprise
}

/*************************************************************************************/
EEnterprise::~EEnterprise()
{
}

/*************************************************************************************/
bool EEnterprise::GetInterface(KERNEL_IID InterfaceID, void** ppInterface)
{
    bool   bFound = false;
    
    *ppInterface = 0;
    switch (InterfaceID)
    {
    case IID_ENTERPRISE:
        *ppInterface = (IEnterprise*)this;
        bFound = true;
        break;
        
    default:
        return EKernelService::GetInterface(InterfaceID,ppInterface);
    }
    return bFound;
}

//**************************************************************************
KERESULT EEnterprise::Save(EHANDLE pData) 
{
    bool bOpenedSection = false;

    // Get peristent handle, m_pParent is IKernelService*
    if(pData == (EHANDLE)NULL)
    {
        KERESULT kerr = EPersistent::SaveSection(_T("EEnterprise"), pData);
        if(kerr != KE_OK) return kerr;
        bOpenedSection = true;
    }
    
    if(! pData)
    {
        return KE_FAILED;
    }
    
    EString strComment;
    strComment.Format(_T("This section generated by EEnterprise component, Unity Connector version %d.%d build %.1f"),
        KERNEL_VERSION_MAJOR,
        KERNEL_VERSION_MINOR,  
        KERNEL_VERSION_BUILD);
    EPersistent::Comment(strComment.GetBuffer(0), pData);

    EPersistentInt xVersion;
    
    EPersistent::Comment(_T("Version"), pData);
    xVersion = 1;
    xVersion.Save(pData);

    EPersistent::Comment(_T("Enterprise Server Address"), pData);
    mx_sEntServerAddress.Save(pData);
        
    EPersistent::Comment(_T("Enterprise Page Address"), pData);
    mx_sEntPageAddress.Save(pData);

    EPersistent::Comment(_T("License ID"), pData);
    mx_sLicenseId.Save(pData);

    EPersistent::Comment(_T("HTTP Port Number"), pData);
    mx_iPort.Save(pData);

    EPersistent::Comment(_T("Registration Retry Period"), pData);
    mx_iRegRetryPeriod.Save(pData);

    EPersistent::Comment(_T("Registration Retry Count (max)"), pData);
    mx_iRegRetryCount.Save(pData);

    EPersistent::Comment(_T("Connect Strategy:1=register when connected,2=force connect then register,3=assume connected already"), pData);
    mx_Strategy = mx_Strategy | m_NoSoapRequired;
    mx_Strategy.Save(pData);
    mx_Strategy = mx_Strategy & ~m_NoSoapRequired;

    EPersistent::Comment(_T("Maximum number of Queue entries"), pData);
    mx_MaxQueueSize.Save(pData);

    EPersistent::Comment(_T("Debug POSTing 0=no, 1=yes"), pData);
    mx_bDebugPost.Save(pData);

    EPersistent::Comment(_T("Debug POST Queue 0=no, 1=yes"), pData);
    mx_bDebugPostQueue.Save(pData);

    EPersistent::Comment(_T("Print Pretty XML 0=no, 1=yes"), pData);
    mx_PrettyXML.Save(pData);

    EPersistent::Comment(_T("Print Verbose XML 0=no, 1=yes"), pData);
    mx_VerboseXML.Save(pData);

    EPersistent::Comment(_T("Allow multiple requests in one Post 0=no, 1=yes"), pData);
    mx_QueueCoalesce.Save(pData);

    EPersistent::Comment(_T("Max Request size when coalesce=true (0 = infinite)"), pData);
    mx_MaxRequestSize.Save(pData);

    EPersistent::Comment(_T("How long to wait for additional data for Post (seconds)"), pData);
    mx_MinRequestAge.Save(pData);

    EPersistent::Comment(_T("How long to wait for additional data while Queue disabled (seconds)"), pData);
    mx_MinRequestAgeQD.Save(pData);

    EPersistent::Comment(_T("How old a request can get before it is Posted (seconds)"), pData);
    mx_MaxRequestAge.Save(pData);

    EPersistent::Comment(_T("How many items until QueueHighThreshold is signaled"), pData);
    mx_QueueHighThreshold.Save(pData);

    EPersistent::Comment(_T("How many items when QueueLowThreshold is signaled"), pData);
    mx_QueueLowThreshold.Save(pData);

    EPersistent::Comment(_T("How long (seconds) Queue is empty before QueueQuiescent is signaled"), pData);
    mx_QueueQuiescentPeriod.Save(pData);

    if(bOpenedSection)
        EPersistent::CloseSection(pData);
    return KE_OK;
}

//**************************************************************************
KERESULT EEnterprise::Restore(EHANDLE pData)
{
    bool openedSection = false;

    if(pData == (EHANDLE)NULL)
    {
        KERESULT kerr = EPersistent::RestoreSection(_T("EEnterprise"), pData);
        if(kerr != KE_OK) return kerr;
        openedSection = true;
    }
    if(! pData)
    {
        return KE_FAILED;
    }
    EPersistentInt xVersion;
    
    EPersistentString mx_sUpgServerAddress;
    EPersistentString mx_sUpgPageAddress;

    xVersion.Restore(pData, 1);
    mx_sEntServerAddress.Restore(pData,mx_sEntServerAddress.GetBuffer(0));
    mx_sUpgServerAddress.Restore(pData,mx_sUpgServerAddress.GetBuffer(0));
    mx_sEntPageAddress.Restore(pData,mx_sEntPageAddress.GetBuffer(0));
    mx_sUpgPageAddress.Restore(pData,mx_sUpgPageAddress.GetBuffer(0));
    mx_sLicenseId.Restore(pData,mx_sLicenseId.GetBuffer(0));
    mx_iPort.Restore(pData,mx_iPort);
    mx_iRegRetryPeriod.Restore(pData, mx_iRegRetryPeriod);
    mx_iRegRetryCount.Restore(pData, mx_iRegRetryCount);
    mx_Strategy.Restore(pData, mx_Strategy);
    m_NoSoapRequired = mx_Strategy & 0x10;
    mx_Strategy = mx_Strategy & 0xF;
    mx_MaxQueueSize.Restore(pData, mx_MaxQueueSize);
    mx_bDebugPost.Restore(pData, mx_bDebugPost);
    mx_bDebugPostQueue.Restore(pData, mx_bDebugPostQueue);
    mx_PrettyXML.Restore(pData, mx_PrettyXML);
    mx_VerboseXML.Restore(pData, mx_VerboseXML);
    mx_QueueCoalesce.Restore(pData, mx_QueueCoalesce);
    mx_MaxRequestSize.Restore(pData, mx_MaxRequestSize);
    mx_MinRequestAge.Restore(pData, mx_MinRequestAge);
    mx_MinRequestAgeQD.Restore(pData, mx_MinRequestAgeQD);
    mx_MaxRequestAge.Restore(pData, mx_MaxRequestAge);
    mx_QueueHighThreshold.Restore(pData, mx_QueueHighThreshold);
    mx_QueueLowThreshold.Restore(pData, mx_QueueLowThreshold);
    mx_QueueQuiescentPeriod.Restore(pData, mx_QueueQuiescentPeriod);
    if(openedSection)
        EPersistent::CloseSection(pData);
    return KE_OK;
}

//**************************************************************************
KERESULT EEnterprise::PrepareToStart()
{
    ILicense    *pLcs = NULL;

    if(GetRunStatus() & PREPARETOSTART)
        return KE_OK;

    // Get Event Log Interface
    if( !GetInterface(IID_KERNEL_LOG, (void**) &m_pIEventLog) )
    {
        return KE_FAILED;
    }

    // Get Web Client Interface
    if( !GetInterface(IID_WEB_CLIENT,(void**)&m_pIWebClient) )
    {
        m_pIEventLog->Report(ET_CRITICAL_ERROR, ENTERPRISE_COMPONENT_NAME,
                            _T("Web Client not found"));
        return KE_FAILED;
    }
    
    // Get SOAP Interface
    if( !GetInterface(IID_SOAP_PARSER,(void**)&m_pISoapParser) )
    {
        m_pIEventLog->Report(ET_CRITICAL_ERROR, ENTERPRISE_COMPONENT_NAME,
                            _T("SOAP Parser not found"));
        return KE_FAILED;
    }

    // Register message handler with soap dispatcher
    if( !GetInterface(IID_SOAP_DISPATCH, (void **) &m_pISoapDispatcher) )
    {
        m_pIEventLog->Report(ET_CRITICAL_ERROR, ENTERPRISE_COMPONENT_NAME,
                            _T("SOAP Dispatcher not found"));
        return KE_FAILED;
    }
    m_pISoapDispatcher->Register(this, _T("EEnterprise"));
    m_pISoapDispatcher->Register(this, _T("EEnterpriseProxy"));
    
    // Register as an action for Notify component
    // this is optional, and done only if Notfy component loaded
    //
    INotifyMgr* pINotifyMgr = NULL;
    GetInterface(IID_NOTIFY_MGR, (void**) &pINotifyMgr);
    
    if (pINotifyMgr)
    {
        IAction* pIAction = (IAction*)this;
        
        if(pINotifyMgr->Register(ENTERPRISE_START,      pIAction) != KE_OK) return KE_FAILED;
        if(pINotifyMgr->Register(ENTERPRISE_STOP,       pIAction) != KE_OK) return KE_FAILED;
        if(pINotifyMgr->Register(ENTERPRISE_NOTIFY,     pIAction) != KE_OK) return KE_FAILED;
        if(pINotifyMgr->Register(ENTERPRISE_ALARM,      pIAction) != KE_OK) return KE_FAILED;
        if(pINotifyMgr->Register(ENTERPRISE_TIMER,      pIAction) != KE_OK) return KE_FAILED;
        if(pINotifyMgr->Register(ENTERPRISE_SNAPSHOT,   pIAction) != KE_OK) return KE_FAILED;
        if(pINotifyMgr->Register(ENTERPRISE_DATASETADD, pIAction) != KE_OK) return KE_FAILED;
        if(pINotifyMgr->Register(ENTERPRISE_EXPRESSION, pIAction) != KE_OK) return KE_FAILED;

        if(RegisterTrigger(ENTERPRISE_QUEUE_READY,    pINotifyMgr) != KE_OK) return KE_FAILED;
        if(RegisterTrigger(ENTERPRISE_QUEUE_EMPTY,    pINotifyMgr) != KE_OK) return KE_FAILED;
        if(RegisterTrigger(ENTERPRISE_QUEUE_FULL,     pINotifyMgr) != KE_OK) return KE_FAILED;
        if(RegisterTrigger(ENTERPRISE_QUEUE_NONEMPTY, pINotifyMgr) != KE_OK) return KE_FAILED;
        if(RegisterTrigger(ENTERPRISE_QUEUE_HIGH,     pINotifyMgr) != KE_OK) return KE_FAILED;
        if(RegisterTrigger(ENTERPRISE_QUEUE_LOW,      pINotifyMgr) != KE_OK) return KE_FAILED;
        if(RegisterTrigger(ENTERPRISE_QUEUE_QUIET,    pINotifyMgr) != KE_OK) return KE_FAILED;
    }

    // Create the Queue processing object
    //
    m_pQueue = new EEntQueue(this);

    if(! m_pQueue)
        return KE_FAILED;

    // Get License interface
    if( !GetInterface(IID_LICENSE,(void**)&pLcs) )
    {
        m_pIEventLog->Report(ET_CRITICAL_ERROR, ENTERPRISE_COMPONENT_NAME,
                            _T("License interface not found"));
        return KE_FAILED;
    }

#if 0
    // Get Device Model Number from license component
    if( pLcs->GetModelNumber(m_sDeviceModelNumber) != KE_OK )
    {
        m_pIEventLog->Report(ET_CRITICAL_ERROR, ENTERPRISE_COMPONENT_NAME,
                            _T("Unable to retrieve device model number"));
        return KE_FAILED;
    }

    // Get Device Serial Number from license component
    if( pLcs->GetSerialNumber(m_sSerialNumber) != KE_OK )
    {
        m_pIEventLog->Report(ET_CRITICAL_ERROR, ENTERPRISE_COMPONENT_NAME,
                            _T("Unable to retrieve device serial number"));
        return KE_FAILED;
    }
#endif

    if( pLcs->GetOwner(m_sOwner) != KE_OK )
    {
        m_pIEventLog->Report(ET_CRITICAL_ERROR, ENTERPRISE_COMPONENT_NAME,
                            _T("Unable to retrieve device owner"));
        return KE_FAILED;
    }
    SetRunStatus(PREPARETOSTART);
    return KE_OK;
}

//**************************************************************************
KERESULT EEnterprise::Start()
{
    if((GetRunStatus() & STARTED) == STARTED)
        return KE_OK;

    // Start Thread to Register this device with the Enterprise
    // server.  This involves posting to a web server, so is very
    // asynchronous.  The rest of the enterprise system is somewhat
    // disabled until the device is registered
    //
    if( mx_sEntServerAddress.IsEmpty() )
    {
        m_pIEventLog->Report(ET_DEBUG,ENTERPRISE_COMPONENT_NAME,
                            _T("Enterprise Server address not found - layer disabled"));
    }
    else
    {
        // if configured, start registration immediately
        //
        if(mx_Strategy == IEnterprise::InitiateConnection)
        {
            // tell web machinery to connect, when it does,
            // the notification (OnInternetStart) should 
            // tell us to start registration, or, we will
            // get the same signal soon, if the connection
            // happens(happened) by itself
            //
            //m_pIWebClient->Connect();
        }
        else if(mx_Strategy == IEnterprise::AssumeConnection)
        {
            EnableEnterpriseComponents();
        }
    }    
    SetRunStatus(STARTED);
    
    return KE_OK;
}

//**************************************************************************
KERESULT EEnterprise::PrepareToStop()
{
    if(GetRunStatus() != STARTED)
        return KE_OK;
    m_bShutdown = true;
    DisableEnterpriseComponents();
    SetRunStatus(PREPARETOSTOP);
    return KE_OK;
}

//**************************************************************************
KERESULT EEnterprise::Stop()
{
    EEnterprise* pProxy = g_pEnterprise;

    g_pEnterprise = NULL;
    if(pProxy) delete pProxy;

    return KE_OK;
}

//**************************************************************************
KERESULT EEnterprise::PostRequest(IEnterpriseRequest* pReq, IEnterprise::QueuePriority priority)
{
    KERESULT    kerr;
    EString     msg;
    EString     address;
    EString     url;
    EString     device;
    EString     serial;
    int         port;

    // current request, if any, should be locked before calling here
    //
    kerr = pReq->GetRequest(msg);
    if(kerr != KE_OK) return kerr;

    kerr = pReq->GetDeviceName(device);
    if(kerr != KE_OK) return kerr;

    kerr = pReq->GetSerialNumber(serial);
    if(kerr != KE_OK) return kerr;

    // the request has reached the end of its usefullness
    //
    if(pReq == m_pCurrentReq)
        m_pCurrentReq = NULL;
    DestroyRequest(pReq);

    GetEnterpriseServerAddress(address, url, port);

    return m_pQueue->AddItem(
                            device.GetBuffer(0),
                            serial.GetBuffer(0),
                            address.GetBuffer(0),
                            port,
                            url.GetBuffer(0),
                            msg.GetBuffer(0),
                            m_PostSequence,
                            priority
                            );
}

//**************************************************************************
KERESULT EEnterprise::EnableEnterpriseComponents(void)
{
    IEnterpriseComponent *pInterface;
    POSITION pos = m_entCompMap.GetStartPosition();
    EString name;

    // Start Queue processing
    //
    if(m_pQueue)
        m_pQueue->StartProcessing();

    // Notify registered enterprise components they can start communication
    // with us
    //
    while( pos )
    {
        m_entCompMap.GetNextAssoc(pos, name, pInterface);
        pInterface->StartEnterpriseCommunication();
    }
    return KE_OK;
}

//**************************************************************************
KERESULT EEnterprise::DisableEnterpriseComponents(void)
{
    IEnterpriseComponent *pInterface;
    POSITION pos = m_entCompMap.GetStartPosition();
    EString name;

    // Notify registered enterprise components they must stop communication
    // with us
    //
    while( pos )
    {
        m_entCompMap.GetNextAssoc(pos, name, pInterface);
        pInterface->StopEnterpriseCommunication();
    }

    // Stop Queue processing
    //
    if(m_pQueue)
        m_pQueue->StopProcessing();

    return KE_OK;
}


//**************************************************************************
KERESULT EEnterprise::RegisterEntComponent(EString& name, IEnterpriseComponent* pInterface)
{
    ELock lock(m_mapMtx);
    m_entCompMap.SetAt(name,pInterface);
    return KE_OK;
}

//**************************************************************************
KERESULT EEnterprise::UnregisterEntComponent(EString& name)
{
    ELock lock(m_mapMtx);
    m_entCompMap.RemoveKey(name);
    return KE_OK;
}

//**************************************************************************
KERESULT EEnterprise::GetEnterpriseServerAddress(EString& adrs, EString& url, int& port)
{
    adrs = mx_sEntServerAddress;
    url  = mx_sEntPageAddress;
    port = mx_iPort;
    return KE_OK;
}

//**************************************************************************
KERESULT EEnterprise::GetRequest(IEnterpriseRequest*& pReq)
{   
    pReq = new EEntRequest(this, mx_PrettyXML != 0, mx_VerboseXML != 0);
    if(mx_QueueCoalesce)
    {
        m_ReqMex.Lock();
        m_pCurrentReq = (EEntRequest*)pReq;
        m_ReqMex.Unlock();
    }
    return KE_OK;
}

//**************************************************************************
KERESULT EEnterprise::ReleaseRequest(IEnterpriseRequest* pReq)
{
    // all set with current USE of the request, so end any open element
    //
    pReq->FinishRequest();
    return KE_OK;
}


//**************************************************************************
KERESULT EEnterprise::CreateRequest(IEnterpriseRequest*& pReq)
{
    KERESULT kerr = KE_OK;

    // called from *outside* the component, so don't include
    // in "current"
    //
    if(mx_QueueCoalesce)
    {
        m_ReqMex.Lock();
        if(m_pCurrentReq)
        {
            kerr = PostRequest(m_pCurrentReq, IEnterprise::Urgent);
        }
        m_ReqMex.Unlock();
    }
    pReq = new EEntRequest(this, mx_PrettyXML != 0, mx_VerboseXML != 0);
    return kerr;
}

//**************************************************************************
KERESULT EEnterprise::DestroyRequest(IEnterpriseRequest* pReq)
{
    if(pReq)    delete (EEntRequest*)pReq;
    return KE_OK;
}
//**************************************************************************
KERESULT EEnterprise::CheckPendingRequest()
{
    time_t now;
    time_t then;
    int   reqsize;
    bool  bShipit = false;
    KERESULT kerr = KE_OK;

    // if not coalescing, no need to do this
    //
    if(! mx_QueueCoalesce)
    {
        ASSERT(! m_pCurrentReq);
        return KE_OK;
    }
    m_ReqMex.Lock();

    // if no current req, no need either
    //
    if(! m_pCurrentReq)
    {
        m_ReqMex.Unlock();
        return KE_OK;
    }

    // if the data has grown past the max size, ship it
    //
    kerr = m_pCurrentReq->GetSize(reqsize);
    if(kerr != KE_OK)
    {
        m_ReqMex.Unlock();
        return kerr;
    }
    if(reqsize > mx_MaxRequestSize && mx_MaxRequestSize > 0)
    {
        if(mx_bDebugPostQueue)
            m_pIEventLog->Report(ET_DEBUG, ENTERPRISE_COMPONENT_NAME,
                _T("Sending coalesced item %p, size %d exceeded %d"),
                m_pCurrentReq, reqsize, ((int)mx_MaxRequestSize));
        bShipit = true;
    }

    if(! bShipit)
    {
        ETime enow = ETime::GetCurrentTime();
        now = enow.GetTime();

        kerr = m_pCurrentReq->GetCreationTime(then);
        if(kerr != KE_OK)
        {
            m_ReqMex.Unlock();
            return kerr;
        }

        // if the request has been around for a while, ship it
        //
        if((now - then) > mx_MaxRequestAge)
        {
            if(mx_bDebugPostQueue)
                m_pIEventLog->Report(ET_DEBUG, ENTERPRISE_COMPONENT_NAME,
                    _T("Sending coalesced item %p, older than %d seconds"),
                    m_pCurrentReq, ((int)mx_MaxRequestAge));
            bShipit = true;
        }

        kerr = m_pCurrentReq->GetAppendTime(then);
        if(kerr != KE_OK)
        {
            m_ReqMex.Unlock();
            return kerr;
        }
        // if the request has been stale for a while, ship it
        //
        if(
            (now - then) >  (m_pQueue->m_Processing ? mx_MinRequestAge : mx_MinRequestAgeQD))
        {
            if(mx_bDebugPostQueue)
                m_pIEventLog->Report(ET_DEBUG, ENTERPRISE_COMPONENT_NAME,
                    _T("Sending coalesced item %p, not added to for %d seconds"),
                    m_pCurrentReq, ((int)mx_MinRequestAge));
            bShipit = true;
        }
    }
    if(bShipit)
    {
        // post req deletes the req
        //
        kerr = PostRequest(m_pCurrentReq);
        m_pCurrentReq = NULL;
    }
    m_ReqMex.Unlock();
    return kerr;
}

//**************************************************************************
KERESULT EEnterprise::SendStatus(unsigned long stat)
{
    KERESULT    kerr;
    IEnterpriseRequest* pReq;

    kerr = CreateRequest(pReq);
    if(kerr != KE_OK) return kerr;
    kerr = pReq->StartRequest(mx_VerboseXML?_T("Status"):_T("St"), BumpPostSequence(), NULL);
    if(kerr != KE_OK) return kerr;
    kerr = pReq->AddStatusInfo(stat);
    if(kerr != KE_OK) return kerr;
    kerr = PostRequest(pReq, IEnterprise::Low);
    DestroyRequest(pReq);
    return kerr;
}

//**************************************************************************
KERESULT EEnterprise::SendRunningStatus()
{
    return SendStatus(1000);
}

//**************************************************************************
KERESULT EEnterprise::SendStopStatus()
{
    return SendStatus(1001);
}

//**************************************************************************
KERESULT EEnterprise::ProcessSoapRPC(
                                          ISoapParser*    pParser,
                                          TCHAR*          pFunctionName,
                                          DWORD           fcnVersion,
                                          soapMethod*     pMethod,
                                          soapMethod*&    pReplyMethod,
                                          soapReplyState& replyState
                                         )
{
    soapParm* pParm = NULL;
    KERESULT  kerr;

    pParser->SoapFirstParm(pMethod, pParm);
    replyState = soapReplyComplete;

    if(_tcsicmp(pFunctionName, _T("Response")) == 0)
    {
        // this is a soap call which just says that our initiated 
        // conversation has been heard, so we do NOT want to give
        // this one a soap reply (which would get a reply, and loop)
        //
        replyState = soapReplySkip; // tells soap to give no response

        unsigned long status;
        unsigned long msgid;
        
        kerr = pParser->SoapGetAttribute(pMethod, _T("MsgID"), msgid);
        if(kerr != KE_OK)
            msgid = 0;

        if(pParser->SoapConvertIntegerParameter(pParm, _T("Status"), status) != KE_OK)
            return KE_FAILED;
       
        // tell the queue about the event
        //
        kerr = m_pQueue->SetPostResult(msgid, status);

        return kerr;
    }
    return KE_FAILED;
}

//**************************************************************************
void EEnterprise::TagAsString(EData& Data, EString& strTagValue, EString& strItemType, bool bVerbose)
{
    switch(Data.GetType())
    {
    default:
    case edataINT4:
    case edataREAL8:
        strItemType = bVerbose?_T("analog"):_T("a");
        break;
    case edataBOOL:
        strItemType = bVerbose?_T("digital"):_T("d");
        break;
    case edataTSTR:
        strItemType = bVerbose?_T("string"):_T("s");
        break;
    }
    strTagValue = (EString)Data.GetString();
}

//**************************************************************************
KERESULT EEnterprise::Invoke(
                             EString&        rActionName,
                             EString&        rSchemaName, 
                             EAttributes*    pAttributes, 
                             DWORD           dwCount,
                             EONEVENTSTRUCT* pEvents,
                             IErrorListener* pIErrorListener
                            )
{
    KERESULT kerr = KE_FAILED | KE_INVALID_PARAMS;
    IEnterpriseRequest* pReq = NULL;

    ELock lock(m_Invokex);

    if(rActionName == ENTERPRISE_ALARM)
    {
        if( !pEvents )
            return kerr;

        // send an alarm notification to the enterprise
        //
        EString  strXMLalarm;
        EString  strTagValue;
        EString  strItemType;
        
        /*********************
        if(m_bOnlyGoodAlarmData)
        {
            // make sure the tag is worth talking about
            //
            if(pEvents->triggerData.GetQuality() != QUALITY_GOOD)
                return KE_FAILED;
        }
        **********************/

        TCHAR*  pSource = pEvents->pszSource;
        
        if(*pSource == _T('.'))
            pSource++;
        
        TagAsString(*pEvents->pData, strTagValue, strItemType, (mx_VerboseXML != 0));
        
        do  // "try"
        {
            if(mx_QueueCoalesce != 0)
                m_ReqMex.Lock();
            
            if(m_pCurrentReq)
            {
                pReq = m_pCurrentReq;
            }
            else
            {
                kerr = GetRequest(pReq);
                if(kerr != KE_OK || ! pReq) break;
            }
            
            kerr = pReq->StartRequest(
                                        mx_VerboseXML?_T("Alarm"):_T("Al"),
                                        BumpPostSequence(),
                                        NULL
                                     );
            if(kerr != KE_OK) break;

            kerr = pReq->AddAlarmInfo(
                                        pEvents->dwSeverity,
                                        pEvents->pszMessage,
                                        pEvents->pszSubconditionName,
                                        /* rSchemaName.GetBuffer(0), */
                                        pEvents->pszSource,
                                        (pEvents->wState & ACTIVE) == ACTIVE,
                                        (pEvents->wState & ACKED)  == ACKED
                                    );
            if(kerr != KE_OK) break;
            kerr = pReq->AddDataItemInfo(
                strItemType.GetBuffer(0), pSource, strTagValue.GetBuffer(0), mx_VerboseXML?_T("good"):_T("g"));
            if(kerr != KE_OK) break;
            
#if defined (DEBUG_POST_ALARMS)

            if(0 && mx_bDebugPostQueue)
            {
                EString strDebug;
                
                strDebug = _T("Posting Alarm: ");
                strDebug+= pSource;
                strDebug+= _T(" is ");
                strDebug+= strTagValue.GetBuffer(0);
                strDebug+= _T(" (");
                strDebug+= pEvents->pszMessage;
                strDebug+= _T(")");
                
                if(m_pIEventLog)
                {
                    m_pIEventLog->Report(ET_INFO, _T(""), strDebug.GetBuffer(0));
                }
            }
#endif
            kerr = ReleaseRequest(pReq);
            if(kerr != KE_OK) { pReq = NULL; break; }
            
            // alarms force posting of the request
            // regardless of coalescing state
            //
            kerr = PostRequest(pReq, IEnterprise::Urgent);
            pReq = NULL;
        }
        while(0); // "catch"

        if(mx_QueueCoalesce != 0)
            m_ReqMex.Unlock();

        if(pReq != NULL)
            ReleaseRequest(pReq);

        // remove requests that had errors 
        if(kerr != KE_OK && pReq)
        {
            DestroyRequest(pReq);
        }
    }
    else if(rActionName == ENTERPRISE_SNAPSHOT)
    {
        // add a data item record for each data item
        // from a comma separated list of tagname, tag value
        //
        bool   bOK      = true;

        if( !pEvents )
            return KE_FAILED;

        // send data item changes to enterprise
        //
        ETag*   pTag = pEvents->pData;
        TCHAR*  ptype;
        TCHAR*  pquality;
        int     msgID;

        do  // "try"
        {
            // see if there is a request in progess
            //
            if(mx_QueueCoalesce != 0)
                m_ReqMex.Lock();
            
            if(m_pCurrentReq)
            {
                pReq = m_pCurrentReq;
                pReq->GetMsgID(msgID);
            }
            else
            {
                kerr  = GetRequest(pReq);
                msgID = BumpPostSequence();
                if(kerr != KE_OK || ! pReq) break;
            }
            
            // format the XML data to send
            kerr = pReq->StartRequest(
                                        mx_VerboseXML?_T("Data"):_T("Da"),
                                        msgID,
                                        &pEvents->ftTime,
                                        pEvents->pszSource,
                                        pEvents->pszSubconditionName
                                      );
            if(kerr != KE_OK) break;
            
            
            //m_pIEventLog->Report(ET_DEBUG, _T("Snapshot"), pEvents->pszMessage);
                       
            while(pTag)
            {
                // only add tags that have good quality
                //
                if(pTag->GetQuality() != edataBAD)
                {
                    switch(pTag->GetType())
                    {
                    case edataREAL8:  ptype = mx_VerboseXML?_T("analog"):	_T("a");      break;
                    case edataINT4:   ptype = mx_VerboseXML?_T("analog"):	_T("a");      break;
                    case edataBOOL:   ptype = mx_VerboseXML?_T("digital"):	_T("d");  	  break;
                    case edataTSTR:   ptype = mx_VerboseXML?_T("string"):	_T("s");   	  break;
                    default:          ptype = mx_VerboseXML?_T("error"):	_T("e");      break;
                    }
                    switch(pTag->GetQuality())
                    {
                    case edataGOOD:      pquality = mx_VerboseXML?_T("good"):	_T("g");   break;
                    case edataBAD:       pquality = mx_VerboseXML?_T("bad"):	_T("b");   break;
                    case edataUNCERTAIN: pquality = mx_VerboseXML?_T("uncertain"):_T("u"); break;
                    default:             pquality = mx_VerboseXML?_T("error"):	_T("e");   break;
                    }
                
                    // add item to request if there is a non-null value
                    // for the corresponding name
                    //
                    kerr = pReq->AddDataItemInfo(
                                                ptype,
                                                pTag->GetName(),
                                                pTag->GetString(),
                                                pquality
                                                );
                    if(kerr != KE_OK) break;
                }
                pTag = pTag->GetNext();
            }
            // all set adding to request, finish it up
            //
            kerr = ReleaseRequest(pReq);

            if(! mx_QueueCoalesce)
            {
                kerr = PostRequest(pReq, IEnterprise::Normal);
                pReq = NULL; // postreq destroys it
            }
        } while(0); // "catch"
        
        // remove requests that had errors 
        if(kerr != KE_OK && pReq)
        {
            if(pReq == m_pCurrentReq)
                m_pCurrentReq = NULL;
            DestroyRequest(pReq);
        }

        if(mx_QueueCoalesce != 0)
        {
            m_ReqMex.Unlock();

            // each time a request is added to the post, check
            // it so that timeout, and or size limits can be
            // looked at.  This is also called by the queue processing
            // thread timer, so make sure lock is unlocked
            //
            kerr = CheckPendingRequest();
        }
    }
    else if(rActionName == ENTERPRISE_DATASETADD)
    {
        if( !pEvents )
            return KE_FAILED;

        EDataSet* pSet = reinterpret_cast<EDataSet*>(pEvents->pMoreData);

        if(! pSet)
            return KE_FAILED;

        EDeviceReg* pDev;

        pDev = m_pQueue->GetRegisteredDevice(pSet->GetDeviceName(), pSet->GetSerialNumber());

        if(! pDev)
        {
            kerr = m_pQueue->RegisterDevice(pSet->GetDeviceName(), pSet->GetSerialNumber());
        }
        else
        {
            kerr = KE_OK;
        }
    }
    else if(rActionName == ENTERPRISE_TIMER)
    {
        kerr = KE_OK;
    }
    else if(rActionName == ENTERPRISE_EXPRESSION)
    {
        kerr = KE_OK;
    }
    else if(rActionName == ENTERPRISE_START)
    {
        // we are being told that the network connection is started up.
        //
        kerr = EnableEnterpriseComponents();
    }
    else if(rActionName == ENTERPRISE_STOP)
    {
        // we are being told that the network connection is shutting down
        //
        DisableEnterpriseComponents();
        kerr = KE_OK;
    }
    else if(rActionName == ENTERPRISE_NOTIFY)
    {
        if(rSchemaName == ENTERPRISE_SHUTDOWN_NOTIFY)
        {
            // we are being told that the system is shutting down
            // attempt to save outstanding queue items?
            //
            /*m_pQueue->SaveQueue(_T("EnterpriseQueue"));*/
            kerr = KE_OK;
        }
        else if(rSchemaName == ENTERPRISE_STARTUP_NOTIFY)
        {
            // we are being told that the system is starting up
            // attempt to restore outstanding queue items?
            //
            /*m_pQueue->RestoreQueue(_T("EnterpriseQueue"));*/
            kerr = KE_OK;
        }
        else
        {
            kerr = KE_FAILED | KE_INVALID_PARAMS;
        }
    }
    else
    {
        kerr = KE_FAILED | KE_INVALID_PARAMS;
    }
    if(kerr != KE_OK)
    {
        if(m_pIEventLog)
        {
            m_pIEventLog->Report(ET_ERROR, ENTERPRISE_COMPONENT_NAME, 
                _T("Enterprise Invocation %s[%s] FAILED err=%08X"),
                rActionName.GetBuffer(0), rSchemaName.GetBuffer(0), kerr);
        }
    }
    return kerr;
}
