//
//  ESLinearRegression.cpp
//
//  Created by Steve Pucci 06 Jul 2017
//  Copyright Emerald Sequoia LLC 2017. All rights reserved.
//
//  Based on C# implementation at
//  https://www.codeproject.com/Articles/25335/An-Algorithm-for-Weighted-Linear-Regression
//

#include "ESLinearRegression.hpp"

#include "ESUtil.hpp"

#include <math.h>
#include <string.h>  // For memcpy

ESLinearRegression::Matrix::Matrix(const ESLinearRegression::Matrix &other)
: rows_(other.rows_),
  cols_(other.cols_)
{
    data_ = new double[rows_ * cols_];
    memcpy(data_, other.data_, rows_ * cols_ * sizeof(double));
}

ESLinearRegression::Matrix &
ESLinearRegression::Matrix::operator=(const ESLinearRegression::Matrix &other)
{
    rows_ = other.rows_;
    cols_ = other.cols_;
    data_ = new double[rows_ * cols_];
    memcpy(data_, other.data_, rows_ * cols_ * sizeof(double));
    return *this;
}

void
ESLinearRegression::Matrix::print() const
{
    for (int i = 0; i < rows_; i++) {
        std::string s;
        for (int j = 0; j < cols_; j++) {
            double val = (*this)(i, j);
            s += ESUtil::stringWithFormat("  %10.3f", val);
        }
        ESErrorReporter::logInfo("ESLinearRegression matrix", "%s", s.c_str());
    }
}

/*static*/ bool 
ESLinearRegression::invertInPlaceSymmetricMatrix(Matrix *matrix)
{
    if (matrix->rows() != matrix->cols()) {
        ESErrorReporter::logError("ESLinearRegression", "Non-square matrix");
        return false;  // symmetric matrices also have to be square
    }
    int N = matrix->rows();

    // ESErrorReporter::logInfo("ESLinearRegression", "Matrix to be inverted:\n");
    // matrix->print();

    double *t = new double[N];
    double *Q = new double[N];
    double *R = new double[N];

    double AB;
    int K, L, M;

    for (M = 0; M < N; M++)
        R[M] = 1;
    K = 0;
    for (M = 0; M < N; M++)
    {
        double Big = 0;
        for (L = 0; L < N; L++)
        {
            AB = fabs((*matrix)(L, L));
            if ((AB > Big) && (R[L] != 0))
            {
                Big = AB;
                K = L;
            }
        }
        if (Big == 0)
        {
            delete [] t;
            delete [] Q;
            delete [] R;
            ESErrorReporter::logError("ESLinearRegression", "Big is 0");
            return false;
        }
        R[K] = 0;
        Q[K] = 1 / (*matrix)(K, K);
        t[K] = 1;
        (*matrix)(K, K) = 0;
        if (K != 0)
        {
            for (L = 0; L < K; L++)
            {
                t[L] = (*matrix)(L, K);
                if (R[L] == 0)
                    Q[L] = (*matrix)(L, K) * Q[K];
                else
                    Q[L] = -(*matrix)(L, K) * Q[K];
                (*matrix)(L, K) = 0;
            }
        }
        if ((K + 1) < N)
        {
            for (L = K + 1; L < N; L++)
            {
                if (R[L] != 0)
                    t[L] = (*matrix)(K, L);
                else
                    t[L] = -(*matrix)(K, L);
                Q[L] = -(*matrix)(K, L) * Q[K];
                (*matrix)(K, L) = 0;
            }
        }
        for (L = 0; L < N; L++)
            for (K = L; K < N; K++)
                (*matrix)(L, K) = (*matrix)(L, K) + t[L] * Q[K];
    }
    M = N;
    L = N - 1;
    for (K = 1; K < N; K++)
    {
        M = M - 1;
        L = L - 1;
        for (int J = 0; J <= L; J++)
            (*matrix)(M, J) = (*matrix)(J, M);
    }

    delete [] t;
    delete [] Q;
    delete [] R;

    return true;
}


ESLinearRegression::~ESLinearRegression() {
    delete [] C;
    delete [] SEC;
    delete [] Ycalc;
    delete [] DY;
}

ESLinearRegression::ESLinearRegression(const double Y[],    // observed results
                                       const Matrix &X,     // row# is variable#, col# is observation#
                                       const double W[])    // weights, one per observation
:   C(new double[X.rows()]),
    SEC(new double[X.rows()]),
    Ycalc(new double[X.cols()]),
    DY(new double[X.cols()]),
    V(X.rows(), X.rows())
{

    // ESErrorReporter::logInfo("ESLinearRegression", "Input matrix:");
    // X.print();

    // Y[j]   = j-th observed data point
    // X[i,j] = j-th value of the i-th independent varialble
    // W[j]   = j-th weight value

    int M = X.cols();             // M = Number of data points
    int N = X.rows();         // N = Number of linear terms
    int NDF = M - N;              // Degrees of freedom
    // If not enough data, don't attempt regression
    if (NDF < 1)
    {
        ESErrorReporter::logError("ESLinearRegression", "NDF (%d) too small (%d sample(s), %d variable(s))", NDF, M, N);
        valid = false;
        return;
    }
    double *B = new double[N];   // Vector for LSQ

    // Clear the matrices to start out
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            V(i, j) = 0;

    // Form Least Squares Matrix
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            V(i, j) = 0;
            for (int k = 0; k < M; k++)
                V(i, j) = V(i, j) + W[k] * X(i, k) * X(j, k);
        }
        B[i] = 0;
        for (int k = 0; k < M; k++)
            B[i] = B[i] + W[k] * X(i, k) * Y[k];
    }
    // V now contains the raw least squares matrix
    if (!invertInPlaceSymmetricMatrix(&V))
    {
        delete [] B;
        valid = false;
        ESErrorReporter::logError("ESLinearRegression", "Matrix inversion failed");
        return;
    }
    // V now contains the inverted least square matrix
    // Matrix multpily to get coefficients C = VB
    for (int i = 0; i < N; i++)
    {
        C[i] = 0;
        for (int j = 0; j < N; j++)
            C[i] = C[i] + V(i, j) * B[j];
    }
    
    // Calculate statistics
    double TSS = 0;
    double RSS = 0;
    double YBAR = 0;
    double WSUM = 0;
    for (int k = 0; k < M; k++)
    {
        YBAR = YBAR + W[k] * Y[k];
        WSUM = WSUM + W[k];
    }
    YBAR = YBAR / WSUM;
    for (int k = 0; k < M; k++)
    {
        Ycalc[k] = 0;
        for (int i = 0; i < N; i++)
            Ycalc[k] = Ycalc[k] + C[i] * X(i, k);
        DY[k] = Ycalc[k] - Y[k];
        TSS = TSS + W[k] * (Y[k] - YBAR) * (Y[k] - YBAR);
        RSS = RSS + W[k] * DY[k] * DY[k];
    }
    double SSQ = RSS / NDF;
    RYSQ = 1 - RSS / TSS;
    FReg = 9999999;
    if (RYSQ < 0.9999999)
        FReg = RYSQ / (1 - RYSQ) * NDF / (N - 1);
    SDV = sqrt(SSQ);
    
    // Calculate var-covar matrix and std error of coefficients
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
            V(i, j) = V(i, j) * SSQ;
        SEC[i] = sqrt(V(i, i));
    }
    delete [] B;
    valid = true;
}
