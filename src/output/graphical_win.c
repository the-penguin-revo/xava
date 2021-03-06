#include "graphical_win.h"
#include "graphical.h"
#include <stdio.h>
#include <time.h>

const char szAppName[] = "XAVA";
const char wcWndName[] = "XAVA";

HWND xavaWinWindow;
MSG xavaWinEvent;
HMODULE xavaWinModule;
WNDCLASSEX xavaWinClass;	// same thing as window classes in Xlib
HDC xavaWinFrame;
HGLRC xavaWinGLFrame;
unsigned int fgcolor, bgcolor;
unsigned int *gradientColor, grad = 0;
unsigned int shadowColor, shadow = 0;
double opacity[2] = {1.0, 1.0};

LRESULT CALLBACK WindowFunc(HWND hWnd,UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE: break;
	case WM_DESTROY: 
 		// Perform cleanup tasks.
		PostQuitMessage(0); 
    break; 
        default:
            return DefWindowProc(hWnd,msg,wParam,lParam);
    }

    return 0;
}

unsigned char register_window_win(HINSTANCE HIn) {
    xavaWinClass.cbSize=sizeof(WNDCLASSEX);
    xavaWinClass.style=CS_HREDRAW | CS_VREDRAW;
    xavaWinClass.lpfnWndProc=WindowFunc;
    xavaWinClass.cbClsExtra=0;
    xavaWinClass.cbWndExtra=0;
    xavaWinClass.hInstance=HIn;
    xavaWinClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    xavaWinClass.hIcon=LoadIcon(NULL,IDI_APPLICATION);
    xavaWinClass.hCursor=LoadCursor(NULL,IDC_ARROW);
    xavaWinClass.hbrBackground=(HBRUSH)CreateSolidBrush(0x00000000);
    xavaWinClass.lpszMenuName=NULL;
    xavaWinClass.lpszClassName=szAppName;
    xavaWinClass.hIconSm=LoadIcon(NULL,IDI_APPLICATION);

    return RegisterClassEx(&xavaWinClass);
}

void GetDesktopResolution(int *horizontal, int *vertical) {
   RECT desktop;

   // Get a handle to the desktop window
   const HWND hDesktop = GetDesktopWindow();
   // Get the size of screen to the variable desktop
   GetWindowRect(hDesktop, &desktop);
   
   // return dimensions
   (*horizontal) = desktop.right;
   (*vertical) = desktop.bottom;

   return;
}

void init_opengl_win() {
	glEnable(GL_ALPHA_TEST);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	if(transparentFlag) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	glClearColor(0, 0, 0, 0);
	return;
}

unsigned char CreateHGLRC(HWND hWnd) {
    PIXELFORMATDESCRIPTOR pfd = {
      sizeof(PIXELFORMATDESCRIPTOR),
      1,                                // Version Number
      PFD_DRAW_TO_WINDOW      |         // Format Must Support Window
      PFD_SUPPORT_OPENGL      |         // Format Must Support OpenGL
      PFD_SUPPORT_COMPOSITION |         // Format Must Support Composition
      PFD_DOUBLEBUFFER,                 // Must Support Double Buffering
      PFD_TYPE_RGBA,                    // Request An RGBA Format
      32,                               // Select Our Color Depth
      0, 0, 0, 0, 0, 0,                 // Color Bits Ignored
      8,                                // An Alpha Buffer
      0,                                // Shift Bit Ignored
      0,                                // No Accumulation Buffer
      0, 0, 0, 0,                       // Accumulation Bits Ignored
      24,                               // 16Bit Z-Buffer (Depth Buffer)
      8,                                // Some Stencil Buffer
      0,                                // No Auxiliary Buffer
      PFD_MAIN_PLANE,                   // Main Drawing Layer
      0,                                // Reserved
      0, 0, 0                           // Layer Masks Ignored
   };     

   HDC hdc = GetDC(hWnd);
   int PixelFormat = ChoosePixelFormat(hdc, &pfd);
   if (PixelFormat == 0) {
      assert(0);
      return FALSE ;
   }

   BOOL bResult = SetPixelFormat(hdc, PixelFormat, &pfd);
   if (bResult==FALSE) {
      assert(0);
      return FALSE ;
   }

   xavaWinGLFrame = wglCreateContext(hdc);
   if (!xavaWinGLFrame){
      assert(0);
      return FALSE;
   }

   ReleaseDC(hWnd, hdc);

   return TRUE;
}

