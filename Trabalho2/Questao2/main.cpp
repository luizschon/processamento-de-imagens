#include <opencv2\core\core.hpp>
#include <opencv2/imgproc.hpp>
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>

void DFTSpectrum(const cv::Mat& sourceImg, cv::Mat& outputImg);
void fftshift(const cv::Mat& sourceImg, cv::Mat& outputImg);
void ButterworthNotch(cv::Mat& source, int nBW = 4, double D0 = 9);
void filterNotchReject(const cv::Mat& sourceImg, cv::Mat& outputImg, const cv::Mat& filterH);

int main(int argc, char** argv)
{
    // Declarando matrizes que serão usadas
    cv::Mat sourceImg, paddedImg, outputImg;

    // Lendo imagem de entrada
    sourceImg = cv::imread("moire.tif", cv::IMREAD_GRAYSCALE);

    // Verificando se imagem foi lida corretamente
    if (sourceImg.empty())
    {
        std::cout << "Nao foi possivel ler a imagem corretamente.\nCertifique-se de que o arquivo utilizado esta no diretorio de trabalho.";
        return -1;
    }

    cv::imshow("Original", sourceImg);
    cv::waitKey(0);

    // Formando imagem preenchida de tamanho 2*Mx2*N
    cv::copyMakeBorder(sourceImg, paddedImg, 0, sourceImg.rows, 0, sourceImg.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));
    cv::imshow("Imagem preenchida", paddedImg);
    cv::waitKey(0);

    // Calculando filtro rejeita-notch Butterworth com. n = 4 e D0 = 9
    cv::Mat butterworthH(paddedImg.size(), CV_32F, cv::Scalar(1));
    ButterworthNotch(butterworthH, 4, 20);

    // Filtrando imagem preenchida usando filtro Butterworth calculado
    fftshift(butterworthH, butterworthH);
    filterNotchReject(paddedImg, outputImg, butterworthH);
    cv::waitKey(0);

    // Desfazendo preenchimento na imagem resultante
    cv::Mat croppedResult(outputImg, cv::Rect(0, 0, sourceImg.cols, sourceImg.rows));

    // Normalizando imagem resultante para valores entre 0 e 255
    croppedResult.convertTo(croppedResult, CV_8U);
    cv::normalize(croppedResult, croppedResult, 0, 255, cv::NORM_MINMAX);
    cv::imshow("Resultado da filtragem com preenchimento", croppedResult);
    cv::waitKey(0);

    return 0;
}