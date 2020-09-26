#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

typedef void* FORTRAN_HANDLE;

//External Fortran functions in libmlinwrf.so
extern void* init_neural_net_c(const char*, int*);
extern void free_neural_net_c(FORTRAN_HANDLE);
extern void neural_net_predict_c(double*, int*, int*, FORTRAN_HANDLE, double*);

#define ARRAY_LEN(a) (sizeof(a)/sizeof(a[0]))

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

//Create dynamic 2d array of doubles (in contiguous memory space)
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
        printf("  %.3f", vec[i]);
    }
    printf("\n");
}

int main(int argc, char* argv[]){
    printf("Begin testing libmlinwrf\n");

    test_eval_poly();

    const char* filename = "/glade/p/ral/wsap/petzke/mlsurfacelayer/cabauw_surface_layer_models_30Min_20191121/moisture_scale-neural_network_fortran.nc";
    FORTRAN_HANDLE neural_net_model_handle;
    int batch_size = 1;
    printf("Calling neural_net_init_c\n");
    neural_net_model_handle = init_neural_net_c(filename, &batch_size);

    int row_count = 2;
    int col_count = 16;
    int max_rand = 10;
    double** test_input = malloc2d_d(row_count, col_count);
    printf("C: test_input\n");
    init_random2d_d(row_count, col_count, test_input, max_rand);
    print2d_d(row_count, col_count, test_input);
    double* predict = (double*)malloc(sizeof(double*)*row_count);
    printf("Calling neural_net_predict_c\n");
    neural_net_predict_c(&(test_input[0][0]), &col_count, &row_count, neural_net_model_handle, predict);
    printf("C: predictions (from Fortran)\n");
    print1d_d(row_count, predict);
    printf("Calling free_neural_net_c\n");
    free_neural_net_c(neural_net_model_handle);
    printf("End testing libmlinwrf\n");
}
