#include <streams.h>

#include "PushSource.h"
#include "PushGuids.h"
#include "DibHelper.h"

#include <wmsdkidl.h>

//
// CheckMediaType
// I think VLC calls this once per each enumerated media type that it likes (3 times)
// just to "make sure" that it's a real valid option
// so we could "probably" just return true here, but do some checking anyway...
//
// We will accept 8, 16, 24 or 32 bit video formats, in any
// image size that gives room to bounce.
// Returns E_INVALIDARG if the mediatype is not acceptable
//
HRESULT CPushPinDesktop::CheckMediaType(const CMediaType *pMediaType)
{
	CAutoLock cAutoLock(m_pFilter->pStateLock());

    CheckPointer(pMediaType, E_POINTER);
	
	if(m_bFormatAlreadySet) {
		// then it must be the same as our current...see SetFormat msdn
	    if(m_mt == *pMediaType) {
			return S_OK;
		} else {
  		   return VFW_E_TYPE_NOT_ACCEPTED;
		}
	}

	const GUID Type = *(pMediaType->Type());
    if(Type != GUID_NULL && (Type != MEDIATYPE_Video) ||   // we only output video, GUID_NULL means any
        !(pMediaType->IsFixedSize()))                  // in fixed size samples
    {                                                  
        return E_INVALIDARG;
    }

    // Check for the subtypes we support
    if (pMediaType->Subtype() == NULL)
        return E_INVALIDARG;

	
	const GUID SubType2 = *pMediaType->Subtype();

	if(SubType2 != GetBitmapSubtype(&savedVideoFormat.bmiHeader))
		return E_INVALIDARG;

    // Get the format area of the media type
    VIDEOINFO *pvi = (VIDEOINFO *) pMediaType->Format();
    if(pvi == NULL)
        return E_INVALIDARG; // usually never see this...

    // Don't accept formats with negative height, which would cause the desktop
    // image to be displayed upside down.
	// also reject 0's, that would be weird.
    if (pvi->bmiHeader.biHeight <= 0)
        return E_INVALIDARG;

    if (pvi->bmiHeader.biWidth <= 0)
        return E_INVALIDARG;

	if(memcmp(&pvi->bmiHeader, &savedVideoFormat.bmiHeader, sizeof(BITMAPINFOHEADER)) == 0)
		return S_OK;
	else
		return E_INVALIDARG;

} // CheckMediaType


//
// SetMediaType
//
// Called when a media type is agreed between filters (i.e. they call GetMediaType+GetStreamCaps/ienumtypes I guess till they find one they like, then they call SetMediaType).
// all this after calling Set Format, if they even do, I guess...
// pMediaType is assumed to have passed CheckMediaType "already" and be good to go...
// except WFMLE sends us a junk type, so we check it anyway LODO do we? Or is it the other method Set Format that they call in vain? Or it first?
HRESULT CPushPinDesktop::SetMediaType(const CMediaType *pMediaType)
{
    CAutoLock cAutoLock(m_pFilter->pStateLock());

    // Pass the call up to my base class
    HRESULT hr = CSourceStream::SetMediaType(pMediaType); // assigns our local m_mt via m_mt.Set(*pmt) ... 

    if(SUCCEEDED(hr))
    {
        VIDEOINFO *pvi = (VIDEOINFO *) m_mt.Format();
        if (pvi == NULL)
            return E_UNEXPECTED;

		LocalOutput("bitcount requested/negotiated: %d\n", pvi->bmiHeader.biBitCount);
    
      // The frame rate at which your filter should produce data is determined by the AvgTimePerFrame field of VIDEOINFOHEADER
	  if(pvi->AvgTimePerFrame) // or should Set Format accept this? hmm...
	    m_rtFrameLength = pvi->AvgTimePerFrame; // allow them to set whatever fps they request, i.e. if it's less than the max default.  VLC command line can specify this, for instance...
    }
	
    return hr;

} // SetMediaType

#define DECLARE_PTR(type, ptr, expr) type* ptr = (type*)(expr);

