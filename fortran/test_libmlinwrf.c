#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#include <unistd.h>

/*
Application provides a test framework for loading .nc and .csv ML model output generated by models.py and testing them in the libmlinwrf.so library created from fortran modules module_neural_net.f90 and module_random_forest.f90
*/

//#define TEST 1

typedef void* FORTRAN_HANDLE;

//External Fortran functions in libmlinwrf.so
extern FORTRAN_HANDLE init_neural_net_c(const char*, int*, int*);
extern void free_neural_net_c(FORTRAN_HANDLE);
extern void neural_net_predict_c(double*, int*, int*, FORTRAN_HANDLE, double*);

extern FORTRAN_HANDLE load_random_forest_c(const char*,int*);
extern void free_random_forest_c(FORTRAN_HANDLE);
extern void random_forest_predict_c(double*, int*, int*, FORTRAN_HANDLE, double*);

#define ARRAY_LEN(a) (sizeof(a)/sizeof(a[0]))

//Evaluate polynomial represented as integer coefficients at x in linear time.
double eval_poly(double coeff[], int coeff_len, double x){
    int i;
    double sum = coeff[coeff_len-1];
    for(i = 1; i < coeff_len; i++){
      sum = sum*x+coeff[coeff_len-i-1];
    }
    return sum;
}

void test_eval_poly(){
    printf("RUNNING test_eval_poly\n");
    //Test f(x)=4x^3+3x^2+2x+1, f(1)=10
    double coeff[] = {1,2,3,4};
    assert(fabs(eval_poly(coeff,ARRAY_LEN(coeff),1.0)-10.0)< 1e-7 && "eval_poly function failed");

    //Test f(x)=4x^3+2x, f(2)=36
    coeff[0] = coeff[2] = 0;
    assert(fabs(eval_poly(coeff,ARRAY_LEN(coeff),2.0)-36.0)< 1e-7 && "eval_poly function failed");
    printf("SUCCESS test_eval_poly\n");
}

void random_floats(long seed, double min, double max, unsigned int n, double* xl){
    //Linear congruential random number generator to create n random float values in a range of [min,max]
    //Constants from Numerical Recipes
    //Modulus 2**32
    long m = 4294967296; 
    long a = 1664525;
    long c = 1013904223;

    long rand_max = m-1;
    double rand_range = max-min;
    long xi;
    double xf;

    xi = seed%m;    
    xf = min+xi/rand_max*rand_range;
    xl[0] = xf;
    int i;
    for (i=1; i<n; i++){
        xi = (a*xi + c)%m;
        xf = min+(double)(xi)/(double)(rand_max)*rand_range;
	xl[i] = xf;
    }
}

//Generates random train and test X,y data. NULL can be passed for any of the 2D pointers to ignore populating these values.
void gen_random_train_test(unsigned int train_n, unsigned int test_n, double** x_train, double** y_train, double** x_test, double** y_test){
    long seed = 31;
    unsigned int data_len = train_n+test_n;
    double xs[data_len];
    random_floats(seed, -1.5, 3, train_n+test_n,xs);
    //5x^5 - 20x^4 + 5x^3 + 50x^2 - 20x - 40
    double coeff[] = {-40,-20,50,5,-20,5};
    double ys[data_len];
    int i;
    for(i=0; i < data_len; i++){
      ys[i] = eval_poly(coeff,ARRAY_LEN(coeff),xs[i]);
    }
    for(i=0; i < train_n; i++){
      if (x_train) x_train[i][0] = xs[i];
      if (y_train) y_train[i][0] = ys[i];
    }
    for(i=0; i < test_n; i++){
      if (x_test) x_test[i][0] = xs[train_n+i];
      if (y_test) y_test[i][0] = ys[train_n+i];
    }
}

//Create dynamic 2d array of doubles (in contiguous memory space)
//Free using free(A[0]); free(A);
double** malloc2d_d(int row_count, int col_count) {
    double* data = (double*)malloc(row_count*col_count*sizeof(double));
    double** array= (double**)malloc(row_count*sizeof(double*));
    int i;
    for (i=0; i<row_count; i++){
        array[i] = &(data[col_count*i]);
    }
    return array;
}

//Initialize dynamic 2d array of doubles with random values
void init_random2d_d(int row_count, int col_count, double** matrix, double max_random) {
    srand(time(0));
    int i;
    int j;
    for (i=0; i<row_count; i++){
        for(j=0; j<col_count; j++){
            matrix[i][j] = ((double)rand()/(double)(RAND_MAX))*max_random;
	}
    }
}

