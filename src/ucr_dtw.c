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
#include <sys/types.h>

#include "deque.h"
#include "ucr_dtw.h"

#define min(x,y) ((x)<(y)?(x):(y))
#define max(x,y) ((x)>(y)?(x):(y))
#define dist(x,y) ((x-y)*(x-y))

/// Sorting function for the query, sort by abs(z_norm(q[i])) from high to low
int
ucr_comp(const void *a, const void* b)
{
    struct ucr_index* x = (struct ucr_index*)a;
    struct ucr_index* y = (struct ucr_index*)b;
    return abs((int64_t)(y->value)) - abs((int64_t)(x->value));   // high to low
}

/// Finding the envelop of min and max value for LB_Keogh
/// Implementation idea is intoruduced by Danial Lemire in his paper
/// "Faster Retrieval with a Two-Pass Dynamic-Time-Warping Lower Bound", Pattern Recognition 42(9), 2009.
void
ucr_lower_upper_lemire(double *t, int len, int r, double *l, double *u)
{
    struct deque du, dl;
    int i = 0;

    deq_new(&du, 2 * r + 2);
    deq_new(&dl, 2 * r + 2);

    deq_push_back(&du, 0);
    deq_push_back(&dl, 0);

    for (i = 1; i < len; i++)
    {
        if (i > r)
        {
            u[i - r - 1] = t[deq_front(&du)];
            l[i - r - 1] = t[deq_front(&dl)];
        }
        if (t[i] > t[i - 1])
        {
            deq_pop_back(&du);
            while (!deq_empty(&du) && t[i] > t[deq_back(&du)])
            {
                deq_pop_back(&du);
            }
        }
        else
        {
            deq_pop_back(&dl);
            while (!deq_empty(&dl) && t[i] < t[deq_back(&dl)])
            {
                deq_pop_back(&dl);
            }
        }

        deq_push_back(&du, i);
        deq_push_back(&dl, i);
        if (i == 2 * r + 1 + deq_front(&du))
        {
            deq_pop_front(&du);
        }
        else if (i == 2 * r + 1 + deq_front(&dl))
        {
            deq_pop_front(&dl);            
        }
    }

    for (i = len; i < len + r + 1; i++)
    {
        u[i - r - 1] = t[deq_front(&du)];
        l[i - r - 1] = t[deq_front(&dl)];
        if (i - deq_front(&du) >= 2 * r + 1)
        {
            deq_pop_front(&du);
        }
        if (i - deq_front(&dl) >= 2 * r + 1)
        {
            deq_pop_front(&dl);
        }
    }

    deq_free(&du);
    deq_free(&dl);
}

/// Calculate quick lower bound
/// Usually, LB_Kim take time O(m) for finding top,bottom,fist and last.
/// However, because of z-normalization the top and bottom cannot give siginifant benefits.
/// And using the first and last points can be computed in constant time.
/// The prunning power of LB_Kim is non-trivial, especially when the query is not long, say in length 128.
double
ucr_lb_kim_hierarchy(double *t, double *q, int j, int len, double mean, double std, double bsf)
{
    /// 1 point at front and back
    double d, lb;
    double x0 = (t[j] - mean) / std;
    double y0 = (t[(len - 1 + j)] - mean) / std;

    lb = dist(x0, q[0]) + dist(y0, q[len - 1]);
    if (lb >= bsf)
    {
        return lb;
    }

    /// 2 points at front
    double x1 = (t[(j + 1)] - mean) / std;

    d = min(dist(x1, q[0]), dist(x0, q[1]));
    d = min(d, dist(x1, q[1]));
    lb += d;
    if (lb >= bsf)
    {
        return lb;
    }

    /// 2 points at back
    double y1 = (t[(len - 2 + j)] - mean) / std;

    d = min(dist(y1, q[len - 1]), dist(y0, q[len - 2]));
    d = min(d, dist(y1, q[len - 2]));
    lb += d;
    if (lb >= bsf)
    {
        return lb;
    }

    /// 3 points at front
    double x2 = (t[(j + 2)] - mean) / std;

    d = min(dist(x0, q[2]), dist(x1, q[2]));
    d = min(d, dist(x2, q[2]));
    d = min(d, dist(x2, q[1]));
    d = min(d, dist(x2, q[0]));
    lb += d;
    if (lb >= bsf)
    {
        return lb;
    }
    /// 3 points at back
    double y2 = (t[(len - 3 + j)] - mean) / std;

    d = min(dist(y0, q[len - 3]), dist(y1, q[len - 3]));
    d = min(d, dist(y2, q[len - 3]));
    d = min(d, dist(y2, q[len - 2]));
    d = min(d, dist(y2, q[len - 1]));
    lb += d;

    return lb;
}

