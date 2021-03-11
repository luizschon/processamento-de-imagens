#include <opencv2\core\core.hpp>
#include <opencv2/imgproc.hpp>
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include <cmath>
#include <vector>

void DFTSpectrum(const cv::Mat& complexImg, cv::Mat& outputImg)
{
    // Declarando eixos complexos para separar a imagem em parte real e parte imaginaria
    cv::Mat complexAxis[2];

    // Separando as dimensao Real e Complexa da imagem no dominio da frequencia
    cv::split(complexImg, complexAxis);

    // Calculando o espectro para representar a imagem no dominio da frequencia, uma vez que 
    // a representacao nas dimensoes real e imaginaria nao traz informacoes relevantes ao processamento da imagem.
    // O espectro e calculada por: sqrt( pow( Re(DFT(image)), 2 ) + pow( Im(DFT(image)), 2 ) ), onde Re e a parte real e Im e a parte imaginaria
    cv::magnitude(complexAxis[0], complexAxis[1], complexAxis[0]);
    outputImg = complexAxis[0];

    // Para melhorar a visualizacao, podemos transformar a escala linear da transformada em uma escala logaritima, onde M1 = log (1 + M);
    outputImg += cv::Scalar::all(1);
    cv::log(outputImg, outputImg);

    // Normalizando imagem para valores visualizaveis em ponto flutuante (entre 0 e 1)
    cv::normalize(outputImg, outputImg, 0, 1, cv::NORM_MINMAX);
}

void fftshift(const cv::Mat& source, cv::Mat& output)
{
    // Copia imagem de entrada na imagem de saída
    output = source.clone();

    // Calculando o centro da imagem para reorganizar os quadrantes da imagem trnasformada de forma que a origem seja o centro da imagem
    int centerX = output.cols / 2;
    int centerY = output.rows / 2;

    // Definindo quadrantes da imagem
    cv::Mat quadrant1(output, cv::Rect(centerX, 0, centerX, centerY));        // Diagonal superior-direita 
    cv::Mat quadrant2(output, cv::Rect(0, 0, centerX, centerY));              // Diagonal superior-esquerda
    cv::Mat quadrant3(output, cv::Rect(0, centerY, centerX, centerY));        // Diagonal inferior-esquerda
    cv::Mat quadrant4(output, cv::Rect(centerX, centerY, centerX, centerY));  // Diagonal inferior-direita

    // Trocando primeiro quadrante com terceiro quadrante e segundo quadrante com quarto quadrante
    cv::Mat temp;
    quadrant1.copyTo(temp);
    quadrant3.copyTo(quadrant1);
    temp.copyTo(quadrant3);
    quadrant2.copyTo(temp);
    quadrant4.copyTo(quadrant2);
    temp.copyTo(quadrant4);
}

void ButterworthNotch(cv::Mat& inputOutputH, int nBW = 4, double D0 = 9)
{
    // Inicializando vetor que armazena as distancias dos pares de filtros Butterworth
    std::vector < std::vector <float> > distances = { {84,-55} , {77,59} , {160,59} , {165,-55} };
    float Dk[2] = {0,0};
    float* p = nullptr;

    // Calculando H(u,v) de acordo com a fórmula do filtro rejeita-notch para 4 pares de filtros Butterworth
    for (int u = 0; u < inputOutputH.rows; u++)
    {
        p = inputOutputH.ptr<float>(u);
        for (int v = 0; v < inputOutputH.cols; v++)
        {
            for (int i = 0; i < 4; i++)
            {
                Dk[0] = sqrt(pow(((double)u - inputOutputH.rows / 2 - distances[i][0]), 2) + pow(((double)v - inputOutputH.cols / 2 - distances[i][1]), 2));
                Dk[1] = sqrt(pow(((double)u - inputOutputH.rows / 2 + distances[i][0]), 2) + pow(((double)v - inputOutputH.cols / 2 + distances[i][1]), 2));
                p[v] *= (1 / (1 + pow((D0 / Dk[0]), nBW))) * (1 / (1 + pow((D0 / Dk[1]), nBW)));
            }
        }
    }
    cv::imshow("Filtro Rejeita-Notch Butterworth", inputOutputH);
}

void filterNotchReject(const cv::Mat& sourceImg, cv::Mat& outputImg, const cv::Mat& filterH)
{
    // Declarando matrizes que serao usadas
    cv::Mat complexImg;

    // Gerando uma imagem com uma dimensao real (contendo sourceImg) e uma dimensao imaginaria (contendo zeros)
    // a partir da fusao de duas matrizes (complexAxis), uma vez que a Tranfomada Discreta de Fourier 2D tem resultado complexo.
    cv::Mat complexAxis[] = { cv::Mat_<float>(sourceImg.clone()), cv::Mat::zeros(sourceImg.size(), CV_32F) };
    cv::merge(complexAxis, 2, complexImg);

    // Fazendo Transformada Discreta de Fourier para vizualizar o espectro da imagem
    cv::Mat temp = complexImg.clone();
    cv::dft(temp, temp);
    DFTSpectrum(temp, temp);
    fftshift(temp, temp);
    cv::imshow("Espectro da imagem", temp);

    // Fazendo Transformada Discreta de Fourier em complexImg para filtragem e inversao posterior
    cv::dft(complexImg, complexImg, cv::DFT_SCALE);

    // Gerando dimensoes real (contendo o filtro) e imaginaria (contendo zeros) do filtro H(u,v) e unindo elas em complexH
    cv::Mat complexH;
    cv::Mat complexAxisH[] = { cv::Mat_<float>(filterH.clone()), cv::Mat::zeros(filterH.size(), CV_32F) };
    cv::merge(complexAxisH, 2, complexH);

    // Multiplicando espectros do filtro e da imagem complexa para executar filtragem
    cv::Mat complexFilteredImg;
    cv::mulSpectrums(complexImg, complexH, complexFilteredImg, 0);

    // Fazendo Transformada de Fourier Inversa e extraindo a dimensao real para obter imagem filtrada (complexAxis[0] => dimensao real)
    cv::idft(complexFilteredImg, complexFilteredImg);
    cv::split(complexFilteredImg, complexAxis);
    outputImg = complexAxis[0];
}