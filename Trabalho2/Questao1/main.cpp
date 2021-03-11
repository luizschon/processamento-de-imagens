#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

cv::Mat imgSmootheningSharpening(cv::Mat sourceImg, cv::Mat_<int> kernel, double stdDeviation = 0, int gaussianFilterSize = 3);

int main(int argc, char** argv)
{
	// Declarando matrizes que serao utilizadas
	cv::Mat sourceImg, outputImg;
	cv::Mat kernel;

	// Lendo imagem de entrada
	sourceImg = cv::imread("im1.jpg", cv::IMREAD_ANYCOLOR);

	// Verificando se imagem foi lida corretamente
	if (sourceImg.empty())
	{
		std::cout << "Nao foi possivel ler a imagem corretamente.\nCertifique-se de que o arquivo utilizado esta no diretorio de trabalho.";
		return -2;
	}

	unsigned int aux, gaussianSize;
	double sigma;

	std::cout << "    [ 0, -1,  0  " << std::endl;
	std::cout << "1:   -1,  4, -1  " << std::endl;
	std::cout << "      0, -1,  0 ]" << std::endl << std::endl;

	std::cout << "    [ 0,  1,  0  " << std::endl;
	std::cout << "2:    1, -4,  1  " << std::endl;
	std::cout << "      0,  1,  0 ]" << std::endl << std::endl;

	std::cout << "    [-1, -1, -1  " << std::endl;
	std::cout << "3:   -1,  8, -1  " << std::endl;
	std::cout << "     -1, -1, -1 ]" << std::endl << std::endl;

	std::cout << "    [ 1,  1,  1  " << std::endl;
	std::cout << "4:    1, -8,  1  " << std::endl;
	std::cout << "      1,  1,  1 ]" << std::endl << std::endl;

	// Atribuindo Operador Laplaciano
	std::cout << "Escolha o Operador Laplaciano: ";
	std::cin  >> aux;

	// Inicializando kernel correto de acordo com a entrada do usuario
	switch (aux)
	{
	case 1: kernel = (cv::Mat_<int>(3, 3) <<  0, -1,  0,
											 -1,  4, -1,
											  0, -1,  0); break;

	case 2: kernel = (cv::Mat_<int>(3, 3) <<  0,  1,  0,
											  1, -4,  1,
											  0,  1,  0); break;

	case 3: kernel = (cv::Mat_<int>(3, 3) << -1, -1, -1,
											 -1,  8, -1,
											 -1, -1, -1); break;

	case 4: kernel = (cv::Mat_<int>(3, 3) <<  1,  1,  1,
											  1, -8,  1,
											  1,  1,  1); break;

	default: std::cout << "Filtro Laplaciano invalido" << std::endl; return -3;
	}

	// Atribuindo desvio padrão
	std::cout << "Digite o desvio padrao do filtro Gaussiano de suavizacao: ";
	std::cin  >> sigma;

	// Atribuindo tamanho no filtro
	if (sigma > 0)
	{
		std::cout << "Digite o tamanho (S) do filtro Gaussiano (SxS): ";
		std::cin  >> gaussianSize;
	}
	else
	{
		sigma = 0;
		gaussianSize = 0;
	}

	outputImg = imgSmootheningSharpening(sourceImg, kernel, sigma, gaussianSize);

	cv::imshow("Original", sourceImg);
	cv::imshow("Resultado", outputImg);
	cv::waitKey(0);

	return 0;
}