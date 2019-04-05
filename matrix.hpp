#ifndef MATRIX
#define MATRIX

#include <vector>
#include <iostream>
#include <cassert>
#include <boost/type_traits.hpp>

template<typename E> class matrix_expr { // expression template base class
protected:
  size_t num_rows_;
  size_t num_cols_;

public:
  size_t num_rows() const {
    return num_rows_;
  }

  size_t num_cols() const {
    return num_cols_;
  }

  // at() will be called on each individual part of expr, to evaluate.
  // this parent at() calls derived at()
  auto at(size_t row, size_t col) const{ 
    return static_cast<E const&>(*this).at(row,col);
  }
  
  friend std::ostream& operator<<(std::ostream& stream, const matrix_expr<E> & expr)  {
    if (expr.num_rows() == 0)
      return stream;

    for (size_t i = 0; i < expr.num_rows(); i++) {
      for (size_t j = 0; j < expr.num_cols(); j++) {
        stream << expr.at(i,j) << ' '; 
      }
      stream << '\n';
    }
    return stream;
  }
};

template<typename T>
class matrix : public matrix_expr<matrix<T> > {
private:
  using matrix_expr<matrix<T> >::num_rows_;
  using matrix_expr<matrix<T> >::num_cols_;
  // using 1D vector gives contiguous memory, with some overhead
  std::vector<T> matrix_;
  
public:  
  // could use a variety of different constructors
  // from 1D or 2D containers, etc., but these are sufficient for now 

  matrix<T>() = default;
  
  matrix<T>(size_t rows, size_t columns, std::vector<T> data) : matrix_(data) {
    num_rows_ = rows;
    num_cols_ = columns;
  }
      
  // the following initializes a matrix from a list like so:
  // matrix m = { {1, 2, 3}, {4, 5, 6}, {7, 8, 9} };
  matrix<T>(const std::initializer_list<std::initializer_list<T>> list) {
    size_t cur_row = 0;
    size_t cur_col = 0;
    num_rows_ = list.size();
    num_cols_ = list.begin()->size();

    matrix_.resize(num_rows_* num_cols_);
    
    for (const auto& row : list) {
      cur_col = 0;
      assert(row.size() == num_cols_);
        
      for (const auto& elem : row) {
        this->at(cur_row, cur_col) = elem;
        cur_col++;
      }
      cur_row++;
    }
  }

  T at(size_t row, size_t col) const {
    assert(col <= this->num_cols_);
    assert(row <= this->num_rows_);
    return static_cast<T>(matrix_[row * num_cols_ + col]);
  }

  T& at(size_t row, size_t col) {
    assert(col <= num_cols_);
    assert(row <= num_rows_); 
    return static_cast<T&>(matrix_[row * num_cols_ + col]);
  }

  T max() { // uses generic lambda
    T max_val = matrix_[0];
    for_each(matrix_.begin(),matrix_.end(), [&max_val](auto cur_val) {
        max_val = max_val > cur_val ? max_val : cur_val;
      });
    return max_val;
  }
      
  
  // ctor from any matrix_expr, forces evaluation
  template<typename E>
  matrix<T>(matrix_expr<E> const& expr) {   
    num_rows_ = expr.num_rows();
    num_cols_ = expr.num_cols();
    matrix_.resize(num_rows_ * num_cols_);
    
    for (size_t i = 0; i < num_rows_; i++) 
      for (size_t j = 0; j < num_cols_; j++)
        matrix_[i * num_cols_ + j] = static_cast<T>(expr.at(i,j));
  }
};

// addition expression
template<typename E1, typename E2>
class matrix_sum : public matrix_expr<matrix_sum<E1, E2> > {
  E1 const& lhs_;
  E2 const& rhs_;
  using matrix_expr<matrix_sum<E1, E2> >::num_rows_;
  using matrix_expr<matrix_sum<E1, E2> >::num_cols_;
  
public:
  matrix_sum(E1 const& u, E2 const& v) : lhs_(u), rhs_(v) {
    assert((lhs_.num_rows() == rhs_.num_rows()) &&
           (lhs_.num_cols() == rhs_.num_cols()));
    num_rows_ = lhs_.num_rows();
    num_cols_ = lhs_.num_cols();
  }

  auto at(size_t row, size_t col) const {
    return lhs_.at(row, col) + rhs_.at(row,col);
  }
};
    

template<typename E1, typename E2>
matrix_sum<E1,E2> operator+(E1 const& lhs, E2 const& rhs) {
  return matrix_sum<E1,E2>(lhs,rhs);
}

template<typename E1, typename E2>
matrix_sum<E1,E2> operator+=(E1 const& lhs, E2 const& rhs) {
  return matrix_sum<E1,E2>(lhs,rhs);
}

