// dibview.cpp : implementation of the CDibView class
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1998 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"
#include "diblook.h"

#include "dibdoc.h"
#include "dibview.h"
#include "dibapi.h"
#include "mainfrm.h"

#include "math.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define PI 3.14159265358979323846264338327950

#define BEGIN_PROCESSING() INCEPUT_PRELUCRARI()

#define END_PROCESSING(Title) SFARSIT_PRELUCRARI(Title)

#define INCEPUT_PRELUCRARI() \
	CDibDoc* pDocSrc=GetDocument();										\
	CDocTemplate* pDocTemplate=pDocSrc->GetDocTemplate();				\
	CDibDoc* pDocDest=(CDibDoc*) pDocTemplate->CreateNewDocument();		\
	BeginWaitCursor();													\
	HDIB hBmpSrc=pDocSrc->GetHDIB();									\
	HDIB hBmpDest = (HDIB)::CopyHandle((HGLOBAL)hBmpSrc);				\
	if ( hBmpDest==0 ) {												\
		pDocTemplate->RemoveDocument(pDocDest);							\
		return;															\
	}																	\
	BYTE* lpD = (BYTE*)::GlobalLock((HGLOBAL)hBmpDest);					\
	BYTE* lpS = (BYTE*)::GlobalLock((HGLOBAL)hBmpSrc);					\
	int iColors = DIBNumColors((char *)&(((LPBITMAPINFO)lpD)->bmiHeader)); \
	RGBQUAD *bmiColorsDst = ((LPBITMAPINFO)lpD)->bmiColors;	\
	RGBQUAD *bmiColorsSrc = ((LPBITMAPINFO)lpS)->bmiColors;	\
	BYTE * lpDst = (BYTE*)::FindDIBBits((LPSTR)lpD);	\
	BYTE * lpSrc = (BYTE*)::FindDIBBits((LPSTR)lpS);	\
	int dwWidth  = ::DIBWidth((LPSTR)lpS);\
	int dwHeight = ::DIBHeight((LPSTR)lpS);\
	int w=WIDTHBYTES(dwWidth*((LPBITMAPINFOHEADER)lpS)->biBitCount);	\

#define BEGIN_SOURCE_PROCESSING \
	CDibDoc* pDocSrc=GetDocument();										\
	BeginWaitCursor();													\
	HDIB hBmpSrc=pDocSrc->GetHDIB();									\
	BYTE* lpS = (BYTE*)::GlobalLock((HGLOBAL)hBmpSrc);					\
	int iColors = DIBNumColors((char *)&(((LPBITMAPINFO)lpS)->bmiHeader)); \
	RGBQUAD *bmiColorsSrc = ((LPBITMAPINFO)lpS)->bmiColors;	\
	BYTE * lpSrc = (BYTE*)::FindDIBBits((LPSTR)lpS);	\
	int dwWidth  = ::DIBWidth((LPSTR)lpS);\
	int dwHeight = ::DIBHeight((LPSTR)lpS);\
	int w=WIDTHBYTES(dwWidth*((LPBITMAPINFOHEADER)lpS)->biBitCount);	\
	


#define END_SOURCE_PROCESSING	\
	::GlobalUnlock((HGLOBAL)hBmpSrc);								\
    EndWaitCursor();												\
/////////////////////////////////////////////////////////////////////////////


//---------------------------------------------------------------
#define SFARSIT_PRELUCRARI(Titlu)	\
	::GlobalUnlock((HGLOBAL)hBmpDest);								\
	::GlobalUnlock((HGLOBAL)hBmpSrc);								\
    EndWaitCursor();												\
	pDocDest->SetHDIB(hBmpDest);									\
	pDocDest->InitDIBData();										\
	pDocDest->SetTitle((LPCSTR)Titlu);									\
	CFrameWnd* pFrame=pDocTemplate->CreateNewFrame(pDocDest,NULL);	\
	pDocTemplate->InitialUpdateFrame(pFrame,pDocDest);
/////////////////////////////////////////////////////////////////////////////
// CDibView

IMPLEMENT_DYNCREATE(CDibView, CScrollView)

