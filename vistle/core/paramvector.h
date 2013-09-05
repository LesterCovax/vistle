#ifndef PARAMVECTOR_H
#define PARAMVECTOR_H

#include <ostream>
#include <vector>
#include "scalar.h"
#include "dimensions.h"
#include "exception.h"
#include "export.h"

namespace vistle {

template<typename S>
class V_COREEXPORT ParameterVector {
public:
   typedef S Scalar;

   ParameterVector(const int dim, const S values[]);
   ParameterVector(const S x, const S y, const S z, const S w);
   ParameterVector(const S x, const S y, const S z);
   ParameterVector(const S x, const S y);
   ParameterVector(const S x);
   ParameterVector();
   ParameterVector(const ParameterVector &other);

   ParameterVector &operator=(const ParameterVector &rhs);

   static ParameterVector min(int dim=MaxDimension);
   static ParameterVector max(int dim=MaxDimension);

   int dim;
   std::vector<S> v;
   std::vector<S> m_min, m_max;
   S &x, &y, &z, &w;

#if 0
   // negate
   ParameterVector operator-() const;

   // scalar multiplication
   ParameterVector operator*(S const & rhs) const;

   // vector addition
   ParameterVector operator+(ParameterVector const & rhs) const;

   // vector supstraction
   ParameterVector operator-(ParameterVector const & rhs) const;

   // scalar product
   S operator*(ParameterVector const & rhs) const;
#endif

   S &operator[](int i) { return v[i]; }
   const S &operator[](int i) const { return v[i]; }

   operator S *() { return v; }
   operator const S *() const { return v; }

   operator S &() { assert(dim==1); return v[0]; }
   operator S() const { assert(dim==1); return dim>0 ? v[0] : S(); }

   std::string str() const;

public:
   // for vector_indexing_suite
   typedef Scalar value_type;
   typedef size_t size_type;
   typedef size_t index_type;
   typedef ssize_t difference_type;
   size_t size() { return dim; }
   typedef typename std::vector<S>::iterator iterator;
   void erase(iterator s) { throw except::not_implemented(); v.erase(s); }
   void erase(iterator s, iterator e) { throw except::not_implemented(); v.erase(s, e); }
   iterator begin() { return v.begin(); }
   iterator end() { return v.begin()+dim; }
   void insert(iterator pos, const S &value) { throw except::not_implemented(); v.insert(pos, value); }
   template<class InputIt>
   void insert(iterator pos, InputIt first, InputIt last) { throw except::not_implemented(); v.insert(pos, first, last); }
   ParameterVector(iterator begin, iterator end);
   void push_back(const S &value) {
      if (dim >= MaxDimension) {
         throw vistle::exception("out of range");
      }
      v[dim] = value;
      ++dim;
   }
};


template<typename S>
V_COREEXPORT bool operator==(const ParameterVector<S> &v1, const ParameterVector<S> &v2);

template<typename S>
V_COREEXPORT bool operator<(const ParameterVector<S> &v1, const ParameterVector<S> &v2);
template<typename S>
V_COREEXPORT bool operator>(const ParameterVector<S> &v1, const ParameterVector<S> &v2);

//typedef ParameterVector<Scalar> ScalarVector;
//typedef ScalarVector Vector;
typedef ParameterVector<double> ParamVector;

} // namespace vistle

template<typename S>
V_COREEXPORT std::ostream &operator<<(std::ostream &out, const vistle::ParameterVector<S> &v);
#endif // PARAMVECTOR_H


#ifdef VISTLE_VECTOR_IMPL
#include "paramvector_impl.h"
#endif
