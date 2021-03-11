#include <opencv2\core\core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>

int main()
{
    // Declarando matrizes que serao usadas
    cv::Mat sourceImg, outputImg;

    // Lendo imagem de entrada
    sourceImg = cv::imread("pcb.jpg", cv::IMREAD_GRAYSCALE);
    
    // Verificando se imagem foi lida corretamente
    if (sourceImg.empty())
    {
        std::cout << "Nao foi possivel ler a imagem corretamente.\nCertifique-se de que o arquivo utilizado esta no diretorio de trabalho.";
        return -1;
    }

    cv::imshow("Original", sourceImg);
    cv::waitKey(0);

    // Invertendo a imagem binaria original
    cv::threshold(sourceImg, sourceImg, 127, 255, cv::THRESH_BINARY_INV);
    cv::imshow("Imagem binária invertida", sourceImg);
    cv::waitKey(0);

    // Fazendo operação morfologica de abertura na imagem invertida por um kernel circular com o intuito de ligar as pontas desconexas dos buracos analizados
    cv::Mat kernel(27, 27, CV_8U, cv::Scalar(0));
    cv::circle(kernel, cv::Point(13, 13), 13, 255, cv::FILLED);
 
    cv::morphologyEx(sourceImg, outputImg, cv::MORPH_OPEN, kernel);
    cv::imshow("Resultado abertura", outputImg);
    cv::waitKey(0);

    // Criando uma borda de 5px na imagem (sem alterar o tamanho da imagem) para facilitar o preenchimento de preto na imagem pela funcao floodFill()
    cv::Mat borderImg(outputImg.size(), CV_8U, cv::Scalar(255));
    const int BORDER_SIZE = 5;

    cv::Mat paddedImg(outputImg, cv::Rect(BORDER_SIZE - 1, BORDER_SIZE - 1, sourceImg.cols - 2 * BORDER_SIZE, sourceImg.rows - 2 * BORDER_SIZE));
    paddedImg.copyTo(borderImg(cv::Rect(BORDER_SIZE - 1, BORDER_SIZE - 1, paddedImg.cols, paddedImg.rows)));

    // Preenchendo de preto as regioes brancas que nao sao de interesse 
    cv::floodFill(borderImg, cv::Point(0, 0), 0);
    outputImg = borderImg.clone();

    cv::imshow("Resultado do preenchimento (floodFill())", outputImg);
	cv::waitKey(0);

    // Extraindo o numero de rotulos da imagem (numero de regioes conectadas) com o intuito calcular o numero de buracos na imagem
    cv::Mat labels;
    int nLabel = cv::connectedComponents(outputImg, labels);

    // Como o background preto é rotulado, o numero total de buracos no imagem é (nLabel - 1)
    int nHoles = nLabel - 1;
    std::cout << "Numero de buracos contados: " << nHoles << std::endl;

    // Fazendo processo de abertura utilizando kernels circulares cada vez maiores até que o número de rótulos (desconsiderando o background) 
    // seja menor que o numero de buracos na imagem.
    // Quando o numero de rotulos for menor que o numero de buracos, quer dizer que o kernel é maior que o buraco.
    int kernelRadius = 1;
    int kernelDiameter = 0;
    while (nLabel - 1 == nHoles)
    {
        cv::Mat temp;
        cv::Mat holeKernel(2 * kernelRadius + 1, 2 * kernelRadius + 1, CV_8U, cv::Scalar(0));
        cv::circle(holeKernel, cv::Point(kernelRadius, kernelRadius), kernelRadius, 255, cv::FILLED);
        kernelDiameter = holeKernel.rows;
        cv::morphologyEx(outputImg, temp, cv::MORPH_OPEN, holeKernel);

        nLabel = cv::connectedComponents(temp, labels);
        kernelRadius++;
    }

    // Considerando que o tamanho do kernel incrementa em 2 a cada vez, pode-se concluir que o tamanho do buraco é 2px menor que o kernel
    std::cout << "Diametro dos buracos: " << kernelDiameter - 2 << "px\n";
    cv::waitKey(0);

    return 0;
}