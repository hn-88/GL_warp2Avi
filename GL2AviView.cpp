// GL2AviView.cpp : implementation of the CGL2AviView class
// from http://www.codeproject.com/Articles/1418/A-class-to-easily-generate-AVI-video-with-OpenGL-a
// with modifications by Hari Nandakumar www.saispace.in

#include "stdafx.h"
#include "GL2Avi.h"

#include "GL2AviDoc.h"
#include "GL2AviView.h"
#include <vfw.h>
#include <math.h>
//#include <GL/glut.h>
// removed, since glutswapbuffers crashes on my system

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGL2AviView

IMPLEMENT_DYNCREATE(CGL2AviView, CView)

BEGIN_MESSAGE_MAP(CGL2AviView, CView)
	//{{AFX_MSG_MAP(CGL2AviView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGL2AviView construction/destruction

CGL2AviView::CGL2AviView()
{
	// TODO: add construction code here

	// adding this from Nehe lesson 35
	frame=0;
	data=0;
	hdc = CreateCompatibleDC(0);	

	// adding this from lens.c
	// hard coded! hack!
	camera.screenwidth = 2048;
   camera.screenheight = 2048;

   // adding this from vlc-warp's vout_display_opengl_ReadMesh function
   // https://github.com/pdbourke/vlc-warp-2.1/
   // /modules/video_output/opengl.c

   FILE *input = NULL;

   input = fopen("EP_xyuv_1920.map", "r");
   // hard coded filename, hack

   /* Set rows and columns to 2 initially, as this is the size of the default mesh. */
    int dummy, rows = 2, cols = 2;

    if (input != NULL)  {
		fscanf(input, " %d %d %d ", &dummy, &cols, &rows) ;
		float x, y, u, v, l;
		meshrows=rows;
		meshcolumns=cols;

		// the following is adapted from the code at
		// http://paulbourke.net/dataformats/meshwarp/


		mesh		= (meshpoint*)calloc(rows*cols, sizeof(meshpoint));
		 
		//  (meshpoint*) explicit cast required for c++
		//	  have to do an explicit cast
		// since this is C++ and not C
	// http://cboard.cprogramming.com/windows-programming/69549-%27initializing%27-cannot-convert-%27void-*%27-%27int-*%27.html


	     for (int r = 0; r < rows ; r++) {
             for (int c = 0; c < cols ; c++) {
		                
                fscanf(input, "%f %f %f %f %f", &x, &y, &u, &v, &l) ;                 
                //   using the code adapted from 
				// http://paulbourke.net/dataformats/meshwarp/
				// and from vlc-warp
				//   We pack the values for each node into a 1d array.  
    
                mesh[cols*r+c].x = x;
                mesh[cols*r+c].y = y;
                mesh[cols*r+c].u = u;
                mesh[cols*r+c].v = v;
                mesh[cols*r+c].i = l;
 
				

			}
			 
		}
	}



}

CGL2AviView::~CGL2AviView()
{
	//adding this from Nehe lesson 35
	CloseAVI();		

	// freeing memory allocated
	free(mesh);
}

BOOL CGL2AviView::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style |=   WS_HSCROLL | WS_VSCROLL;
	// added | WS_HSCROLL | WS_VSCROLL to make gl window larger than screensize if needed
	// orig was cs.style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN ;

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CGL2AviView drawing

void CGL2AviView::OnDraw(CDC* pDC)
{
	CGL2AviDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// making gl context current
	BOOL bResult = wglMakeCurrent (pDC->m_hDC, m_hrc);
	if (!bResult)
	{
		TRACE("wglMakeCurrent Failed %x\r\n", GetLastError() ) ;
		return ;
	}


	DrawGL();  
	

	// Swap buffers.
	SwapBuffers(pDC->m_hDC) ;
}

/////////////////////////////////////////////////////////////////////////////
// CGL2AviView diagnostics

