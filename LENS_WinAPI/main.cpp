 #define WIN32_LEAN_AND_MEAN //No agrega librerías que no se vayan a utilizar

#include <Windows.h>
#include <stdio.h>
#include <dinput.h>
#include <gdiplus.h>
#include <gl/gl.h>
#include <gl/glu.h>

using namespace Gdiplus;

//Variables constantes
/* Variable constante para calcular el ancho de la ventana */
const int ANCHO_VENTANA = 800;

/* Variable constante para calcular el alto de la ventana */
const int ALTO_VENTANA = 600;

/* Variable constante que define la cantidad de bytes por pixel, usada en las operaciones de desplegar sprites/imagenes en pantalla */
const int BPP = 4;

/* Variable constante que define el intervalo del contador o timer en milisegundos, 
	con cada TICK del contador se ejecuta el codigo dentro del case WM_TIMER en la funcion WndProc */
const int TICK = 100 ;

/* Variables constantes de los colores primarios de un pixel de 32 bits */
const unsigned int BLUE = 0xFF0000FF;
const unsigned int GREEN = 0xFF00FF00;
const unsigned int RED = 0xFFFF0000;

/* Estructura con las coordenadas de los sprites en pantalla en un plano 2D */
struct POSITION {
	int X;
	int Y;
};

/* Estructura con las dimensiones de los sprites a cargar y desplegar en pantalla */
struct DIMENSION {
	int ANCHO;
	int ALTO;
};

/* Estructura con la enumeracion de algunas teclas.
	Se tiene un objeto o variable del tipo de esta estructura, llamado 'input' 
	que sera para acceder a cada uno de las elementos de la enumeracion; ejemplo:
	input.A para la tecla 'A'.*/
struct Input
{
	enum Keys
	{
		Backspace = 0x08, Tab,
		Clear = 0x0C, Enter,
		Shift = 0x10, Control, Alt,
		Escape = 0x1B,
		Space = 0x20, PageUp, PageDown, End, Home, Left, Up, Right, Down,
		Zero = 0x30, One, Two, Three, Four, Five, Six, Seven, Eight, Nine,
		A = 0x41, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
		NumPad0 = 0x60, NumPad1, NumPad2, NumPad3, NumPad4, NumPad5, NumPad6, NumPad7, NumPad8, NumPad9, 
		F1 = 0x70, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12
	};
}input /* declaracion del objeto de la estructura Input */;

//Variables Globales
bool linkMirror = false, NPC1mirror = false, zeldaMirror = false, zeldaRet = false;
int bigDoodRotateIndex = 0;
int linkSprtIndex = 0, NPCsprtIndex = 0, zeldaSprtIndex = 0;
int *ptrBuffer;
int backMove = 0;
unsigned char *ptrBackground, *ptrLinkSprt, *ptrNPC1, *ptrNPC2, *ptrNPC3, *ptrBigDood, *ptrZelda;
DIMENSION dmnBack, dmnlinkSprt, dmnNPC1, dmnNPC2, dmnNPC3, dmnBigDood, bigDoodScale{ 1,1 }, dmnZelda;
POSITION sprt1Pos, zeldaPos{350, 300};
bool KEYS[256];

//Declaracion de funciones
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void MainRender(HWND hWnd);
void Init();
void KeysEvents();
unsigned char * CargaImagen(WCHAR rutaImagen[], DIMENSION * dmn);
POSITION setPosition(int x, int y);
void dibujaFondo(int *buffer, int *imagen, DIMENSION dmn);
void dibujaSprite(int *buffer, int *sourcce, DIMENSION dmn, POSITION sprtPos,int totalSprts, int sprtIndex, int mirror, int notDraw, int canal, bool scale, int sacaleW, int scaleH, int rotate);

