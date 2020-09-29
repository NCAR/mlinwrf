#!/usr/bin/env python

import argparse
import os

import numpy as np

#Ignore Tensorflow Warnings, Info
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '2' 
import models

"""
Script to train/test neural net and random forest models with .nc and .csv output for prediction testing in Fortran module_neural_net.f90 and module_random_forest.f90

Example Usage: ./getn_test_modules.py
               ./getn_test_modules.py -v
"""

np.set_printoptions(formatter={'float':lambda x: "%.3f"%x})


def eval_poly(coeff,x):
    """
    Evaluate polynomial represented as integer coefficients at x in linear time.

    Test f(x)=4x^3+3x^2+2x+1, f(1)=10
    >>> abs(eval_poly([1,2,3,4],1.0)-10) < 1e-7
    True

    Test f(x)=4x^3+2x, f(2)=36
    >>> abs(eval_poly([0,2,0,4],2.0)-36) < 1e-7
    True
    """
    sum = coeff[-1]
    for a in reversed(coeff[:-1]):
        sum = sum*x+a
    return sum


def random_floats(seed, min, max, n):
    """
    Linear congruential random number generator to create n random float values in a range of [min,max]
    """
    #Constants from Numerical Recipes
    #Modulus 2**32
    m = 4294967296 
    a = 1664525
    c = 1013904223

    rand_max = m-1
    rand_range = float(max-min)
    xi = seed%m
    xf = min+float(xi)/float(rand_max)*rand_range
    xl = [xf]
    for i in range(n-1):
        xi = (a*xi + c)%m
        xf = min+float(xi)/float(rand_max)*rand_range
        xl.append(xf)
    return xl
    

def gen_random_train_test(sample_size,test_size):
    """
    Returns x_train,y_train,x_test,y_test arrays for training/testing models.  Values are generated using a 5th order polynomial with random x values in the range [-1.5,3]
    """
    seed = 31
    train_size = 1-test_size
    #5x^5 - 20x^4 + 5x^3 + 50x^2 - 20x - 40
    coeff = [-40,-20,50,5,-20,5]
    xs = random_floats(seed,-1.5, 3, sample_size)
    ys = [eval_poly(coeff,x) for x in xs]
    x_test,y_test = xs[int(train_size*sample_size):],ys[int(train_size*sample_size):]
    x_train,y_train = xs[:int(train_size*sample_size)],ys[:int(train_size*sample_size)]
    return np.array(x_train).reshape(len(x_train),1),np.array(y_train),np.array(x_test).reshape(len(x_test),1),np.array(y_test)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-v","--verbose",dest="verbose",help="Provide extra debugging information",action="store_true")
    args = parser.parse_args()

    if args.verbose: print("Starting gen_test_models.py")
    if args.verbose: print("Initializing sample data");
    x_train,y_train,x_test, y_test = gen_random_train_test(200,0.25)
    if args.verbose:
        print("x_train:\n",x_train)
        print("y_train:\n",y_train)
        print("x_test:\n", x_test)
        print("y_test:\n",y_test)

    if args.verbose: print("Initializing neural network")
    nn = models.DenseNeuralNetwork()
    nn.build_neural_network(1,1)
    if args.verbose: print("Training neural network")
    nn.fit(x_train,y_train)
    if args.verbose: print("Saving neural network")
    nn.save_fortran_model("1d_test.nc")
    if args.verbose: print("Predicting test set")
    y_hat = nn.predict(x_test)
    for y in y_hat:
        print("%.3f"%y)

if __name__ == '__main__':
    main()