/// LB_Keogh 1: Create Envelop for the query
/// Note that because the query is known, envelop can be created once at the begenining.
///
/// Variable Explanation,
/// order : sorted indices for the query.
/// uo, lo: upper and lower envelops for the query, which already sorted.
/// t     : a circular array keeping the current data.
/// j     : index of the starting location in t
/// cb    : (output) current bound at each position. It will be used later for early abandoning in DTW.
double
ucr_lb_keogh_cumulative(int* order, double *t, double *uo, double *lo, double *cb, int j, int len,
                        double mean, double std, double bsf)
{
    double  lb = 0;
    double  x, d;
    int     i = 0;

    for (i = 0; i < len && lb < bsf; i++)
    {
        x = (t[(order[i] + j)] - mean) / std;
        d = 0;
        if (x > uo[i])
        {
            d = dist(x, uo[i]);
        }
        else if(x < lo[i])
        {
            d = dist(x, lo[i]);
        }
        lb += d;
        cb[order[i]] = d;
    }
    return lb;
}

/// LB_Keogh 2: Create Envelop for the data
/// Note that the envelops have been created (in main function) when each data point has been read.
///
/// Variable Explanation,
/// tz: Z-normalized data
/// qo: sorted query
/// cb: (output) current bound at each position. Used later for early abandoning in DTW.
/// l, u: lower and upper envelop of the current data
double
ucr_lb_keogh_data_cumulative(int* order, double *tz, double *qo, double *cb, double *l, double *u,
                             int len, double mean, double std, double bsf)
{
    double  lb = 0;
    double  uu, ll, d;
    int     i = 0;

    for (i = 0; i < len && lb < bsf; i++)
    {
        uu = (u[order[i]] - mean) / std;
        ll = (l[order[i]] - mean) / std;
        d = 0;
        if (qo[i] > uu)
        {
            d = dist(qo[i], uu);
        }
        else if(qo[i] < ll)
        {
            d = dist(qo[i], ll);
        }
        lb += d;
        cb[order[i]] = d;
    }
    return lb;
}

/// Calculate Dynamic Time Wrapping distance
/// A,B: data and query, respectively
/// cb : cummulative bound used for early abandoning
/// r  : size of Sakoe-Chiba warpping band
double
ucr_dtw(double* A, double* B, double *cb, int m, int r, double bsf)
{
    double  *cost;
    double  *cost_prev;
    double  *cost_tmp;
    int     i, j, k;
    double  x, y, z, min_cost;

    /// Instead of using matrix of size O(m^2) or O(mr), we will reuse two array of size O(r).
    cost = (double*)malloc(sizeof(double) * (2 * r + 1));
    for(k = 0; k < 2 * r + 1; k++)
    {
        cost[k] = INFINITY;
    }

    cost_prev = (double*)malloc(sizeof(double) * (2 * r + 1));
    for(k = 0; k < 2 * r + 1; k++)
    {
        cost_prev[k] = INFINITY;
    }

    for (i = 0; i < m; i++)
    {
        k = max(0, r - i);
        min_cost = INFINITY;

        for(j = max(0, i - r); j <= min(m - 1, i + r); j++, k++)
        {
            /// Initialize all row and column
            if ((i == 0) && (j == 0))
            {
                cost[k] = dist(A[0], B[0]);
                min_cost = cost[k];
                continue;
            }

            if ((j - 1 < 0) || (k - 1 < 0))     y = INFINITY;
            else                                y = cost[k - 1];
            if ((i - 1 < 0) || (k + 1 > 2 * r)) x = INFINITY;
            else                                x = cost_prev[k + 1];
            if ((i - 1 < 0) || (j - 1 < 0))     z = INFINITY;
            else                                z = cost_prev[k];

            /// Classic DTW calculation
            cost[k] = min(min(x, y), z) + dist(A[i], B[j]);

            /// Find minimum cost in row for early abandoning (possibly to use column instead of row).
            if (cost[k] < min_cost)
            { 
                min_cost = cost[k];
            }
        }

        /// We can abandon early if the current cummulative distace with lower bound together are larger than bsf
        if (i + r < m - 1 && min_cost + cb[i + r + 1] >= bsf)
        {   free(cost);
            free(cost_prev);
            return min_cost + cb[i + r + 1];
        }

        /// Move current array to previous array.
        cost_tmp = cost;
        cost = cost_prev;
        cost_prev = cost_tmp;
    }
    k--;

    /// the DTW distance is in the last cell in the matrix of size O(m^2) or at the middle of our array.
    double final_dtw = cost_prev[k];
    free(cost);
    free(cost_prev);
    return final_dtw;
}