int WINAPI wWinMain(HINSTANCE hInstance, 
					 HINSTANCE hPrevInstance, 
					 PWSTR pCmdLine, 
					 int nCmdShow)
{
	WNDCLASSEX wc;									// Windows Class Structure
	HWND hWnd;
	MSG msg;

	TCHAR szAppName[] = TEXT("LENS proj");		
	TCHAR szAppTitle[] = TEXT("Final proj");

	hInstance = GetModuleHandle(NULL);				// Grab An Instance For Our Window

	wc.cbSize			= sizeof(WNDCLASSEX);
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc		= (WNDPROC) WndProc;					// WndProc Handles Messages
	wc.cbClsExtra		= 0;									// No Extra Window Data
	wc.cbWndExtra		= 0;									// No Extra Window Data
	wc.hInstance		= hInstance;							// Set The Instance	
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground	= (HBRUSH) (COLOR_WINDOW + 1);			// No Background Required For GL
	wc.lpszMenuName		= NULL;									// We Don't Want A Menu	
	wc.lpszClassName	= szAppName;							// Set The Class Name
	wc.hIconSm			= LoadIcon(NULL, IDI_APPLICATION);
	
	if (!RegisterClassEx(&wc))									// Attempt To Register The Window Class
	{
		MessageBox(NULL,
			L"Fallo al registrar clase (Failed To Register The Window Class).",
			L"ERROR",
			MB_OK|MB_ICONEXCLAMATION);
		return 0;
	}

	hWnd = CreateWindowEx(	
		WS_EX_CLIENTEDGE | WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,	// Extended Style For The Window
		szAppName,							// Class Name
		szAppTitle,							// Window Title
		WS_OVERLAPPEDWINDOW |				// Defined Window Style
		WS_CLIPSIBLINGS |					// Required Window Style
		WS_CLIPCHILDREN,					// Required Window Style
		0, 0,								// Window Position
		ANCHO_VENTANA,						// Calculate Window Width
		ALTO_VENTANA,						// Calculate Window Height
		NULL,								// No Parent Window
		NULL,								// No Menu
		hInstance,							// Instance
		NULL);								// Pass this class To WM_CREATE								

	if(hWnd == NULL) {
		MessageBox(NULL, 
			L"Error al crear ventana (Window Creation Error).", 
			L"ERROR", 
			MB_OK|MB_ICONEXCLAMATION);
		return 0;
	}
		
	SetForegroundWindow(hWnd);						// Slightly Higher Priority
	Init();
	ShowWindow(hWnd, nCmdShow);
	SetFocus(hWnd);

	SetTimer(hWnd, TICK, TICK, NULL);
	ZeroMemory(&msg, sizeof(MSG));

	while(GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	return(int)msg.wParam;
}

/* Funcion tipo Callback para el manejo de los eventos de la ventana. 
	*/
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)									// Check For Windows Messages
	{
		case WM_TIMER:
			if(wParam == TICK)
			{
				MainRender(hWnd);
			}
			break;
		case WM_PAINT:
			{
				HDC hdc; 
				PAINTSTRUCT ps;
				hdc = BeginPaint(hWnd, &ps);

				BITMAP bm;
				HBITMAP h_CMC = CreateBitmap(ANCHO_VENTANA, ALTO_VENTANA, 1, 32, ptrBuffer);
				HDC hdcMem = CreateCompatibleDC(hdc);
				HBITMAP hbmOld = (HBITMAP) SelectObject(hdcMem, h_CMC);
				GetObject(h_CMC, sizeof(bm), &bm);

				BitBlt(hdc, 0, 0, ANCHO_VENTANA, ALTO_VENTANA, hdcMem, 0, 0, SRCCOPY);

				DeleteObject(h_CMC);
				SelectObject(hdcMem, hbmOld);
				DeleteDC(hdcMem);
				DeleteObject(hbmOld);
			}
			break;		
		case WM_KEYDOWN:							
			{
				KEYS[ wParam ] = true;
			}
			break;
		case WM_KEYUP:
			{
				KEYS[ wParam ] = false;
			}
			break;
		case WM_CLOSE: 
			{
				DestroyWindow(hWnd);
			}
			break;
		case WM_DESTROY: //Send A Quit Message
			{
				KillTimer(hWnd, TICK);
				PostQuitMessage(0);
			}
			break;
		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}

/* Funcion usada para la inicializacion de variables y reserva de espacio en memoria.
	*/