BEGIN_MESSAGE_MAP(CDibView, CScrollView)
	//{{AFX_MSG_MAP(CDibView)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_MESSAGE(WM_DOREALIZE, OnDoRealize)
	ON_COMMAND(ID_PROCESSING_PARCURGERESIMPLA, OnProcessingParcurgereSimpla)
	//}}AFX_MSG_MAP

	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CScrollView::OnFilePrintPreview)
	ON_COMMAND(ID_PROCESSING_RANSACLINE, &CDibView::OnProcessingRansacline)
	ON_COMMAND(ID_PROCESSING_RANSACCERC, &CDibView::OnProcessingRansaccerc)
	ON_COMMAND(ID_HOUGHLINIE_STEP1, &CDibView::OnHoughlinieStep1)
	ON_COMMAND(ID_HOUGHLINIE_STEP2, &CDibView::OnHoughlinieStep2)
	ON_COMMAND(ID_HOUGHLINIE_STEP3, &CDibView::OnHoughlinieStep3)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDibView construction/destruction

CDibView::CDibView()
{
}

CDibView::~CDibView()
{
}

/////////////////////////////////////////////////////////////////////////////
// CDibView drawing

void CDibView::OnDraw(CDC* pDC)
{
	CDibDoc* pDoc = GetDocument();

	HDIB hDIB = pDoc->GetHDIB();
	if (hDIB != NULL)
	{
		LPSTR lpDIB = (LPSTR) ::GlobalLock((HGLOBAL) hDIB);
		int cxDIB = (int) ::DIBWidth(lpDIB);         // Size of DIB - x
		int cyDIB = (int) ::DIBHeight(lpDIB);        // Size of DIB - y
		::GlobalUnlock((HGLOBAL) hDIB);
		CRect rcDIB;
		rcDIB.top = rcDIB.left = 0;
		rcDIB.right = cxDIB;
		rcDIB.bottom = cyDIB;
		CRect rcDest;
		if (pDC->IsPrinting())   // printer DC
		{
			// get size of printer page (in pixels)
			int cxPage = pDC->GetDeviceCaps(HORZRES);
			int cyPage = pDC->GetDeviceCaps(VERTRES);
			// get printer pixels per inch
			int cxInch = pDC->GetDeviceCaps(LOGPIXELSX);
			int cyInch = pDC->GetDeviceCaps(LOGPIXELSY);

			//
			// Best Fit case -- create a rectangle which preserves
			// the DIB's aspect ratio, and fills the page horizontally.
			//
			// The formula in the "->bottom" field below calculates the Y
			// position of the printed bitmap, based on the size of the
			// bitmap, the width of the page, and the relative size of
			// a printed pixel (cyInch / cxInch).
			//
			rcDest.top = rcDest.left = 0;
			rcDest.bottom = (int)(((double)cyDIB * cxPage * cyInch)
					/ ((double)cxDIB * cxInch));
			rcDest.right = cxPage;
		}
		else   // not printer DC
		{
			rcDest = rcDIB;
		}
		::PaintDIB(pDC->m_hDC, &rcDest, pDoc->GetHDIB(),
			&rcDIB, pDoc->GetDocPalette());
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDibView printing

BOOL CDibView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

/////////////////////////////////////////////////////////////////////////////
// CDibView commands


LRESULT CDibView::OnDoRealize(WPARAM wParam, LPARAM)
{
	ASSERT(wParam != NULL);
	CDibDoc* pDoc = GetDocument();
	if (pDoc->GetHDIB() == NULL)
		return 0L;  // must be a new document

	CPalette* pPal = pDoc->GetDocPalette();
	if (pPal != NULL)
	{
		CMainFrame* pAppFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
		ASSERT_KINDOF(CMainFrame, pAppFrame);

		CClientDC appDC(pAppFrame);
		// All views but one should be a background palette.
		// wParam contains a handle to the active view, so the SelectPalette
		// bForceBackground flag is FALSE only if wParam == m_hWnd (this view)
		CPalette* oldPalette = appDC.SelectPalette(pPal, ((HWND)wParam) != m_hWnd);

		if (oldPalette != NULL)
		{
			UINT nColorsChanged = appDC.RealizePalette();
			if (nColorsChanged > 0)
				pDoc->UpdateAllViews(NULL);
			appDC.SelectPalette(oldPalette, TRUE);
		}
		else
		{
			TRACE0("\tSelectPalette failed in CDibView::OnPaletteChanged\n");
		}
	}

	return 0L;
}

void CDibView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();
	ASSERT(GetDocument() != NULL);

	SetScrollSizes(MM_TEXT, GetDocument()->GetDocSize());
}


void CDibView::OnActivateView(BOOL bActivate, CView* pActivateView,
					CView* pDeactiveView)
{
	CScrollView::OnActivateView(bActivate, pActivateView, pDeactiveView);

	if (bActivate)
	{
		ASSERT(pActivateView == this);
		OnDoRealize((WPARAM)m_hWnd, 0);   // same as SendMessage(WM_DOREALIZE);
	}
}

void CDibView::OnEditCopy()
{
	CDibDoc* pDoc = GetDocument();
	// Clean clipboard of contents, and copy the DIB.

	if (OpenClipboard())
	{
		BeginWaitCursor();
		EmptyClipboard();
		SetClipboardData (CF_DIB, CopyHandle((HANDLE) pDoc->GetHDIB()) );
		CloseClipboard();
		EndWaitCursor();
	}
}



void CDibView::OnUpdateEditCopy(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetDocument()->GetHDIB() != NULL);
}


