#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

using namespace cv;
using namespace std;

Mat sourceImg, sourceGray, crackImg, binaryCrackImg, outputImg;
int thresholdValue = 0;
const int maxValue = 255;

bool loop = true;
bool drawing = false;

void MTMFilter(Mat& inputImg, Mat& outputImg, Mat& mask, int windowSize);

// Fun��o para apresenta��o do slider de valor de threshold
static void thresholdDemo(int, void*)
{
	threshold(crackImg, binaryCrackImg, thresholdValue, maxValue, THRESH_BINARY_INV);
	imshow("Threshold Demo", binaryCrackImg);
}

// Fun��o callBack para capta��o de eventos com mouse. Intera��o com usuario para remover craquelados
static void callBackFunc(int event, int x, int y, int flags, void* userdata)
{
	Mat mask = *(Mat*)userdata;
	uchar fillValue = 255;

	if (event == EVENT_LBUTTONDOWN)
		drawing = true;

	// Fill na m�scara passada como argumento na posi��o do mouse
	else if (event == EVENT_MOUSEMOVE)
	{
		if (drawing)
			if (x >= 0 && x < binaryCrackImg.cols && y >= 0 && y < binaryCrackImg.rows)
				floodFill(binaryCrackImg, mask, Point(x, y), Scalar(255), 0, Scalar(), Scalar(), 8 | FLOODFILL_MASK_ONLY | (fillValue << 8));
	}
	else if (event == EVENT_LBUTTONUP)
		drawing = false;

	else if (event == EVENT_RBUTTONDOWN)
		loop = false;
}

int main()
{
	// L� imagem de entrada
	sourceImg = imread("teste.png", IMREAD_COLOR);

	if (sourceImg.empty())
	{
		cout << "Deu errado!" << endl;
		return 1;
	}

	// Converte para grayscale
	cvtColor(sourceImg, sourceGray, COLOR_BGR2GRAY);

	int tipoElemento;
	int tamanhoElemento;
	bool apresentar = true;
	while (apresentar)
	{
		cout << "1 - Quadrado" << endl << "2 - Elipse" << endl << endl << "Tipo de elemento estruturante : ";
		cin >> tipoElemento;

		cout << endl << "Tamanho do elemento estruturante : ";
		cin >> tamanhoElemento;


		// Faz tranformada Bottom-hat com elemento estruturante escolhido por usu�rio
		Mat structuringElem;  
		
		switch (tipoElemento)
		{
		case 1: structuringElem = getStructuringElement(MORPH_RECT, Size(tamanhoElemento, tamanhoElemento)); break;
		case 2: structuringElem = getStructuringElement(MORPH_ELLIPSE, Size(tamanhoElemento, tamanhoElemento)); break;
		}

		morphologyEx(sourceGray, crackImg, MORPH_BLACKHAT, structuringElem);

		imshow("Original", sourceImg);
		imshow("Craquelado", crackImg);

		namedWindow("Threshold Demo", WINDOW_AUTOSIZE);
		createTrackbar("Valor", "Threshold Demo", &thresholdValue, maxValue, thresholdDemo);
		
		waitKey();
		destroyAllWindows();

		char resp;
		cout << endl << "Repetir procedimento com elemento estruturante diferente? (Y/N) : ";
		cin >> resp;

		switch (resp)
		{
		case 'N':
			apresentar = false; break;
		default:
			break;
		}
	}

	// Cria m�cara para ser preenchida
	Mat mask = binaryCrackImg.clone();
	copyMakeBorder(mask, mask, 1, 1, 1, 1, BORDER_REPLICATE, Scalar(255));

	// Mostra imagem original para facilitar sele��o interativa de rachaduras
	imshow("Pintura original", sourceImg);
	
	// Cria janela interativa para remo��o dos craquelados ----------------------
	//
	//	- Bot�o esquerdo do mouse para remover �rea clicada;
	//	- Bot�o direito do mouse para interromper sele��o de rachaduras;
	//---------------------------------------------------------------------------
	
	namedWindow("Selecione craquelados para serem removidos");
	while (loop)
	{
		setMouseCallback("Selecione craquelados para serem removidos", callBackFunc, &mask);
		imshow("Selecione craquelados para serem removidos", mask);
		waitKey(60);
	}

	// Corta bordas de 1px da m�scara
	Mat croppedMask(mask, Rect(1, 1, binaryCrackImg.cols, binaryCrackImg.rows));
	
	// Inverte m�scara modificada pelo usu�rio e imagem original dos craquelados e subtrai resultados
	bitwise_not(croppedMask, croppedMask);
	bitwise_not(binaryCrackImg, binaryCrackImg);
	subtract(binaryCrackImg, croppedMask, croppedMask);

	// Pergunta ao usu�rio se ser� feita dilata��o com kernel 3x3 para melhora o resultado em caso de bordas mais claras ao lado da rachadura
	char resp;
	cout << endl << "Fazer dilatacao na mascara? (Y/N) : ";
	cin >> resp;

	Mat structuringElem = getStructuringElement(MORPH_RECT, Size(3, 3));

	switch (resp)
	{
	case 'Y': morphologyEx(croppedMask, croppedMask, MORPH_DILATE, structuringElem); break;
	default: break;
	}

	int tamanhoJanela = 0;
	cout << "Tamanho da janela de filtragem (impar) : ";
	cin >> tamanhoJanela;

	// Faz filtragem MTM com janela de filtragem do tamanho da entrada do usu�rio
	MTMFilter(sourceImg, outputImg, croppedMask, tamanhoJanela);

	imshow("Mascara para filtragem", croppedMask);
	imshow("Resultado Final", outputImg);

	waitKey();
	destroyAllWindows();

	return 0;
}