void resize_framebuffer(int width,int height) {
	glViewport(0, 0, (double)width, (double)height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	glOrtho(0, (double)width, 0, (double)height, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

int init_window_win(char *color, char *bcolor, double foreground_opacity, int col, int bgcol, int gradient_count, char **gradient_colors, unsigned int shdw, unsigned int shdw_col, int w, int h) {

	// get handle
	xavaWinModule = GetModuleHandle(NULL);
	FreeConsole();
	
	// register window class
	if(!register_window_win(xavaWinModule)) {
		MessageBox(NULL, "RegisterClassEx - failed", "Error", MB_OK | MB_ICONERROR);
		return 1;
	}
	
	// get window size etc..
	int screenWidth, screenHeight;
	GetDesktopResolution(&screenWidth, &screenHeight);
	// adjust window position etc...
	if(!strcmp(windowAlignment, "top")){
		windowX = (screenWidth - w) / 2 + windowX;
	}else if(!strcmp(windowAlignment, "bottom")){
		windowX = (screenWidth - w) / 2 + windowX;
		windowY = (screenHeight - h) + (-1*windowY);
	}else if(!strcmp(windowAlignment, "top_left")){
		// Nothing to do here :P
	}else if(!strcmp(windowAlignment, "top_right")){
		windowX = (screenHeight - w) + (-1*windowX);
	}else if(!strcmp(windowAlignment, "left")){
		windowY = (screenHeight - h) / 2;
	}else if(!strcmp(windowAlignment, "right")){
		windowX = (screenWidth - w) + (-1*windowX);
		windowY = (screenHeight - h) / 2 + windowY;
	}else if(!strcmp(windowAlignment, "bottom_left")){
		windowY = (screenHeight - h) + (-1*windowY);
	}else if(!strcmp(windowAlignment, "bottom_right")){
		windowX = (screenWidth - w) + (-1*windowX);
		windowY = (screenHeight - h) + (-1*windowY);
	}else if(!strcmp(windowAlignment, "center")){
		windowX = (screenWidth - w) / 2 + windowX;
		windowY = (screenHeight - h) / 2 + windowY;
	}
	// Some error checking
	#ifdef DEBUG
		if(windowX > screenWidth - w) printf("Warning: Screen out of bounds (X axis)!");
		if(windowY > screenHeight - h) printf("Warning: Screen out of bounds (Y axis)!");
	#endif
	
	// create window
	xavaWinWindow = CreateWindowEx(interactable ? WS_EX_APPWINDOW : WS_EX_COMPOSITED | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST, szAppName, wcWndName, WS_VISIBLE | WS_POPUP, windowX, windowY, w, h, NULL, NULL, xavaWinModule, NULL);
	if(xavaWinWindow == NULL) {
		MessageBox(NULL, "CreateWindowEx - failed", "Error", MB_OK | MB_ICONERROR);
		return 1;
	}
	// transparency fix
	SetLayeredWindowAttributes(xavaWinWindow, 0, 255, LWA_ALPHA);
	SetWindowPos(xavaWinWindow, keepInBottom ? HWND_BOTTOM : HWND_TOPMOST, windowX, windowY, w, h, SWP_SHOWWINDOW);

	
	// we need the desktop window manager to enable transparent background (from Vista ...onward)
	DWM_BLURBEHIND bb = {0};
	HRGN hRgn = CreateRectRgn(0, 0, -1, -1);
	bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
	bb.hRgnBlur = hRgn;
	bb.fEnable = transparentFlag;
	DwmEnableBlurBehindWindow(xavaWinWindow, &bb);
	
	CreateHGLRC(xavaWinWindow);
	xavaWinFrame = GetDC(xavaWinWindow);
	wglMakeCurrent(xavaWinFrame, xavaWinGLFrame);	
	
	// process colors
	if(!strcmp(color, "default")) {
		// instead of messing around with average colors like on Xlib
		// we'll just get the accent color (which is way easier and an better thing to do)

		WINBOOL opaque = 1;
		DWORD fancyVariable;
		HRESULT error = DwmGetColorizationColor(&fancyVariable, &opaque);
		fgcolor = fancyVariable;
		if(!SUCCEEDED(error)) {
			MessageBox(NULL, "DwmGetColorizationColor - failed", "Error", MB_OK | MB_ICONERROR);
			return 1;
		}
	} else if(color[0] != '#') {
		switch(col)
		{
			case 0: fgcolor = 0x000000; break;
			case 1: fgcolor = 0xFF0000; break;
			case 2: fgcolor = 0x00FF00; break;
			case 3: fgcolor = 0xFFFF00; break;
			case 4: fgcolor = 0x0000FF; break;
			case 5: fgcolor = 0xFF00FF; break;
			case 6: fgcolor = 0x00FFFF; break;
			case 7: fgcolor = 0xFFFFFF; break;
			default: MessageBox(NULL, "Missing color.\nPlease send a bug report!", "Error", MB_OK | MB_ICONERROR); return 1;
		}
	} else if(color[0] == '#') sscanf(color, "#%x", &fgcolor);

	if(!strcmp(bcolor, "default")) {
		// instead of messing around with average colors like on Xlib
		// we'll just get the accent color (which is way easier and a better thing to do)

		WINBOOL opaque = 1;
		DWORD fancyVariable;
		HRESULT error = DwmGetColorizationColor(&fancyVariable, &opaque);
		bgcolor = fancyVariable;
		if(!SUCCEEDED(error)) {
			MessageBox(NULL, "DwmGetColorizationColor - failed", "Error", MB_OK | MB_ICONERROR);
			return 1;
		}
	} else if(bcolor[0] != '#') {
		switch(bgcol)
		{
			case 0: bgcolor = 0x000000; break;
			case 1: bgcolor = 0xFF0000; break;
			case 2: bgcolor = 0x00FF00; break;
			case 3: bgcolor = 0xFFFF00; break;
			case 4: bgcolor = 0x0000FF; break;
			case 5: bgcolor = 0xFF00FF; break;
			case 6: bgcolor = 0x00FFFF; break;
			case 7: bgcolor = 0xFFFFFF; break;
			default: MessageBox(NULL, "Missing color.\nPlease send a bug report!", "Error", MB_OK | MB_ICONERROR); return 1;
		}
	} else if(bcolor[0] == '#') sscanf(bcolor, "#%x", &fgcolor);
	
	// parse all of the values
	grad = gradient_count;
	gradientColor = malloc(sizeof(int)*grad);
	for(int i=0; i<grad; i++) {
		sscanf(gradient_colors[i], "#%06x", &gradientColor[i]);
	}
	shadow = shdw;
	shadowColor = shdw_col;
	opacity[0] = foreground_opacity;
	//opacity[1] = ;

	// set up opengl and stuff
	init_opengl_win();
	return 0;
}

void apply_win_settings(int w, int h, int framerate) {
	resize_framebuffer(w, h);
	ReleaseDC(xavaWinWindow, xavaWinFrame);

	if(!transparentFlag) glClearColor(((bgcolor>>16)%256)/255.0, ((bgcolor>>8)%256)/255.0,(bgcolor%256)/255.0, opacity[1]);
	PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = wglGetProcAddress("wglSwapIntervalEXT"); 
	wglSwapIntervalEXT(framerate);
	return;
}

int get_window_input_win(int *should_reload, int *bs, double *sens, int *bw, int *w, int *h) {
	while(!*should_reload && PeekMessage(&xavaWinEvent, NULL, WM_KEYFIRST, WM_MOUSELAST, PM_REMOVE)) {	
		TranslateMessage(&xavaWinEvent);
		DispatchMessage(&xavaWinEvent);	// windows handles the rest
		switch(xavaWinEvent.message) {
			case WM_KEYDOWN:
				switch(xavaWinEvent.wParam) {
					// should_reload = 1
					// resizeTerminal = 2
					// bail = -1
					case 'A':
						(*bs)++;
						return 2;
					case 'S':
						if((*bs) > 0) (*bs)--;
						return 2;
					case 'F': // fullscreen
						//fs = !fs;
						return 2;
					case VK_UP:
						(*sens) *= 1.05;
						break;
					case VK_DOWN:
						(*sens) *= 0.95;
						break;
					case VK_LEFT:
						(*bw)++;
						return 2;
					case VK_RIGHT:
						if ((*bw) > 1) (*bw)--;
						return 2;
					case 'R': //reload config
						(*should_reload) = 1;
						return 1;
					case 'Q':
						return -1;
					case VK_ESCAPE:
						return -1;
					case 'B':
						if(transparentFlag) break;
						bgcolor = (rand()<<16)+rand();
						return 3;
					case 'C':
						if(grad) break;
						fgcolor = (rand()<<16)+rand();
						return 3;
			    default: break;
				}
			  break;
			case WM_CLOSE:
			 	return -1;
			case WM_DESTROY:
				return -1;
			case WM_QUIT:
				return -1;
			case WM_SIZE:
			{
				RECT rect;
				if(GetWindowRect(xavaWinWindow, &rect)) {
					(*w) = rect.right - rect.left;
					(*h) = rect.bottom - rect.top;
				}
				return 2;
			}
		}
	}
	return 0;
}

void draw_graphical_win(int window_height, int bars_count, int bar_width, int bar_spacing, int rest, int gradient, int f[200]) {
	HDC hdc = GetDC(xavaWinWindow);
        wglMakeCurrent(hdc, xavaWinGLFrame);

	// clear color and calculate pixel witdh in double
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
	float glColors[8] = {((fgcolor>>16)%256)/255.0, ((fgcolor>>8)%256)/255.0, (fgcolor%256)/255.0, opacity[0], ((unsigned int)shadowColor>>24)%256/255.0,
		 ((unsigned int)shadowColor>>16)%256/255.0, ((unsigned int)shadowColor>>8)%256/255.0, (unsigned int)shadowColor%256/255.0};
	float gradColors[24] = {0.0};
	for(int i=0; i<grad; i++) {
		gradColors[i*3] = ((gradientColor[i]>>16)%256)/255.0;
		gradColors[i*3+1] = ((gradientColor[i]>>8)%256)/255.0;
		gradColors[i*3+2] = (gradientColor[i]%256)/255.0;;
	}

	if(drawGLBars(rest, bar_width, bar_spacing, bars_count, window_height, transparentFlag ? shadow : 0, gradient?grad:0, glColors, gradColors, f)) exit(EXIT_FAILURE);	
	glFlush();

	// swap buffers	
	SwapBuffers(hdc);
	ReleaseDC(xavaWinWindow, hdc);
}

void cleanup_graphical_win(void) {
	free(gradientColor);
	wglMakeCurrent(NULL, NULL);
        wglDeleteContext(xavaWinGLFrame);
	ReleaseDC(xavaWinWindow, xavaWinFrame);
	DestroyWindow(xavaWinWindow);
	UnregisterClass(szAppName, xavaWinModule);	
	CloseHandle(xavaWinModule);
}