#ifdef _DEBUG
void CGL2AviView::AssertValid() const
{
	CView::AssertValid();
}

void CGL2AviView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CGL2AviDoc* CGL2AviView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGL2AviDoc)));
	return (CGL2AviDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CGL2AviView message handlers

int CGL2AviView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

		
	CClientDC dc(this) ;

	//
	// Fill in the pixel format descriptor.
	//
	PIXELFORMATDESCRIPTOR pfd ;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR)) ;
	pfd.nSize      = sizeof(PIXELFORMATDESCRIPTOR); 
	pfd.nVersion   = 1 ; 
	pfd.dwFlags    = PFD_DOUBLEBUFFER |
	                PFD_SUPPORT_OPENGL |
	                PFD_DRAW_TO_WINDOW ;
	pfd.iPixelType = PFD_TYPE_RGBA ;
	pfd.cColorBits = 16 ;
	pfd.cDepthBits = 32 ;
	pfd.cStencilBits = 8 ;
	pfd.iLayerType = PFD_MAIN_PLANE ;

	// Choosing avaible pixel format
   int nPixelFormat = ChoosePixelFormat(dc.m_hDC, &pfd);
   if (nPixelFormat == 0)
   {
      TRACE("ChoosePixelFormat Failed %d\r\n",GetLastError()) ;
      return -1 ;
   }
   TRACE("Pixel Format %d\r\n",nPixelFormat) ;

	// setting the pixel format
   BOOL bResult = SetPixelFormat (dc.m_hDC, nPixelFormat, &pfd);
   if (!bResult)
   {
      TRACE("SetPixelFormat Failed %d\r\n",GetLastError()) ;
      return -1 ;
   }

   //
   // Create rendering
   m_hrc = wglCreateContext(dc.m_hDC);
   if (!m_hrc)
   {
      TRACE("wglCreateContext Failed %x\r\n", GetLastError()) ;
      return -1;
   }

   CGL2AviApp* pApp=(CGL2AviApp*)AfxGetApp();
   pApp->SetIdleView(this);

	return 0;
}

void CGL2AviView::OnDestroy() 
{
	CView::OnDestroy();
	
	if (m_hrc)
	{
	   wglDeleteContext(m_hrc) ;
	   m_hrc = NULL ;
	}	
}

BOOL CGL2AviView::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;
}

void CGL2AviView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	ReSizeGLScene(cx,cy);

	InvalidateRect(NULL,FALSE);
}

int CGL2AviView::SetDCBITMAPPixelFormat(HDC hdc)
{
    static PIXELFORMATDESCRIPTOR pfd = {
        sizeof (PIXELFORMATDESCRIPTOR),             // Size of this structure
        1,                                          // Version number
        PFD_DRAW_TO_BITMAP |                        // Flags
        PFD_DRAW_TO_WINDOW |
        PFD_SUPPORT_OPENGL
        ,
        PFD_TYPE_RGBA,                              // RGBA pixel values
        16,                                         // 24-bit color
        0, 0, 0, 0, 0, 0,                           // Don't care about these
        0, 0,                                       // No alpha buffer
        0, 0, 0, 0, 0,                              // No accumulation buffer
        32,                                         // 32-bit depth buffer
        0,                                          // No stencil buffer
        0,                                          // No auxiliary buffers
        PFD_MAIN_PLANE,                             // Layer type
        0,                                          // Reserved (must be 0)
        0, 0, 0                                     // No layer masks
    };

    int nPixelFormat;
    
    nPixelFormat = ChoosePixelFormat (hdc, &pfd);
    if (SetPixelFormat(hdc, nPixelFormat, &pfd) == FALSE) {
      // SetPixelFormat error
      return FALSE ;
    }

    if (DescribePixelFormat(hdc, nPixelFormat, sizeof(PIXELFORMATDESCRIPTOR),&pfd) == 0) {
      // DescribePixelFormat error
      return FALSE ;
    }

    if (pfd.dwFlags & PFD_NEED_PALETTE) {
        // Need palete !
    }
    return TRUE ;
}

