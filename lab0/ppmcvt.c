// THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING

// A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Kevin Ding

#include "pbm.h"
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>

void bitmap(const char *inputFile, const char *outputFile)
{
    PPMImage *ppm = read_ppmfile(inputFile);
    unsigned int height = ppm->height;
    unsigned int width = ppm->width;

    PBMImage *pbm = new_pbmimage(width, height);
    for (int m = 0; m < height; m++)
    {
        for (int n = 0; n < width; n++)
        {
            double average = (ppm->pixmap[0][m][n] + ppm->pixmap[1][m][n] + ppm->pixmap[2][m][n]) / 3.0;
            double threshold = ppm->max / 2.0;
            if (average < threshold)
                pbm->pixmap[m][n] = 1;
            else
                pbm->pixmap[m][n] = 0;
        }
    }
    del_ppmimage(ppm);
    write_pbmfile(pbm, outputFile);
    del_pbmimage(pbm);
}

void grayScale(const char *inputFile, const char *outputFile, int gmax)
{
    PPMImage *ppm = read_ppmfile(inputFile);
    unsigned int height = ppm->height;
    unsigned int width = ppm->width;

    PGMImage *pgm = new_pgmimage(width, height, gmax);
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            double average = 1.0 * (ppm->pixmap[0][i][j] + ppm->pixmap[1][i][j] + ppm->pixmap[2][i][j]) / 3.0;
            pgm->pixmap[i][j] = (unsigned int)average * gmax / (ppm->max);
        }
    }
    del_ppmimage(ppm);
    write_pgmfile(pgm, outputFile);
    del_pgmimage(pgm);
}

// only leaves the specific channel open, that is, if blue, then all colors other than blue are zero
void isolate(const char *inputFile, const char *outputFile, char *ichannel)
{
    PPMImage *ppm = read_ppmfile(inputFile);
    unsigned int height = ppm->height;
    unsigned int width = ppm->width;
    unsigned int max = ppm->max;
    int channel = -1;
    PPMImage *outputPPM = new_ppmimage(width, height, max);

    if (strcmp(ichannel, "red") == 0)
        channel = 0;
    else if (strcmp(ichannel, "green") == 0)
        channel = 1;
    else
        channel = 2;

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < height; j++)
        {
            for (int k = 0; k < width; k++)
            {
                if (i == channel)
                {
                    outputPPM->pixmap[i][j][k] = ppm->pixmap[i][j][k];
                }
                else
                {
                    outputPPM->pixmap[i][j][k] = 0;
                }
            }
        }
    }
    del_ppmimage(ppm);
    write_ppmfile(outputPPM, outputFile);
    del_ppmimage(outputPPM);
}

void rem(const char *inputFile, const char *outputFile, const char *rchannel)
{
    PPMImage *ppm = read_ppmfile(inputFile);

    unsigned int height = ppm->height;
    unsigned int width = ppm->width;
    unsigned int max = ppm->max;
    int channel = -1;
    PPMImage *outputPPM = new_ppmimage(width, height, max);

    if (strcmp(rchannel, "red") == 0)
        channel = 0;
    else if (strcmp(rchannel, "green") == 0)
        channel = 1;
    else
        channel = 2;

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < height; j++)
        {
            for (int k = 0; k < width; k++)
            {
                if (i != channel)
                {
                    outputPPM->pixmap[i][j][k] = ppm->pixmap[i][j][k];
                }
                else
                {
                    outputPPM->pixmap[i][j][k] = 0;
                }
            }
        }
    }
    del_ppmimage(ppm);
    write_ppmfile(outputPPM, outputFile);
    del_ppmimage(outputPPM);
}

