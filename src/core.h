struct ud_index
{   
    double      value;
    int64_t     index;
};

void
lower_upper_lemire(double *t, int len, int r, double *l, double *u);

double
lb_kim_hierarchy(double *t, double *q, int j, int len, double mean, double std, double bsf);

double
lb_keogh_cumulative(int* order, double *t, double *uo, double *lo, double *cb, int j, int len, double mean, double std, double bsf);

double
lb_keogh_data_cumulative(int* order, double *tz, double *qo, double *cb, double *l, double *u, int len, double mean, double std, double bsf);

double
dtw(double* A, double* B, double *cb, int m, int r, double bsf);

int
match(double *q, int m, double r, double *buffer, int buflen, struct ud_index *result);