// sets fps, size, (etc.) maybe, or maybe just saves it away for later use...
HRESULT STDMETHODCALLTYPE CPushPinDesktop::SetFormat(AM_MEDIA_TYPE *pmt)
{
    CAutoLock cAutoLock(m_pFilter->pStateLock());

	// I *think* it can go back and forth, then.  You can call GetStreamCaps to enumerate, then call
	// SetFormat, then later calls to GetMediaType/GetStreamCaps/EnumMediatypes will all "have" to just give this one
	// though theoretically they could also call EnumMediaTypes, then Set MediaType, and not call SetFormat
	// does flash call both? what order for flash/ffmpeg/vlc calling both?
	// LODO update msdn

	// "they" [can] call this...see msdn for SetFormat

	// NULL means reset to default type...
	if(pmt != NULL)
	{
		if(pmt->formattype != FORMAT_VideoInfo)  // same as {CLSID_KsDataTypeHandlerVideo} 
			return E_FAIL;
	
		// LODO I should do more here...http://msdn.microsoft.com/en-us/library/dd319788.aspx I guess [meh]
	    // LODO should fail if we're already streaming... [?]

		VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *) pmt->pbFormat;

		if(CheckMediaType((CMediaType *) pmt) != S_OK) {
			return E_FAIL; // just in case :P [did skype get here once?]
		}

		// now save it away...for being able to re-offer it later. We could use Set MediaType but we're just being lazy and re-using m_mt for many things I guess
	    m_mt = *pmt;  
	}

    IPin* pin;
    ConnectedTo(&pin);
    if(pin)
    {
        IFilterGraph *pGraph = m_pParent->GetGraph();
        HRESULT res = pGraph->Reconnect(this);
		if(res != S_OK) // LODO check first, and then just re-use the old one?
		  return res; // else return early...not really sure how to handle this...since we already set m_mt...but it's a pretty rare case I think...
		// plus ours is a weird case...
    } else {
		// graph hasn't been built yet...
		// so we're ok with "whatever" format they pass us, we're just in the setup phase...
	}
	

	// success of some type
	if(pmt == NULL) {		
		m_bFormatAlreadySet = false;
	} else {
		m_bFormatAlreadySet = true;
	}

    return S_OK;
}

// get's the current format...I guess...
// or get default if they haven't called SetFormat yet...
// LODO the default, which probably we don't do yet...unless they've already called GetStreamCaps then it'll be the last index they used LOL.
HRESULT STDMETHODCALLTYPE CPushPinDesktop::GetFormat(AM_MEDIA_TYPE **ppmt)
{
    CAutoLock cAutoLock(m_pFilter->pStateLock());

    *ppmt = CreateMediaType(&m_mt); // windows internal method, also does copy
    return S_OK;
}


HRESULT STDMETHODCALLTYPE CPushPinDesktop::GetNumberOfCapabilities(int *piCount, int *piSize)
{
    *piCount = 1;
    *piSize = sizeof(VIDEO_STREAM_CONFIG_CAPS); // VIDEO_STREAM_CONFIG_CAPS is an MS struct
    return S_OK;
}