void Init() 
{
	for(int i = 0; i < 256; i++)
	{
		KEYS[i] = false;
	}

	//Inicializar el puntero tipo int 'ptrBuffer' que contiene la direccion inicial  del area de memoria reservada para el despliegue de sprites/imagenes.
	ptrBuffer = new int[ANCHO_VENTANA * ALTO_VENTANA];

	//Inicializar el puntero tipo unsigned char 'ptrBack' que contiene la direccion inicial en memoria del arreglo de pixeles de la imagen especificada en el primer parametro
	//y en la variable dmnBack de tipo DIMENSION* estan los valores de ANCHO y ALTO de la imagen.
	ptrBackground = CargaImagen(TEXT("background.png"), &dmnBack); //puntero a la imagen
	ptrLinkSprt = CargaImagen(TEXT("linkSpriteSheet.png"), &dmnlinkSprt);
	ptrNPC1 = CargaImagen(TEXT("NPC1.png"), &dmnNPC1);
	ptrNPC2 = CargaImagen(TEXT("NPC1.png"), &dmnNPC2);
	ptrNPC3 = CargaImagen(TEXT("NPC1.png"), &dmnNPC3);
	ptrZelda = CargaImagen(TEXT("zelda.png"), &dmnZelda);
	ptrBigDood = CargaImagen(TEXT("bigDood.png"), &dmnBigDood);
	
}

/* Funcion principal. Encargada de hacer el redibujado en pantalla cada intervalo (o "tick") del timer que se haya creado.
	@param hWnd. Manejador de la ventana.
	*/
void MainRender(HWND hWnd) 
{
	KeysEvents();

	if (NPCsprtIndex <= 5)NPCsprtIndex++; else NPCsprtIndex = 0;
	if (bigDoodRotateIndex <= 4) bigDoodRotateIndex++; else bigDoodRotateIndex = 0;
	if (zeldaSprtIndex <= 11){zeldaMirror = false; zeldaSprtIndex++;} else zeldaSprtIndex = 0;
	

	dibujaFondo(ptrBuffer, (int*)ptrBackground, dmnBack);
	dibujaSprite(ptrBuffer, (int*)ptrLinkSprt, dmnlinkSprt, sprt1Pos, 12, linkSprtIndex, linkMirror, RED, 0xFF000000, FALSE, 0, 0, 0);
	dibujaSprite(ptrBuffer, (int*)ptrNPC1, dmnNPC1, setPosition(500, 300), 6, NPCsprtIndex, NPC1mirror, BLUE, BLUE, FALSE, 0, 0, 0);
	dibujaSprite(ptrBuffer, (int*)ptrNPC1, dmnNPC2, setPosition(250, 250), 6, NPCsprtIndex, NPC1mirror, BLUE, RED, FALSE, 0, 0, 0);
	dibujaSprite(ptrBuffer, (int*)ptrNPC1, dmnNPC3, setPosition(420, 420), 6, NPCsprtIndex, NPC1mirror, BLUE, GREEN, FALSE, 0, 0, 0);
	dibujaSprite(ptrBuffer, (int*)ptrZelda, dmnZelda, zeldaPos, 11, zeldaSprtIndex, zeldaMirror, RED, 0xFF00000000, FALSE, 0, 0, 0);
	dibujaSprite(ptrBuffer, (int*)ptrBigDood, dmnBigDood, setPosition(250, 100), 1, 0, 0, BLUE, RED, TRUE, bigDoodScale.ANCHO, bigDoodScale.ALTO, 0);
	dibujaSprite(ptrBuffer, (int*)ptrBigDood, dmnBigDood, setPosition(550, 265), 1, 0, 0, BLUE, 0xFF000000, FALSE,0 , 0, bigDoodRotateIndex);
	//Funciones que deberan estar el final de la funcion de Render.
	
	InvalidateRect(hWnd, NULL, FALSE);
	UpdateWindow(hWnd);
}

/* Funcion que regresa la posicion del sprite en pantalla.
	@param x. Coordenada X en la ventana.
	@param y. Coordenada Y en la ventana.
	*/
POSITION setPosition(int x, int y) {
	POSITION p;
	p.X = x;
	p.Y = y;
	return p;
}

/* Funcion para manejar eventos del teclado dependiendo de la(s) tecla(s) que se haya(n) presionado.
	*/
