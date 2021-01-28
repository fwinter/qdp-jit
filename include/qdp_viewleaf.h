#ifndef QDP_VIEWLEAF
#define QDP_VIEWLEAF

namespace QDP {

template<class T>
struct LeafFunctor<QDPTypeJIT<T,OLatticeJIT<T> >, ViewLeaf>
{
  typedef typename REGType<T>::Type_t Type_t;
  inline static
  Type_t apply(const QDPTypeJIT<T,OLatticeJIT<T> > & s, const ViewLeaf& v)
  {
    Type_t reg;
    reg.setup( s.elem( v.getLayout() , v.getIndex() ) );
    return reg;
  }
};


#if 1
  //  QDP::PSpinMatrix<QDP::PColorMatrix<QDP::RComplex<QDP::Word<float> >, 3>, 4>

template<class T1>
struct LeafFunctor<QDPTypeJIT< PSpinMatrixJIT<T1,4> , OLatticeJIT< PSpinMatrixJIT<T1,4> > >, ViewLeafSpin>
{
  typedef typename REGType<T1>::Type_t Type_t;
  inline static
  Type_t apply(const QDPTypeJIT< PSpinMatrixJIT<T1,4> , OLatticeJIT< PSpinMatrixJIT<T1,4> > > & s, const ViewLeafSpin& v)
  {
    Type_t reg;
    reg.setup( s.elem( v.getLayout() , v.getIndex() ).getJitElem( v.spin_i , v.spin_j ) );
    return reg;
  }
};

  
template<class T1>
struct LeafFunctor<QDPTypeJIT< PScalarJIT<T1> , OLatticeJIT< PScalarJIT<T1> > >, ViewLeafSpin>
{
  typedef typename REGType<T1>::Type_t Type_t;
  inline static
  Type_t apply(const QDPTypeJIT< PScalarJIT<T1> , OLatticeJIT< PScalarJIT<T1> > > & s, const ViewLeafSpin& v)
  {
    Type_t reg;
    reg.setup( s.elem( v.getLayout() , v.getIndex() ).arrayF(0) );
    return reg;
  }
};

template<class T1>
struct LeafFunctor<QDPTypeJIT< PSpinMatrixJIT<T1,4> , OScalarJIT< PSpinMatrixJIT<T1,4> > >, ViewLeafSpin>
{
  typedef typename REGType<T1>::Type_t Type_t;
  inline static
  Type_t apply(const QDPTypeJIT< PSpinMatrixJIT<T1,4> ,OScalarJIT< PSpinMatrixJIT<T1,4> > > & s, const ViewLeafSpin& v)
  {
    Type_t reg;
    reg.setup( s.elem().getJitElem( v.spin_i , v.spin_j ) );
    return reg;
  }
};

template<class T1>
struct LeafFunctor<QDPTypeJIT< PScalarJIT<T1> , OScalarJIT< PScalarJIT<T1> > >, ViewLeafSpin>
{
  typedef typename REGType<T1>::Type_t Type_t;
  inline static
  Type_t apply(const QDPTypeJIT< PScalarJIT<T1> ,OScalarJIT< PScalarJIT<T1> > > & s, const ViewLeafSpin& v)
  {
    Type_t reg;
    reg.setup( s.elem().arrayF(0) );
    return reg;
  }
};


  
#if 0
template<class T>
struct LeafFunctor<OScalarJIT<T>, ViewLeafSpin>
{
  typedef typename REGType<T>::Type_t Type_t;
  inline static
  Type_t apply(const OScalarJIT<T> & s, const ViewLeafSpin& v)
  {
    Type_t reg;
    reg.setup( s.elem() );
    return reg;
  }
};
#endif
  
  
#endif
  

template<class T>
struct LeafFunctor<QDPTypeJIT<T,OScalarJIT<T> >, ViewLeaf>
{
  typedef typename REGType<T>::Type_t Type_t;
  inline static
  Type_t apply(const QDPTypeJIT<T,OScalarJIT<T> > & s, const ViewLeaf& v)
  {
    Type_t reg;
    reg.setup( s.elem() );
    return reg;
  }
};


template<class T>
struct LeafFunctor<OScalarJIT<T>, ViewLeaf>
{
  typedef typename REGType<T>::Type_t Type_t;
  inline static
  Type_t apply(const OScalarJIT<T> & s, const ViewLeaf& v)
  {
    Type_t reg;
    reg.setup( s.elem() );
    return reg;
  }
};

}

#endif