void CGL2AviView::ReSizeGLScene(GLsizei width, GLsizei height)
{
	if (height==0)										// Prevent A Divide By Zero By
	{
		height=1;										// Making Height Equal One
	}

	glViewport(0,0,width,height);						// Reset The Current Viewport
	//glViewport(0,0,1920,1080);							// hack - hard coding


	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
}

void CGL2AviView::InitGL()
{
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	 

	//below this is from NeHe lesson 35
	 

	// Start Of User Initialization
	angle		= 0.0f;											// Set Starting Angle To Zero
	hdd = DrawDibOpen();										// Grab A Device Context For Our Dib
	glClearColor (0.0f, 0.0f, 0.0f, 0.5f);						// Black Background
	glClearDepth (1.0f);										// Depth Buffer Setup
	glDepthFunc (GL_LEQUAL);									// The Type Of Depth Testing (Less Or Equal)
	glEnable(GL_DEPTH_TEST);									// Enable Depth Testing
	  
	quadratic=gluNewQuadric();									// Create A Pointer To The Quadric Object
	gluQuadricNormals(quadratic, GLU_SMOOTH);					// Create Smooth Normals 
	gluQuadricTexture(quadratic, GL_TRUE);						// Create Texture Coords 

	glEnable(GL_TEXTURE_2D);									// Enable Texture Mapping
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);	// Set Texture Max Filter
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);	// Set Texture Min Filter

	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);		// Set The Texture Generation Mode For S To Sphere Mapping
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);		// Set The Texture Generation Mode For T To Sphere Mapping

	///////////////////////////////////////////////////////////////////////////////
	// Open the avi file
		// open file name code
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

	 
  ////////////////
	 
	if (GetOpenFileName(&ofn)==TRUE)
		OpenAVI(_T( ofn.lpstrFile ));

	else
		OpenAVI("data/face2.avi");	
	
	// Code above opens The AVI File
	////////////////////////////////////////////////////////////////////////////////

	// Create The Texture
	
	GrabAVIFrame(frame); // adding this to initialize data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2048, 2048, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

	///////////////////////////
	// the below calls are from
	// lens.c 

	glEnable(GL_DEPTH_TEST);
   glDisable(GL_LINE_SMOOTH);
   glDisable(GL_POINT_SMOOTH);
   glDisable(GL_POLYGON_SMOOTH); 
   glDisable(GL_DITHER);
   glDisable(GL_CULL_FACE);
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

   glLineWidth(1.0);
   glPointSize(1.0);

   glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
   glFrontFace(GL_CW);
   glClearColor(0.0,0.0,0.0,0.0);
   glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
   glEnable(GL_COLOR_MATERIAL);
   glPixelStorei(GL_UNPACK_ALIGNMENT,1);

   
	 

}