void CDibView::OnEditPaste()
{
	HDIB hNewDIB = NULL;

	if (OpenClipboard())
	{
		BeginWaitCursor();

		hNewDIB = (HDIB) CopyHandle(::GetClipboardData(CF_DIB));

		CloseClipboard();

		if (hNewDIB != NULL)
		{
			CDibDoc* pDoc = GetDocument();
			pDoc->ReplaceHDIB(hNewDIB); // and free the old DIB
			pDoc->InitDIBData();    // set up new size & palette
			pDoc->SetModifiedFlag(TRUE);

			SetScrollSizes(MM_TEXT, pDoc->GetDocSize());
			OnDoRealize((WPARAM)m_hWnd,0);  // realize the new palette
			pDoc->UpdateAllViews(NULL);
		}
		EndWaitCursor();
	}
}


void CDibView::OnUpdateEditPaste(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(::IsClipboardFormatAvailable(CF_DIB));
}

void CDibView::OnProcessingParcurgereSimpla() 
{
	// TODO: Add your command handler code here
	BEGIN_PROCESSING();

	
	// Makes a grayscale image by equalizing the R, G, B components from the LUT
	for (int k=0;  k < iColors ; k++)
		bmiColorsDst[k].rgbRed=bmiColorsDst[k].rgbGreen=bmiColorsDst[k].rgbBlue=k;

	
	//  Goes through the bitmap pixels and performs their negative	
	for (int i=0;i<dwHeight;i++)
		for (int j=0;j<dwWidth;j++)
		  {	
			lpDst[i*w+j]= 255 - lpSrc[i*w+j]; //makes image negative
	  }


	END_PROCESSING("Operation name");
}



