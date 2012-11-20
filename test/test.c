/*
    libdtw is derived from the UCR Suite.  The original license is below.
 */
    
/***********************************************************************/
/************************* DISCLAIMER **********************************/
/***********************************************************************/
/** This UCR Suite software is copyright protected © 2012 by          **/
/** Thanawin Rakthanmanon, Bilson Campana, Abdullah Mueen,            **/
/** Gustavo Batista and Eamonn Keogh.                                 **/
/**                                                                   **/
/** Unless stated otherwise, all software is provided free of charge. **/
/** As well, all software is provided on an "as is" basis without     **/
/** warranty of any kind, express or implied. Under no circumstances  **/
/** and under no legal theory, whether in tort, contract,or otherwise,**/
/** shall Thanawin Rakthanmanon, Bilson Campana, Abdullah Mueen,      **/
/** Gustavo Batista, or Eamonn Keogh be liable to you or to any other **/
/** person for any indirect, special, incidental, or consequential    **/
/** damages of any character including, without limitation, damages   **/
/** for loss of goodwill, work stoppage, computer failure or          **/
/** malfunction, or for any and all other damages or losses.          **/
/**                                                                   **/
/** If you do not agree with these terms, then you you are advised to **/
/** not use this software.                                            **/
/***********************************************************************/
/***********************************************************************/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "ucr_dtw.h"

int
main(int argc , char *argv[])
{
    int     i, j;
    int     m = -1;
    int     EPOCH = 100000;
    double  d, r = -1;
    double  *q, *buffer;
    FILE    *fp;
    FILE    *qp;
    struct ucr_index result;

    /// If not enough input, display an error.
    if (argc <= 3)
        exit(1);

    /// read size of the query
    if (argc >= 3)
        m = atol(argv[3]);

    /// read warping windows
    if (argc >= 4)
        r = atof(argv[4]);

    fp = fopen(argv[1],"r");
    if(fp == NULL)
        exit(1);

    qp = fopen(argv[2],"r");
    if(qp == NULL)
        exit(1);

    q = (double *)malloc(sizeof(double) * m);
    if( q == NULL )
        exit(1);

    buffer = (double *)malloc(sizeof(double) * EPOCH);
    if(buffer == NULL)
        exit(1);

    i = 0;
    j = 0;

    while(fscanf(qp,"%lf",&d) != EOF && i < m)
    {
        q[i] = d;
        i++;
    }
    fclose(qp);

    int done = 0;
    int it = 0, ep = 0, k = 0;

    while(done == 0)
    {
        /// Read first m-1 points
        ep = 0;
        if (it == 0)
        {   
            for(k = 0; k < m - 1; k++)
                if (fscanf(fp,"%lf",&d) != EOF)
                    buffer[k] = d;
        }
        else
        {   
            for(k = 0; k < m - 1; k++)
                buffer[k] = buffer[EPOCH - m + 1 + k];
        }

        /// Read buffer of size EPOCH or when all data has been read.
        ep = m - 1;
        while(ep < EPOCH)
        {   
            if (fscanf(fp,"%lf",&d) == EOF)
                break;
            buffer[ep] = d;
            ep++;
        }

        /// Data are read in chunk of size EPOCH.
        /// When there is nothing to read, the loop is end.
        if (ep < EPOCH)
        {
            done = 1;
        }
        else
        {
            if(ucr_query(q, m, r, buffer, ep, &result) < 0)
            {
                printf("Match failed!\n");
                exit(1);
            }

            printf("Location : %lli\n", result.index + (it)*(EPOCH-m+1));
            printf("Distance : %2.5f\n", result.value);
        }
        it++;
    }

    return 0;
}