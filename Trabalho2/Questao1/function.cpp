#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>

//--------------------------------------------------------------------------------------------------|
//	  Função que implementa sharpening da imagem de entrada utilizando Operador Laplaciano, com		|
//	suavização opcional de por um filtro Gaussiano.													|
//																									|
//		- sourceImg:		    imagem de entrada													|
//		- kernel:			   operador Laplaciano 3x3												|
//		- gaussianFilterSize:  tamanho do filtro Gaussiano de suavização, 3x3 (3) por padrão		|
//		- stdDeviation:		   desvio padrão para calcular índices do filtro Gaussiano, 0 por		|
//	padrão. Desvio padrão 0 desabilita a suavização Gaussiana										| 
//																									|
//		Retorna imagem processada																	|
//--------------------------------------------------------------------------------------------------|

cv::Mat imgSmootheningSharpening(cv::Mat sourceImg, cv::Mat_<int> kernel, double stdDeviation = 0, int gaussianFilterSize = 3)
{
	// Instanciando imagem de saída
	cv::Mat outputImg;

	// Inicializando tamanho do filtro Gaussiano e valor do centro do operador Laplaciano
	cv::Size_<int> ksize(gaussianFilterSize, gaussianFilterSize);
	auto kernelCenter = kernel(1, 1);

	// Aplica filtro Gaussiano de suavização caso stdDeviation seja diferente de 0,
	// caso contrário, copia sourceImg em outputImg
	if (stdDeviation)
		cv::GaussianBlur(sourceImg, outputImg, ksize, stdDeviation);
	else
		sourceImg.convertTo(outputImg, CV_32F);

	// Fazendo filtragem no dominio do espaco usando kernel inicializado e armazenando em laplacianImg.
	// A profundidade de 32 bits do tipo float (CV_32F) foi utilizada para evitar overflow 
	// durante a filtragem, uma vez que a imagem tem profundidade de 8 bits (CV_8U).
	cv::filter2D(outputImg, outputImg, CV_32F, kernel);

	// Somando/subtraindo imagem filtrada na imagem original, para obter resultado de sharpening e armazenando em outputImg
	sourceImg.convertTo(sourceImg, CV_32F);
	outputImg = ( kernelCenter < 0) ? sourceImg - outputImg : sourceImg + outputImg;

	// Convertendo a imagem novamente para CV_8U
	outputImg.convertTo(outputImg, CV_8U);

	return outputImg;
}