// subtraction expression
template<typename E1, typename E2>
class matrix_sub : public matrix_expr<matrix_sub<E1, E2> > {
  E1 const& lhs_;
  E2 const& rhs_;
  using matrix_expr<matrix_sub<E1, E2> >::num_rows_;
  using matrix_expr<matrix_sub<E1, E2> >::num_cols_;
  
public:
  matrix_sub(E1 const& u, E2 const& v) : lhs_(u), rhs_(v) {
    assert((lhs_.num_rows() == rhs_.num_rows()) &&
           (lhs_.num_cols() == rhs_.num_cols()));
    num_rows_ = lhs_.num_rows();
    num_cols_ = lhs_.num_cols();
  }

  auto at(size_t row, size_t col) const {
    return lhs_.at(row, col) - rhs_.at(row,col);
  }
};
    

template<typename E1, typename E2>
matrix_sub<E1,E2> operator-(E1 const& lhs, E2 const& rhs) {
  return matrix_sub<E1,E2>(lhs,rhs);
}

template<typename E1, typename E2>
matrix_sub<E1,E2> operator-=(E1 const& lhs, E2 const& rhs) {
  return matrix_sub<E1,E2>(lhs,rhs);
}

// multiplication expression
// uses extremely inefficient algorithm
//
// uses template specializations to handle the differences between
// matrix * matrix multiplication and matrix * scalar multiplication.
//
// doesn't handle 1x1 matrices as scalar -- use a scalar type instead
template<typename E1, typename E2, typename enable = void>
class matrix_prod : public matrix_expr<matrix_prod<E1, E2> > {
  E1 const& lhs_;
  E2 const& rhs_;
  size_t const shared_dim;
  using matrix_expr<matrix_prod<E1, E2> >::num_rows_;
  using matrix_expr<matrix_prod<E1, E2> >::num_cols_;

public:
  matrix_prod(E1 const& lhs, E2 const& rhs) : lhs_(lhs), rhs_(rhs),
                                              shared_dim(lhs.num_cols()) {
    assert(lhs_.num_cols() == rhs_.num_rows());
    num_rows_ = lhs_.num_rows();
    num_cols_ = rhs_.num_cols();
  }
  
  auto at(size_t row, size_t col) const {
    auto dot_product = 0;
    for (size_t i = 0; i < shared_dim; i++) {
      dot_product += lhs_.at(row, i) * rhs_.at(i, col);
    }
    return dot_product;
  }
};
  
template<typename E1, typename E2> // specialization when lhs is scalar
class matrix_prod<E1, E2, typename std::enable_if<std::is_scalar<E1>::value ||
                                                  boost::is_complex<E1>::value
                                                  >::type
                  > : public matrix_expr<matrix_prod<E1, E2> > {
  E1 const& lhs_;
  E2 const& rhs_;
  using matrix_expr<matrix_prod<E1, E2> >::num_rows_;
  using matrix_expr<matrix_prod<E1, E2> >::num_cols_;

public:
  explicit matrix_prod(E1 const& lhs, E2 const& rhs) : lhs_(lhs), rhs_(rhs) {
    num_rows_ = rhs_.num_rows();
    num_cols_ = rhs_.num_cols();
  }

  auto at(size_t row, size_t col) const {
    return lhs_ * rhs_.at(row,col);
  }
};

template<typename E1, typename E2> // specialization when rhs is scalar
class matrix_prod<E1, E2, typename std::enable_if<std::is_scalar<E2>::value ||
                                                  boost::is_complex<E2>::value
                                                  >::type
                  > : public matrix_expr<matrix_prod<E1, E2> > {
  E1 const& lhs_;
  E2 const& rhs_;
  using matrix_expr<matrix_prod<E1, E2> >::num_rows_;
  using matrix_expr<matrix_prod<E1, E2> >::num_cols_;

public:
  explicit matrix_prod(E1 const& lhs, E2 const& rhs) : lhs_(lhs), rhs_(rhs) {
    num_rows_ = lhs_.num_rows();
    num_cols_ = lhs_.num_cols();
  }

  auto at(size_t row, size_t col) const {
    return rhs_ * lhs_.at(row,col);
  }
};

template<typename E1, typename E2>
matrix_prod<E1,E2> operator*(E1 const& lhs, E2 const& rhs) {
  return matrix_prod<E1,E2>(lhs,rhs);
}

template<typename E1, typename E2>
matrix_prod<E1,E2> operator*=(E1 const& lhs, E2 const& rhs) {
  return matrix_prod<E1,E2>(lhs,rhs);
}

#endif
