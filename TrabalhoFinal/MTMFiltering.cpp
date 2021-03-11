#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

using namespace cv;
using namespace std;

// Função que garante que pixel selecionado na janela de filtragem não está fora dos limites da imagem
bool isValidPixel(int maxRow, int maxCol, int row, int col)
{
	if (row < 0 || col < 0 || row > maxRow - 1 || col > maxCol - 1)
		return false;
	return true;
}

// Executa filtragem MTM na imagem de entrada usando a máscara
void MTMFilter(Mat& inputImg, Mat& outputImg, Mat& mask, int windowSize)
{
	// Copia imagem de entada na imagem de saída
	outputImg = inputImg.clone();

	// Inicializando variáveis 
	int nChannels = inputImg.channels();
	int nRows	  = inputImg.rows;
	int nCols	  = inputImg.cols;

	/// Loop que percorre toda imagem
	for (int i = 0; i < nRows; i++)
	{
		for (int j = 0; j < nCols; j++)
		{
			// Para cada pixel branco da máscara percorre a janela de filtragem
			if (mask.at<uchar>(i, j) == 255)
			{
				// Inicializa valor de pixel para cada canal da imagem
				double pixelValues[3] = { 0 };
				int divider[3] = { 0 };

				for (int r = -1 * (windowSize - 1) / 2; r <= (windowSize - 1) / 2; r++)
				{
					for (int s = -1 * (windowSize - 1) / 2; s <= (windowSize - 1) / 2; s++)
					{
						if (r != 0 && s != 0)
						{
							if (isValidPixel(inputImg.rows, inputImg.cols, i+r, j+s))
							{
								// Verifica se pixel nao pertence ao elemento rachadura na máscara
								if (mask.at<uchar>(i+r, j+s) == 0)
								{
									for (int c = 0; c < nChannels; c++)
									{
										// Calcula somatório de divisores da equação do filtro
										divider[c]++;

										// Calcula somatório dos valores de pixeis que não corresponde a rachadura
										if (nChannels == 1)
											pixelValues[c] += int(outputImg.at<uchar>(i + r, j + s));
										else
											pixelValues[c] += int(outputImg.at<Vec3b>(i + r, j + s)[c]);
									}
								}
							}
						}
					}
				}
				for (int c = 0; c < nChannels; c++)
				{
					if (nChannels == 1)
						outputImg.at<uchar>(i, j) = floor(pixelValues[c] / divider[c]);
					else
						outputImg.at<Vec3b>(i, j)[c] = floor(pixelValues[c] / divider[c]);
				}
			}
		}
	}
}