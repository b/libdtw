### libdtw

libdtw is a fast, dynamic time warping library based on the [UCR Suite](http://www.cs.ucr.edu/~eamonn/UCRsuite.html).

### Building

	cd src && make

This will produce libdtw.a. You will need to link against this library.

### Usage

The only interface is the ucr_query() function. The function accepts 6 arguments:

	int ucr_query(double *q, int m, double r, double *buffer, int buflen, struct ucr_index *result);

_q_ is the pattern to be matched.
_m_ is the number of elements in q. 
_r_ is the size of the warping window.
_buffer_ is the data.
_buflen_ is the length of the data array.
_result_ is the output.

result->index is the index of the first element in the best matching sequence in the data. result->value is the
DTW distance between the query and the matching sequence in the data.

ucr_query() returns 0 on success and -1 on failure.