BOOL CGL2AviView::DrawGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer

	// from NeHe lesson 35
	
	GrabAVIFrame(frame);										// Grab A Frame From The AVI

	glLoadIdentity();										// Reset The Modelview Matrix

	 
		// code below is from 
   // lens.c in HandleDisplay

	   int i,j;
   XYZ right,focus;
   unsigned int textureid;
   GLfloat white[4] = {1.0,1.0,1.0,1.0};


   /* Copy the image to be used as a texture */
   /* this is not needed for us, since we get the texture from avi frame from grabaviframe
   if ((thetex = (CGL2AviView::PIXELA *)malloc(camera.screenwidth*camera.screenheight*sizeof(PIXELA))) == NULL) {
      fprintf(stderr,"Failed to allocate memory for the texture\n");
      return FALSE;
   }*/
   
   //glReadPixels(0,0,camera.screenwidth,camera.screenheight,GL_RGBA,GL_UNSIGNED_BYTE,thetex);
   
   //glGenTextures(1,&textureid);
   
   //glBindTexture(GL_TEXTURE_2D,textureid);
   // this bindtexture seems to break the already loaded texture via grabaviframe
   
   glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
   glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); 
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
   
   /*glTexImage2D(GL_TEXTURE_2D,0,4,
      camera.screenwidth,camera.screenheight,
      0,GL_RGBA,GL_UNSIGNED_BYTE,thetex); */
   // in grabaviframe, it is
   // glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, 2048, 2048, GL_RGB, GL_UNSIGNED_BYTE, data);
   // in glTexImage2D, 3rd parameter is format of the data.
   // since target texture is already set, we comment this line out.
   
   glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
   //glDrawBuffer(GL_BACK_LEFT);
   //let's draw directly to front buffer, since glutSwapBuffers crashes my system
   
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   

   //glOrtho(-camera.screenwidth/2,camera.screenwidth/2,
   //   -camera.screenheight/2,camera.screenheight/2,1.0,10000.0);

   // we're changing this since the code in 
   // http://paulbourke.net/dataformats/meshwarp/
   // assumes x and y ranges 
   // "In the later case the horizontal range (x) will be +- the aspect ratio 
   // and the vertical range (y) will be +- 1 (ie: OpenGL style normalised coordinates)."


   glOrtho(-1.77778 , 1.77778 , -1.0 , 1.0 , 1.0 , 10000.0);

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   
   gluLookAt(0.0,0.0,1.0,0.0,0.0,0.0,0.0,1.0,0.0);
   glNormal3f(0.0,0.0,1.0);
   glColor3f(1.0,1.0,1.0);
   glDisable(GL_LIGHTING);
   glShadeModel(GL_SMOOTH); // changed to GL_FLAT to GL_SMOOTH to remove "tiling" effect
   glLightModelfv(GL_LIGHT_MODEL_AMBIENT,white);
   glPolygonMode(GL_FRONT_AND_BACK,GL_FILL); 
   //glEnable(GL_TEXTURE_2D);
   //glBindTexture(GL_TEXTURE_2D,textureid);
   CreateGrid();
   //glDisable(GL_TEXTURE_2D);
   //glutSwapBuffers(); 	
   // for some reason, drawing to back buffer and then calling
   // glutSwapBuffers crashes on my system
		 
		glFlush ();													// Flush The GL Rendering Pipeline

		frame++;

		if (frame>=lastframe)										// Are We At Or Past The Last Frame?
		{
			frame=0; // Reset The Frame Back To Zero (Start Of Video)
			 
		}										


	return TRUE;		
}

 



////////////////////

void CGL2AviView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();

	CClientDC dc(this);
	// making gl context current
	BOOL bResult = wglMakeCurrent (dc.m_hDC, m_hrc);
	if (!bResult)
	{
		TRACE("wglMakeCurrent Failed %x\r\n", GetLastError() ) ;
		return ;
	}
	
	InitGL();	
	m_rTri=0;
	m_rQuad=0;

	InvalidateRect(NULL,FALSE);
}

/// below from NeHe lesson 35

void CGL2AviView::flipIt(void* buffer)										// Flips The Red And Blue Bytes (1920x1080) hard coded dimensions! hack!
{
	void* b = buffer;											// Pointer To The Buffer
	__asm														// Assembler Code To Follow
	{
		mov ecx, 2048*2048										// Counter Set To Dimensions Of Our Memory Block
		mov ebx, b												// Points ebx To Our Data (b)
		label:													// Label Used For Looping
			mov al,[ebx+0]										// Loads Value At ebx Into al
			mov ah,[ebx+2]										// Loads Value At ebx+2 Into ah
			mov [ebx+2],al										// Stores Value In al At ebx+2
			mov [ebx+0],ah										// Stores Value In ah At ebx
			
			add ebx,3											// Moves Through The Data By 3 Bytes
			dec ecx												// Decreases Our Loop Counter
			jnz label											// If Not Zero Jump Back To Label
	}
}