struct ucr_query*
ucr_query_new(double *query, int32_t m, double r)
{
    int32_t             i = 0, o = 0;;
    struct ucr_index    *q_tmp;
    struct ucr_query    *udq;
    double              ex = 0, ex2 = 0, mean = 0, std = 0;

    udq = (struct ucr_query*)malloc(sizeof(struct ucr_query));
    if(udq == NULL)
        goto query_new_cleanup;

    udq->q = (double *)malloc(sizeof(double) * m);
    if(udq->q == NULL)
        goto query_new_cleanup;

    udq->qo = (double *)malloc(sizeof(double) * m);
    if(udq->qo == NULL)
        goto query_new_cleanup;

    udq->lo = (double *)malloc(sizeof(double) * m);
    if(udq->lo == NULL)
        goto query_new_cleanup;

    udq->uo = (double *)malloc(sizeof(double) * m);
    if(udq->uo == NULL)
        goto query_new_cleanup;

    udq->order = (int32_t *)malloc(sizeof(int32_t) * m);
    if(udq->order == NULL)
        goto query_new_cleanup;

    udq->l = (double *)malloc(sizeof(double) * m);
    if(udq->l == NULL)
        goto query_new_cleanup;

    udq->u = (double *)malloc(sizeof(double) * m);
    if(udq->u == NULL)
        goto query_new_cleanup;

    /// Initialize query components
    udq->m = m;
    if (r <= 1)
        udq->r = floor(r * m);
    else
        udq->r = floor(r);

    for(i = 0; i < m; i++)
    {
        udq->q[i] = query[i];
        ex += query[i];
        ex2 += query[i] * query[i];
    }

    /// Do z-normalize the query, keep in same array, q
    mean = ex / m;
    std = ex2 / m;
    std = sqrt(std - mean * mean);

    for(i = 0; i < m; i++)
    {
         udq->q[i] = (udq->q[i] - mean) / std;
    }

    /// Create envelope of the query: lower envelope, l, and upper envelope, u
    ucr_lower_upper_lemire(udq->q, udq->m, udq->r, udq->l, udq->u);

    q_tmp = (struct ucr_index*)malloc(sizeof(struct ucr_index) * m);
    if(q_tmp == NULL)
        goto query_new_cleanup;

    /// Sort the query one time by abs(z-norm(q[i]))
    for(i = 0; i < m; i++)
    {
        q_tmp[i].value = udq->q[i];
        q_tmp[i].index = i;
    }
    qsort(q_tmp, m, sizeof(struct ucr_index), ucr_comp);

    /// also create another arrays for keeping sorted envelope
    for(i = 0; i < m; i++)
    {
        o = q_tmp[i].index;
        udq->order[i] = o;
        udq->qo[i] = udq->q[o];
        udq->lo[i] = udq->l[o];
        udq->uo[i] = udq->u[o];
    }
    free(q_tmp);

    return udq;

query_new_cleanup:
    ucr_query_free(udq);

    return NULL;
}

void
ucr_query_free(struct ucr_query* query)
{
    if(query == NULL)
        return;
    if(query->q != NULL)
        free(query->q);
    if(query->u != NULL)
        free(query->u);
    if(query->l != NULL)
        free(query->l);
    if(query->uo != NULL)
        free(query->uo);
    if(query->lo != NULL)
        free(query->lo);
    if(query->qo != NULL)
        free(query->qo);

    return;
}