// returns the "range" of fps, etc. for this index
HRESULT STDMETHODCALLTYPE CPushPinDesktop::GetStreamCaps(int iIndex, AM_MEDIA_TYPE **pmt, BYTE *pSCC)
{
    CAutoLock cAutoLock(m_pFilter->pStateLock());
	HRESULT hr = GetMediaType(iIndex, &m_mt); // ensure setup/re-use m_mt ...
	// some are indeed shared, apparently.
    if(FAILED(hr))
    {
        return hr;
    }

    *pmt = CreateMediaType(&m_mt); // a windows lib method, also does a copy for us
	if (*pmt == NULL) return E_OUTOFMEMORY;

	
    DECLARE_PTR(VIDEO_STREAM_CONFIG_CAPS, pvscc, pSCC);
	
    /*
	  most of these are listed as deprecated by msdn... yet some still used, apparently. odd.
	*/

    pvscc->VideoStandard = AnalogVideo_None;
	pvscc->InputSize.cx = m_iCaptureWidth;
	pvscc->InputSize.cy = m_iCaptureHeight;

	// most of these values are fakes..
	pvscc->MinCroppingSize.cx = m_iCaptureWidth;
    pvscc->MinCroppingSize.cy = m_iCaptureHeight;

    pvscc->MaxCroppingSize.cx = m_iCaptureWidth;
    pvscc->MaxCroppingSize.cy = m_iCaptureHeight;

    pvscc->CropGranularityX = 1;
    pvscc->CropGranularityY = 1;
    pvscc->CropAlignX = 1;
    pvscc->CropAlignY = 1;

    pvscc->MinOutputSize.cx = m_iCaptureWidth;
    pvscc->MinOutputSize.cy = m_iCaptureHeight;
    pvscc->MaxOutputSize.cx = m_iCaptureWidth;
    pvscc->MaxOutputSize.cy = m_iCaptureHeight;
    pvscc->OutputGranularityX = 1;
    pvscc->OutputGranularityY = 1;

    pvscc->StretchTapsX = 1; // We do 1 tap. I guess...
    pvscc->StretchTapsY = 1;
    pvscc->ShrinkTapsX = 1;
    pvscc->ShrinkTapsY = 1;

	pvscc->MinFrameInterval = m_rtFrameLength; // the larger default is actually the MinFrameInterval, not the max
	pvscc->MaxFrameInterval = m_rtFrameLength; // 0.02 fps :) [though it could go lower, really...]

    VIDEOINFO *pvi = (VIDEOINFO *) m_mt.Format();
	pvscc->MaxBitsPerSecond = pvscc->MinBitsPerSecond = (LONG) (pvi->bmiHeader.biSizeImage*GetFps());

	return hr;
}


// QuerySupported: Query whether the pin supports the specified property.
HRESULT CPushPinDesktop::QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport)
{
    if (guidPropSet != AMPROPSETID_Pin) return E_PROP_SET_UNSUPPORTED;
    if (dwPropID != AMPROPERTY_PIN_CATEGORY) return E_PROP_ID_UNSUPPORTED;
    // We support getting this property, but not setting it.
    if (pTypeSupport) *pTypeSupport = KSPROPERTY_SUPPORT_GET; 
    return S_OK;
}

STDMETHODIMP CPushSourceDesktop::Stop(){

	CAutoLock filterLock(m_pLock);

	//Default implementation
	HRESULT hr = CBaseFilter::Stop();

	//Reset pin resources
	m_pPin->m_iFrameNumber = 0;

	return hr;
}


// according to msdn...
HRESULT CPushSourceDesktop::GetState(DWORD dw, FILTER_STATE *pState)
{
    CheckPointer(pState, E_POINTER);
    *pState = m_State;
    if (m_State == State_Paused)
        return VFW_S_CANT_CUE;
    else
        return S_OK;
}

HRESULT CPushPinDesktop::QueryInterface(REFIID riid, void **ppv)
{   
    // Standard OLE stuff, needed for capture source
    if(riid == _uuidof(IAMStreamConfig))
        *ppv = (IAMStreamConfig*)this;
    else if(riid == _uuidof(IKsPropertySet))
        *ppv = (IKsPropertySet*)this;
    else
        return CSourceStream::QueryInterface(riid, ppv);

    AddRef(); // avoid interlocked decrement error... // I think
    return S_OK;
}



//////////////////////////////////////////////////////////////////////////
// IKsPropertySet
//////////////////////////////////////////////////////////////////////////


HRESULT CPushPinDesktop::Set(REFGUID guidPropSet, DWORD dwID, void *pInstanceData, 
                        DWORD cbInstanceData, void *pPropData, DWORD cbPropData)
{
	// Set: we don't have any specific properties to set...that we advertise yet anyway, and who would use them anyway?
    return E_NOTIMPL;
}

