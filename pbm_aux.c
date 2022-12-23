// THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING

// A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Kevin Ding

#include "pbm.h"
#include <stdio.h>
#include <stdlib.h>

PPMImage *new_ppmimage(unsigned int w, unsigned int h, unsigned int m)
{
    PPMImage *ppm = malloc(sizeof(PPMImage));

    ppm->height = h;
    ppm->width = w;
    ppm->max = m;

    // pixmap is hxw two dimensional
    ppm->pixmap[0] = malloc(sizeof(unsigned int **) * h);
    ppm->pixmap[1] = malloc(sizeof(unsigned int **) * h);
    ppm->pixmap[2] = malloc(sizeof(unsigned int **) * h);

    for (int i = 0; i < 3; i++)
        for (int j = 0; j < h; j++)
            ppm->pixmap[i][j] = malloc(sizeof(unsigned int *) * w);

    return ppm;
}

PBMImage *new_pbmimage(unsigned int w, unsigned int h)
{
    PBMImage *pbm = malloc(sizeof(PBMImage));

    pbm->height = h;
    pbm->width = w;

    pbm->pixmap = malloc(sizeof(unsigned int *) * h);
    for (int i = 0; i < h; i++)
    {
        pbm->pixmap[i] = malloc(sizeof(unsigned int) * w);
    }
    return pbm;
}

PGMImage *new_pgmimage(unsigned int w, unsigned int h, unsigned int m)
{
    PGMImage *pgm = malloc(sizeof(PGMImage));

    pgm->height = h;
    pgm->width = w;
    pgm->max = m;

    pgm->pixmap = malloc(sizeof(unsigned int *) * h);
    for (int i = 0; i < h; i++)
    {
        pgm->pixmap[i] = malloc(sizeof(unsigned int) * w);
    }
    return pgm;
}

void del_ppmimage(PPMImage *p)
{
    unsigned int height = p->height;

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < height; j++)
        {
            free(p->pixmap[i][j]);
        }
    }
    free(p->pixmap[0]);
    free(p->pixmap[1]);
    free(p->pixmap[2]);

    free(p);
}

void del_pgmimage(PGMImage *p)
{
    unsigned int height = p->height;
    for (int i = 0; i < height; i++)
    {
        free(p->pixmap[i]);
    }
    free(p->pixmap);
    free(p);
}

void del_pbmimage(PBMImage *p)
{
    unsigned int height = p->height;
    for (int i = 0; i < height; i++)
    {
        free(p->pixmap[i]);
    }
    free(p->pixmap);
    free(p);
}