void KeysEvents() 
{
	if(KEYS[input.W] || KEYS[input.Up]) 
	{ 
		sprt1Pos.Y -= 4;
	}
	if(KEYS[input.D] || KEYS[input.Right]) 
	{ 
		sprt1Pos.X += 4;
		linkMirror = false;
		NPC1mirror = false;
		if (linkSprtIndex <= 10)
			linkSprtIndex++;
		else
			linkSprtIndex = 0;
		if (backMove < (dmnBack.ANCHO/4))
		{
			backMove += 4;
		}
	}
	if(KEYS[input.S] || KEYS[input.Down]) 
	{ 
		if (sprt1Pos.Y < (ALTO_VENTANA-dmnlinkSprt.ALTO*2))
		{
			sprt1Pos.Y += 4;
		}
	}
	if(KEYS[input.A] || KEYS[input.Left]) 
	{ 
		sprt1Pos.X -= 4;
		linkMirror = true;
		NPC1mirror = true;
		if (linkSprtIndex <= 10)
			linkSprtIndex++;
		else
			linkSprtIndex = 0;
		backMove -= 4;
	}
	if (KEYS[input.J])
	{
		if (bigDoodScale.ANCHO < 3)
		{
			bigDoodScale.ANCHO += 1;
			bigDoodScale.ALTO += 1;
		}
	}
	if (KEYS[input.K])
	{
		if (bigDoodScale.ALTO > 1)
		{
			bigDoodScale.ANCHO -= 1;
			bigDoodScale.ALTO -= 1;
		}
		
	}
}

/* Funcion para cargar imagenes y obtener un puntero al area de memoria reservada para la misma.
	@param rutaImagen.			Nombre o ruta de la imagen a cargar en memoria.
	@return unsigned char *.	Direccion base de la imagen.
	*/
unsigned char * CargaImagen(WCHAR rutaImagen[], DIMENSION * dmn)
{
	unsigned char * ptrImagen;
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR  gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	Bitmap *bitmap=new Bitmap(rutaImagen);
	BitmapData *bitmapData=new BitmapData;

	dmn->ANCHO = bitmap->GetWidth();
	dmn->ALTO = bitmap->GetHeight();

	Rect rect(0, 0, dmn->ANCHO, dmn->ALTO);

	//Reservamos espacio en memoria para la imagen
	bitmap->LockBits(&rect, ImageLockModeRead, PixelFormat32bppRGB, bitmapData);

	//"pixels" es el puntero al area de memoria que ocupa la imagen
	unsigned char* pixels = (unsigned char*)bitmapData->Scan0;

	//"tamaño" lo usaremos para reservar los bytes que necesita la imagen. 
	//Para calcular la cantidad de bytes total necesitamos multiplicamos el area de la imagen * 4. 
	//Se multiplica por 4 debido a que cada pixel ocupa 4 bytes de memoria. Noten el 3er parametro de la funcion LockBits, dos lineas de codigo arriba.
	//PixelFormat32bppARGB -> Specifies that the format is 32 bits per pixel; 8 bits each are used for the alpha, red, green, and blue components.
	//Mas info: https://msdn.microsoft.com/en-us/library/system.drawing.imaging.pixelformat(v=vs.110).aspx
	int tamaño;
	tamaño = dmn->ANCHO * dmn->ALTO * 4;
	//hagamos un try de la reserva de memoria
	try
	{
		ptrImagen = new unsigned char [tamaño]; 
	}
	catch(...)
	{
		return NULL;
	}

	//Después de este for, ptrImagen contiene la direccion en memoria de la imagen.
	for(int i=0, j=tamaño; i < j; i++)
	{
		ptrImagen[i]=pixels[i];
	}

	//Es necesario liberar el espacio en memoria, de lo contrario marcaria una excepcion de no hay espacio de memoria suficiente.
	bitmap->UnlockBits(bitmapData);
	delete bitmapData;
	delete bitmap;
	  
	GdiplusShutdown(gdiplusToken);

	return ptrImagen;
}