void CDibView::OnProcessingRansacline()
{
	// TODO: Add your command handler code here
	BEGIN_PROCESSING();
	
	int t = 10; //prag de validitate pentru distanta unui punct
	double p = 0.99; //probabilitatea ca cel putin unul din esantioane nu contine zgomot
	double vv = 0.3; //probabilitatea ca orice punct selectat este valid => epsilon = 1 - w
	int s = 2; // cardinalitatea setului = 2 puncte determina o linie

	int N; // numarul de iteratii
	int T; //prag - nr de puncte care satisfac modelul

	double a, b, c; //parametrii unei drepte
	double d; //distanta unui punct la o dreapta

	int pct_x[150], pct_y[150]; //coordonatele punctelor gasite
	int nr_pct = 0; //contor pt puncte

	// determinare puncte
	for (int i=0;i<dwHeight;i++) {
		for (int j=0;j<dwWidth;j++) {	
			if(lpSrc[i*w+j] == 0) {
				pct_x[nr_pct] = j;
				pct_y[nr_pct++] = i;
			}
		}
	}

	int rd1, rd2; // random numbers
	N = (int) log(1-p)/log(1-pow(vv, s));
	T = (int) (vv*nr_pct);

	int max_cnt = 0; //numarul maxim de puncte potrivite
	int pct1, pct2; // punctele alese

	// incepere algoritm
	for(int i=0; i<N; ++i) {
		//alegere 2 puncte random
		rd1 = rand() % nr_pct;
		do {
			rd2 = rand() % nr_pct;
		}while(rd2 == rd1);

		//ecuatia liniei care trece prin cele doua puncte
		a = pct_y[rd1] - pct_y[rd2];
		b = pct_x[rd2] - pct_x[rd1];
		c = pct_x[rd1]*pct_y[rd2] - pct_x[rd2]*pct_y[rd1];

		// calculare distanta de la puncte la dreapta obtinuta si numarare puncte
		double numitor = sqrt(pow(a, 2) + pow(b, 2));
		if(numitor == 0) {
			numitor = 10e-7;
		}
		int cnt = 0;
		for(int j=0; j<nr_pct; ++j) {
			if(j != rd1 && j != rd2) {
				d = abs(a*pct_x[j] + b*pct_y[j] + c)/numitor;
				if(d < t) {
					++cnt;
				}
			}
			
		}
		// verificare prag T - numarul necesar de puncte
		if(cnt >= T) {
			max_cnt = cnt;
			pct1 = rd1;
			pct2 = rd2;
			i = N;
		} else if(cnt > max_cnt) {
			max_cnt = cnt;
			pct1 = rd1;
			pct2 = rd2;
		}
	}
	

CDC dc;
dc.CreateCompatibleDC(0);
CBitmap ddBitmap;
HBITMAP hDDBitmap = CreateDIBitmap(::GetDC(0),
&((LPBITMAPINFO)lpS)->bmiHeader, CBM_INIT, lpSrc,
(LPBITMAPINFO)lpS, DIB_RGB_COLORS);
ddBitmap.Attach(hDDBitmap);
CBitmap* pTempBmp = dc.SelectObject(&ddBitmap);
CPen pen(PS_SOLID, 1, RGB(255,0,0));
CPen *pTempPen = dc.SelectObject(&pen);
// drawing a line from point (x1,y1) to point (x2,y2)
int x1=pct_x[pct1];
int y1=pct_y[pct1];
int x2=pct_x[pct2];
int y2=pct_y[pct2];
dc.MoveTo(x1,dwHeight-1-y1);
dc.LineTo(x2,dwHeight-1-y2);
dc.SelectObject(pTempPen);
dc.SelectObject(pTempBmp); 
GetDIBits(dc.m_hDC, ddBitmap, 0, dwHeight, lpDst,
(LPBITMAPINFO)lpD, DIB_RGB_COLORS);


	END_PROCESSING("RANSAC line");
}