void CGL2AviView::OpenAVI(LPCSTR szFile)										// Opens An AVI File (szFile)
{
	//TCHAR	title[100];											// Will Hold The Modified Window Title

	AVIFileInit();												// Opens The AVIFile Library

	// Opens The AVI Stream
	if (AVIStreamOpenFromFile(&pavi, szFile, streamtypeVIDEO, 0, OF_READ, NULL) !=0)
	{
		// An Error Occurred Opening The Stream
		//MessageBox (HWND_DESKTOP, "Failed To Open The AVI Stream", "Error", MB_OK | MB_ICONEXCLAMATION);
		MessageBox ( "Failed To Open The AVI Stream");

	}

	AVIStreamInfo(pavi, &psi, sizeof(psi));						// Reads Information About The Stream Into psi
	width=psi.rcFrame.right-psi.rcFrame.left;					// Width Is Right Side Of Frame Minus Left
	height=psi.rcFrame.bottom-psi.rcFrame.top;					// Height Is Bottom Of Frame Minus Top

	lastframe=AVIStreamLength(pavi);							// The Last Frame Of The Stream

	mpf=AVIStreamSampleToTime(pavi,lastframe)/lastframe;		// Calculate Rough Milliseconds Per Frame

	bmih.biSize = sizeof (BITMAPINFOHEADER);					// Size Of The BitmapInfoHeader
	bmih.biPlanes = 1;											// Bitplanes	
	bmih.biBitCount = 24;										// Bits Format We Want (24 Bit, 3 Bytes)
	bmih.biWidth = 2048;											// Width We Want  
	bmih.biHeight = 2048;										// Height We Want 
	// hard coded output dimensions - hack!!
	bmih.biCompression = BI_RGB;								// Requested Mode = RGB

	hBitmap = CreateDIBSection (hdc, (BITMAPINFO*)(&bmih), DIB_RGB_COLORS, (void**)(&data), NULL, NULL);
	SelectObject (hdc, hBitmap);								// Select hBitmap Into Our Device Context (hdc)

	// from http://forum.doom9.org/showthread.php?t=69601
	// second param can't be null for xvid
	//pgf=AVIStreamGetFrameOpen(pavi, NULL);						// Create The PGETFRAME	Using Our Request Mode

	// sample code from
	// http://us.generation-nt.com/answer/avistreamgetframeopen-help-26357342.html
	 

	pgf = AVIStreamGetFrameOpen( pavi, LPBITMAPINFOHEADER (AVIGETFRAMEF_BESTDISPLAYFMT) );

	if (pgf==NULL)
	{
		// An Error Occurred Opening The Frame
		//MessageBox (HWND_DESKTOP, "Failed To Open The AVI Frame", "Error", MB_OK | MB_ICONEXCLAMATION);
		MessageBox (  "Failed To Open The AVI Frame" );
	}

	 
}

void CGL2AviView::GrabAVIFrame(int frame)									// Grabs A Frame From The Stream
{
	LPBITMAPINFOHEADER lpbi;									// Holds The Bitmap Header Information
	lpbi = (LPBITMAPINFOHEADER)AVIStreamGetFrame(pgf, frame);	// Grab Data From The AVI Stream
	pdata=(char *)lpbi+lpbi->biSize+lpbi->biClrUsed * sizeof(RGBQUAD);	// Pointer To Data Returned By AVIStreamGetFrame

	// Convert Data To Requested Bitmap Format
	// hard coded output dimensions - hack!
	DrawDibDraw (hdd, hdc, 0, 0, 2048, 2048, lpbi, pdata, 0, 0, width, height, 0);

	flipIt(data);												// Swap The Red And Blue Bytes (GL Compatability)

	// Update The Texture
	glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, 2048, 2048, GL_RGB, GL_UNSIGNED_BYTE, data);
}

