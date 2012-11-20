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

struct ucr_index
{   
    double      value;
    int64_t     index;
};

struct ucr_buffer
{
    int32_t     len;
    int32_t     last;
    double      *data;
};

struct ucr_query
{
    int32_t         m, r;
    int32_t         *order;
    double          *q, *u, *l, *qo, *uo, *lo;
};

struct ucr_query*
ucr_query_new(double* query, int32_t m, double r);

void
ucr_query_free(struct ucr_query* query);

int32_t
ucr_query_execute(struct ucr_query *query, struct ucr_buffer *buffer, struct ucr_index *result);

int32_t
ucr_query(double *q, int32_t m, double r, double *buffer, int32_t buflen, struct ucr_index *result);
