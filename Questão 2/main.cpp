#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>

using namespace std;
using namespace cv;

Mat convertRGBToGrayscale(Mat imgRGB);
Mat convertGrayscaleC1ToC3(Mat imgC1);
void travelToNeighbour(Mat img, int row, int col, int finishRow, int finishCol);

int main()
{
    Mat MRGB = imread("Mars.bmp");

	if ( MRGB.empty() )
	{
		cout << "Nao foi possivel encontrar a imagem \"Mars.bmp\"\n";
		return -1;
	}
	
	int rowStart, colStart, rowFinish, colFinish;

	try
	{
		cout << "Digite a linha do inicio do percurso: ";
		cin >> rowStart;

		if (rowStart < 0 || rowStart >= MRGB.rows)
			throw invalid_argument("Linha de inicio fora do alcance da imagem");

		cout << "Digite a coluna do inicio do percurso: ";
		cin >> colStart;

		if (colStart < 0 || colStart >= MRGB.cols)
			throw invalid_argument("Coluna de inicio fora do alcance da imagem");

		cout << "Digite a linha do destino do percurso: ";
		cin >> rowFinish;

		if (rowFinish < 0 || rowFinish>= MRGB.rows)
			throw invalid_argument("Linha de destino fora do alcance da imagem");

		cout << "Digite a coluna do destino do percurso: ";
		cin >> colFinish;

		if (colFinish < 0 || colFinish >= MRGB.cols)
			throw invalid_argument("Coluna de destino fora do alcance da imagem");
	}
	catch (invalid_argument &exp)
	{
		cout << "Uma excessao foi encontrada: " << exp.what() << '\n';
		return -2;
	}

    Mat MGray = convertRGBToGrayscale(MRGB);
    Mat MHeq;
    equalizeHist(MGray, MHeq);

	// Armazena imagem equalizada em imagem de 3 canais para dar mais destaque para o caminho percorrido
	Mat Output = convertGrayscaleC1ToC3(MHeq);
	travelToNeighbour(Output, rowStart, colStart, rowFinish, colFinish);

	imshow("Imagem Equalizada", MHeq);
	imshow("Imagem com percurso de menor custo", Output);
	waitKey(0);

    return 0;
}