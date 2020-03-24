#pragma once

#include <cmath>
#include <iostream>
#include <cassert>
#include <vector>


template <size_t DimCols, size_t DimRows, typename T>
struct mat;

/* Vector */ 
template <size_t DIM, typename T>
struct vec {
    vec() {
        for (size_t i = DIM; i--; data_[i] = T());
    }

    // access with index
    T& operator[](const size_t i) {
        assert(i < DIM);
        return data_[i];
    }

private:
    T data_[DIM];
};

template <typename T>
struct vec<2, T> {
    // constructors    
    vec() 
    : x(T()), y(T()) {}

    vec(T X, T Y)
    : x(X), y(Y) {}

    template <typename U>
    vec<2, T>(const vec<2, U>& v);
    

    T& operator[](const size_t i) {
        assert(i < 2);
        return (i == 0) ? x : y;
    }

    const T& operator[](const size_t i) const {
        assert(i < 2);
        return (i == 0) ? x : y;
    }

    T x, y;
};

template <typename T>
struct vec<3, T> {
    // constructors
    vec()
    : x(T()), y(T()), z(T()) {}
    
    vec(T X, T Y, T Z) 
    : x(X), y(Y), z(Z) {}

    template <typename U>
    vec<3, T>(const vec<3, U>& v);

    
    T& operator[](size_t i) {
        assert(i < 3);
        // if (i == 0) return x;
        // else if (i == 1) return y;
        // else return z;
        return (i == 0) ? x : ((i == 1) ? y : z);
    }

    const T& operator[](size_t i) const {
        assert(i < 3);
        // if (i == 0) return x;
        // else if (i == 1) return y;
        // else return z;
        return (i == 0) ? x : ((i == 1) ? y : z);
    }

    float norm() {
        return std::sqrt(x*x + y*y + z*z);
    }

    vec<3, T> normalize(T l = 1) {
        // *this = (*this) * (l / this->norm());
        // return *this;
        float n = this->norm();
        vec<3, T> ret;
        ret[0] = this->x * (l / n);
        ret[1] = this->y * (l / n);
        ret[2] = this->z * (l / n);
        // std::cout << ret[0] << " " << ret[1] << " " << ret[2] << std::endl;
        return ret;
    }

    T x, y, z;
};


/* override ops for vec */
template <size_t DIM, typename T>
T operator*(const vec<DIM, T>& va, const vec<DIM, T>& vb) {
    T ret = T();
    for (size_t i = DIM; i--; ret += va[i] * vb[i]);
    return ret;
}

// template <size_t DIM, typename T>
// vec<DIM, T> operator*(vec<DIM, T> va, const vec<DIM, T>& vb) {
//     for (size_t i = DIM; i--; va[i] *= vb[i]);
//     return va;
// }

template <size_t DIM, typename T>
vec<DIM, T> operator+(vec<DIM, T> va, const vec<DIM, T>& vb) {
    for (size_t i = DIM; i--; va[i] += vb[i]);
    return va;
}

template <size_t DIM, typename T> 
vec<DIM, T> operator-(vec<DIM, T> va, const vec<DIM, T>& vb) {
    for (size_t i = DIM; i--; va[i] = vb[i] - va[i]);
    return va;
}

template <size_t DIM, typename T>
vec<DIM, T> operator/(vec<DIM, T> va, const vec<DIM, T>& vb) {
    for (size_t i = DIM; i--; va[i] /= vb[i]);
    return va;
}


template <size_t LEN, size_t DIM, typename T> 
vec<LEN, T> embed(const vec<DIM,T>& v, T fill = 1) {
    vec<LEN,T> ret;
    for (size_t i=LEN; i--; ret[i] = (i < DIM ? v[i] : fill));
    return ret; 
}


template <size_t LEN, size_t DIM, typename T>
vec<LEN, T> proj(const vec<DIM, T>& v) {
    vec<LEN, T> ret;
    for (size_t i = LEN; i--; ret[i] = v[i])
    return ret;
}

template <typename T>
vec<3, T> cross_product(vec<3, T> va, vec<3, T> vb) {
    return vec<3, T>(va.y*vb.z - va.z*vb.y, va.z*vb.x - va.x*vb.z, va.x*vb.y - va.y*vb.x);
}

template <size_t DIM, typename T>
std::ostream& operator<<(std::ostream& out, vec<DIM, T>& v) {
    for (unsigned int i = 0; i < DIM; ++i) {
        out << v[i] << " ";
    }
    return out;
}


/* dt */
template<size_t DIM,typename T> 
struct dt {
    static T det(const mat<DIM,DIM,T>& src) {
        T ret=0;
        for (size_t i=DIM; i--; ret += src[0][i]*src.cofactor(0,i));
        return ret;
    }
};

