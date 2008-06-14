// $Id: qdp_scalarsite_sse_blas_dble.h,v 1.2 2008-06-13 21:20:38 bjoo Exp $

/*! @file
 * @brief Generic Scalarsite  optimization hooks
 * 
 * 
 */


#ifndef QDP_SCALARSITE_SSE_BLAS_DOUBLE_H
#define QDP_SCALARSITE_SSE_BLAS_DOUBLE_H

#include "scalarsite_sse/sse_blas_vaxpy4_double.h"
#include "scalarsite_sse/sse_blas_vaxmyz4_double.h"
#include "scalarsite_sse/sse_blas_vaxpbyz4_double.h"
#include "scalarsite_sse/sse_blas_vaxmbyz4_double.h"
#include "scalarsite_generic/generic_blas_vscal.h"
#include "scalarsite_generic/generic_blas_local_sumsq.h"
#include "scalarsite_generic/generic_blas_local_vcdot.h"
#include "scalarsite_generic/generic_blas_local_vcdot_real.h"

namespace QDP {

// Types needed for the expression templates. 
// Bugger Staggered! For Wilson, Ns=4 is nice and cache line
// aligned on a 64byte line (4 vectors=>12 complex=24 doubles.

typedef PSpinVector<PColorVector<RComplex<REAL64>, 3>, 4> TVec;
typedef PScalar<PScalar<RScalar<REAL64> > >  TScal;

// #define DEBUG_BLAS
// TVec is the LatticeFermion from qdp_dwdefs.h with the OLattice<> stripped
// from around it

// TScalar is the usual Real, with the OScalar<> stripped from it
//
// THis is simply to make the code more readable, and reduces < < s and > >s
// in the template arguments

// d += Scalar*Vec
template<>
inline
void evaluate(OLattice< TVec >& d, 
	      const OpAddAssign& op, 
	      const QDPExpr<BinaryNode<OpMultiply, 
	      Reference< QDPType< TScal, OScalar < TScal > > >,
	      Reference< QDPType< TVec, OLattice< TVec > > > >,
	      OLattice< TVec > > &rhs,
	      const Subset& s)
{

#ifdef DEBUG_BLAS
  QDPIO::cout << "y += a*x" << endl;
#endif

  const OLattice< TVec >& x = static_cast<const OLattice< TVec > &>(rhs.expression().right());
  const OScalar< TScal >& a = static_cast<const OScalar< TScal > &> (rhs.expression().left());
  
  REAL64 ar = a.elem().elem().elem().elem();
  REAL64* aptr = &ar;

  if( s.hasOrderedRep() ) { 
    REAL64* xptr = (REAL64 *)&(x.elem(s.start()).elem(0).elem(0).real());
    REAL64* yptr = &(d.elem(s.start()).elem(0).elem(0).real());
  // cout << "Specialised axpy a ="<< ar << endl;
  
    int n_4vec = (s.end()-s.start()+1);
    vaxpy4(yptr, aptr, xptr, n_4vec);
  }
  else { 
    const int* tab = s.siteTable().slice();
    for(int j=0; j < s.numSiteTable(); j++) { 
      int i=tab[j];
      REAL64* xptr = (REAL64 *)&(x.elem(i).elem(0).elem(0).real());
      REAL64* yptr = &(d.elem(i).elem(0).elem(0).real());
      vaxpy4(yptr, aptr, xptr, 1);
    }
  }
}

// d -= Scalar*Vec
template<>
inline
void evaluate(OLattice< TVec >& d, 
	      const OpSubtractAssign& op, 
	      const QDPExpr<BinaryNode<OpMultiply, 
	      Reference< QDPType< TScal, OScalar < TScal > > >,
	      Reference< QDPType< TVec, OLattice< TVec > > > >,
	      OLattice< TVec > > &rhs,
	      const Subset& s)
{

#ifdef DEBUG_BLAS
  QDPIO::cout << "y -= a*x" << endl;
#endif

  const OLattice< TVec >& x = static_cast<const OLattice< TVec > &>(rhs.expression().right());
  const OScalar< TScal >& a = static_cast<const OScalar< TScal > &> (rhs.expression().left());

  // - sign as y -= ax <=> y = y-ax = -ax + y = axpy with -a 
  REAL64 ar = -( a.elem().elem().elem().elem());
  REAL64* aptr = &ar;
  if( s.hasOrderedRep() ) { 
    REAL64* xptr = (REAL64 *)&(x.elem(s.start()).elem(0).elem(0).real());
    REAL64* yptr = &(d.elem(s.start()).elem(0).elem(0).real());
    
    int n_4vec = (s.end()-s.start()+1);
    vaxpy4(yptr, aptr, xptr, n_4vec);
  }
  else { 
    const int* tab = s.siteTable().slice();
    for(int j=0; j < s.numSiteTable(); j++) { 
      int i=tab[j];
      REAL64* xptr = (REAL64 *)&(x.elem(i).elem(0).elem(0).real());
      REAL64* yptr = &(d.elem(i).elem(0).elem(0).real());
      vaxpy4(yptr, aptr, xptr, 1);
    }
  }
	
}

// z = ax + y
template<>
inline
void evaluate( OLattice< TVec > &d,
	       const OpAssign &op,
	       const QDPExpr< 
	       BinaryNode<OpAdd,
	        BinaryNode<OpMultiply, 
	         Reference< QDPType< TScal, OScalar< TScal > > >,
	         Reference< QDPType< TVec, OLattice< TVec > > > >,
	        Reference< QDPType< TVec, OLattice< TVec > > > >,
	        OLattice< TVec > > &rhs,
	       const Subset& s)
{

#ifdef DEBUG_BLAS
  QDPIO::cout << "z = a*x + y" << endl;
#endif

  // Peel the stuff out of the expression
  // y is the right side of rhs
  const OLattice< TVec >& y = static_cast<const OLattice< TVec >&> (rhs.expression().right());

  // ax is the left side of rhs and is in a binary node
  typedef BinaryNode<OpMultiply, 
    Reference< QDPType< TScal, OScalar< TScal > > >,
    Reference< QDPType< TVec, OLattice< TVec > > > > BN;

  // get the binary node
  const BN &mulNode = static_cast<const BN&> (rhs.expression().left());

  // get a and x out of the bynary node
  const OScalar< TScal >& a = static_cast<const OScalar< TScal >&>(mulNode.left());
  const OLattice< TVec >& x = static_cast<const OLattice< TVec >&>(mulNode.right());
  // Set pointers 
  REAL64 ar =  a.elem().elem().elem().elem();
  REAL64 *aptr = (REAL64 *)&ar;
  if( s.hasOrderedRep() ) { 
    
    REAL64 *xptr = (REAL64 *) &(x.elem(s.start()).elem(0).elem(0).real());
    REAL64 *yptr = (REAL64 *) &(y.elem(s.start()).elem(0).elem(0).real());
    REAL64* zptr =  &(d.elem(s.start()).elem(0).elem(0).real());
    
    
    // Get the no of 3vecs. s.start() and s.end() are inclusive so add +1
    int n_4vec = (s.end()-s.start()+1);
    vaxpyz4(zptr, aptr, xptr, yptr, n_4vec);
  }
  else { 
    const int* tab = s.siteTable().slice();
    for(int j=0; j < s.numSiteTable(); j++) { 
      int i=tab[j];
      REAL64* xptr = (REAL64 *)&(x.elem(i).elem(0).elem(0).real());
      REAL64* yptr = (REAL64 *)&(y.elem(i).elem(0).elem(0).real());
      REAL64* zptr =  &(d.elem(i).elem(0).elem(0).real());
      vaxpyz4(zptr, aptr, xptr, yptr, 1);
    }
  }

}


// Vec = Vec + Scal*Vec
template<>
inline
void evaluate( OLattice< TVec > &d,
	       const OpAssign &op,
	       const QDPExpr< 
	       BinaryNode<OpAdd,
	        Reference< QDPType< TVec, OLattice< TVec > > >,
	        BinaryNode<OpMultiply, 
	         Reference< QDPType< TScal, OScalar< TScal > > >,
	         Reference< QDPType< TVec, OLattice< TVec > > > > >,
	       OLattice< TVec > > &rhs,
	       const Subset& s)
{
#ifdef DEBUG_BLAS
  QDPIO::cout << "z = y + a*x" << endl;
#endif


  // Peel the stuff out of the expression

  // y is the left side of rhs
  const OLattice< TVec >& y = static_cast<const OLattice< TVec >&> (rhs.expression().left());

  // ax is the right side of rhs and is in a binary node
  typedef BinaryNode<OpMultiply, 
    Reference< QDPType< TScal, OScalar< TScal > > >,
    Reference< QDPType< TVec, OLattice< TVec > > > > BN;

  // get the binary node
  const BN &mulNode = static_cast<const BN&> (rhs.expression().right());

  // get a and x out of the bynary node
  const OScalar< TScal >& a = static_cast<const OScalar< TScal >&>(mulNode.left());
  const OLattice< TVec >& x = static_cast<const OLattice< TVec >&>(mulNode.right());
  // Set pointers 
  REAL64 ar =  a.elem().elem().elem().elem();
  REAL64 *aptr = (REAL64 *)&ar;
  if( s.hasOrderedRep() ) { 
    REAL64 *xptr = (REAL64 *) &(x.elem(s.start()).elem(0).elem(0).real());
    REAL64 *yptr = (REAL64 *) &(y.elem(s.start()).elem(0).elem(0).real());
    REAL64* zptr =  &(d.elem(s.start()).elem(0).elem(0).real());
    
    // Get the no of 3vecs. s.start() and s.end() are inclusive so add +1
    int n_4vec = (s.end()-s.start()+1);
    vaxpyz4(zptr, aptr, xptr, yptr, n_4vec);
  }
  else { 
    const int* tab = s.siteTable().slice();
    for(int j=0; j < s.numSiteTable(); j++) { 
      int i=tab[j];
      REAL64* xptr = (REAL64 *)&(x.elem(i).elem(0).elem(0).real());
      REAL64* yptr = (REAL64 *)&(y.elem(i).elem(0).elem(0).real());
      REAL64* zptr =  &(d.elem(i).elem(0).elem(0).real());
      vaxpyz4(zptr, aptr, xptr, yptr, 1);
    }
  }

}

// Vec = Scalar*Vec - Vec
template<>
inline
void evaluate( OLattice< TVec > &d,
	       const OpAssign &op,
	       const QDPExpr< 
	       BinaryNode<OpSubtract,
	        BinaryNode<OpMultiply, 
	         Reference< QDPType< TScal, OScalar< TScal > > >,
	         Reference< QDPType< TVec, OLattice< TVec > > > >,
	        Reference< QDPType< TVec, OLattice< TVec > > > >,
	        OLattice< TVec > > &rhs,
	       const Subset& s)
{
#ifdef DEBUG_BLAS
  QDPIO::cout << "z = a*x - y" << endl;
#endif


  const OLattice< TVec >& y = static_cast<const OLattice< TVec >&> (rhs.expression().right());

  // ax is the left side of rhs and is in a binary node
  typedef BinaryNode<OpMultiply, 
    Reference< QDPType< TScal, OScalar< TScal > > >,
    Reference< QDPType< TVec, OLattice< TVec > > > > BN;

  // get the binary node
  const BN &mulNode = static_cast<const BN&> (rhs.expression().left());

  // get a and x out of the bynary node
  const OScalar< TScal >& a = static_cast<const OScalar< TScal >&>(mulNode.left());
  const OLattice< TVec >& x = static_cast<const OLattice< TVec >&>(mulNode.right());
  // Set pointers 
  REAL64 ar =  a.elem().elem().elem().elem();
  REAL64 *aptr = (REAL64 *)&ar;
  if( s.hasOrderedRep() ) {
    REAL64 *xptr = (REAL64 *) &(x.elem(s.start()).elem(0).elem(0).real());
    REAL64 *yptr = (REAL64 *) &(y.elem(s.start()).elem(0).elem(0).real());
    REAL64* zptr =  &(d.elem(s.start()).elem(0).elem(0).real());
    
    
    // Get the no of 3vecs. s.start() and s.end() are inclusive so add +1
    int n_3vec = (s.end()-s.start()+1);
    vaxmyz4(zptr, aptr, xptr, yptr, n_3vec);
  }
  else { 
    const int* tab = s.siteTable().slice();
    for(int j=0; j < s.numSiteTable(); j++) { 
      int i=tab[j];
      REAL64* xptr = (REAL64 *)&(x.elem(i).elem(0).elem(0).real());
      REAL64* yptr = (REAL64 *)&(y.elem(i).elem(0).elem(0).real());
      REAL64* zptr =  &(d.elem(i).elem(0).elem(0).real());
      vaxmyz4(zptr, aptr, xptr, yptr, 1);
    }
  }

}

template<>
inline
void evaluate( OLattice< TVec > &d,
	       const OpAssign &op,
	       const QDPExpr< 
	       BinaryNode<OpSubtract,
	        Reference< QDPType< TVec, OLattice< TVec > > >,
	        BinaryNode<OpMultiply, 
	         Reference< QDPType< TScal, OScalar< TScal > > >,
	         Reference< QDPType< TVec, OLattice< TVec > > > > >,
	       OLattice< TVec > > &rhs,
	       const Subset& s)
{
#ifdef DEBUG_BLAS
  QDPIO::cout << "z = y - a*x" << endl;
#endif

  const OLattice< TVec >& y = static_cast<const OLattice< TVec >&> (rhs.expression().left());

  // ax is the right side of rhs and is in a binary node
  typedef BinaryNode<OpMultiply, 
    Reference< QDPType< TScal, OScalar< TScal > > >,
    Reference< QDPType< TVec, OLattice< TVec > > > > BN;

  // get the binary node
  const BN &mulNode = static_cast<const BN&> (rhs.expression().right());

  // get a and x out of the bynary node
  const OScalar< TScal >& a = static_cast<const OScalar< TScal >&>(mulNode.left());
  const OLattice< TVec >& x = static_cast<const OLattice< TVec >&>(mulNode.right());
  // Set pointers etc.

  // -ve sign as y - ax = -ax + y  = axpy with -a.
  REAL64 ar =  -a.elem().elem().elem().elem();
  REAL64 *aptr = (REAL64 *)&ar;
  if( s.hasOrderedRep() ) { 

    REAL64 *xptr = (REAL64 *) &(x.elem(s.start()).elem(0).elem(0).real());
    REAL64 *yptr = (REAL64 *) &(y.elem(s.start()).elem(0).elem(0).real());
    REAL64* zptr =  &(d.elem(s.start()).elem(0).elem(0).real());

    
    // Get the no of 3vecs. s.start() and s.end() are inclusive so add +1
    int n_4vec = (s.end()-s.start()+1);
    vaxpyz4(zptr, aptr, xptr, yptr, n_4vec);
  }
  else { 
    const int* tab = s.siteTable().slice();
    for(int j=0; j < s.numSiteTable(); j++) { 
      int i=tab[j];
      REAL64* xptr = (REAL64 *)&(x.elem(i).elem(0).elem(0).real());
      REAL64* yptr = (REAL64 *)&(y.elem(i).elem(0).elem(0).real());
      REAL64* zptr =  &(d.elem(i).elem(0).elem(0).real());
      vaxpyz4(zptr, aptr, xptr, yptr, 1);
    }
  }

}

// Vec += Vec * Scalar (AXPY)
template<>
inline
void evaluate(OLattice< TVec >& d, 
	      const OpAddAssign& op, 
	      const QDPExpr<BinaryNode<OpMultiply, 
	      Reference< QDPType< TVec, OLattice< TVec > > >,
	      Reference< QDPType< TScal, OScalar < TScal > > > >,
	      OLattice< TVec > > &rhs,
	      const Subset& s)
{

#ifdef DEBUG_BLAS
  QDPIO::cout << "y += x*a" << endl;
#endif

  const OLattice< TVec >& x = static_cast<const OLattice< TVec > &>(rhs.expression().left());
  const OScalar< TScal >& a = static_cast<const OScalar< TScal > &> (rhs.expression().right());
  
  REAL64 ar = a.elem().elem().elem().elem();
  REAL64* aptr = &ar;
  
  if( s.hasOrderedRep() ) { 
    REAL64* xptr = (REAL64 *)&(x.elem(s.start()).elem(0).elem(0).real());
    REAL64* yptr = &(d.elem(s.start()).elem(0).elem(0).real());
    // cout << "Specialised axpy a ="<< ar << endl;
    
    int n_4vec = (s.end()-s.start()+1);
    vaxpy4(yptr, aptr, xptr, n_4vec);
  }
  else { 
    const int* tab = s.siteTable().slice();
    for(int j=0; j < s.numSiteTable(); j++) { 
      int i=tab[j];
      REAL64* xptr = (REAL64 *)&(x.elem(i).elem(0).elem(0).real());
      REAL64* yptr = &(d.elem(i).elem(0).elem(0).real());
      
      vaxpy4(yptr, aptr, xptr, 1);
    }
  }


}


// Vec -= Vec *Scalar 
template<>
inline
void evaluate(OLattice< TVec >& d, 
	      const OpSubtractAssign& op, 
	      const QDPExpr<BinaryNode<OpMultiply, 
	      Reference< QDPType< TVec, OLattice< TVec > > >,
	      Reference< QDPType< TScal, OScalar < TScal > > > >,
	      OLattice< TVec > > &rhs,
	      const Subset& s)
{

#ifdef DEBUG_BLAS
  QDPIO::cout << "y -= x*a" << endl;
#endif

  const OLattice< TVec >& x = static_cast<const OLattice< TVec > &>(rhs.expression().left());
  const OScalar< TScal >& a = static_cast<const OScalar< TScal > &> (rhs.expression().right());

  // - sign as y -= ax <=> y = y-ax = -ax + y = axpy with -a 
  REAL64 ar = -( a.elem().elem().elem().elem());
  REAL64* aptr = &ar;

  if( s.hasOrderedRep() ) { 
    REAL64* xptr = (REAL64 *)&(x.elem(s.start()).elem(0).elem(0).real());
    REAL64* yptr = &(d.elem(s.start()).elem(0).elem(0).real());
    
    int n_4vec = (s.end()-s.start()+1);
    vaxpy4(yptr, aptr, xptr, n_4vec);
  } 
  else { 
    const int* tab = s.siteTable().slice();
    for(int j=0; j < s.numSiteTable(); j++) { 
      int i=tab[j];
      REAL64* xptr = (REAL64 *)&(x.elem(i).elem(0).elem(0).real());
      REAL64* yptr = &(d.elem(i).elem(0).elem(0).real());
      
      vaxpy4(yptr, aptr, xptr, 1);
    }
  }

}


// Vec = Vec *Scalar  + Vec (AXPY)
template<>
inline
void evaluate( OLattice< TVec > &d,
	       const OpAssign &op,
	       const QDPExpr< 
	       BinaryNode<OpAdd,
	        BinaryNode<OpMultiply, 
	         Reference< QDPType< TVec, OLattice< TVec > > >,
	         Reference< QDPType< TScal, OScalar< TScal > > > >,
	        Reference< QDPType< TVec, OLattice< TVec > > > >,
	        OLattice< TVec > > &rhs,
	       const Subset& s)
{

#ifdef DEBUG_BLAS
  QDPIO::cout << "z = x*a + y" << endl;
#endif

  // Peel the stuff out of the expression
  // y is the right side of rhs
  const OLattice< TVec >& y = static_cast<const OLattice< TVec >&> (rhs.expression().right());

  // ax is the right side of rhs and is in a binary node
  typedef BinaryNode<OpMultiply, 
    Reference< QDPType< TVec, OLattice< TVec > > >,    
    Reference< QDPType< TScal, OScalar< TScal > > > > BN;

  // get the binary node
  const BN &mulNode = static_cast<const BN&> (rhs.expression().left());

  // get a and x out of the bynary node
  const OScalar< TScal >& a = static_cast<const OScalar< TScal >&>(mulNode.right());
  const OLattice< TVec >& x = static_cast<const OLattice< TVec >&>(mulNode.left());
  // Set pointers 
  REAL64 ar =  a.elem().elem().elem().elem();
  REAL64 *aptr = (REAL64 *)&ar;
  if( s.hasOrderedRep() ) { 

    REAL64 *xptr = (REAL64 *) &(x.elem(s.start()).elem(0).elem(0).real());
    REAL64 *yptr = (REAL64 *) &(y.elem(s.start()).elem(0).elem(0).real());
    REAL64* zptr =  &(d.elem(s.start()).elem(0).elem(0).real());
    
    
    // Get the no of 3vecs. s.start() and s.end() are inclusive so add +1
    int n_4vec = (s.end()-s.start()+1);
    vaxpyz4(zptr, aptr, xptr, yptr, n_4vec);
  } 
  else { 
    const int* tab = s.siteTable().slice();
    for(int j=0; j < s.numSiteTable(); j++) { 
      int i=tab[j];

      REAL64* xptr = (REAL64 *)&(x.elem(i).elem(0).elem(0).real());
      REAL64* yptr = (REAL64 *) &(y.elem(i).elem(0).elem(0).real());
      REAL64* zptr = (REAL64 *)  &(d.elem(i).elem(0).elem(0).real());
      vaxpyz4(zptr, aptr, xptr, yptr, 1);
    }
  }

}


// Vec = Vec + Vec * Scalar (AXPY)
template<>
inline
void evaluate( OLattice< TVec > &d,
	       const OpAssign &op,
	       const QDPExpr< 
	       BinaryNode<OpAdd,
	        Reference< QDPType< TVec, OLattice< TVec > > >,
	        BinaryNode<OpMultiply, 
	         Reference< QDPType< TVec, OLattice< TVec > > >,
	         Reference< QDPType< TScal, OScalar< TScal > > > > >,
	       OLattice< TVec > > &rhs,
	       const Subset& s)
{
#ifdef DEBUG_BLAS
  QDPIO::cout << "z = y + x*a" << endl;
#endif


  // Peel the stuff out of the expression

  // y is the left side of rhs
  const OLattice< TVec >& y = static_cast<const OLattice< TVec >&> (rhs.expression().left());

  // ax is the right side of rhs and is in a binary node
  typedef BinaryNode<OpMultiply, 
    Reference< QDPType< TVec, OLattice< TVec > > >,    
    Reference< QDPType< TScal, OScalar< TScal > > > > BN;

  // get the binary node
  const BN &mulNode = static_cast<const BN&> (rhs.expression().right());

  // get a and x out of the bynary node
  const OScalar< TScal >& a = static_cast<const OScalar< TScal >&>(mulNode.right());
  const OLattice< TVec >& x = static_cast<const OLattice< TVec >&>(mulNode.left());
  // Set pointers 
  REAL64 ar =  a.elem().elem().elem().elem();
  REAL64 *aptr = (REAL64 *)&ar;

  if( s.hasOrderedRep() ) {
    REAL64 *xptr = (REAL64 *) &(x.elem(s.start()).elem(0).elem(0).real());
    REAL64 *yptr = (REAL64 *) &(y.elem(s.start()).elem(0).elem(0).real());
    REAL64* zptr =  &(d.elem(s.start()).elem(0).elem(0).real());
    
    // Get the no of 3vecs. s.start() and s.end() are inclusive so add +1
    int n_4vec = (s.end()-s.start()+1);
    vaxpyz4(zptr, aptr, xptr, yptr, n_4vec);
  } 
  else { 
    const int* tab = s.siteTable().slice();
    for(int j=0; j < s.numSiteTable(); j++) { 
      int i=tab[j];
      
      REAL64* xptr = (REAL64 *)&(x.elem(i).elem(0).elem(0).real());
      REAL64* yptr = (REAL64 *)&(y.elem(i).elem(0).elem(0).real());
      REAL64* zptr = (REAL64 *)&(d.elem(i).elem(0).elem(0).real());
      vaxpyz4(zptr, aptr, xptr, yptr, 1);
    }
  }

}


// Vec = Vec*Scalar - Vec (AXMY)
template<>
inline
void evaluate( OLattice< TVec > &d,
	       const OpAssign &op,
	       const QDPExpr< 
	       BinaryNode<OpSubtract,
	        BinaryNode<OpMultiply, 
	         Reference< QDPType< TVec, OLattice< TVec > > >,
	         Reference< QDPType< TScal, OScalar< TScal > > > >,
	        Reference< QDPType< TVec, OLattice< TVec > > > >,
	        OLattice< TVec > > &rhs,
	       const Subset& s)
{
#ifdef DEBUG_BLAS
  QDPIO::cout << "z = x*a - y" << endl;
#endif

  const OLattice< TVec >& y = static_cast<const OLattice< TVec >&> (rhs.expression().right());


  typedef BinaryNode<OpMultiply, 
    Reference< QDPType< TVec, OLattice< TVec > > >,    
    Reference< QDPType< TScal, OScalar< TScal > > > > BN;

  // get the binary node
  const BN &mulNode = static_cast<const BN&> (rhs.expression().left());

  // get a and x out of the bynary node
  const OScalar< TScal >& a = static_cast<const OScalar< TScal >&>(mulNode.right());
  const OLattice< TVec >& x = static_cast<const OLattice< TVec >&>(mulNode.left());
  // Set pointers 
  REAL64 ar =  a.elem().elem().elem().elem();
  REAL64 *aptr = (REAL64 *)&ar;

  if( s.hasOrderedRep() ) { 
    REAL64 *xptr = (REAL64 *) &(x.elem(s.start()).elem(0).elem(0).real());
    REAL64 *yptr = (REAL64 *) &(y.elem(s.start()).elem(0).elem(0).real());
    REAL64* zptr =  &(d.elem(s.start()).elem(0).elem(0).real());
    
    
    // Get the no of 3vecs. s.start() and s.end() are inclusive so add +1
    int n_4vec = (s.end()-s.start()+1);
    vaxmyz4(zptr, aptr, xptr, yptr, n_4vec);
  }
  else { 
    const int* tab = s.siteTable().slice();
    for(int j=0; j < s.numSiteTable(); j++) { 
      int i=tab[j];
      
      REAL64* xptr = (REAL64 *)&(x.elem(i).elem(0).elem(0).real());
      REAL64* yptr = (REAL64 *)&(y.elem(i).elem(0).elem(0).real());
      REAL64* zptr =  &(d.elem(i).elem(0).elem(0).real());
      vaxmyz4(zptr, aptr, xptr, yptr, 1);
    }
  }

}


// Vec = Vec - Vec*Scalar (AXPY with -Scalar)
template<>
inline
void evaluate( OLattice< TVec > &d,
	       const OpAssign &op,
	       const QDPExpr< 
	       BinaryNode<OpSubtract,
	        Reference< QDPType< TVec, OLattice< TVec > > >,
	        BinaryNode<OpMultiply, 
	         Reference< QDPType< TVec, OLattice< TVec > > >,
	         Reference< QDPType< TScal, OScalar< TScal > > > > >,
	       OLattice< TVec > > &rhs,
	       const Subset& s)
{
#ifdef DEBUG_BLAS
  QDPIO::cout << "z = y - x*a" << endl;
#endif

  const OLattice< TVec >& y = static_cast<const OLattice< TVec >&> (rhs.expression().left());

  typedef BinaryNode<OpMultiply, 
    Reference< QDPType< TVec, OLattice< TVec > > >,    
    Reference< QDPType< TScal, OScalar< TScal > > > > BN;

  // get the binary node
  const BN &mulNode = static_cast<const BN&> (rhs.expression().right());

  // get a and x out of the bynary node
  const OScalar< TScal >& a = static_cast<const OScalar< TScal >&>(mulNode.right());
  const OLattice< TVec >& x = static_cast<const OLattice< TVec >&>(mulNode.left());
  // Set pointers etc.

  // -ve sign as y - ax = -ax + y  = axpy with -a.
  REAL64 ar =  -a.elem().elem().elem().elem();
  REAL64 *aptr = (REAL64 *)&ar;

  if( s.hasOrderedRep() ) { 

    REAL64 *xptr = (REAL64 *) &(x.elem(s.start()).elem(0).elem(0).real());
    REAL64 *yptr = (REAL64 *) &(y.elem(s.start()).elem(0).elem(0).real());
    REAL64* zptr =  &(d.elem(s.start()).elem(0).elem(0).real());
    
    
    // Get the no of 3vecs. s.start() and s.end() are inclusive so add +1
    int n_4vec = (s.end()-s.start()+1);
    vaxpyz4(zptr, aptr, xptr, yptr, n_4vec);
  }
  else { 
    const int* tab = s.siteTable().slice();
    for(int j=0; j < s.numSiteTable(); j++) { 
      int i=tab[j];
      
      REAL64* xptr = (REAL64 *)&(x.elem(i).elem(0).elem(0).real());
      REAL64* yptr = (REAL64 *)&(y.elem(i).elem(0).elem(0).real());
      REAL64* zptr =  &(d.elem(i).elem(0).elem(0).real());
      vaxpyz4(zptr, aptr, xptr, yptr, 1);
    }
  }

}


template<>
inline
void evaluate( OLattice< TVec > &d,
	       const OpAssign &op,
	       const QDPExpr< 
	       BinaryNode<OpAdd,
	       Reference< QDPType< TVec, OLattice< TVec > > >,
	       Reference< QDPType< TVec, OLattice< TVec > > > >,
	       OLattice< TVec > > &rhs,
	       const Subset& s)
{
#ifdef DEBUG_BLAS
  cout << "BJ: v+v " << endl;
#endif

  const OLattice< TVec >& x = static_cast<const OLattice< TVec >&>(rhs.expression().left());
  const OLattice< TVec >& y = static_cast<const OLattice< TVec >&>(rhs.expression().right());

  REAL64 one = 1;

  if( s.hasOrderedRep() ) { 
    REAL64 *xptr = (REAL64 *) &(x.elem(s.start()).elem(0).elem(0).real());
    REAL64 *yptr = (REAL64 *) &(y.elem(s.start()).elem(0).elem(0).real());
    REAL64* zptr =  &(d.elem(s.start()).elem(0).elem(0).real());
    

    // Get the no of 3vecs. s.start() and s.end() are inclusive so add +1
    int n_4vec = (s.end()-s.start()+1);
    vaxpyz4(zptr,&one, xptr, yptr, n_4vec);
  }
  else { 
    const int* tab = s.siteTable().slice();
    for(int j=0; j < s.numSiteTable(); j++) { 
      int i=tab[j];
      
      REAL64* xptr = (REAL64 *)&(x.elem(i).elem(0).elem(0).real());
      REAL64* yptr = (REAL64 *)&(y.elem(i).elem(0).elem(0).real());
      REAL64* zptr =  &(d.elem(i).elem(0).elem(0).real());
      vaxpyz4(zptr,&one, xptr, yptr, 1);
    }
  }


}

template<>
inline
void evaluate( OLattice< TVec > &d,
	       const OpAssign &op,
	       const QDPExpr< 
	       BinaryNode<OpSubtract,
	       Reference< QDPType< TVec, OLattice< TVec > > >,
	       Reference< QDPType< TVec, OLattice< TVec > > > >,
	       OLattice< TVec > > &rhs,
	       const Subset& s)
{
#ifdef DEBUG_BLAS
  cout << "BJ: v-v " << endl;
#endif

  const OLattice< TVec >& x = static_cast<const OLattice< TVec >&>(rhs.expression().left());
  const OLattice< TVec >& y = static_cast<const OLattice< TVec >&>(rhs.expression().right());
  REAL64 one=1;

  if( s.hasOrderedRep() ) { 

    REAL64 *xptr = (REAL64 *) &(x.elem(s.start()).elem(0).elem(0).real());
    REAL64 *yptr = (REAL64 *) &(y.elem(s.start()).elem(0).elem(0).real());
    REAL64* zptr =  &(d.elem(s.start()).elem(0).elem(0).real());
    
    // Get the no of 3vecs. s.start() and s.end() are inclusive so add +1
    int n_4vec = (s.end()-s.start()+1);

    vaxmyz4(zptr,&one, xptr, yptr, n_4vec);
  }
  else { 
    const int* tab = s.siteTable().slice();
    for(int j=0; j < s.numSiteTable(); j++) { 
      int i=tab[j];
      REAL64 *xptr = (REAL64 *) &(x.elem(i).elem(0).elem(0).real());
      REAL64 *yptr = (REAL64 *) &(y.elem(i).elem(0).elem(0).real());
      REAL64* zptr =  &(d.elem(i).elem(0).elem(0).real());
      
      vaxmyz4(zptr,&one, xptr, yptr, 1);
     
    }
  }

}

// Vec = Scal * Vec
template<>
inline
void evaluate( OLattice< TVec > &d,
	       const OpAssign &op,
	       const QDPExpr< 
	       BinaryNode<OpMultiply,
	       Reference< QDPType< TScal, OScalar< TScal > > >,
	       Reference< QDPType< TVec, OLattice< TVec > > > >,
	       OLattice< TVec > > &rhs,
	       const Subset& s)
{
#ifdef DEBUG_BLAS
  cout << "BJ: v = a*v " << endl;
#endif
  const OLattice< TVec > &x = static_cast<const OLattice< TVec >&>(rhs.expression().right());
  const OScalar< TScal > &a = static_cast<const OScalar< TScal >&>(rhs.expression().left());

  REAL64 ar =  a.elem().elem().elem().elem();
  REAL64 *aptr = &ar;  
  
  if( s.hasOrderedRep() ) {

    REAL64 *xptr = (REAL64 *) &(x.elem(s.start()).elem(0).elem(0).real());
    REAL64 *zptr =  &(d.elem(s.start()).elem(0).elem(0).real());
    int n_3vec = (s.end()-s.start()+1)*Ns;

    vscal(zptr, aptr, xptr, n_3vec);
  }
  else { 
    const int* tab = s.siteTable().slice();
    for(int j=0; j < s.numSiteTable(); j++) { 
      int i=tab[j];
      REAL64 *xptr = (REAL64 *) &(x.elem(i).elem(0).elem(0).real());
      REAL64 *zptr =  &(d.elem(i).elem(0).elem(0).real());

      
      vscal(zptr, aptr, xptr, Ns);
    }
  }

}

template<>
inline
void evaluate( OLattice< TVec > &d,
	       const OpAssign &op,
	       const QDPExpr< 
	       BinaryNode<OpMultiply,
	       Reference< QDPType< TVec, OLattice< TVec > > >,
	       Reference< QDPType< TScal, OScalar< TScal > > > >,
	       OLattice< TVec > > &rhs,
	       const Subset& s)
{
#ifdef DEBUG_BLAS
  cout << "BJ: v = v*a " << endl;
#endif

  const OLattice< TVec > &x = static_cast<const OLattice< TVec >&>(rhs.expression().left());
  const OScalar< TScal > &a = static_cast<const OScalar< TScal >&>(rhs.expression().right());

  REAL64 ar =  a.elem().elem().elem().elem();
  REAL64 *aptr = &ar;  

  if( s.hasOrderedRep() ) { 
    REAL64 *xptr = (REAL64 *) &(x.elem(s.start()).elem(0).elem(0).real());
    REAL64 *zptr =  &(d.elem(s.start()).elem(0).elem(0).real());
    int n_3vec = (s.end()-s.start()+1)*Ns;
    
    vscal(zptr, aptr, xptr, n_3vec);
  }
  else { 
    const int* tab = s.siteTable().slice();
    for(int j=0; j < s.numSiteTable(); j++) { 
      int i=tab[j];
      REAL64 *xptr = (REAL64 *) &(x.elem(i).elem(0).elem(0).real());
      REAL64 *zptr =  &(d.elem(i).elem(0).elem(0).real());
      
      vscal(zptr, aptr, xptr, Ns);
    }
  }
}

// v *= a
template<>
inline
void evaluate( OLattice< TVec > &d,
	       const OpMultiplyAssign &op,
	       const QDPExpr< 
	       UnaryNode<OpIdentity,
	       Reference< QDPType< TScal, OScalar< TScal > > > >,
	       OScalar< TScal > > &rhs,
	       const Subset& s)
{
  const OScalar< TScal >& a = static_cast< const OScalar<TScal >&>(rhs.expression().child());


#ifdef DEBUG_BLAS
  QDPIO::cout << "BJ: v *= a, a = " << a << endl;
#endif
  
  REAL64 ar = a.elem().elem().elem().elem();
  if( s.hasOrderedRep() ) { 

    REAL64* xptr = &(d.elem(s.start()).elem(0).elem(0).real());
    REAL64* zptr = xptr;
    int n_3vec = (s.end()-s.start()+1)*Ns;
    vscal(zptr,&ar, xptr, n_3vec);
  }
  else { 
    const int* tab = s.siteTable().slice();
    for(int j=0; j < s.numSiteTable(); j++) { 
      int i=tab[j];

      REAL64* xptr = &(d.elem(i).elem(0).elem(0).real());
      REAL64* zptr = xptr;
      
      vscal(zptr,&ar, xptr, Ns);
    }
  }
}

// v /= a
template<>
inline
void evaluate( OLattice< TVec > &d,
	       const OpDivideAssign &op,
	       const QDPExpr< 
	       UnaryNode<OpIdentity,
	       Reference< QDPType< TScal, OScalar< TScal > > > >,
	       OScalar< TScal > > &rhs,
	       const Subset& s)
{
  const OScalar< TScal >& a = static_cast< const OScalar<TScal >&>(rhs.expression().child());


#ifdef DEBUG_BLAS
  QDPIO::cout << "BJ: v /= a, a = " << a << endl;
#endif
  
  REAL64 ar = (REAL64)1/a.elem().elem().elem().elem();
  if( s.hasOrderedRep() ) { 
    REAL64* xptr = &(d.elem(s.start()).elem(0).elem(0).real());
    REAL64* zptr = xptr;
    int n_3vec = (s.end()-s.start()+1)*Ns;
    vscal(zptr,&ar, xptr, n_3vec);
  }
  else { 
    const int* tab = s.siteTable().slice();
    for(int j=0; j < s.numSiteTable(); j++) { 
      int i=tab[j];

      REAL64* xptr = &(d.elem(i).elem(0).elem(0).real());
      REAL64* zptr = xptr;
      
      vscal(zptr,&ar, xptr, Ns);
    }
  }
}

// v += v
template<>
inline
void evaluate( OLattice< TVec > &d,
	       const OpAddAssign &op,
	       const QDPExpr< 
	       UnaryNode<OpIdentity,
	       Reference< QDPType< TVec, OLattice< TVec > > > >,
	       OLattice< TVec > > &rhs,
	       const Subset& s)
{
  const OLattice< TVec >& x = static_cast< const OLattice<TVec >&>(rhs.expression().child());

 

#ifdef DEBUG_BLAS
  QDPIO::cout << "BJ: v += v" << endl;
#endif
  REAL64 one = 1;

  if( s.hasOrderedRep() ) {

    
    REAL64 *xptr = (REAL64 *)(&x.elem(s.start()).elem(0).elem(0).real());
    REAL64 *yptr = (REAL64 *)(&d.elem(s.start()).elem(0).elem(0).real());
    int n_4vec = (s.end() - s.start()+1);  
    vaxpy4(yptr, &one, xptr,n_4vec);
  }
  else { 
    const int* tab = s.siteTable().slice();
    for(int j=0; j < s.numSiteTable(); j++) { 
      int i=tab[j];

      REAL64 *xptr = (REAL64 *)(&x.elem(i).elem(0).elem(0).real());
      REAL64 *yptr = (REAL64 *)(&d.elem(i).elem(0).elem(0).real());
      
      vaxpy4(yptr, &one, xptr,1);

    }
  }

}

// v -= v
template<>
inline
void evaluate( OLattice< TVec > &d,
	       const OpSubtractAssign &op,
	       const QDPExpr< 
	       UnaryNode<OpIdentity,
	       Reference< QDPType< TVec, OLattice< TVec > > > >,
	       OLattice< TVec > > &rhs,
	       const Subset& s)
{
  const OLattice< TVec >& x = static_cast< const OLattice<TVec >&>(rhs.expression().child());

 

#ifdef DEBUG_BLAS
  QDPIO::cout << "BJ: v -= v" << endl;
#endif
  REAL64 one = 1;
    
  if( s.hasOrderedRep() ) { 

    REAL64 *xptr = (REAL64 *)(&x.elem(s.start()).elem(0).elem(0).real());
    REAL64 *yptr = (REAL64 *)(&d.elem(s.start()).elem(0).elem(0).real());

    int n_4vec = (s.end() - s.start()+1);    
    vaxmyz4(yptr, &one, yptr, xptr, n_4vec);
  }
  else { 
    const int* tab = s.siteTable().slice();
    for(int j=0; j < s.numSiteTable(); j++) { 
      int i=tab[j];
      REAL64 *xptr = (REAL64 *)(&x.elem(i).elem(0).elem(0).real());
      REAL64 *yptr = (REAL64 *)(&d.elem(i).elem(0).elem(0).real());
    
      vaxmyz4(yptr, &one, yptr, xptr, 1);
   
    }
  }

}


// z = ax + by
template<>
inline
void evaluate( OLattice< TVec > &d,
	       const OpAssign &op,
	       const QDPExpr< 
	       BinaryNode<OpAdd,
	        BinaryNode<OpMultiply, 
	         Reference< QDPType< TScal, OScalar< TScal > > >,
	         Reference< QDPType< TVec, OLattice< TVec > > > >,
	        BinaryNode<OpMultiply, 
	         Reference< QDPType< TScal, OScalar< TScal > > >,
	         Reference< QDPType< TVec, OLattice< TVec > > > > >,
	        OLattice< TVec > > &rhs,
	       const Subset& s)
{

#ifdef DEBUG_BLAS
  QDPIO::cout << "z = a*x + b*y" << endl;
#endif

  // Peel the stuff out of the expression
  // y is the right side of rhs

  // ax is the left side of rhs and is in a binary node
  typedef BinaryNode<OpMultiply, 
    Reference< QDPType< TScal, OScalar< TScal > > >,
    Reference< QDPType< TVec, OLattice< TVec > > > > BN;

  // get the binary node
  const BN &mulNode1 = static_cast<const BN&> (rhs.expression().left());
  const BN &mulNode2 = static_cast<const BN&> (rhs.expression().right());

  // get a and x out of the binary node
  const OScalar< TScal >& a = static_cast<const OScalar< TScal >&>(mulNode1.left());
  const OLattice< TVec >& x = static_cast<const OLattice< TVec >&>(mulNode1.right());
  
  // get b and y out of the binary node
  const OScalar< TScal >& b = static_cast<const OScalar< TScal >&>(mulNode2.left());
  const OLattice< TVec >& y = static_cast<const OLattice< TVec >&>(mulNode2.right());

  
  // Set pointers 
  REAL64 *aptr = (REAL64 *)&(a.elem().elem().elem().elem());
  REAL64 *bptr = (REAL64 *)&(b.elem().elem().elem().elem());

  if( s.hasOrderedRep() ) { 
    REAL64 *xptr = (REAL64 *) &(x.elem(s.start()).elem(0).elem(0).real());
    REAL64 *yptr = (REAL64 *) &(y.elem(s.start()).elem(0).elem(0).real());
    REAL64* zptr =  &(d.elem(s.start()).elem(0).elem(0).real());

    // Get the no of 3vecs. s.start() and s.end() are inclusive so add +1
    int n_4vec = (s.end()-s.start()+1);
    vaxpbyz4(zptr, aptr, xptr, bptr, yptr, n_4vec);
  }
  else { 
    const int* tab = s.siteTable().slice();
    for(int j=0; j < s.numSiteTable(); j++) { 
      int i=tab[j];
      REAL64 *xptr = (REAL64 *) &(x.elem(i).elem(0).elem(0).real());
      REAL64 *yptr = (REAL64 *) &(y.elem(i).elem(0).elem(0).real());
      REAL64* zptr =  &(d.elem(i).elem(0).elem(0).real());
      
      // Get the no of 3vecs. s.start() and s.end() are inclusive so add +1
      vaxpbyz4(zptr, aptr, xptr, bptr, yptr, 1);
   
    }
  }

}


// z = xa + by
template<>
inline
void evaluate( OLattice< TVec > &d,
	       const OpAssign &op,
	       const QDPExpr< 
	       BinaryNode<OpAdd,
	        BinaryNode<OpMultiply, 
	         Reference< QDPType< TVec, OLattice< TVec > > >,
	         Reference< QDPType< TScal, OScalar< TScal > > > >,
	        BinaryNode<OpMultiply, 
	         Reference< QDPType< TScal, OScalar< TScal > > >,
	         Reference< QDPType< TVec, OLattice< TVec > > > > >,
	        OLattice< TVec > > &rhs,
	       const Subset& s)
{

#ifdef DEBUG_BLAS
  QDPIO::cout << "z = x*a + b*y" << endl;
#endif

  // Peel the stuff out of the expression
  // y is the right side of rhs

  // ax is the left side of rhs and is in a binary node
  typedef BinaryNode<OpMultiply, 
    Reference< QDPType< TVec, OLattice< TVec > > >,
    Reference< QDPType< TScal, OScalar< TScal > > > > BN1;

  typedef BinaryNode<OpMultiply, 
    Reference< QDPType< TScal, OScalar< TScal > > >,
    Reference< QDPType< TVec, OLattice< TVec > > > > BN2;


  
  // get the binary node
  const BN1 &mulNode1 = static_cast<const BN1&> (rhs.expression().left());
  const BN2 &mulNode2 = static_cast<const BN2&> (rhs.expression().right());

  // get a and x out of the binary node
  const OLattice< TVec >& x = static_cast<const OLattice< TVec >&>(mulNode1.left());

  const OScalar< TScal >& a = static_cast<const OScalar< TScal >&>(mulNode1.right());
  
  // get b and y out of the binary node
  const OScalar< TScal >& b = static_cast<const OScalar< TScal >&>(mulNode2.left());
  const OLattice< TVec >& y = static_cast<const OLattice< TVec >&>(mulNode2.right());

  
  // Set pointers 
  REAL64 *aptr = (REAL64 *)&(a.elem().elem().elem().elem());
  REAL64 *bptr = (REAL64 *)&(b.elem().elem().elem().elem());
  
  if( s.hasOrderedRep() ) { 
    REAL64 *xptr = (REAL64 *) &(x.elem(s.start()).elem(0).elem(0).real());
    REAL64 *yptr = (REAL64 *) &(y.elem(s.start()).elem(0).elem(0).real());
    REAL64* zptr =  &(d.elem(s.start()).elem(0).elem(0).real());
    
    
    // Get the no of 3vecs. s.start() and s.end() are inclusive so add +1
    int n_4vec = (s.end()-s.start()+1);
    vaxpbyz4(zptr, aptr, xptr, bptr, yptr, n_4vec);
  }
  else { 
    const int* tab = s.siteTable().slice();
    for(int j=0; j < s.numSiteTable(); j++) { 
      int i=tab[j];
   
      REAL64 *xptr = (REAL64 *) &(x.elem(i).elem(0).elem(0).real());
      REAL64 *yptr = (REAL64 *) &(y.elem(i).elem(0).elem(0).real());
      REAL64* zptr =  &(d.elem(i).elem(0).elem(0).real());
          
      // Get the no of 3vecs. s.start() and s.end() are inclusive so add +1
      vaxpbyz4(zptr, aptr, xptr, bptr, yptr, 1);
   
    }
  }
}

// z = ax + yb
template<>
inline
void evaluate( OLattice< TVec > &d,
	       const OpAssign &op,
	       const QDPExpr< 
	       BinaryNode<OpAdd,
	        BinaryNode<OpMultiply, 
	         Reference< QDPType< TScal, OScalar< TScal > > >,
	         Reference< QDPType< TVec, OLattice< TVec > > > >,
	        BinaryNode<OpMultiply, 
	         Reference< QDPType< TVec, OLattice< TVec > > >,
	         Reference< QDPType< TScal, OScalar< TScal > > > > >,
	        OLattice< TVec > > &rhs,
	       const Subset& s)
{

#ifdef DEBUG_BLAS
  QDPIO::cout << "z = a*x + y*b" << endl;
#endif

  // Peel the stuff out of the expression
  // y is the right side of rhs

  // type of a*x
  typedef BinaryNode<OpMultiply, 
    Reference< QDPType< TScal, OScalar< TScal > > >,
    Reference< QDPType< TVec, OLattice< TVec > > > > BN1;

  // type of y*b
  typedef BinaryNode<OpMultiply, 
    Reference< QDPType< TVec, OLattice< TVec > > >,
    Reference< QDPType< TScal, OScalar< TScal > > > > BN2;


  
  // get the binary nodes
  // a*x node
  const BN1 &mulNode1 = static_cast<const BN1&> (rhs.expression().left());

  // y*b node
  const BN2 &mulNode2 = static_cast<const BN2&> (rhs.expression().right());

  // get a and x out of the binary node
  const OScalar< TScal >& a = static_cast<const OScalar< TScal >&>(mulNode1.left());

  const OLattice< TVec >& x = static_cast<const OLattice< TVec >&>(mulNode1.right());

  
  // get b and y out of the binary node
  const OLattice< TVec >& y = static_cast<const OLattice< TVec >&>(mulNode2.left());

  const OScalar< TScal >& b = static_cast<const OScalar< TScal >&>(mulNode2.right());

  
  // Set pointers 
  REAL64 *aptr = (REAL64 *)&(a.elem().elem().elem().elem());
  REAL64 *bptr = (REAL64 *)&(b.elem().elem().elem().elem());

  if( s.hasOrderedRep() ) { 
    REAL64 *xptr = (REAL64 *) &(x.elem(s.start()).elem(0).elem(0).real());
    REAL64 *yptr = (REAL64 *) &(y.elem(s.start()).elem(0).elem(0).real());
    REAL64* zptr =  &(d.elem(s.start()).elem(0).elem(0).real());


    // Get the no of 3vecs. s.start() and s.end() are inclusive so add +1
    int n_4vec = (s.end()-s.start()+1);
    vaxpbyz4(zptr, aptr, xptr, bptr, yptr, n_4vec);
  }
  else { 
    const int* tab = s.siteTable().slice();
    for(int j=0; j < s.numSiteTable(); j++) { 
      int i=tab[j];
   
      REAL64 *xptr = (REAL64 *) &(x.elem(i).elem(0).elem(0).real());
      REAL64 *yptr = (REAL64 *) &(y.elem(i).elem(0).elem(0).real());
      REAL64* zptr =  &(d.elem(i).elem(0).elem(0).real());
          
      // Get the no of 3vecs. s.start() and s.end() are inclusive so add +1
      vaxpbyz4(zptr, aptr, xptr, bptr, yptr, 1);
   
    }
  }
}

// z = xa + yb
template<>
inline
void evaluate( OLattice< TVec > &d,
	       const OpAssign &op,
	       const QDPExpr< 
	       BinaryNode<OpAdd,
	        BinaryNode<OpMultiply, 
	         Reference< QDPType< TVec, OLattice< TVec > > >,
	         Reference< QDPType< TScal, OScalar< TScal > > > >,
	        BinaryNode<OpMultiply, 
	         Reference< QDPType< TVec, OLattice< TVec > > >,
	         Reference< QDPType< TScal, OScalar< TScal > > > > >,
	        OLattice< TVec > > &rhs,
	       const Subset& s)
{

#ifdef DEBUG_BLAS
  QDPIO::cout << "z = x*a + y*b" << endl;
#endif

  // Peel the stuff out of the expression
  // y is the right side of rhs

  // ax is the left side of rhs and is in a binary node
  typedef BinaryNode<OpMultiply, 
    Reference< QDPType< TVec, OLattice< TVec > > >,
    Reference< QDPType< TScal, OScalar< TScal > > > > BN;

  // get the binary node
  const BN &mulNode1 = static_cast<const BN&> (rhs.expression().left());
  const BN &mulNode2 = static_cast<const BN&> (rhs.expression().right());

  // get a and x out of the binary node
  const OLattice< TVec >& x = static_cast<const OLattice< TVec >&>(mulNode1.left());
  const OScalar< TScal >& a = static_cast<const OScalar< TScal >&>(mulNode1.right());
  
  // get b and y out of the binary node
  const OLattice< TVec >& y = static_cast<const OLattice< TVec >&>(mulNode2.left());

  const OScalar< TScal >& b = static_cast<const OScalar< TScal >&>(mulNode2.right());
  
  // Set pointers 
  REAL64 *aptr = (REAL64 *)&(a.elem().elem().elem().elem());
  REAL64 *bptr = (REAL64 *)&(b.elem().elem().elem().elem());

  if( s.hasOrderedRep() ) { 
    REAL64 *xptr = (REAL64 *) &(x.elem(s.start()).elem(0).elem(0).real());
    REAL64 *yptr = (REAL64 *) &(y.elem(s.start()).elem(0).elem(0).real());
    REAL64* zptr =  &(d.elem(s.start()).elem(0).elem(0).real());

    
    // Get the no of 3vecs. s.start() and s.end() are inclusive so add +1
    int n_4vec = (s.end()-s.start()+1);
    vaxpbyz4(zptr, aptr, xptr, bptr, yptr, n_4vec);
  }
  else { 
    const int* tab = s.siteTable().slice();
    for(int j=0; j < s.numSiteTable(); j++) { 
      int i=tab[j];
   
      REAL64 *xptr = (REAL64 *) &(x.elem(i).elem(0).elem(0).real());
      REAL64 *yptr = (REAL64 *) &(y.elem(i).elem(0).elem(0).real());
      REAL64* zptr =  &(d.elem(i).elem(0).elem(0).real());
          
      // Get the no of 3vecs. s.start() and s.end() are inclusive so add +1
      vaxpbyz4(zptr, aptr, xptr, bptr, yptr, 1);
   
    }
  }
}

// z = ax - by
template<>
inline
void evaluate( OLattice< TVec > &d,
	       const OpAssign &op,
	       const QDPExpr< 
	       BinaryNode<OpSubtract,
	        BinaryNode<OpMultiply, 
	         Reference< QDPType< TScal, OScalar< TScal > > >,
	         Reference< QDPType< TVec, OLattice< TVec > > > >,
	        BinaryNode<OpMultiply, 
	         Reference< QDPType< TScal, OScalar< TScal > > >,
	         Reference< QDPType< TVec, OLattice< TVec > > > > >,
	        OLattice< TVec > > &rhs,
	       const Subset& s)
{

#ifdef DEBUG_BLAS
  QDPIO::cout << "z = a*x - b*y" << endl;
#endif

  // Peel the stuff out of the expression
  // y is the right side of rhs

  // ax is the left side of rhs and is in a binary node
  typedef BinaryNode<OpMultiply, 
    Reference< QDPType< TScal, OScalar< TScal > > >,
    Reference< QDPType< TVec, OLattice< TVec > > > > BN;

  // get the binary node
  const BN &mulNode1 = static_cast<const BN&> (rhs.expression().left());
  const BN &mulNode2 = static_cast<const BN&> (rhs.expression().right());

  // get a and x out of the binary node
  const OScalar< TScal >& a = static_cast<const OScalar< TScal >&>(mulNode1.left());
  const OLattice< TVec >& x = static_cast<const OLattice< TVec >&>(mulNode1.right());
  
  // get b and y out of the binary node
  const OScalar< TScal >& b = static_cast<const OScalar< TScal >&>(mulNode2.left());
  const OLattice< TVec >& y = static_cast<const OLattice< TVec >&>(mulNode2.right());

  
  // Set pointers 
  REAL64 *aptr = (REAL64 *)&(a.elem().elem().elem().elem());
  REAL64 *bptr = (REAL64 *)&(b.elem().elem().elem().elem());

  if( s.hasOrderedRep() ) {
    REAL64 *xptr = (REAL64 *) &(x.elem(s.start()).elem(0).elem(0).real());
    REAL64 *yptr = (REAL64 *) &(y.elem(s.start()).elem(0).elem(0).real());
    REAL64* zptr =  &(d.elem(s.start()).elem(0).elem(0).real());


    // Get the no of 3vecs. s.start() and s.end() are inclusive so add +1
    int n_4vec = (s.end()-s.start()+1);
    vaxmbyz4(zptr, aptr, xptr, bptr, yptr, n_4vec);
  }
  else { 
    const int* tab = s.siteTable().slice();
    for(int j=0; j < s.numSiteTable(); j++) { 
      int i=tab[j];
   
      REAL64 *xptr = (REAL64 *) &(x.elem(i).elem(0).elem(0).real());
      REAL64 *yptr = (REAL64 *) &(y.elem(i).elem(0).elem(0).real());
      REAL64* zptr =  &(d.elem(i).elem(0).elem(0).real());
          
      // Get the no of 3vecs. s.start() and s.end() are inclusive so add +1
      vaxmbyz4(zptr, aptr, xptr, bptr, yptr, 1);
   
    }
  }
}


// z = xa - by
template<>
inline
void evaluate( OLattice< TVec > &d,
	       const OpAssign &op,
	       const QDPExpr< 
	       BinaryNode<OpSubtract,
	        BinaryNode<OpMultiply, 
	         Reference< QDPType< TVec, OLattice< TVec > > >,
	         Reference< QDPType< TScal, OScalar< TScal > > > >,
	        BinaryNode<OpMultiply, 
	         Reference< QDPType< TScal, OScalar< TScal > > >,
	         Reference< QDPType< TVec, OLattice< TVec > > > > >,
	        OLattice< TVec > > &rhs,
	       const Subset& s)
{

#ifdef DEBUG_BLAS
  QDPIO::cout << "z = x*a - b*y" << endl;
#endif

  // Peel the stuff out of the expression
  // y is the right side of rhs

  // ax is the left side of rhs and is in a binary node
  typedef BinaryNode<OpMultiply, 
    Reference< QDPType< TVec, OLattice< TVec > > >,
    Reference< QDPType< TScal, OScalar< TScal > > > > BN1;

  typedef BinaryNode<OpMultiply, 
    Reference< QDPType< TScal, OScalar< TScal > > >,
    Reference< QDPType< TVec, OLattice< TVec > > > > BN2;


  
  // get the binary node
  const BN1 &mulNode1 = static_cast<const BN1&> (rhs.expression().left());
  const BN2 &mulNode2 = static_cast<const BN2&> (rhs.expression().right());

  // get a and x out of the binary node
  const OLattice< TVec >& x = static_cast<const OLattice< TVec >&>(mulNode1.left());

  const OScalar< TScal >& a = static_cast<const OScalar< TScal >&>(mulNode1.right());
  
  // get b and y out of the binary node
  const OScalar< TScal >& b = static_cast<const OScalar< TScal >&>(mulNode2.left());
  const OLattice< TVec >& y = static_cast<const OLattice< TVec >&>(mulNode2.right());

  
  // Set pointers 
  REAL64 *aptr = (REAL64 *)&(a.elem().elem().elem().elem());
  REAL64 *bptr = (REAL64 *)&(b.elem().elem().elem().elem());
  
  if( s.hasOrderedRep() ) { 
    REAL64 *xptr = (REAL64 *) &(x.elem(s.start()).elem(0).elem(0).real());
    REAL64 *yptr = (REAL64 *) &(y.elem(s.start()).elem(0).elem(0).real());
    REAL64* zptr =  &(d.elem(s.start()).elem(0).elem(0).real());
    
    
    // Get the no of 3vecs. s.start() and s.end() are inclusive so add +1
    int n_4vec = (s.end()-s.start()+1);
    vaxmbyz4(zptr, aptr, xptr, bptr, yptr, n_4vec);
  }
  else { 
    const int* tab = s.siteTable().slice();
    for(int j=0; j < s.numSiteTable(); j++) { 
      int i=tab[j];
   
      REAL64 *xptr = (REAL64 *) &(x.elem(i).elem(0).elem(0).real());
      REAL64 *yptr = (REAL64 *) &(y.elem(i).elem(0).elem(0).real());
      REAL64* zptr =  &(d.elem(i).elem(0).elem(0).real());
          
      // Get the no of 3vecs. s.start() and s.end() are inclusive so add +1
      vaxmbyz4(zptr, aptr, xptr, bptr, yptr, 1);
   
    }
  }
}

// z = ax - yb
template<>
inline
void evaluate( OLattice< TVec > &d,
	       const OpAssign &op,
	       const QDPExpr< 
	       BinaryNode<OpSubtract,
	        BinaryNode<OpMultiply, 
	         Reference< QDPType< TScal, OScalar< TScal > > >,
	         Reference< QDPType< TVec, OLattice< TVec > > > >,
	        BinaryNode<OpMultiply, 
	         Reference< QDPType< TVec, OLattice< TVec > > >,
	         Reference< QDPType< TScal, OScalar< TScal > > > > >,
	        OLattice< TVec > > &rhs,
	       const Subset& s)
{

#ifdef DEBUG_BLAS
  QDPIO::cout << "z = a*x - y*b" << endl;
#endif

  // Peel the stuff out of the expression
  // y is the right side of rhs

  // type of a*x
  typedef BinaryNode<OpMultiply, 
    Reference< QDPType< TScal, OScalar< TScal > > >,
    Reference< QDPType< TVec, OLattice< TVec > > > > BN1;

  // type of y*b
  typedef BinaryNode<OpMultiply, 
    Reference< QDPType< TVec, OLattice< TVec > > >,
    Reference< QDPType< TScal, OScalar< TScal > > > > BN2;


  
  // get the binary nodes
  // a*x node
  const BN1 &mulNode1 = static_cast<const BN1&> (rhs.expression().left());

  // y*b node
  const BN2 &mulNode2 = static_cast<const BN2&> (rhs.expression().right());

  // get a and x out of the binary node
  const OScalar< TScal >& a = static_cast<const OScalar< TScal >&>(mulNode1.left());

  const OLattice< TVec >& x = static_cast<const OLattice< TVec >&>(mulNode1.right());

  
  // get b and y out of the binary node
  const OLattice< TVec >& y = static_cast<const OLattice< TVec >&>(mulNode2.left());

  const OScalar< TScal >& b = static_cast<const OScalar< TScal >&>(mulNode2.right());

  
  // Set pointers 
  REAL64 *aptr = (REAL64 *)&(a.elem().elem().elem().elem());
  REAL64 *bptr = (REAL64 *)&(b.elem().elem().elem().elem());

  if( s.hasOrderedRep() ) { 
    REAL64 *xptr = (REAL64 *) &(x.elem(s.start()).elem(0).elem(0).real());
    REAL64 *yptr = (REAL64 *) &(y.elem(s.start()).elem(0).elem(0).real());
    REAL64* zptr =  &(d.elem(s.start()).elem(0).elem(0).real());
    
    
    // Get the no of 3vecs. s.start() and s.end() are inclusive so add +1
    int n_4vec = (s.end()-s.start()+1);
    vaxmbyz4(zptr, aptr, xptr, bptr, yptr, n_4vec);
  }
  else { 
    const int* tab = s.siteTable().slice();
    for(int j=0; j < s.numSiteTable(); j++) { 
      int i=tab[j];
   
      REAL64 *xptr = (REAL64 *) &(x.elem(i).elem(0).elem(0).real());
      REAL64 *yptr = (REAL64 *) &(y.elem(i).elem(0).elem(0).real());
      REAL64* zptr =  &(d.elem(i).elem(0).elem(0).real());
          
      // Get the no of 3vecs. s.start() and s.end() are inclusive so add +1
      vaxmbyz4(zptr, aptr, xptr, bptr, yptr, 1);
   
    }
  }
}

// z = xa - yb
template<>
inline
void evaluate( OLattice< TVec > &d,
	       const OpAssign &op,
	       const QDPExpr< 
	       BinaryNode<OpSubtract,
	        BinaryNode<OpMultiply, 
	         Reference< QDPType< TVec, OLattice< TVec > > >,
	         Reference< QDPType< TScal, OScalar< TScal > > > >,
	        BinaryNode<OpMultiply, 
	         Reference< QDPType< TVec, OLattice< TVec > > >,
	         Reference< QDPType< TScal, OScalar< TScal > > > > >,
	        OLattice< TVec > > &rhs,
	       const Subset& s)
{

#ifdef DEBUG_BLAS
  QDPIO::cout << "z = x*a - y*b" << endl;
#endif

  // Peel the stuff out of the expression
  // y is the right side of rhs

  // ax is the left side of rhs and is in a binary node
  typedef BinaryNode<OpMultiply, 
    Reference< QDPType< TVec, OLattice< TVec > > >,
    Reference< QDPType< TScal, OScalar< TScal > > > > BN;

  // get the binary node
  const BN &mulNode1 = static_cast<const BN&> (rhs.expression().left());
  const BN &mulNode2 = static_cast<const BN&> (rhs.expression().right());

  // get a and x out of the binary node
  const OLattice< TVec >& x = static_cast<const OLattice< TVec >&>(mulNode1.left());
  const OScalar< TScal >& a = static_cast<const OScalar< TScal >&>(mulNode1.right());
  
  // get b and y out of the binary node
  const OLattice< TVec >& y = static_cast<const OLattice< TVec >&>(mulNode2.left());

  const OScalar< TScal >& b = static_cast<const OScalar< TScal >&>(mulNode2.right());
  
  // Set pointers 
  REAL64 *aptr = (REAL64 *)&(a.elem().elem().elem().elem());
  REAL64 *bptr = (REAL64 *)&(b.elem().elem().elem().elem());
  if( s.hasOrderedRep() ) { 

    REAL64 *xptr = (REAL64 *) &(x.elem(s.start()).elem(0).elem(0).real());
    REAL64 *yptr = (REAL64 *) &(y.elem(s.start()).elem(0).elem(0).real());
    REAL64* zptr =  &(d.elem(s.start()).elem(0).elem(0).real());
    

    // Get the no of 3vecs. s.start() and s.end() are inclusive so add +1
    int n_4vec = (s.end()-s.start()+1);
    vaxmbyz4(zptr, aptr, xptr, bptr, yptr, n_4vec);
  }
  else { 
    const int* tab = s.siteTable().slice();
    for(int j=0; j < s.numSiteTable(); j++) { 
      int i=tab[j];
   
      REAL64 *xptr = (REAL64 *) &(x.elem(i).elem(0).elem(0).real());
      REAL64 *yptr = (REAL64 *) &(y.elem(i).elem(0).elem(0).real());
      REAL64* zptr =  &(d.elem(i).elem(0).elem(0).real());
          
      // Get the no of 3vecs. s.start() and s.end() are inclusive so add +1
      vaxmbyz4(zptr, aptr, xptr, bptr, yptr, 1);
   
    }
  }
}


// Global norm squared of a vector...
template<>
inline UnaryReturn<OLattice< TVec >, FnNorm2>::Type_t
norm2(const QDPType<TVec ,OLattice< TVec > >& s1, const Subset& s)
{
#ifdef DEBUG_BLAS
  QDPIO::cout << "Using BJ sumsq" << endl;
#endif

  if ( s.hasOrderedRep() ) {

#ifdef DEBUG_BLAS
    QDPIO::cout << "BJ sumsq " << endl;
#endif
    int n_3vec = (s.end() - s.start() + 1)*Ns;
    const REAL64 *s1ptr =  &(s1.elem(s.start()).elem(0).elem(0).real());
    
    // Has Type OScalar< PScalar < PScalar < RScalar < REAL64 > > > >

    DOUBLE lsum =(double)0;
    
    local_sumsq(&lsum,(REAL64 *)s1ptr, n_3vec); 
    UnaryReturn< OLattice< TVec >, FnNorm2>::Type_t  gsum(lsum);
    Internal::globalSum(gsum);
    return gsum;
  }
  else {

    // Has Type OScalar< PScalar < PScalar < RScalar < REAL64 > > > >
    DOUBLE lsum =(DOUBLE)0;
    DOUBLE ltmp =(DOUBLE)0;

    const int* tab=s.siteTable().slice();
    for(int j=0; j < s.numSiteTable(); j++) { 
      int i=tab[j];
      REAL64* s1ptr = (REAL64 *)&(s1.elem(i).elem(0).elem(0).real());
      local_sumsq(&ltmp,s1ptr,Ns); 
      lsum +=ltmp;
    }

    UnaryReturn< OLattice< TVec >, FnNorm2>::Type_t  gsum(lsum);
    Internal::globalSum(gsum);
    return gsum;
  }
}


template<>
inline UnaryReturn<OLattice< TVec >, FnNorm2>::Type_t
norm2(const QDPType<TVec ,OLattice< TVec > >& s1)
{
#ifdef DEBUG_BLAS
  QDPIO::cout << "Using BJ sumsq all" << endl;
#endif

  int n_3vec = (all.end() - all.start() + 1)*Ns;
  const REAL64 *s1ptr =  &(s1.elem(all.start()).elem(0).elem(0).real());
    


  DOUBLE lsum = 0;
  local_sumsq(&lsum, (REAL64 *)s1ptr, n_3vec); 
  UnaryReturn< OLattice< TVec >, FnNorm2>::Type_t  gsum(lsum);
  Internal::globalSum(gsum);
  return gsum;
}



template<>
inline  BinaryReturn< OLattice<TVec>, OLattice<TVec>, FnInnerProduct>::Type_t
innerProduct(const QDPType< TVec, OLattice<TVec> > &v1,
	     const QDPType< TVec, OLattice<TVec> > &v2)
{
#ifdef DEBUG_BLAS
  QDPIO::cout << "BJ: innerProduct all" << endl;
#endif

  // This BinaryReturn has Type_t
  // OScalar<OScalar<OScalar<RComplex<PScalar<REAL64> > > > >
  BinaryReturn< OLattice<TVec>, OLattice<TVec>, FnInnerProduct>::Type_t lprod;
  // Inner product is accumulated internally in DOUBLE
  DOUBLE ip[2];
  ip[0]=0;
  ip[1]=0;

  // Length of subset 
  unsigned long n_3vec = (all.end() - all.start() + 1)*Ns;
    
  // Call My CDOT
  local_vcdot(&(ip[0]), &(ip[1]),
	      (REAL64 *)&(v1.elem(all.start()).elem(0).elem(0).real()),
	      (REAL64 *)&(v2.elem(all.start()).elem(0).elem(0).real()),
	      n_3vec);


  // Global sum -- still on a vector of doubles
  Internal::globalSumArray(ip,2);

  // Downcast (and possibly lose precision) here 
  lprod.elem().elem().elem().real() = ip[0];
  lprod.elem().elem().elem().imag() = ip[1];

  // Return
  return lprod;
}

template<>
inline  BinaryReturn< OLattice<TVec>, OLattice<TVec>, FnInnerProduct>::Type_t
innerProduct(const QDPType< TVec, OLattice<TVec> > &v1,
	     const QDPType< TVec, OLattice<TVec> > &v2, 
	     const Subset& s)
{
  
  if( s.hasOrderedRep() ) {
#ifdef DEBUG_BLAS
    QDPIO::cout << "BJ: innerProduct s" << endl;
#endif

    // This BinaryReturn has Type_t
    // OScalar<OScalar<OScalar<RComplex<PScalar<REAL64> > > > >
    BinaryReturn< OLattice<TVec>, OLattice<TVec>, FnInnerProduct>::Type_t lprod;
    DOUBLE ip[2];
    ip[0] = 0;
    ip[1] = 0;

    unsigned long n_3vec = (s.end() - s.start() + 1)*Ns;
    local_vcdot(&(ip[0]), &(ip[1]),
		(REAL64 *)&(v1.elem(s.start()).elem(0).elem(0).real()),
		(REAL64 *)&(v2.elem(s.start()).elem(0).elem(0).real()),
		n_3vec);


    Internal::globalSumArray(ip,2);

    lprod.elem().elem().elem().real() = ip[0];
    lprod.elem().elem().elem().imag() = ip[1];
    

    return lprod;
  }
  else {

    BinaryReturn< OLattice<TVec>, OLattice<TVec>, FnInnerProduct>::Type_t lprod;
    DOUBLE ip[2], ip_tmp[2];
    ip[0] = 0;
    ip[1] = 0;

    const int *tab = s.siteTable().slice();
    for(int j=0; j < s.numSiteTable(); j++) { 

      int i=tab[j];
      
      local_vcdot(&(ip_tmp[0]), &(ip_tmp[1]),
		  (REAL64 *)&(v1.elem(i).elem(0).elem(0).real()),
		  (REAL64 *)&(v2.elem(i).elem(0).elem(0).real()),
		  Ns);
      
      ip[0] += ip_tmp[0];
      ip[1] += ip_tmp[1];
    }

    Internal::globalSumArray(ip,2);

    lprod.elem().elem().elem().real() = ip[0];
    lprod.elem().elem().elem().imag() = ip[1];
    

    return lprod;

  }
}



// Inner Product Real
template<>
inline  
BinaryReturn< OLattice<TVec>, OLattice<TVec>, FnInnerProductReal>::Type_t
innerProductReal(const QDPType< TVec, OLattice<TVec> > &v1,
		 const QDPType< TVec, OLattice<TVec> > &v2)
{
#ifdef DEBUG_BLAS
  QDPIO::cout << "BJ: innerProductReal all" << endl;
#endif

  // This BinaryReturn has Type_t
  // OScalar<OScalar<OScalar<RScalar<PScalar<REAL64> > > > >
  BinaryReturn< OLattice<TVec>, OLattice<TVec>, FnInnerProductReal>::Type_t lprod;
  // Inner product is accumulated internally in DOUBLE
  DOUBLE ip_re=0;

  // Length of subset 
  unsigned long n_3vec = (all.end() - all.start() + 1)*Ns;

  // Call My CDOT
  local_vcdot_real(&ip_re,
		   (REAL64 *)&(v1.elem(all.start()).elem(0).elem(0).real()),
		   (REAL64 *)&(v2.elem(all.start()).elem(0).elem(0).real()),
		   n_3vec);

  // Global sum
  Internal::globalSum(ip_re);

  // Whether CDOT did anything or not ip_re and ip_im should 
  // now be right. Assign them to the ReturnType
  lprod.elem().elem().elem().elem() = ip_re;


  // Return
  return lprod;
}


template<>
inline  
BinaryReturn< OLattice<TVec>, OLattice<TVec>, FnInnerProductReal>::Type_t
innerProductReal(const QDPType< TVec, OLattice<TVec> > &v1,
		 const QDPType< TVec, OLattice<TVec> > &v2, 
		 const Subset& s)
{
  if( s.hasOrderedRep() ) {
#ifdef DEBUG_BLAS
    QDPIO::cout << "BJ: innerProductReal s" << endl;
#endif

    // This BinaryReturn has Type_t
    // OScalar<OScalar<OScalar<RScalar<PScalar<REAL64> > > > >
    BinaryReturn< OLattice<TVec>, OLattice<TVec>, FnInnerProductReal>::Type_t lprod;
    DOUBLE ip_re=0;

    unsigned long n_3vec = (s.end() - s.start() + 1)*Ns;
    local_vcdot_real(&ip_re,
		     (REAL64 *)&(v1.elem(s.start()).elem(0).elem(0).real()),
		     (REAL64 *)&(v2.elem(s.start()).elem(0).elem(0).real()),
		     n_3vec);

    Internal::globalSum(ip_re);
    lprod.elem().elem().elem().elem() = ip_re;


    return lprod;
  }
  else {


    BinaryReturn< OLattice<TVec>, OLattice<TVec>, FnInnerProductReal>::Type_t lprod;
    DOUBLE ip_re=0, ip_re_tmp;


    const int *tab = s.siteTable().slice();
    for(int j=0; j < s.numSiteTable(); j++) { 

      int i=tab[j];
      
      local_vcdot_real(&ip_re_tmp,
		  (REAL64 *)&(v1.elem(i).elem(0).elem(0).real()),
		  (REAL64 *)&(v2.elem(i).elem(0).elem(0).real()),
		  Ns);
      
      ip_re += ip_re_tmp;
    }
    Internal::globalSum(ip_re);
    lprod.elem().elem().elem().elem() = ip_re;
    return lprod;
  }
}


template<>
inline UnaryReturn<OLattice< TVec >, FnNorm2>::Type_t
norm2(const multi1d< OLattice< TVec > >& s1)
{
#ifdef DEBUG_BLAS
  QDPIO::cout << "Using SSE multi1d sumsq all" << endl;
#endif

  int n_3vec = (all.end() - all.start() + 1)*Ns;
  DOUBLE ltmp = 0;
  for(int n=0; n < s1.size(); ++n)
  {
    const REAL64* s1ptr =  &(s1[n].elem(all.start()).elem(0).elem(0).real());
    
    // I am relying on this being a Double here 
    DOUBLE lltmp;
    local_sumsq(&lltmp, (REAL64*)s1ptr, n_3vec); 

    ltmp += lltmp;
  }

  UnaryReturn< OLattice< TVec >, FnNorm2>::Type_t  lsum(ltmp);
  Internal::globalSum(lsum);
  return lsum;
}


template<>
inline  BinaryReturn< OLattice<TVec>, OLattice<TVec>, FnInnerProduct>::Type_t
innerProduct(const multi1d< OLattice<TVec> > &v1,
	     const multi1d< OLattice<TVec> > &v2)
{
#ifdef DEBUG_BLAS
  QDPIO::cout << "BJ: multi1d innerProduct all" << endl;
#endif

  // This BinaryReturn has Type_t
  // OScalar<OScalar<OScalar<RComplex<PScalar<REAL64> > > > >
  BinaryReturn< OLattice<TVec>, OLattice<TVec>, FnInnerProduct>::Type_t lprod;
  // Inner product is accumulated internally in DOUBLE
  DOUBLE ip[2];
  ip[0]=0;
  ip[1]=0;

  // Length of subset 
  unsigned long n_3vec = (all.end() - all.start() + 1)*Ns;
    
  for(int n=0; n < v1.size(); ++n)
  {
    DOUBLE iip[2];
    iip[0]=0;
    iip[1]=0;

    // Call My CDOT
    local_vcdot(&(iip[0]), &(iip[1]),
		(REAL64 *)&(v1[n].elem(all.start()).elem(0).elem(0).real()),
		(REAL64 *)&(v2[n].elem(all.start()).elem(0).elem(0).real()),
		n_3vec);
    
    ip[0] += iip[0];
    ip[1] += iip[1];
  }

  // Global sum -- still on a vector of doubles
  Internal::globalSumArray(ip,2);

  // Downcast (and possibly lose precision) here 
  lprod.elem().elem().elem().real() = ip[0];
  lprod.elem().elem().elem().imag() = ip[1];

  // Return
  return lprod;
}


// Inner Product Real
template<>
inline  
BinaryReturn< OLattice<TVec>, OLattice<TVec>, FnInnerProductReal>::Type_t
innerProductReal(const multi1d< OLattice<TVec> > &v1,
		 const multi1d< OLattice<TVec> > &v2)
{
#ifdef DEBUG_BLAS
  QDPIO::cout << "BJ: innerProductReal(multi1d) all" << endl;
#endif

  // This BinaryReturn has Type_t
  // OScalar<OScalar<OScalar<RScalar<PScalar<REAL64> > > > >
  BinaryReturn< OLattice<TVec>, OLattice<TVec>, FnInnerProductReal>::Type_t lprod;
  // Inner product is accumulated internally in DOUBLE
  DOUBLE ip_re=0;

  // Length of subset 
  unsigned long n_3vec = (all.end() - all.start() + 1)*Ns;

  for(int n=0; n < v1.size(); ++n)
  {
    DOUBLE iip_re=0;

    // Call My CDOT
    local_vcdot_real(&iip_re,
		     (REAL64 *)&(v1[n].elem(all.start()).elem(0).elem(0).real()),
		     (REAL64 *)&(v2[n].elem(all.start()).elem(0).elem(0).real()),
		     n_3vec);

    ip_re += iip_re;
  }

  // Global sum
  Internal::globalSum(ip_re);

  // Whether CDOT did anything or not ip_re and ip_im should 
  // now be right. Assign them to the ReturnType
  lprod.elem().elem().elem().elem() = ip_re;


  // Return
  return lprod;
}


} // namespace QDP;

#endif  // guard
 