#pragma region LENS_CODE
void dibujaFondo(int * buffer, int * imagen, DIMENSION dmn) {
	int width = dmn.ANCHO;
	int height = dmn.ALTO;
	__asm {

			mov esi, imagen
			mov edi, buffer

			mov ecx, height			;mueve el alto del sprite al registro 'ecx' para poder repetir el ciclo de dibujado de cada
									;linea por lo que mide el alto

			mov eax, backMove
			mul BPP
			cmp eax, 0				; Compara para que el recorrido de la ventana no sea menor a cero y cause una excepción
			jl noincBack			; brinca a la etiqueta 'noincBack' para que no se le agregue lo que hay en 'eax' al source
			
			add esi, eax			; Agrega el valor del registro 'eax' al registro 'esi' para poder acceder a más pixeles de la imagen
			
			noincBack:
			transferHeight:
			push ecx
			mov ecx, ANCHO_VENTANA
			rep movsd					;Tranfiere una 'doble palabra' desde el registro 'esi' al 'edi', este proceso lo repite
										;con la instrucción 'rep' la cantidad de veces dadas por el valor del registro 'ecx'
			mov eax, width
			mul BPP 
			add esi, eax 
			mov eax, ANCHO_VENTANA
			mul BPP 
			sub esi, eax 
			pop ecx 
		loop transferHeight


	}
}
void dibujaSprite(int *buffer, int *source, DIMENSION dmn, POSITION sprtPos,int totalSprts, int sprtIndex, int  mirror, int notDraw, int canal, bool scale, int scaleW, int scaleH, int rotate)
{
	int windowW = 0;
	int width = dmn.ANCHO;
	int height = dmn.ALTO;
	int sprtWidth;

	int posX = sprtPos.X;
	int posY = sprtPos.Y;
	_asm
	{

		mov edi, buffer
		mov esi, source

		mov eax, ANCHO_VENTANA
		mul BPP 
		mov windowW, eax 

		mov eax, 0
		mov edx, 0					;Se setea el registro 'EDX' en cero ya que puede contener el residuo de alguna operación previa, lo que puede causar algunos problemas para realizar la división 
									;En este registro se guarda el residuo de la operación de división 
		mov eax, width				;Pasa el valor del ancho de la tira de sprites al registro 'eax' para poder realizar la operación de división
		div totalSprts				;Se divide el valor del ancho que se encuentra en 'eax' entre la cantidad de sprites en la tira para así obtener el tamaño de cada cuadro de la tira
		mov sprtWidth, eax			;Se pasa el valor de 'eax', el cual contiene el tamaño de cada cuadro de la tira de sprites a la variable 'sprtWidth', la cual se usará para imprimir cada sprite
									;Esto se hace para tener el valor ancho de cada cuadro a parte del valor del tamaño de la tira de sprites, ya que se utilizarán los dos valores separados
		
		mov eax, sprtWidth			;tranfiere el valor del ancho del sprite al registro 'eax'
		mul sprtIndex				;multiplica el ancho del sprite por el indice del sprite 
		mul BPP						;multiplica el producto de la multiplicacion del indice del sprite por el ancho del sprite por los bytes en un pixel
		add esi, eax				;le agrega el producto de las dos multiplicaciones anteriores al registro 'esi' para poder brincar de cuadro en la hoja de sprites 

		mov eax, posX				;Pasa las unidades que se le van a sumar a la posición actual al registro EAX
		cmp eax, 0
		jl endXAdd 
		mul BPP						;Despues de multiplicar la posicion en X por los bytes por pixel de la ventana, le suma la nueva poicion en X al registro de destino
		add edi, eax				;Agrega la posición en X al sprite
		endXAdd:

		mov eax, posY				;Pasa la nueva posicion en Y al registro eax para poder ser multiplicado por el ancho de la ventanaq
		
	    cmp eax, 0					;Se comprar el valor de la posición en Y transferido al registro 'eax'
		jl noincY					;En caso de que el valor de la posición en Y menor a 0, se hará un salto condicional el cual brincará la parte del incremento de la posición  
		mul ANCHO_VENTANA			;Se multiplica por el ancho de la ventana ya que seran las veces que se le aumentará el ancho de la ventana para brincar de linea para dibujar el sprite
		mul BPP						;También se multiplica por los bytes por pixel para despues sumarlos al registro destino y así setear la nueva posición
		cmp eax, 0
		add edi, eax				;Se le suma el valor de 'eax' al registro destino para setear la nueva posición 
		noincY:

		cmp rotate, 1
		je rotate1
		cmp rotate, 2 
		je rotate2
		cmp rotate, 3
		je rotate3

		cmp scale, TRUE 
		je scaleDraw 

		mov ecx, height
		drawHeight:
			 push ecx
			 mov ecx, sprtWidth
				 cmp mirror, TRUE  
				 je mirrorDraw		;Despues de comprar la bandera del mirror, hace un brinco condicional a la etiquita 'mirrorDraw' para hacer el proceso 
				 drawWidth:
					 mov eax, [esi]				;Carga en contenido de la dirección del registro source al registro 'eax'
					 cmp eax, notDraw			;compara el contenido de 'eax' con el color que no se va a imprimir 
					 je noDraw					;si es igual brinca a la etiqueta 'noDraw' para brincar el paso del contenido del registro 'eax' al destino ('edi')
					 xor eax, canal				
					 mov [edi], eax				;pasa el contenido del registro 'eax' al destino 
					 noDraw:					;Etiqueta para el salto del color que no se va a imprimir en la pantalla 
					 add esi, BPP				;Agrega 4 bytes al registro 'esi' para brincar al siguente pixel 
					 add edi, BPP				;Agrega 4 bytes al registro 'edi' para acomodar el destino en el siguiente pixel de la pantalla 
				loop drawWidth
				jmp endNormalDraw				;Despues de dibujar el sprite normalmente, hace un salto incondicional para brincar la parte del 'mirrorDraw'
				mirrorDraw:					;Etiqueta para dibujar el sprite con el efecto mirror 
				mov eax, sprtWidth			;Se pasa el ancho del sprite a dibujar al registro eax 
				mul BPP						;Se multiplica por los bytes que hay en cada pixel 
				add esi, eax				;Se le suma al registro fuente el contenido del registro 'eax' para posicionarlo al fin del sprite 
				drawMirror:					;Comenza el ciclo de dibujado 
					mov eax, [esi] 					;Se pasa lo que hay en la dirección del registro fuente al registro 'eax'
					cmp eax, notDraw
					je noDrawM 
					xor eax, canal
					mov [edi], eax				;Se pasa el contenido del registro 'eax' al destino 
					noDrawM:
					sub esi, BPP			;Se decrementa la dirección del registro fuente para así poder dibujar desde el último pixel en la linea hasta el primero 
					add edi, BPP			;Se incementa el registro destino ya que lo va a dibujar como normalmente lo haría, pero con la diferencia de que cada linea se va a dibujar desde
				loop drawMirror				;el último pixel hasta el primero de cada una  
				endNormalDraw:

			mov eax, width					;Pasa el ancho original al registro 'eax'
			mul BPP							;Lo multiplica por los bytes por pixel 
			add esi, eax					;Se lo suma al reistro fuente, para poder brincar de linea a dibujar 
			cmp mirror, TRUE
			je endSprtHeight
			mov eax, sprtWidth				;Pasa el ancho del sprite actual 
			mul BPP							;multiplicar por los bytes por pixel 
			sub esi, eax

			endSprtHeight:
			add edi, windowW				;Se agrega el valor de la variable que contiene el ancho de la ventana multiplicada por los bytes por pixel al registro destino para poder brincar de linea en la ventana
			mov eax, sprtWidth				;mueve el valor del ancho del sprite al registro 'eax'
			mul BPP							;multiplica el valor del ancho del sprite por los bytes en el pixel 
			sub edi, eax					;le sustrae al registro destino el producto de la multiplicación anterior para poder empezar a dibujar una linea debajo de la otra 
			pop ecx							;saca el último valor ingresado en la pila y lo tranfiere al registro contador 'ecx', en este caso será el valor del alto 
			loop drawHeight

			jmp endDraw 

			scaleDraw:										;Etiqueta del dibujado del escalado 
				mov ecx, height
				heightLoop:									;Ciclo para el ancho del sprite 
					push ecx								;guarda el valor el alto de sprite en la pila para almacenarlo y utilizarlo luego 
					mov ecx, scaleH							;Para el valor de la cantidad de veces que se va a escalar el ancho al registro 'ecx'
					heightScale:							;comienza el ciclo de escalado del alto 
						push ecx							;guarda el valor de la cantidad de veces que se va a escalar el alto en la pila 
						mov ecx, sprtWidth					;pasa el valor del ancho del sprite al registro contador 
						widthLoop:							;comienza el ciclo de dibujado del ancho
							mov eax, [esi]					;Se pasa el valor del pixel en de la dirección del registro fuente al registro 'eax'
							push ecx						;se guarda el valor del ancho del sprite en la pila 
							mov ecx, scaleW					;Se pasa el valor de la cantidad de veces que se va a escalar el ancho al registro contador 
								scaleWidth:					;Comienza el ciclo de escalado del ancho 
									cmp eax, notDraw		;Se compara el contenido del registro 'eax' con el color que se desea brincar 
									je jumpPixel			;En caso de que sea igual se brinca a la etiquita 'jumpPixel'
									xor eax, canal
									mov [edi], eax			;Se pasa el contenido el registro 'eax' al destino (la pantalla)
									jumpPixel:				;Etiquita 'jumpPixel'
									add edi, BPP			;Se suma la cantidad de bytes por pixel al registro 'edi' para poder brincar el siguiente pixel en la pantalla 
								loop scaleWidth
							pop ecx							;Se saca el último valor ingresado a la pila, en este caso es el valor del ancho del sprite y se guarda en el registro contador
							add esi, BPP					;Se le agregan cuatro byes al registro source para así poder brincar al siguiente pixel en la imagen 
						loop widthLoop						
						pop ecx								;Se saca de la pila el siguiente valor ingresado en ella, en este caso, el valor de la cantidad de veces que se va a escaalr el alto y se guarda en 'ecx'
						cmp ecx, 1							;Se compara el valor del registro 'ecx' con 1 para ver si se llegó a la última linea con la que se va a aumentar el alto de la imagen 
						je noLineRet						;Si con iguales se brinca a la etiqueta 'noLineRet'(no Regresa a la linea, la linea anterior)
					    mov eax, width						;Se pasa el valor del ancho de la imagen 					
						mul BPP								;Se multiplica por los bytes por pixel 
						sub esi, eax						;y se restan del registro esi para así poder regresar a la linea anterior y poder volver a dibujar esa linea 
						noLineRet:

						add edi, windowW
					    mov eax, sprtWidth
						mul scaleW
						mul BPP
						sub edi, eax
					loop heightScale
					pop ecx
				loop heightLoop	
		jmp endDraw 
		rotate1:
		mov ecx, width									   ;Se pasa el valor del ancho del sprite al registro contador, esto para poder utilizar el valor del ancho como si fuera el valor del alto de la imagen 
			rotate1Height:	
				push ecx								   ;Se guarda el valor del registro 'ecx' en la pila, esto para poder reemplazar el valor del registro con el valor del alto de la imagen, que nos servirá como valor de ancho en este caso  
				mov ecx, height							   ;Se pasa el valor del alto de la imágen al registro contador para poder comenzar a trabajar con ese valor
				sub ecx, 1								   
					rotate1Width:
						mov eax, [esi]
						cmp eax, notDraw
						je notPassToDest 
							xor eax, canal
							mov [edi], eax
						notPassToDest: 
						mov eax, width
						mul BPP 
						add esi, eax 
						add edi, BPP 
					loop rotate1Width 
					mov eax, width
					sub eax, 1 
					mul height 
					mul BPP 
					sub esi, eax
					add edi, windowW
					mov eax, height
					sub eax, 1
					mul BPP 
					sub edi, eax
					pop ecx 
			loop rotate1Height 
		jmp endDraw 
		rotate2:
		mov eax, width
		mul height
		sub eax, 1 
		mul BPP 
		add esi, eax 

		mov ecx, width
			rotate2Height:
				push ecx 
				mov ecx, height
				sub ecx, 1 
					rotate2Width:
						mov eax,[esi]
						cmp eax, notDraw
						je jumpColor
						xor eax, canal
						mov [edi], eax
						jumpColor:
						mov eax, width
						mul BPP
						sub esi, eax 
						add edi, BPP 
					loop rotate2Width 
					mov eax, width 
					sub eax, 1 
					mul height 
					mul BPP 
					add esi, eax
					add edi, windowW
					mov eax, height
					sub eax, 1 
					mul BPP
					sub edi, eax 
					pop ecx 
			loop rotate2Height 
		jmp endDraw 
		rotate3:
		mov ecx, height
		mov eax, width
		mul height
		mul BPP 
		add esi, eax 
		rotate3Height:
			push ecx 
			mov ecx, width
				rotate3Width:
					mov eax,[esi]
					cmp eax, notDraw
					je noDrawPixel 
					xor eax, canal
					mov [edi], eax 
					noDrawPixel:
					sub esi, BPP 
					add edi, BPP 
				loop rotate3Width 
				add edi, windowW
				mov eax, width
				mul BPP 
				sub edi, eax 
				pop ecx 
		loop rotate3Height 
	 
		endDraw:
	}
}
#pragma endregion