//Print 2d dynamic array of doubles
void print2d_d(int row_count, int col_count, double** matrix) {
    int i;
    int j;
    for (i=0; i<row_count; i++){
        for(j=0; j<col_count; j++){
	  printf("  %.3f", matrix[i][j]);
	}
	printf("\n");
    }
}

//Prints out a 1d array of doubles
void print1d_d(int count, double* vec){
    int i;
    for (i=0; i<count; i++){
        printf("%.3f\n", vec[i]);
    }
}

void print_usage(){
  fprintf(stderr, "Usage: %s [-v] model_type model_path\n", "test_libmlinwrf");
}

void test_neural_net(const char* model_path, int verbose, unsigned int row_count, unsigned int col_count, double** x_test, double* predict){
    FORTRAN_HANDLE neural_net_model_handle;
    int batch_size = 1;
    if (verbose) printf("Calling neural_net_init_c\n");
    int model_path_len = strlen(model_path);
    neural_net_model_handle = init_neural_net_c(model_path, &model_path_len, &batch_size);
    if (verbose) printf("Calling neural_net_predict_c\n");
    neural_net_predict_c(&(x_test[0][0]), &col_count, &row_count, neural_net_model_handle, predict);
    if (verbose) printf("C: predictions (from Fortran)\n");
    print1d_d(row_count, predict);
    if (verbose) printf("Calling free_neural_net_c\n");
    free_neural_net_c(neural_net_model_handle);
}

void test_random_forest(const char* model_path, int verbose, unsigned int row_count, unsigned int col_count, double** x_test, double* predict){
    FORTRAN_HANDLE random_forest_handle;
    if (verbose) printf("Calling load_random_forest_c\n");
    int model_path_len = strlen(model_path);
    random_forest_handle = load_random_forest_c(model_path,&model_path_len);
    if (verbose) printf("Calling random_forest_predict_c\n");
    random_forest_predict_c(&(x_test[0][0]), &col_count, &row_count, random_forest_handle, predict);
    if (verbose) printf("C: predictions (from Fortran)\n");
    print1d_d(row_count, predict);
    if (verbose) printf("Calling free_random_forest_c\n");
    free_random_forest_c(random_forest_handle);
}

int main(int argc, char* argv[]){
    int verbose = 0;
    int opt;

    while ((opt = getopt(argc, argv, "v")) != -1) {
       switch (opt) {
       case 'v': verbose = 1; break;
       case '?':
	   print_usage();
           exit(2);
       }
    }
    if (verbose) printf("Begin testing libmlinwrf\n");

    int argidx = optind;
    if (argidx == argc){
        print_usage();
        exit(EXIT_FAILURE); 
    }
    const char* model_type = argv[optind++];
    if (argidx == argc){
        print_usage();
        exit(EXIT_FAILURE); 
    }
    const char* model_path = argv[optind++];
    
#ifdef TEST
    test_eval_poly();
#endif
    
    if (verbose) printf("Initializing sample data\n");
    unsigned int sample_size = 200;
    double test_size = 0.25;
    double train_size = 1-test_size;
    unsigned int train_n = (unsigned int)(train_size*sample_size);
    unsigned int test_n = (unsigned int)(test_size*sample_size);
    double**  x_test = malloc2d_d(test_n,1);
    double**  y_test = malloc2d_d(test_n,1);
    gen_random_train_test(train_n,test_n,NULL,NULL,x_test,y_test);
    if (verbose) {
	printf("x_test:\n");
	print2d_d(test_n,1,x_test);
	printf("y_test:\n");
	print2d_d(test_n,1,y_test);
    }

    unsigned int row_count = test_n;
    unsigned int col_count = 1;
    double* predict = (double*)malloc(sizeof(double*)*row_count);

    if (strncmp(model_type,"neural_net_nc",strlen("neural_net_nc")) == 0){
        if (verbose) printf("Calling test_neural_net\n");
        test_neural_net(model_path, verbose, row_count, col_count, x_test, predict);
    }
    else if(strncmp(model_type,"random_forest_csv",strlen("random_forest_csv")) == 0){
        if (verbose) printf("Calling test_random_forest\n");
	test_random_forest(model_path, verbose, row_count, col_count, x_test, predict);
    }
    else{
        printf("Error: Unknown model_type\n");
    }

    if (verbose) printf("Freeing C arrays\n");
    free(x_test[0]);
    free(x_test);
    free(y_test[0]);
    free(y_test);
    free(predict);

    if (verbose) printf("End testing libmlinwrf\n");
}