void CDibView::OnProcessingRansaccerc()
{
	BEGIN_PROCESSING();
	
	int t = 10; //prag de validitate pentru distanta unui punct
	double p = 0.99; //probabilitatea ca cel putin unul din esantioane nu contine zgomot
	double vv = 0.5; //probabilitatea ca orice punct selectat este valid => epsilon = 1 - w
	int s = 3; // cardinalitatea setului = 3 puncte determina un cerc

	int N; // numarul de iteratii
	int T; //prag - nr de puncte care satisfac modelul

	double ma, mb; //pantele dreptelor
	double d; //distanta unui punct la un cerc
	int xc, yc; //coordonatele centrului cercului
	double raza; //raza cercului

	int pct_x[150], pct_y[150]; //coordonatele punctelor gasite
	int nr_pct = 0; //contor pt puncte

	// determinare puncte
	for (int i=0;i<dwHeight;i++) {
		for (int j=0;j<dwWidth;j++) {	
			if(lpSrc[i*w+j] == 0) {
				pct_x[nr_pct] = j;
				pct_y[nr_pct++] = i;
			}
		}
	}

	int rd1, rd2, rd3; // random numbers
	N = (int) log(1-p)/log(1-pow(vv, s));
	T = (int) (vv*nr_pct);

	int max_cnt = 0; //numarul maxim de puncte potrivite
	int Xc, Yc, R; // punctele alese

	// incepere algoritm
	for(int i=0; i<N; ++i) {
		//alegere 3 puncte random
		rd1 = rand() % nr_pct;
		do {
			rd2 = rand() % nr_pct;
		}while(rd2 == rd1);
		do {
			rd3 = rand() % nr_pct;
		}while(rd3 == rd1 || rd3 == rd2);


		//ecuatia pantei liniei care trece prin fiecare doua puncte
		ma = (pct_y[rd2] - pct_y[rd1])/(pct_x[rd2] - pct_x[rd1] + 10e-7);
		mb = (pct_y[rd3] - pct_y[rd2])/(pct_x[rd3] - pct_x[rd2] + 10e-7);

		// calculare centrul cercului
		xc = (int) ((ma*mb*(pct_y[rd1]-pct_y[rd3])+mb*(pct_x[rd1]+pct_x[rd2])-ma*(pct_x[rd2]+pct_x[rd3]))/(2*(mb-ma)+10e-7));
		yc = (int) (((pct_x[rd1]+pct_x[rd2])/2-xc)/(ma+10e-7) + (pct_y[rd1]+pct_y[rd2])/2);
		//calculare raza
		raza = sqrt(pow((double)(xc-pct_x[rd1]), 2)+pow((double)(yc-pct_y[rd1]), 2));

		// calculare distanta de la puncte la cercul obtinut si numarare puncte
		int cnt = 0;
		for(int j=0; j<nr_pct; ++j) {
			if(j != rd1 && j != rd2) {
				d = abs(sqrt(pow((double)(xc-pct_x[j]), 2)+pow((double)(yc-pct_y[j]), 2)) - raza);
				if(d < t) {
					++cnt;
				}
			}
			
		}
		// verificare prag T - numarul necesar de puncte
		if(cnt >= T) {
			max_cnt = cnt;
			Xc = xc;
			Yc = yc;
			R = raza;
			i = N;
		} else if(cnt > max_cnt) {
			max_cnt = cnt;
			Xc = xc;
			Yc = yc;
			R = raza;
		}
	}


	/* Drawing a circle on the image */
CDC dc;
dc.CreateCompatibleDC(0);
CBitmap ddBitmap;
HBITMAP hDDBitmap = CreateDIBitmap(::GetDC(0),
&((LPBITMAPINFO)lpS)->bmiHeader, CBM_INIT, lpSrc,
(LPBITMAPINFO)lpS, DIB_RGB_COLORS);
ddBitmap.Attach(hDDBitmap);
CBitmap* pTempBmp = dc.SelectObject(&ddBitmap);
CPen pen(PS_SOLID, 1, RGB(255,0,0));
CPen *pTempPen = dc.SelectObject(&pen);
/* draw a circle having radius r and center a point of
coordinates (x,y)*/
int x = Xc;
int y = Yc;
int r = R;
dc.MoveTo ( (int)(x + r), dwHeight-1-y );
dc.AngleArc(x, dwHeight-1-y, r, 0, 360);
dc.SelectObject(pTempPen);
dc.SelectObject(pTempBmp);
GetDIBits(dc.m_hDC, ddBitmap, 0, dwHeight, lpDst,
(LPBITMAPINFO)lpD, DIB_RGB_COLORS);

	END_PROCESSING("RANSAC cerc");
}

int *H;
int ro_min = 0, ro_max, d_ro = 1;
int teta_min = 0, teta_max, d_teta = 1;

void CDibView::OnHoughlinieStep1()
{
		// TODO: Add your command handler code here
	BEGIN_PROCESSING();

	ro_max = (int) (sqrt(pow((float)dwWidth, 2)+pow((float)dwHeight, 2)));
	teta_max = 360;

	int sizeH = ro_max*teta_max;
	H = new int[sizeH];
	memset(H, 0, sizeH*sizeof(int));

	
	// incepere algoritm
	for (int y=0; y<dwHeight; y++) {
		for (int x=0; x<dwWidth; x++) {	
			if(lpSrc[y*w+x] == 255) {
				for (int teta=teta_min; teta<teta_max; teta+=d_teta) {
					double teta_rad = (teta*PI)/180;
					int ro = (int)(x*cos(teta_rad) + y*sin(teta_rad));
					if (ro>=ro_min && ro<=ro_max) {
						H[ro*teta_max+teta]++;
					}
				}
			}
		}
	}


	END_PROCESSING("First step finished!");
}

