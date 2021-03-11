#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>

using namespace cv;
using namespace std;

Mat im_chscaledepth(Mat imgInput, int numBitsOutput, double redimensioningFactor);

int main()
{
    // Imagem sendo lida deve ser mudada aqui, verifique se a imagem sendo lida está contida do diretório de trabalho do Visual Studio
    Mat img = imread("im1.jpg", IMREAD_ANYCOLOR);
    Mat output;

    if (img.empty())
    {
        cout << "Nao foi possivel ler a imagem im1.jpg\n";
        return -1;
    }

    int bits;
    double factor;
    try
    {
        cout << "Digite o numero de bits de brilho da imagem da saida: ";
        cin >> bits;
        
        if (bits < 0 || bits > 8)
            throw invalid_argument("Numero de bits invalido");

        cout << "Digite o fator de redimensionamento da imagem: ";
        cin >> factor;

        if (factor <= 0)
            throw invalid_argument("Fator de redimensionamento invalido");
    }
    catch (invalid_argument &exp)
    {
        cout << "Uma excessao foi encontrada: " << exp.what() << '\n';
        return -2;
    }
 
    output = im_chscaledepth(img, bits, factor);
   
    imshow("Imagem Original", img);
    imshow("Resultado", output);
    waitKey(0);
    
    return 0;
}