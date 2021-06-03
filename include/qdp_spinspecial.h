#ifndef QDP_SPINSPECIAL
#define QDP_SPINSPECIAL

namespace QDP
{
  template<class T> struct Reference;

  template<class LeafType, class LeafTag>
  struct LeafFunctor
  { };

  template<class Expr, class FTag, class CTag, class Enable = void>
  struct ForEach
  {
    typedef typename LeafFunctor<Expr, FTag>::Type_t Type_t;
    // inline static
    // Type_t apply(const Expr &expr, const FTag &f, const CTag &)
    // {
    //   return LeafFunctor<Expr, FTag>::apply(expr, f);
    // }
  };

  struct TreeCombine  {};
  struct JIT2BASE  {};

  
  template<class T>
  struct Strip
  {
    typedef T Type_t;
  };

  template<class T,int N>
  struct Strip<Reference<PSpinMatrix<T,N> > >
  {
    typedef PSpinMatrix<float,N> Type_t;
  };

  template<class T,int N>
  struct Strip<PSpinMatrix<T,N> >
  {
    typedef PSpinMatrix<float,N> Type_t;
  };

  
  template<class A>
  struct IsSpinMatrix
  {
    typedef typename ForEach<A , JIT2BASE , TreeCombine >::Type_t A_a;

    constexpr static bool value = is_same<typename Strip< typename ForEach<A_a, EvalLeaf1, OpCombine>::Type_t >::Type_t , PSpinMatrix<float, 4> >::value;
  };

  
  template<class Op, class A, class B, class FTag>
  struct IsSpecial
  {
    static constexpr bool value = false;
  };

  
  
} // namespace QDP

  
#endif