void CDibView::OnHoughlinieStep2()
{
	BEGIN_PROCESSING();

	// calculare valoare maxima
	int sizeH = ro_max * teta_max;
	int max_val = 0;
	for (int i=0; i<sizeH; ++i) {
		if (H[i]>max_val) {
			max_val = H[i];
		}
	}
	// normalizare acumulator
	double norm_factor = 255./max_val;
	for (int i=0; i<dwWidth*dwHeight; ++i) {
		lpDst[i] = (int) (H[i]*norm_factor);
	}

	END_PROCESSING("Second step finished!");
}

void CDibView::OnHoughlinieStep3()
{
	// TODO: Add your command handler code here
	BEGIN_PROCESSING();

	// determinare maxime locale
	int maxime_val[15000];
	int maxime_ro[15000];
	int maxime_teta[15000];
	int contor = 0;
	int max_val = 0;
	int min_val = 2000000;
	int k = 1;
	for (int ro=(ro_min+k); ro<(ro_max-k); ro+=d_ro) {
		for (int teta=(teta_min+k); teta<(teta_max-k); teta+=d_teta) {
			// verificare pe fereastra de 3x3
			bool e_max = true;
			for (int i=-k; i<=k; ++i) {
				for (int j=-k; j<=k; ++j) {
					if (H[ro*teta_max+teta]<H[(ro+i)*teta_max+(teta+j)]) {
						e_max = false;
						i = j = k;
					}
				}
			}
			if (e_max && H[ro*teta_max+teta]) {
				maxime_ro[contor] = ro;
				maxime_teta[contor] = teta;
				maxime_val[contor++] = H[ro*teta_max+teta];
				if (max_val<H[ro*teta_max+teta]) {
					max_val = H[ro*teta_max+teta];
				} else if (min_val>H[ro*teta_max+teta]){
					min_val = H[ro*teta_max+teta];
				}
			}
		}
	}

	int avg ;
	int cnt ;
	bool done = false;
	do {
		cnt = 0;
		avg = (int)((max_val + min_val)/2);
		for (int i=0; i<contor; ++i) {
			if (maxime_val[i]>=avg) {
				++cnt;
				if (cnt>20) {
					i=contor;
				}
			}
		}

		if (cnt>20) {
			min_val = avg;
		} else if (cnt<10) {
			max_val = avg;
		} else {
			done = true;
		}
		
	} while(!done);

	CDC dc;
	dc.CreateCompatibleDC(0);
	CBitmap ddBitmap;
	HBITMAP hDDBitmap = CreateDIBitmap(::GetDC(0),
	&((LPBITMAPINFO)lpS)->bmiHeader, CBM_INIT, lpSrc,
	(LPBITMAPINFO)lpS, DIB_RGB_COLORS);
	ddBitmap.Attach(hDDBitmap);
	CBitmap* pTempBmp = dc.SelectObject(&ddBitmap);
	CPen pen(PS_SOLID, 1, RGB(255,0,0));
	CPen *pTempPen = dc.SelectObject(&pen);
			
	int x1=0;
	int x2=dwWidth;
	for (int i=0; i<contor; ++i) {
		if (maxime_val[i]>avg) {
			// drawing a line from point (x1,y1) to point (x2,y2)
			double teta_rad = (maxime_teta[i]*PI)/180; 
			int y1=(int)((maxime_ro[i]-x1*cos(teta_rad))/(sin(teta_rad)));
			int y2=(int)((maxime_ro[i]-x2*cos(teta_rad))/(sin(teta_rad)));
			dc.MoveTo(x1,dwHeight-1-y1);
			dc.LineTo(x2,dwHeight-1-y2);
			
		}
	}

	dc.SelectObject(pTempPen);
	dc.SelectObject(pTempBmp); 
	GetDIBits(dc.m_hDC, ddBitmap, 0, dwHeight, lpDst,
	(LPBITMAPINFO)lpD, DIB_RGB_COLORS);

	END_PROCESSING("Third step finished!");
}
