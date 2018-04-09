//
//  tp2.cpp
//  Exemple de convolution d'image avec lodepng
//
//  Créé par Julien-Charles Lévesque
//  Copyright 2015 Université Laval. Tous droits réservés.
//


#include "lodepng.h"
#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <sstream>

#include "Chrono.hpp"
#include "PACC/Tokenizer.hpp"

using namespace std;

//Aide pour le programme
void usage(char* inName) {
    cout << endl << "Utilisation> " << inName << " fichier_image fichier_noyau [fichier_sortie=output.png]" << endl;
    exit(1);
}

//Décoder à partir du disque dans un vecteur de pixels bruts en un seul appel de fonction
void decode(const char* inFilename,  vector<unsigned char>& outImage, unsigned int& outWidth, unsigned int& outHeight)
{
    //Décoder
    unsigned int lError = lodepng::decode(outImage, outWidth, outHeight, inFilename);

    //Montrer l'erreur s'il y en a une.
    if(lError)
        cout << "Erreur de décodage " << lError << ": " << lodepng_error_text(lError) << endl;

    //Les pixels sont maintenant dans le vecteur outImage, 4 octets par pixel, organisés RGBARGBA...
}

//Encoder à partir de pixels bruts sur le disque en un seul appel de fonction
//L'argument inImage contient inWidth * inHeight pixels RGBA ou inWidth * inHeight * 4 octets
void encode(const char* inFilename, vector<unsigned char>& inImage, unsigned int inWidth, unsigned int inHeight)
{
    //Encoder l'image
    unsigned lError = lodepng::encode(inFilename, inImage, inWidth, inHeight);

    //Montrer l'erreur s'il y en a une.
    if(lError)
        cout << "Erreur d'encodage " << lError << ": "<< lodepng_error_text(lError) << endl;
}

string splitString(string stringToSplit){
    istringstream iss(stringToSplit);
    std::vector<std::string> tokens;
    std::string token;
    while (std::getline(iss, token, '.')) {
        if (!token.empty())
            tokens.push_back(token);
    }

    return tokens[0];
}

void executerParallel(string fileName, string fichierNoyau, string outFileName){
    Chrono lChrono(true);

    // Lire le noyau.
    ifstream lConfig;
    lConfig.open(fichierNoyau);
    if (!lConfig.is_open()) {
        cerr << "Le fichier noyau fourni (" << fichierNoyau << ") est invalide." << endl;
        exit(1);
    }

    string tempName = splitString(outFileName);

    outFileName = tempName + "2.png";

    PACC::Tokenizer lTok(lConfig);
    lTok.setDelimiters(" \n","");

    string lToken;
    lTok.getNextToken(lToken);

    int lK = atoi(lToken.c_str());
    int lHalfK = lK/2;

    cout << "Taille du noyau: " <<  lK << endl;

    //Lecture du filtre
    double* lFilter = new double[lK*lK];

    //#pragma whynot
    //#pragma acc parallel loop // Ne fonctionne pas : 'lToken' referenced in target region does not have a mappable type
    for (int i = 0; i < lK; i++) {
        for (int j = 0; j < lK; j++) {
            lTok.getNextToken(lToken);
            lFilter[i*lK+j] = atof(lToken.c_str());
        }
    }

    //Lecture de l'image
    //Variables à remplir
    unsigned int lWidth, lHeight;
    vector<unsigned char> lImage; //Les pixels bruts

    #pragma acc data create(lImage)
    //Appeler lodepng
    decode(fileName.c_str(), lImage, lWidth, lHeight);

    //Variables contenant des indices
    int fy, fx;
    //Variables temporaires pour les canaux de l'image
    double lR, lG, lB;

    // #pragma acc kernels
    #pragma acc parallel loop
    for(int x = lHalfK; x < (int)lWidth - lHalfK; x++)
    {
        #pragma acc loop independent
        for (int y = lHalfK; y < (int)lHeight - lHalfK; y++)
        {
            lR = 0.;
            lG = 0.;
            lB = 0.;

            #pragma acc loop independent reduction(+:lR, lG, lB)
            for (int j = -lHalfK; j <= lHalfK; j++) {
                fy = j + lHalfK;

                for (int i = -lHalfK; i <= lHalfK; i++) {
                    fx = i + lHalfK;
                    //R[x + i, y + j] = Im[x + i, y + j].R * Filter[i, j]
                    lR += double(lImage[(y + j)*lWidth*4 + (x + i)*4]) * lFilter[fx + fy*lK];
                    lG += double(lImage[(y + j)*lWidth*4 + (x + i)*4 + 1]) * lFilter[fx + fy*lK];
                    lB += double(lImage[(y + j)*lWidth*4 + (x + i)*4 + 2]) * lFilter[fx + fy*lK];
                }
            }
            //Placer le résultat dans l'image.
            lImage[y*lWidth*4 + x*4] = (unsigned char)lR;
            lImage[y*lWidth*4 + x*4 + 1] = (unsigned char)lG;
            lImage[y*lWidth*4 + x*4 + 2] = (unsigned char)lB;
        }
    }

    //Sauvegarde de l'image dans un fichier sortie
    encode(outFileName.c_str(),  lImage, lWidth, lHeight);

    lChrono.pause();

    cout << "L'image a été filtrée et enregistrée dans " << outFileName << " avec succès!" << endl;

    cout << "Temps d'exécution: " << lChrono.get() << endl;

    delete lFilter;
}

