//
//  ESLinearRegression.hpp
//
//  Created by Steve Pucci 06 Jul 2017
//  Copyright Emerald Sequoia LLC 2017. All rights reserved.
//
//  Based on C# implementation at
//  https://www.codeproject.com/Articles/25335/An-Algorithm-for-Weighted-Linear-Regression
//
//  and C++ matrix code from 
//  https://isocpp.org/wiki/faq/operator-overloading#matrix-subscript-op
//

#ifndef _ESLINEARREGRESSION_HPP_
#define _ESLINEARREGRESSION_HPP_

#include "ESErrorReporter.hpp"

/** class description */
class ESLinearRegression {
  public:
    class Matrix {
      public:
                                Matrix(int rows, int cols);
                                ~Matrix();                             // Destructor
                                Matrix(const Matrix& m);               // Copy constructor

        Matrix                  &operator= (const Matrix& m);          // Assignment operator

        double                  &operator() (int row, int col);        // Subscript operator
        double                  operator() (int row, int col) const;   // Subscript operator
        int                     rows() const { return rows_; }
        int                     cols() const { return cols_; }
        void                    print() const;

      private:
        int rows_;
        int cols_;
        double* data_;
    };

                            ESLinearRegression(const double Y[],     // observed results
                                               const Matrix &X,      // row# is variable#, col# is observation#
                                               const double W[]);    // weights, one per observation (use 1.0 for unweighted)
                            ~ESLinearRegression();

    static bool             invertInPlaceSymmetricMatrix(Matrix *matrix);

    bool                    valid;   // True if linear regression was successful

    Matrix                  V;       // Least squares and var/covar matrix
    double                  *C;      // Coefficients
    double                  *SEC;    // Std Error of coefficients
    double                  RYSQ;    // Multiple correlation coefficient
    double                  SDV;     // Standard deviation of errors
    double                  FReg;    // Fisher F statistic for regression
    double                  *Ycalc;  // Calculated values of Y
    double                  *DY;     // Residual values of Y
};

inline
ESLinearRegression::Matrix::Matrix(int rows, int cols)
:   rows_ (rows), 
    cols_ (cols) 
{
    ESAssert(rows > 0 && cols > 0);
    data_ = new double[rows * cols];
}

inline
ESLinearRegression::Matrix::~Matrix()
{
  delete[] data_;
}

inline
double &
ESLinearRegression::Matrix::operator() (int row, int col)
{
    ESAssert(row >= 0 && row < rows_);
    ESAssert(col >= 0 && col < cols_);
    return data_[cols_ * row + col];
}

inline double 
ESLinearRegression::Matrix::operator() (int row, int col) const
{
    ESAssert(row >= 0 && row < rows_);
    ESAssert(col >= 0 && col < cols_);
    return data_[cols_ * row + col];
}        

#endif  // _ESLINEARREGRESSION_HPP_
