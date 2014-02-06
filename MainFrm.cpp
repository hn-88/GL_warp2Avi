// MainFrm.cpp : implementation of the CMainFrame class
//
// from http://www.codeproject.com/Articles/1418/A-class-to-easily-generate-AVI-video-with-OpenGL-a
//
// with modifications February 2014 by Hari Nandakumar www.saispace.in

#include "stdafx.h"
#include "GL2Avi.h"

#include "MainFrm.h"
#include "GL2AviDoc.h"
#include "GL2AviView.h"
#include "AVIGenerator.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_AVIGENERATION_GENERATE, OnAvigenerationGenerate)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


void CMainFrame::OnAvigenerationGenerate() 
{
	UINT i,nFrames=200;

	CAVIGenerator AviGen;
	LPBITMAPINFOHEADER lpbih;
	BYTE* bmBits;	
	CGL2AviView* pView=(CGL2AviView*)GetActiveView();
	HRESULT hr;

	nFrames=pView->lastframe;

	CProgressBar Bar(_T("Generating movie"), 40, nFrames, true);
	Bar.SetStep(1);

	BeginWaitCursor();

	// set fps
	AviGen.SetRate(pView->psi.dwRate);
	AviGen.SetScale(pView->psi.dwScale);
	// fps = rate/scale
	// this was a bug in the original code
	
	// give info about bitmap
	AviGen.SetBitmapHeader(pView);	

	// save file name code
	// from http://msdn.microsoft.com/en-us/library/windows/desktop/ms646829%28v=vs.85%29.aspx#open_file

	OPENFILENAME ofn;       // common dialog box structure
	char szFile[260];       // buffer for file name

	// Initialize OPENFILENAME
ZeroMemory(&ofn, sizeof(ofn));
ofn.lStructSize = sizeof(ofn);
ofn.hwndOwner = m_hWnd;
ofn.lpstrFile = szFile;
// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
// use the contents of szFile to initialize itself.
ofn.lpstrFile[0] = '\0';
ofn.nMaxFile = sizeof(szFile);
ofn.lpstrFilter = "All\0*.*\0Avi\0*.AVI\0";
ofn.nFilterIndex = 1;
ofn.lpstrFileTitle = NULL;
ofn.nMaxFileTitle = 0;
ofn.lpstrInitialDir = NULL;
ofn.Flags = OFN_PATHMUSTEXIST ;

	// set filename, extension ".avi" is appended if necessary
	// timestamp code from http://stackoverflow.com/questions/1425227/how-to-create-files-named-with-current-time
	char timestamp[19];
	time_t rawtime = time(0);
	tm *now = localtime(&rawtime);

	if(rawtime != -1) 
     strftime(timestamp,18,"%Y%m%d_%H%M%S",now); 
	 
  ////////////////
	//AviGen.SetFileName(_T("test.avo"));
	if (GetSaveFileName(&ofn)==TRUE)
		AviGen.SetFileName(_T( ofn.lpstrFile ));

	else
		AviGen.SetFileName(_T( timestamp ));

	// retreiving size of image
	lpbih=AviGen.GetBitmapHeader();

	// allocating memory
	bmBits=new BYTE[lpbih->biSizeImage];

	hr=AviGen.InitEngine();
	if (FAILED(hr))
	{
		AfxMessageBox( AviGen.GetLastErrorMessage());
		goto Cleaning;
	}

	//Before starting, we have to initialize the input avi to first frame

	pView->frame=0;

	// reading back buffer
	glReadBuffer(GL_BACK);
	for(i=0;i<nFrames;i++)
	{
		// render frame
		pView->DrawGL();
		// Copy from OpenGL to buffer
		glReadPixels(0,0,lpbih->biWidth,lpbih->biHeight,GL_BGR_EXT,GL_UNSIGNED_BYTE,bmBits); 
		// send to avi engine
		hr=AviGen.AddFrame(bmBits);
		if (FAILED(hr))
		{
			AfxMessageBox( AviGen.GetLastErrorMessage());
			goto Cleaning;
		}
		Bar.StepIt();
	}

Cleaning:
	// releasing engine and memory
	AviGen.ReleaseEngine();
	delete[] bmBits;

	glReadBuffer(GL_FRONT);

	EndWaitCursor();
}