// if above 255 then make it 255
void sepia(char *inputFile, char *outputFile)
{
    PPMImage *ppm = read_ppmfile(inputFile);
    unsigned int height = ppm->height;
    unsigned int width = ppm->width;
    unsigned int max = ppm->max;

    PPMImage *outputPPM = new_ppmimage(width, height, max);
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < height; j++)
        {
            for (int k = 0; k < width; k++)
            {
                if (i == 0)
                {
                    double oldr = 0.393 * ppm->pixmap[i][j][k];
                    double oldg = 0.769 * ppm->pixmap[i + 1][j][k];
                    double oldb = 0.189 * ppm->pixmap[i + 2][j][k];
                    double sum = oldr + oldg + oldb;
                    if (sum > 255.0)
                        outputPPM->pixmap[i][j][k] = 255;
                    else
                        outputPPM->pixmap[i][j][k] = (unsigned int)(oldr + oldg + oldb);
                }
                else if (i == 1)
                {
                    double oldr = 0.349 * ppm->pixmap[i - 1][j][k];
                    double oldg = 0.686 * ppm->pixmap[i][j][k];
                    double oldb = 0.168 * ppm->pixmap[i + 1][j][k];
                    double sum = oldr + oldg + oldb;
                    if (sum > 255.0)
                        outputPPM->pixmap[i][j][k] = 255;
                    else
                        outputPPM->pixmap[i][j][k] = (unsigned int)(oldr + oldg + oldb);
                }
                else
                {
                    double oldr = 0.272 * ppm->pixmap[i - 2][j][k];
                    double oldg = 0.534 * ppm->pixmap[i - 1][j][k];
                    double oldb = 0.131 * ppm->pixmap[i][j][k];
                    double sum = oldr + oldg + oldb;
                    if (sum > 255.0)
                        outputPPM->pixmap[i][j][k] = 255;
                    else
                        outputPPM->pixmap[i][j][k] = (unsigned int)(oldr + oldg + oldb);
                }
            }
        }
    }
    del_ppmimage(ppm);
    write_ppmfile(outputPPM, outputFile);
    del_ppmimage(outputPPM);
}

void mirror(char *inputFile, char *outputFile)
{
    PPMImage *ppm = read_ppmfile(inputFile);
    unsigned int height = ppm->height;
    unsigned int width = ppm->width;
    unsigned int max = ppm->max;

    PPMImage *outputPPM = new_ppmimage(width, height, max);
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < height; j++)
        {
            if (width % 2 == 0) // if width is even
            {
                int middle = width / 2;
                for (int k = 0; k < middle; k++)
                    outputPPM->pixmap[i][j][k] = ppm->pixmap[i][j][k];
                for (int k = middle; k < width; k++)
                    outputPPM->pixmap[i][j][k] = ppm->pixmap[i][j][width - k - 1];
            }
            else
            {
                int middle = width / 2;
                outputPPM->pixmap[i][j][middle] = ppm->pixmap[i][j][middle];
                for (int k = 0; k <= middle - 1; k++)
                    outputPPM->pixmap[i][j][k] = ppm->pixmap[i][j][k];
                for (int k = middle + 1; k < width; k++)
                    outputPPM->pixmap[i][j][k] = ppm->pixmap[i][j][width - k - 1];
            }
        }
    }
    del_ppmimage(ppm);
    write_ppmfile(outputPPM, outputFile);
    del_ppmimage(outputPPM);
}

void thumbnail(char *inputFile, char *outputFile, int tscale)
{
    PPMImage *ppm = read_ppmfile(inputFile);
    unsigned int height = ppm->height;
    unsigned int width = ppm->width;
    unsigned int max = ppm->max;

    unsigned int newHeight;
    unsigned int newWidth;
    if (height % tscale != 0)
        newHeight = ((int)height / tscale) + 1;
    else
        newHeight = (int)height / tscale;

    if (width % tscale != 0)
        newWidth = ((int)width / tscale) + 1;
    else
        newWidth = (int)width / tscale;

    PPMImage *outputPPM = new_ppmimage(newWidth, newHeight, max);

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < newHeight; j++)
        {
            for (int k = 0; k < newWidth; k++)
            {
                outputPPM->pixmap[i][j][k] = ppm->pixmap[i][j * tscale][k * tscale];
            }
        }
    }

    del_ppmimage(ppm);
    write_ppmfile(outputPPM, outputFile);
    del_ppmimage(outputPPM);
}

void nup(char *inputFile, char *outputFile, int nscale)
{
    PPMImage *ppm = read_ppmfile(inputFile);
    unsigned int height = ppm->height;
    unsigned int width = ppm->width;
    unsigned int max = ppm->max;

    unsigned int hbound;
    unsigned int wbound;
    if (height % nscale != 0)
        hbound = ((int)height / nscale) + 1;
    else
        hbound = (int)height / nscale;

    if (width % nscale != 0)
        wbound = ((int)width / nscale) + 1;
    else
        wbound = (int)width / nscale;

    PPMImage *outputPPM = new_ppmimage(width, height, max);
    int rowIndex = 0;
    int colIndex = 0;

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < height; j++)
        {
            if (rowIndex == hbound)
                rowIndex = 0;
            for (int k = 0; k < width; k++)
            {
                if (colIndex == wbound)
                    colIndex = 0;
                outputPPM->pixmap[i][j][k] = ppm->pixmap[i][rowIndex * nscale][colIndex * nscale];
                colIndex++;
            }
            rowIndex++;
        }
    }

    del_ppmimage(ppm);
    write_ppmfile(outputPPM, outputFile);
    del_ppmimage(outputPPM);
}