int32_t
ucr_query_execute(struct ucr_query *query, struct ucr_buffer *buffer, struct ucr_index *result)
{
    double          d;
    double          ex, ex2, mean, std, dist, bsf;
    double          lb_kim = 0, lb_k = 0, lb_k2 = 0;
    double          *q, *cb, *cb1, *cb2;
    double          *qo, *uo, *lo;
    double          *t, *tz, *u_buff, *l_buff;
    int64_t         i = 0, j = 0, I = 0, loc = -1;
    int32_t         *order;
    int32_t         r, m, k = 0;

    if(buffer->last < 0)
        goto query_execute_cleanup;

    dist = 0;
    bsf = INFINITY;

    q = query->q;
    m = query->m;
    r = query->r;
    order = query->order;
    qo = query->qo;
    lo = query->lo;
    uo = query->uo;

    cb = (double *)malloc(sizeof(double) * m);
    if(cb == NULL)
        goto query_execute_cleanup;

    cb1 = (double *)malloc(sizeof(double) * m);
    if(cb1 == NULL)
        goto query_execute_cleanup;

    cb2 = (double *)malloc(sizeof(double) * m);
    if(cb2 == NULL)
        goto query_execute_cleanup;

    /// Initialize the cumulative lower bound
    for(i = 0; i < m; i++)
    {
        cb[i] = 0;
        cb1[i] = 0;
        cb2[i] = 0;
    }

    t = (double *)malloc(sizeof(double) * m * 2);
    if(t == NULL)
        goto query_execute_cleanup;

    tz = (double *)malloc(sizeof(double) * m);
    if(tz == NULL)
        goto query_execute_cleanup;

    u_buff = (double *)malloc(sizeof(double) * (buffer->len));
    if(u_buff == NULL)
        goto query_execute_cleanup;

    l_buff = (double *)malloc(sizeof(double) * (buffer->len));
    if(l_buff == NULL)
        goto query_execute_cleanup;

    ucr_lower_upper_lemire(buffer->data, buffer->len, r, l_buff, u_buff);

    /// Do main task here..
    ex = ex2 = 0;
    for(i = 0; i < buffer->len; i++)
    {
        /// A bunch of data has been read and pick one of them at a time to use
        d = buffer->data[i];

        /// Calcualte sum and sum square
        ex += d;
        ex2 += d * d;

        /// t is a circular array for keeping current data
        t[i % m] = d;

        /// Double the size for avoiding using modulo "%" operator
        t[(i % m) + m] = d;

        /// Start the task when there are more than m-1 points in the current chunk
        if( i >= m - 1 )
        {
            mean = ex / m;
            std = ex2 / m;
            std = sqrt(std - mean * mean);

            /// compute the start location of the data in the current circular array, t
            j = (i + 1) % m;
            /// the start location of the data in the current chunk
            I = i - (m - 1);

            /// Use a constant lower bound to prune the obvious subsequence
            lb_kim = ucr_lb_kim_hierarchy(t, q, j, m, mean, std, bsf);

            if (lb_kim < bsf)
            {
                /// Use a linear time lower bound to prune; z_normalization of t will be computed on the fly.
                /// uo, lo are envelop of the query.
                lb_k = ucr_lb_keogh_cumulative(order, t, uo, lo, cb1, j, m, mean, std, bsf);
                if (lb_k < bsf)
                {
                    /// Take another linear time to compute z_normalization of t.
                    /// Note that for better optimization, this can merge to the previous function.
                    for(k = 0; k < m; k++)
                    {
                        tz[k] = (t[(k + j)] - mean) / std;
                    }

                    /// Use another lb_keogh to prune
                    /// qo is the sorted query. tz is unsorted z_normalized data.
                    /// l_buff, u_buff are big envelop for all data in this chunk
                    lb_k2 = ucr_lb_keogh_data_cumulative(order, tz, qo, cb2, l_buff + I, u_buff + I, m, mean, std, bsf);
                    if (lb_k2 < bsf)
                    {
                        /// Choose better lower bound between lb_keogh and lb_keogh2 to be used in early abandoning DTW
                        /// Note that cb and cb2 will be cumulative summed here.
                        if (lb_k > lb_k2)
                        {
                            cb[m - 1] = cb1[m - 1];
                            for(k = m - 2; k >= 0; k--)
                                cb[k] = cb[k + 1] + cb1[k];
                        }
                        else
                        {
                            cb[m - 1] = cb2[m - 1];
                            for(k = m - 2; k >= 0; k--)
                                cb[k] = cb[k + 1] + cb2[k];
                        }

                        /// Compute DTW and early abandoning if possible
                        dist = ucr_dtw(tz, q, cb, m, r, bsf);

                        if( dist < bsf )
                        {   /// Update bsf
                            /// loc is the real starting location of the nearest neighbor in the file
                            bsf = dist;
                            loc = i - m + 1;
                        }
                    }
                }
            }

            /// Reduce obsolute points from sum and sum square
            ex -= t[j];
            ex2 -= t[j] * t[j];
        }
    }

query_execute_cleanup:
    if(t != NULL)
        free(t);
    if(tz != NULL)
        free(tz);
    if(l_buff != NULL)
        free(l_buff);
    if(u_buff != NULL)
        free(u_buff);
    if(cb != NULL)
        free(cb);
    if(cb1 != NULL)
        free(cb1);
    if(cb2 != NULL)
        free(cb2);

    if(loc == -1)
    {
        return loc;
    }

    result->index = loc;
    result->value = sqrt(bsf);

    return 0;
}

int32_t
ucr_query(double* query, int32_t m, double r, double* values, int32_t vlen, struct ucr_index *result)
{
    struct ucr_buffer    *b = NULL;
    struct ucr_query     *q = NULL;
    int32_t              e;

    q = ucr_query_new(query, m, r);
    if(q == NULL)
        return -1;

    b = (struct ucr_buffer *)malloc(sizeof(struct ucr_buffer));
    if(b == NULL)
        return -1;

    b->data = values;
    b->len = vlen;
    b->last = vlen - 1;

    e = ucr_query_execute(q, b, result);
    ucr_query_free(q);

    return e;
}
