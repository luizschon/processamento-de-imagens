#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <vector>
#include <cmath>

using namespace cv;
using namespace std;

// Representa��o de objeto para vizinhos de pixel 
class Neighbour
{
public:
	int row, col;
	int value;
	double distanceFromEndPoint;

	// Calcula distancia entre o vizinho e o o destino final
	void euclideanDistance(int finalCol, int finalRow)
	{
		double dist = pow(col - finalCol, 2) + pow(row - finalRow, 2);
		distanceFromEndPoint = sqrt(dist);
	}

	// Checa se o vizinho existe
	bool isValid(int maxRow, int maxCol)
	{
		if (row < 0 || col < 0 || row > maxRow - 1 || col > maxCol - 1)
			return false;
		return true;
	}
};

/********************************************************
 *	Fun��o que converte imagem RGB em imagem Grayscale	*
 ********************************************************/

Mat convertRGBToGrayscale(Mat imgRGB)
{
	// Instanciando imagem em grayscale de mesmo tamanho da imagem a ser convertida, por�m um s� canal de cor
	Mat imgGrayScale(imgRGB.size(), CV_8UC1);

	int rows = imgRGB.rows;
	int cols = imgRGB.cols;

	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			// Inicializando vetor de 3 dimens�es (B,G,R) para armazenar valores dos canais BGR da imagem original
			Vec3b currentPixel = imgRGB.at<Vec3b>(i, j);

			// Inicializando vari�veis para armazenar o valor de cada um dos canais BGR
			int blueChannel = currentPixel.val[0];
			int greenChannel = currentPixel.val[1];
			int redChannel = currentPixel.val[2];

			// Definindo o nivel de brilho do pixel cinza fazendo a m�dia aritimetica do valor dos 3 canais de cor
			imgGrayScale.at<uchar>(i, j) = (blueChannel + greenChannel + redChannel) / 3;
		}
	}
	return imgGrayScale;
}

/******************************************************************************************************************************
 *	Fun��o que converte imagem Grayscale de 1 canal para imagem de 3 canais com o mesmo nivel de brilho para todos os canais  *
 ******************************************************************************************************************************/

Mat convertGrayscaleC1ToC3(Mat imgC1)
{
	Mat imgC3(imgC1.size(), CV_8UC3);
	uchar* ptr;
	const int numChannels = imgC3.channels();

	for (int i = 0; i < imgC3.rows; i++)
	{
		// Armazna ponteiro para linha da imagem em ptr
		ptr = imgC3.ptr(i);

		for (int j = 0; j < imgC3.cols * numChannels; j += 3)
		{
			// Copia valor do pixel Grayscale para os 3 canais de cor do pixel correspondente de imgC3 
			ptr[j + 0] = imgC1.at<uchar>(i, j / numChannels);
			ptr[j + 1] = imgC1.at<uchar>(i, j / numChannels);
			ptr[j + 2] = imgC1.at<uchar>(i, j / numChannels);
		}
	}
	return imgC3;
}

/*********************************************************************************************************************
 *																													 *
 *	Fun��o que atravessa as melhor op��es de vizinho recursivamente at� encontrar o pixel correspondente ao destino  *
 *																													 *
 *		A escolha de vizinho segue as seguintes regras:																 *
 *																													 *
 *			1) Escolhe, entre os 8 vizinhos, os tr�s com menor dist�ncia euclidiana em rela��o ao pixel final;		 *
 *																													 *
 *			2) Entre os tr�s escolhidos, percorre aquele com o menor valor de brilho;								 *
 *																													 *
 *********************************************************************************************************************/

void travelToNeighbour(Mat img, int initialRow, int initialCol, int finishRow, int finishCol)
{
	int row = initialRow;
	int col = initialCol;

	// Marca o pixel sendo de in�cio com a cor vermelha
	img.at<Vec3b>(row, col) = 0;
	img.at<Vec3b>(row, col)[2] = 255;

	// Repete algoritmo at� que a dist�ncia de um dos vizinhos ate o ponto (finishCol, finishRow) seja igual a 0
	while (true)
	{
		vector<Neighbour> nbVector;
		Neighbour neighbour;

		// Percorre a vizinhan�a do pixel atual
		for (int i = -1; i <= 1; i++)
		{
			for (int j = -1; j <= 1; j++)
			{
				// Executa para todos os pixeis na �rea, exceto o pixel atual
				if (!(i == 0 && j == 0))
				{
					neighbour.row = row + i;
					neighbour.col = col + j;

					// Armazena vizinho com valor, posi��o e dist�ncia ao vetor de vizinhos caso seja validar 
					if (neighbour.isValid(img.rows, img.cols))
					{
						neighbour.value = (int)img.at<Vec3b>(row + i, col + j)[0];
						neighbour.euclideanDistance(finishCol, finishRow);
						nbVector.push_back(neighbour);
					}
				}
			}
		}

		// Fun��o com express�o lambda para organizar vetor de vizinhos de forma crescente das dist�ncias 
		sort(nbVector.begin(), nbVector.end(), [](const Neighbour& a, const Neighbour& b)
		{
			return a.distanceFromEndPoint < b.distanceFromEndPoint;
		});

		// Termina a execu��o do loop, caso a dist�ncia de um dos vizinhos at� o ponto (finishCol, finishRow) seja igual a 0
		if (nbVector[0].distanceFromEndPoint == 0)
		{
			// Marca o pixel sendo percorrido com a cor vermelha
			img.at<Vec3b>(nbVector[0].row, nbVector[0].col) = 0;
			img.at<Vec3b>(nbVector[0].row, nbVector[0].col) = 255;
			return;
		}


		// Instancia objeto de Neighbour referente ao pr�ximo pixel a ser percorrido
		Neighbour nextPosition;
		const int numCandidates = 3;

		// Inicializa valor e posi��o do pr�ximo pixel
		nextPosition.value = 256;
		nextPosition.row = 0;
		nextPosition.col = 0;

		for (int i = 0; i < numCandidates; i++)
		{
			// Armazena valor e posi��o do pixel da vizinhan�a caso o n�vel de brilho seja menor que o valor tempor�rio do pr�ximo pixel
			if (nbVector[i].value < nextPosition.value)
			{
				nextPosition.value = nbVector[i].value;
				nextPosition.row = nbVector[i].row;
				nextPosition.col = nbVector[i].col;
			}
		}
		nbVector.clear();

		row = nextPosition.row;
		col = nextPosition.col;

		// Marca o pixel sendo percorrido com a cor vermelha
		img.at<Vec3b>(row, col) = 0;
		img.at<Vec3b>(row, col)[2] = 255;
	} 
}