template<typename T> 
struct dt<1,T> {
    static T det(const mat<1,1,T>& src) {
        return src[0][0];
    }
};



/* matrix */
template <size_t DimRows, size_t DimCols, typename T>
struct mat {
    
public:
    mat() {}
    vec<DimCols, T>& operator[](const size_t idx) {
        assert(idx < DimRows);
        return rows[idx];
    }
    const vec<DimCols, T>& operator[](const size_t idx) const {
        assert(idx < DimRows);
        return rows[idx];
    }


    vec<DimRows, T> col(const size_t idx) const {
        assert(idx < DimCols);
        vec<DimRows, T> ret;
        for (size_t i = DimRows; i--; ret[i] = rows[i][idx]);
        return ret;
    }

    void set_col(size_t idx, vec<DimRows, T> v) {
        assert(idx < DimCols);
        for (size_t i = DimRows; i--; rows[i][idx] = v[i]);
    }

    void set_row(size_t idx, vec<DimRows, T> v) {
        assert(idx < DimRows);
        // vec<DimRows, T> tmp = proj<DimCols>(v);
        // for (size_t i = DimCols; i--; rows[idx][i] = tmp[i]);
        for (size_t i = DimCols; i--; rows[idx][i] = v[i]);
    }

    static mat<DimRows, DimCols, T> identity() {
        mat<DimRows, DimCols, T> ret;
        for (size_t i = DimRows; i--; ) 
            for (size_t j = DimCols; j--; ret[i][j] = (i == j));
        return ret;
    }

    T det() const {
        return dt<DimCols, T>::det(*this);
    }

    mat<DimRows - 1, DimCols - 1, T>
    get_minor(size_t row, size_t col) const {
        mat<DimRows-1, DimCols-1, T> ret;
        for (size_t i = DimRows; i--; ) 
            for (size_t j=DimCols-1;j--; ret[i][j]=rows[i<row?i:i+1][j<col?j:j+1]);
        
        return ret;
    }

    T cofactor(size_t row, size_t col) const {
        return get_minor(row,col).det()*((row+col)%2 ? -1 : 1);
    }

    mat<DimRows,DimCols,T> adjugate() const {
        mat<DimRows,DimCols,T> ret;
        for (size_t i=DimRows; i--; )
            for (size_t j=DimCols; j--; ret[i][j]=cofactor(i,j));
        return ret;
    }

    mat<DimRows,DimCols,T> invert_transpose() {
        mat<DimRows,DimCols,T> ret = adjugate();
        T tmp = ret[0]*rows[0];
        return ret/tmp;
    }

    mat<DimRows,DimCols,T> invert() {
        return invert_transpose().transpose();
    }

    mat<DimCols,DimRows,T> transpose() {
        mat<DimCols,DimRows,T> ret;
        for (size_t i=DimCols; i--; ret[i]=this->col(i));
        return ret;
    }

    vec<DimCols, T> rows[DimRows];
};




/* override ops */
template<size_t DimRows,size_t DimCols,typename T> 
vec<DimRows, T> operator*(const mat<DimRows, DimCols, T>& lhs, const vec<DimCols, T>& rhs) {
    vec<DimRows, T> ret;
    for (size_t i = DimRows; i--; ret[i] = lhs[i] * rhs);
    return ret;
}

template<size_t R1,size_t C1,size_t C2,typename T>
mat<R1,C2,T> operator*(const mat<R1,C1,T>& lhs, const mat<C1,C2,T>& rhs) {
    mat<R1,C2,T> result;
    for (size_t i=R1; i--; )
        for (size_t j=C2; j--; result[i][j]=lhs[i]*rhs.col(j));
    return result;
}

template<size_t DimRows,size_t DimCols,typename T>
mat<DimCols,DimRows,T> operator/(mat<DimRows,DimCols,T> lhs, const T& rhs) {
    for (size_t i=DimRows; i--; lhs[i]=lhs[i]/rhs);
    return lhs;
}

template <size_t DimRows,size_t DimCols,class T> std::ostream& operator<<(std::ostream& out, mat<DimRows,DimCols,T>& m) {
    for (size_t i=0; i<DimRows; i++) out << m[i] << std::endl;
    return out;
}


/* typedef */
typedef vec<2,  float> Vec2f;
typedef vec<2,  int>   Vec2i;
typedef vec<3,  float> Vec3f;
typedef vec<3,  int>   Vec3i;
typedef vec<4,  float> Vec4f;
typedef mat<4,4,float> Matrix;