// Get: Return the pin category (our only property). 
HRESULT CPushPinDesktop::Get(
    REFGUID guidPropSet,   // Which property set.
    DWORD dwPropID,        // Which property in that set.
    void *pInstanceData,   // Instance data (ignore).
    DWORD cbInstanceData,  // Size of the instance data (ignore).
    void *pPropData,       // Buffer to receive the property data.
    DWORD cbPropData,      // Size of the buffer.
    DWORD *pcbReturned     // Return the size of the property.
)
{
    if (guidPropSet != AMPROPSETID_Pin)             return E_PROP_SET_UNSUPPORTED;
    if (dwPropID != AMPROPERTY_PIN_CATEGORY)        return E_PROP_ID_UNSUPPORTED;
    if (pPropData == NULL && pcbReturned == NULL)   return E_POINTER;
    
    if (pcbReturned) *pcbReturned = sizeof(GUID);
    if (pPropData == NULL)          return S_OK; // Caller just wants to know the size. 
    if (cbPropData < sizeof(GUID))  return E_UNEXPECTED;// The buffer is too small.
        
    *(GUID *)pPropData = PIN_CATEGORY_CAPTURE; // PIN_CATEGORY_PREVIEW ?
    return S_OK;
}


enum FourCC { FOURCC_NONE = 0, FOURCC_I420 = 100, FOURCC_YUY2 = 101, FOURCC_RGB32 = 102 };// from http://www.conaito.com/docus/voip-video-evo-sdk-capi/group__videocapture.html
//
// GetMediaType
//
// Prefer 5 formats - 8, 16 (*2), 24 or 32 bits per pixel
//
// Prefered types should be ordered by quality, with zero as highest quality.
// Therefore, iPosition =
//      0    Return a 24bit mediatype "as the default" since I guessed it might be faster though who knows
//      1    Return a 24bit mediatype
//      2    Return 16bit RGB565
//      3    Return a 16bit mediatype (rgb555)
//      4    Return 8 bit palettised format
//      >4   Invalid
// except that we changed the orderings a bit...
//
HRESULT CPushPinDesktop::GetMediaType(int iPosition, CMediaType *pmt) // AM_MEDIA_TYPE basically == CMediaType
{
    CheckPointer(pmt, E_POINTER);
    CAutoLock cAutoLock(m_pFilter->pStateLock());
	if(m_bFormatAlreadySet) {
		// you can only have one option, buddy, if setFormat already called. (see SetFormat's msdn)
		if(iPosition != 0)
          return E_INVALIDARG;
		VIDEOINFO *pvi = (VIDEOINFO *) m_mt.Format();

		// Set() copies these in for us pvi->bmiHeader.biSizeImage  = GetBitmapSize(&pvi->bmiHeader); // calculates the size for us, after we gave it the width and everything else we already chucked into it
        // pmt->SetSampleSize(pvi->bmiHeader.biSizeImage);
		// nobody uses sample size anyway :P

		pmt->Set(m_mt);
		VIDEOINFOHEADER *pVih1 = (VIDEOINFOHEADER*) m_mt.pbFormat;
		VIDEOINFO *pviHere = (VIDEOINFO  *) pmt->pbFormat;
		return S_OK;
	}

	// do we ever even get past here? hmm

    if(iPosition < 0)
        return E_INVALIDARG;

    // Have we run out of types?
    if(iPosition > 0)
        return VFW_S_NO_MORE_ITEMS;

    VIDEOINFO *pvi = (VIDEOINFO *) pmt->AllocFormatBuffer(sizeof(VIDEOINFO));
    if(NULL == pvi)
        return(E_OUTOFMEMORY);

    // Initialize the VideoInfo structure before configuring its members
    ZeroMemory(pvi, sizeof(VIDEOINFO));

	memcpy(&pvi->bmiHeader, &savedVideoFormat.bmiHeader, sizeof(BITMAPINFOHEADER));
    pmt->SetSampleSize(pvi->bmiHeader.biSizeImage); // use the above size

	pvi->AvgTimePerFrame = m_rtFrameLength; // from our config or default

    SetRectEmpty(&(pvi->rcSource)); // we want the whole image area rendered.
    SetRectEmpty(&(pvi->rcTarget)); // no particular destination rectangle

    pmt->SetType(&MEDIATYPE_Video);
    pmt->SetFormatType(&FORMAT_VideoInfo);
    pmt->SetTemporalCompression(FALSE); // ??

    // Work out the GUID for the subtype from the header info.
	if(*pmt->Subtype() == GUID_NULL) {
      const GUID SubTypeGUID = GetBitmapSubtype(&pvi->bmiHeader);
      pmt->SetSubtype(&SubTypeGUID);
	}

    return NOERROR;

} // GetMediaType

