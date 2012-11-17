/// Data structure (circular array) for finding minimum and maximum for LB_Keogh envolop
struct deque
{
	int *dq;
    int size, capacity;
    int f, r;
};

void deq_new(struct deque *d, int capacity);
void deq_free(struct deque *d);

void deq_push_back(struct deque *d, int v);
void deq_pop_front(struct deque *d);
void deq_pop_back(struct deque *d);

int deq_front(struct deque *d);
int deq_back(struct deque *d);
int deq_empty(struct deque *d);
