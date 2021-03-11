#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <cmath>
#include <iostream>

using namespace cv;
using namespace std;

Mat im_chscaledepth(Mat imgInput, int numBitsOutput, double redimensioningFactor)
{
	const int shadeLevels = 256;

	// Criando uma tabela de pesquisa contendo os possiveis valores de nível de brilho para imagens de 8 bits (0-255)
	uchar table[shadeLevels];

	int divisor = shadeLevels / ( (int)pow(2, numBitsOutput) );
	double product = ((double)shadeLevels - 1) / (pow(2, numBitsOutput) - 1);

	// Calculando possíveis valores de nível de brilho de acordo com o número de bits desejado para a imagem de saída (numBitsOutput) e armazenando na tabela
	for (int i = 0; i < shadeLevels; i++)
		table[i] = (i / divisor) * product;

	int numChannels = imgInput.channels();  // Armazena número de canais na imagem
	int numRows     = imgInput.rows;		// Armazena número de linhas da matriz, correspondente à altura da imagem em pixels
	int numCols     = imgInput.cols;		// Armazena número de colunas da matriz, correspondente à largura da imagem em pixels

	// Inicializando ponteiro que aponta para o endereço da linha atual da matriz da imagem
	uchar * ptrCurrentRow;
	
	int outputRow = 0;
	int outputCol = 0;

	//////////////////////////////////////////
	//     Redução da imagem de entrada     //
	//////////////////////////////////////////

	if (redimensioningFactor < 1)
	{
		// Calculando o incremento entre as linhas e colunas que serão removidas
		double increment = 1 / (1 - redimensioningFactor);

		double rowToDelete = 0;
		double colToDelete = 0;

		// Contando número de linhas na imagem de saída
		for (int i = 0; i < numRows; i++)
		{
			int aux = (int)round(rowToDelete);
			if (i != aux)
				outputRow++;
			else
				rowToDelete += increment;
		}
		// Contando número de colunas na imagem de saída
		for (int j = 0; j < numCols; j++)
		{
			int aux = (int)round(colToDelete);
			if (j != aux)
				outputCol++;
			else
				colToDelete += increment;
		}

		// Instanciando imagens com 1 e 3 canais e com a altura e largura calculadas
		Mat imgOutputC1 (outputRow, outputCol, CV_8UC1, Scalar(0));
		Mat imgOutputC3 (outputRow, outputCol, CV_8UC3, Scalar(0,0,0));

		outputRow   = 0;
		outputCol   = 0;
		rowToDelete = 0;
		colToDelete = 0;

		for (int i = 0; i < numRows; i++)
		{
			int aux1 = (int)round(rowToDelete);

			// A linha não será deletada
			if (i != aux1)
			{
				// Percorre pixels da linha que não será deletada
				for (int j = 0; j < numCols; j++)
				{
					int aux2 = (int)round(colToDelete); 
					
					// O pixel não será deletado
					if (j != aux2)
					{
					// Copia o valor do pixel que não será deletado para a imagem de saída
						if (numChannels == 1)
							imgOutputC1.at<uchar>(outputRow, outputCol) = imgInput.at<uchar>(i, j);

						if (numChannels == 3)
							imgOutputC3.at<Vec3b>(outputRow, outputCol) = imgInput.at<Vec3b>(i, j);

						// É incrementado para cada pixel que por adicionado a imagem de saída
 						outputCol++;
					}
					// O pixel será deletado
					else
						// Atualiza o valor para o próximo pixel a ser deletado
						colToDelete += increment;					
				}
				outputCol = 0;
				colToDelete = 0;
				outputRow++;
			}
			// A linha será deletada
			else
				// Atualiza o valor para a proxima linha a ser deletada
				rowToDelete += increment;
		}
		outputRow = imgOutputC1.rows;
		outputCol = imgOutputC1.cols;

		// Alterando o valor de cada pixel para o valor definido na tabela de pesquisa
		for (int i = 0; i < outputRow; i++)
		{
			// Atribuindo valor do endereço da linha atual da matriz imgOutput ao ponteiro
			ptrCurrentRow = (numChannels == 1) ? imgOutputC1.ptr(i) : imgOutputC3.ptr(i);

			for (int j = 0; j < outputCol * numChannels; j++)
				// Modificando valor de cada elemento da linha atual da matriz de acordo com a tabela
				ptrCurrentRow[j] = table[ptrCurrentRow[j]];
		}

		return (numChannels == 1) ? imgOutputC1 : imgOutputC3;
	}

	//////////////////////////////////////////
	//     Aumento da imagem de entrada     //
	//////////////////////////////////////////

	if (redimensioningFactor > 1)
	{
		// Calculando o incremento entre as linhas que serão adicionadas, caso a escala de redimensionamento seja maior que 2.0, primeiro o tamanho da imagem é dobrada para ser aumentada novamente
		double increment = (redimensioningFactor <= 2) ? 1 / (redimensioningFactor - 1) : 1;

		double rowToAdd = 0;
		double colToAdd = 0;

		// Contando número de linhas na imagem de saída
		for (int i = 0; i < numRows; i++)
		{
			int aux = (int)round(rowToAdd);
			if (i == aux)
			{
				outputRow++;
				rowToAdd += increment;
			}
			outputRow++;
		}
		// Contando número de colunas na imagem de saída
		for (int j = 0; j < numCols; j++)
		{
			int aux = (int)round(colToAdd);
			if (j == aux)
			{
				outputCol++;
				colToAdd += increment;
			}
			outputCol++;
		}

		// Instanciando imagens com 1 e 3 canais e com a altura e largura calculadas e todos os pixels com valor 1
		Mat imgOutputC1 (outputRow, outputCol, CV_8UC1, Scalar(0));
		Mat imgOutputC3 (outputRow, outputCol, CV_8UC3, Scalar(0,0,0));

		// Instanciando matrizes auxiliares que armazenam 1 nos pixels novos e 0 nos pixeis copiados
		Mat auxC1 (outputRow, outputCol, CV_8UC1, Scalar(1));
		Mat auxC3 (outputRow, outputCol, CV_8UC3, Scalar(1,1,1));

		outputRow = 0;
		outputCol = 0;
		rowToAdd  = 0;
		colToAdd  = 0;
		
		// Copiando pixels da imagem original na imagem de saída de forma intercalada
		for (int i = 0; i < numRows; i++)
		{
			for (int j = 0; j < numCols; j++)
			{
				// Copia o valor do pixel da imagem original na imagem da saida
				if (numChannels == 1)
				{
					imgOutputC1.at<uchar>(outputRow, outputCol) = imgInput.at<uchar>(i, j);
					auxC1.at<uchar>(outputRow, outputCol) = 0;
				}

				if (numChannels == 3)
				{
					imgOutputC3.at<Vec3b>(outputRow, outputCol) = imgInput.at<Vec3b>(i, j);
					auxC3.at<Vec3b>(outputRow, outputCol) = 0;
				}

				outputCol++;

				int aux2 = (int)round(colToAdd);

				// Pula o pixel a ser adicionado
				if (j == aux2)
				{
					colToAdd += increment;
					outputCol++;
				}
			}
			outputCol = 0;
			outputRow++;
			colToAdd = 0;

			int aux1 = (int)round(rowToAdd);

			// Pula a linha a ser adicionada
			if (i == aux1)
			{
				rowToAdd += increment;
				outputRow++;
			}
		}
		// Os pixels novos começam a partir do ponto (1, 1)
		rowToAdd  = 1;
		colToAdd  = 1;
	
		outputRow = imgOutputC1.rows;
		outputCol = imgOutputC1.cols;
		
		for (int i = 0; i < outputRow; i++)
		{
			for (int j = 0; j < outputCol; j++)
			{
				// Modifica o valor dos pixels que possuem valor [1], caso a imagem tenha 1 canal
				if (numChannels == 1 && auxC1.at<uchar>(i, j) == 1)
				{
					// Vetor que alojara o valor dos 4-vizinhos do pixel a ser modificado
					uchar neighbours[4];
					int numNeighbours = 0;
					
					if (i > 0 && auxC1.at<uchar>(i - 1, j) != 1)
					{
						neighbours[0] = imgOutputC1.at<uchar>(i - 1, j);
						numNeighbours++;
					}
					else
						neighbours[0] = 0;

					if (j > 0 && auxC1.at<uchar>(i, j - 1) != 1)
					{
						neighbours[1] = imgOutputC1.at<uchar>(i, j - 1);
						numNeighbours++;
					}
					else
						neighbours[1] = 0;

					if (j < outputCol - 1 && auxC1.at<uchar>(i, j + 1) != 1)
					{
						neighbours[2] = imgOutputC1.at<uchar>(i, j + 1);
						numNeighbours++;
					}
					else
						neighbours[2] = 0;

					if (i < outputRow - 1 && auxC1.at<uchar>(i + 1, j) != 1)
					{
						neighbours[3] = imgOutputC1.at<uchar>(i + 1, j);
						numNeighbours++;
					}
					else
						neighbours[3] = 0;

					// Muda o valor do pixel modificado na matriz auxiliar de 1 para 0
					auxC1.at<uchar>(i, j) = 0;

					// Fazendo a média aritimética dos valores dos pixels vizinhos e e atribuido ao pixel sendo modificado
					imgOutputC1.at<uchar>(i, j) = 0;
					
					for (int k = 0; k < 4; k++)
						imgOutputC1.at<uchar>(i, j) += neighbours[k] / numNeighbours;
				}
				// Modifica o valor dos pixels que possuem valor [1,1,1], caso a imagem tenha 3 canal
				if (numChannels == 3 && auxC3.at<Vec3b>(i, j)[0] == 1 && auxC3.at<Vec3b>(i, j)[1] == 1 && auxC3.at<Vec3b>(i, j)[2] == 1)
				{
					Vec3b neighbours[4];
					int numNeighbours = 0;

					if (i > 0 && auxC3.at<Vec3b>(i - 1, j)[0] != 1 && auxC3.at<Vec3b>(i - 1, j)[1] != 1 && auxC3.at<Vec3b>(i - 1, j)[2] != 1)
					{
						neighbours[0] = imgOutputC3.at<Vec3b>(i - 1, j);
						numNeighbours++;
					}
					else
						neighbours[0] = 0;

					if (j > 0 && auxC3.at<Vec3b>(i, j - 1)[0] != 1 && auxC3.at<Vec3b>(i, j - 1)[1] != 1 && auxC3.at<Vec3b>(i, j - 1)[2] != 1)
					{
						neighbours[1] = imgOutputC3.at<Vec3b>(i, j - 1);
						numNeighbours++;
					}
					else
						neighbours[1] = 0;

					if (j < outputCol - 1 && auxC3.at<Vec3b>(i, j + 1)[0] != 1 && auxC3.at<Vec3b>(i, j + 1)[1] != 1 && auxC3.at<Vec3b>(i, j + 1)[2] != 1)
					{
						neighbours[2] = imgOutputC3.at<Vec3b>(i, j + 1);
						numNeighbours++;
					}
					else
						neighbours[2] = 0;

					if (i < outputRow - 1 && auxC3.at<Vec3b>(i + 1, j)[0] != 1 && auxC3.at<Vec3b>(i + 1, j)[1] != 1 && auxC3.at<Vec3b>(i + 1, j)[2] != 1)
					{
						neighbours[3] = imgOutputC3.at<Vec3b>(i + 1, j);
						numNeighbours++;
					}
					else
						neighbours[3] = 0;

					// Muda o valor do pixel modificado na matriz auxiliar de [1,1,1] para [0,0,0]
					auxC3.at<Vec3b>(i, j) = 0;

					// Fazendo a média aritimética dos valores dos pixels vizinhos e e atribuido ao pixel sendo modificado
					imgOutputC3.at<Vec3b>(i, j) = 0;

					for (int k = 0; k < 4; k++)
						imgOutputC3.at<Vec3b>(i, j) += neighbours[k] / numNeighbours;
				}
			}
		}

		// Caso a escala de redimensionamento seja maior que 2.0, a imagem é aumentada de forma recusiva, em que a nova escala é igual a metade da original
		if (redimensioningFactor > 2)
		{
			if (numChannels == 1)
				return im_chscaledepth(imgOutputC1, numBitsOutput, redimensioningFactor / 2);
			if (numChannels == 3)
				return im_chscaledepth(imgOutputC3, numBitsOutput, redimensioningFactor / 2);
		}

		// Alterando o valor de cada pixel para o valor definido na tabela de pesquisa
		for (int i = 0; i < outputRow; i++)
		{
			// Atribuindo valor do endereço da linha atual da matriz imgOutput ao ponteiro
			ptrCurrentRow = (numChannels == 1) ? imgOutputC1.ptr(i) : imgOutputC3.ptr(i);

			for (int j = 0; j < outputCol * numChannels; j++)
				// Modificando valor de cada elemento da linha atual da matriz de acordo com a tabela
				ptrCurrentRow[j] = table[ptrCurrentRow[j]];
		}
		return (numChannels == 1) ? imgOutputC1 : imgOutputC3;
	}

	////////////////////////////////////////////////////
	//     Mesmo tamanho ao da imagem de entrada      //
	////////////////////////////////////////////////////

	Mat imgOutput = imgInput.clone();

	// Alterando o valor de cada pixel para o valor definido na tabela de pesquisa
	for (int i = 0; i < numRows; i++)
	{
		// Atribuindo valor do endereço da linha atual da matriz imgOutput ao ponteiro
		ptrCurrentRow = imgOutput.ptr(i);

		for (int j = 0; j < numCols * numChannels; j++)
			// Modificando valor de cada elemento da linha atual da matriz de acordo com a tabela
			ptrCurrentRow[j] = table[ptrCurrentRow[j]];
	}
	return imgOutput;
}