void executerSequentiel(string fileName, string fichierNoyau, string outFileName){
    Chrono lChrono(true);

    // Lire le noyau.
    ifstream lConfig;
    lConfig.open(fichierNoyau, ios::in);
    if (!lConfig.is_open()) {
        cerr << "Le fichier noyau fourni (" << fichierNoyau << ") est invalide." << endl;
        exit(1);
    }

    PACC::Tokenizer lTok(lConfig);
    lTok.setDelimiters(" \n","");

    string lToken;
    lTok.getNextToken(lToken);

    int lK = atoi(lToken.c_str());
    int lHalfK = lK/2;

    cout << "Taille du noyau: " <<  lK << endl;

    //Lecture du filtre
    double* lFilter = new double[lK*lK];

    for (int i = 0; i < lK; i++) {
        for (int j = 0; j < lK; j++) {
            lTok.getNextToken(lToken);
            lFilter[i*lK+j] = atof(lToken.c_str());
        }
    }

    //Lecture de l'image
    //Variables à remplir
    unsigned int lWidth, lHeight;
    vector<unsigned char> lImage; //Les pixels bruts
    //Appeler lodepng
    decode(fileName.c_str(), lImage, lWidth, lHeight);

    //Variables contenant des indices
    int fy, fx;
    //Variables temporaires pour les canaux de l'image
    double lR, lG, lB;
    for(int x = lHalfK; x < (int)lWidth - lHalfK; x++)
    {
        for (int y = lHalfK; y < (int)lHeight - lHalfK; y++)
        {
            lR = 0.;
            lG = 0.;
            lB = 0.;
            for (int j = -lHalfK; j <= lHalfK; j++) {
                fy = j + lHalfK;
                for (int i = -lHalfK; i <= lHalfK; i++) {
                    fx = i + lHalfK;
                    //R[x + i, y + j] = Im[x + i, y + j].R * Filter[i, j]
                    lR += double(lImage[(y + j)*lWidth*4 + (x + i)*4]) * lFilter[fx + fy*lK];
                    lG += double(lImage[(y + j)*lWidth*4 + (x + i)*4 + 1]) * lFilter[fx + fy*lK];
                    lB += double(lImage[(y + j)*lWidth*4 + (x + i)*4 + 2]) * lFilter[fx + fy*lK];
                }
            }
            //Placer le résultat dans l'image.
            lImage[y*lWidth*4 + x*4] = (unsigned char)lR;
            lImage[y*lWidth*4 + x*4 + 1] = (unsigned char)lG;
            lImage[y*lWidth*4 + x*4 + 2] = (unsigned char)lB;
        }
    }

    //Sauvegarde de l'image dans un fichier sortie
    encode(outFileName.c_str(),  lImage, lWidth, lHeight);

    lChrono.pause();

    cout << "L'image a été filtrée et enregistrée dans " << outFileName << " avec succès!" << endl;

    cout << "Temps d'exécution " << lChrono.get() << endl;

    delete lFilter;
}

int main(int inArgc, char *inArgv[])
{
    if(inArgc < 3 or inArgc > 4) usage(inArgv[0]);
    string lFilename = inArgv[1];
    string lOutFilename;
    if (inArgc == 4)
        lOutFilename = inArgv[3];
    else
        lOutFilename = "output.png";

    string noyau = inArgv[2];

    cout << "Exécution séquentielle" << endl;
    executerSequentiel(lFilename, noyau, lOutFilename);

    cout << "Exécution parallèle" << endl;
    executerParallel(lFilename, noyau, lOutFilename);
    return 0;
}

