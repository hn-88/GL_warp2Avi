// GL2AviView.h : interface of the CGL2AviView class
//
/////////////////////////////////////////////////////////////////////////////
// from http://www.codeproject.com/Articles/1418/A-class-to-easily-generate-AVI-video-with-OpenGL-a
// with modifications February 2014 by Hari Nandakumar www.saispace.in
// 

#if !defined(AFX_GL2AVIVIEW_H__D7A22D25_D7B5_48AD_9F1A_92BE5451CF12__INCLUDED_)
#define AFX_GL2AVIVIEW_H__D7A22D25_D7B5_48AD_9F1A_92BE5451CF12__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vfw.h>

class CGL2AviView : public CView
{
protected: // create from serialization only
	CGL2AviView();
	DECLARE_DYNCREATE(CGL2AviView)
	BOOL DrawGL();
// Attributes
public:
	CGL2AviDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGL2AviView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CGL2AviView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	GLfloat m_rQuad;
	GLfloat m_rTri;

	void InitGL();
	void ReSizeGLScene(GLsizei width, GLsizei height);
	HGLRC m_hrc;
	//{{AFX_MSG(CGL2AviView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	int SetDCBITMAPPixelFormat(HDC hdc);

// code below is from the NeHe Lesson 35
	 

// User Defined Variables
public:
float		angle;												// Used For Rotation
int			next;												// Used For Animation
int			frame; //=0;											// Frame Counter
int			effect;												// Current Effect
bool		sp;													// Space Bar Pressed?
//bool		env; //=TRUE;											// Environment Mapping (Default On)
bool		ep;													// 'E' Pressed?
//bool		bg; //=TRUE;											// Background (Default On)
bool		bp;													// 'B' Pressed?

AVISTREAMINFO		psi;										// Pointer To A Structure Containing Stream Info
PAVISTREAM			pavi;										// Handle To An Open Stream
PGETFRAME			pgf;										// Pointer To A GetFrame Object
BITMAPINFOHEADER	bmih;										// Header Information For DrawDibDraw Decoding
long				lastframe;									// Last Frame Of The Stream
int					width;										// Video Width
int					height;										// Video Height
char				*pdata;										// Pointer To Texture Data
int					mpf;										// Will Hold Rough Milliseconds Per Frame

GLUquadricObj *quadratic;										// Storage For Our Quadratic Objects

HDRAWDIB hdd;													// Handle For Our Dib
HBITMAP hBitmap;												// Handle To A Device Dependant Bitmap
HDC hdc; //= CreateCompatibleDC(0);								// Creates A Compatible Device Context
unsigned char* data; //= 0;										// Pointer To Our Resized Image

// all initializations have been moved to the class constructor.

void flipIt(void* buffer);										// Flips The Red And Blue Bytes (256x256)
void CloseAVI(void);
void GrabAVIFrame(int frame);
void OpenAVI(LPCSTR szFile);

//
// adding this for adapting vlc-warp code into GL2AviView constructor
GLfloat *coords;
GLfloat *uv;
GLfloat *intensity;

int meshrows, meshcolumns;
 
///////////////////////
// the following is from
// Paul Bourke's lens.c and lens.h

typedef struct {
   double x,y,z;
} XYZ;
typedef struct {
   double r,g,b;
} COLOUR;
typedef struct {
   unsigned char r,g,b,a;
} PIXELA;

typedef struct {
   XYZ vp;              /* View position           */
   XYZ vd;              /* View direction vector   */
   XYZ vu;              /* View up direction       */
   XYZ pr;              /* Point to rotate about   */
   double focallength;  /* Focal Length along vd   */
   double aperture;     /* Camera aperture         */
   double eyesep;       /* Eye separation          */
	int screenheight,screenwidth;
} CAMERA;

void CreateGrid(void);
void Transform(int,int,double *,double *);
void Normalise(XYZ *);

CAMERA camera;
//XYZ origin = {0.0,0.0,0.0};

PIXELA *thetex;

#define DTOR            0.0174532925
#define RTOD            57.2957795
#define TWOPI           6.283185307179586476925287
#define PI              3.141592653589793238462643
#define PID2            1.570796326794896619231322

// the following is adapted from the code at
// http://paulbourke.net/dataformats/meshwarp/

typedef struct {
   GLfloat x,y,u,v,i;
} meshpoint;

meshpoint *mesh;

};

#ifndef _DEBUG  // debug version in GL2AviView.cpp
inline CGL2AviDoc* CGL2AviView::GetDocument()
   { return (CGL2AviDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GL2AVIVIEW_H__D7A22D25_D7B5_48AD_9F1A_92BE5451CF12__INCLUDED_)