int main(int argc, char *argv[])
{
    int option;
    int bflag = 0;
    int gflag = 0;
    int gmax;
    int iflag = 0;
    char *ichannel;
    int rflag = 0;
    char *rchannel;
    int sflag = 0;
    int mflag = 0;
    int tflag = 0;
    int tscale;
    int nflag = 0;
    int nscale;
    int oflag = 0;
    int hasOutput = 0;
    int hasInput = 0;
    char *output;
    char *input;
    int malformed = 0;

    int oneOption = 0;

    int inputIndex = 0;

    while ((option = getopt(argc, argv, "bg:i:r:smt:n:o:")) != -1)
    {
        switch (option)
        {
        case 'b':
        {
            bflag++;
            oneOption++;
            break;
        }
        case 'g':
        {
            gflag++;
            oneOption++;
            gmax = strtol(optarg, &optarg, 10);
            break;
        }
        case 'i':
        {
            iflag++;
            oneOption++;
            ichannel = optarg;
            break;
        }
        case 'r':
        {
            rflag++;
            oneOption++;
            rchannel = optarg;
            break;
        }
        case 's':
        {
            sflag++;
            oneOption++;
            break;
        }
        case 'm':
        {
            mflag++;
            oneOption++;
            break;
        }
        case 't':
        {
            tflag++;
            tscale = strtol(optarg, &optarg, 10);
            oneOption++;
            break;
        }
        case 'n':
        {
            nflag++;
            nscale = strtol(optarg, &optarg, 10);
            oneOption++;
            break;
        }
        case 'o':
        {
            oflag++;
            output = optarg;
            hasOutput++;
            break;
        }
        default:
        {
            malformed++;
            break;
        }
        }
    }

    if (malformed == 1)
    {
        printf("Usage: ppmcvt [-bgirsmtno] [FILE]\n");
        exit(1);
    }

    if (oneOption > 1)
    {
        printf("Error: Multiple transformations specified\n");
        exit(1); // 1 means not sucessful
    }

    if (oflag == 0)
    {
        printf("Error: No output file specified\n");
        exit(1);
    }

    if (optind == argc)
    {
        printf("Error: No input file specified\n");
        exit(1);
    }
    else
    {
        input = argv[optind];
    }

    if (oneOption == 0)
    {
        bitmap(input, output);
        exit(0);
    }

    if (bflag == 1)
    {
        bitmap(input, output);
        exit(0); // 0 means sucessful
    }
    if (gflag == 1)
    {
        if (gmax < 1 || gmax > 65535)
        {
            printf("Error: Invalid max grayscale pixel value: %d; must be less than 65,536\n", gmax);
            exit(1);
        }
        else
        {
            grayScale(input, output, gmax);
            exit(0);
        }
    }
    if (iflag == 1)
    {
        if (strcmp(ichannel, "red") == 0 || strcmp(ichannel, "green") == 0 || strcmp(ichannel, "blue") == 0)
        {
            isolate(input, output, ichannel);
            exit(0);
        }
        else
        {
            printf("Error: Invalid channel specification: (%s); should be 'red', 'green' or 'blue'\n", ichannel);
            exit(1);
        }
    }
    if (rflag == 1)
    {
        if (strcmp(rchannel, "red") == 0 || strcmp(rchannel, "green") == 0 || strcmp(rchannel, "blue") == 0)
        {
            rem(input, output, rchannel);
            exit(0);
        }
        else
        {
            printf("Error: Invalid channel specification: (%s); should be 'red', 'green' or 'blue'\n", rchannel);
            exit(1);
        }
    }
    if (sflag == 1)
    {
        sepia(input, output);
        exit(0);
    }
    if (mflag == 1)
    {
        mirror(input, output);
        exit(0);
    }
    if (tflag == 1)
    {
        if (tscale >= 1 && tscale <= 8)
        {
            thumbnail(input, output, tscale);
            exit(0);
        }
        else
        {
            printf("Error: Invalid scale factor: %d; must be 1-8\n", tscale);
            exit(1);
        }
    }
    if (nflag == 1)
    {
        if (nscale >= 1 && nscale <= 8)
        {
            nup(input, output, nscale);
            exit(0);
        }
        else
        {
            printf("Error: Invalid scale factor: %d; must be 1-8\n", nscale);
            exit(1);
        }
    }
    return 0;
}