void CGL2AviView::CloseAVI(void)												// Properly Closes The Avi File
{
	DeleteObject(hBitmap);										// Delete The Device Dependant Bitmap Object
	DrawDibClose(hdd);											// Closes The DrawDib Device Context
	AVIStreamGetFrameClose(pgf);								// Deallocates The GetFrame Resources
	AVIStreamRelease(pavi);										// Release The Stream
	AVIFileExit();												// Release The File
}


 

// the following is from 
// Paul Bourke's lens.c

/*
   Form the grid with distorted texture coordinates
*/
void CGL2AviView::CreateGrid(void)
{
   int i,j;
      
   /*
   using the code from 
   http://paulbourke.net/dataformats/meshwarp/ */
   
   int nx = meshcolumns;
   int ny = meshrows;
   // here, the mesh[i][j].i etc are translated into the
   // mesh[cols*r+c] variable etc in the 1d array
   //  
   //   mesh[i][j].i <===> mesh[nx*j+i].i
   // Thanks, Paul!
     
   glBegin(GL_QUADS);
   for (i=0;i<nx-1;i++) {
      for (j=0;j<ny-1;j++) {
         if (mesh[nx*j+i].i < 0 || mesh[(nx*(j+1))+i].i < 0 || mesh[(nx*(j+1))+(i+1)].i < 0 || mesh[nx*j+i+1].i < 0)
            continue;

         glColor3f(mesh[nx*j+i].i,mesh[nx*j+i].i,mesh[nx*j+i].i);
         glTexCoord2f(mesh[nx*j+i].u,mesh[nx*j+i].v);
         glVertex3f(mesh[nx*j+i].x,mesh[nx*j+i].y,0.0);

         glColor3f(mesh[nx*j+i+1].i,mesh[nx*j+i+1].i,mesh[nx*j+i+1].i);
         glTexCoord2f(mesh[nx*j+i+1].u,mesh[nx*j+i+1].v); 
         glVertex3f(mesh[nx*j+i+1].x,mesh[nx*j+i+1].y,0.0);

         glColor3f(mesh[nx*(j+1)+i+1].i,mesh[nx*(j+1)+i+1].i,mesh[nx*(j+1)+i+1].i);
         glTexCoord2f(mesh[nx*(j+1)+i+1].u,mesh[nx*(j+1)+i+1].v);
         glVertex3f(mesh[nx*(j+1)+i+1].x,mesh[nx*(j+1)+i+1].y,0.0);

         glColor3f(mesh[nx*(j+1)+i].i,mesh[nx*(j+1)+i].i,mesh[nx*(j+1)+i].i);
         glTexCoord2f(mesh[nx*(j+1)+i].u,mesh[nx*(j+1)+i].v);
         glVertex3f(mesh[nx*(j+1)+i].x,mesh[nx*(j+1)+i].y,0.0);

       }
   }
   /* testing code for checking if glortho has correct params
    glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.77778f,  1.0f, -0.0f);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.77778f,  1.0f, -0.0f);
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.77778f, -1.0f, -0.0f);
	glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.77778f, -1.0f, -0.0f);
	*/

   
   glEnd();
}

/*
   Calculate the distorted grid coordinates
*/
void CGL2AviView::Transform(int i,int j,double *ix,double *iy)
{
   double x,y,xnew,ynew;
   double r,theta,rnew,thetanew;

   x = i / (camera.screenwidth/2.0) - 1;
   y = j / (camera.screenwidth/2.0) - 1;
   r = sqrt(x*x+y*y);
   theta = atan2(y,x);
    
      xnew = sin(PID2*x);
      ynew = sin(PID2*y);
       
   *ix = (xnew + 1) / 2.0;
   *iy = (ynew + 1) / 2.0;

   //testing

   *ix = x ;
   *iy= y  ;
        
}

