#include "qdp.h"

namespace QDP {

#if 0
  llvm::Value * datalayout( JitDeviceLayout lay , IndexDomainVector a ) {
    const size_t nIv = 0; // volume
    const size_t nIs = 1; // spin
    const size_t nIc = 2; // color
    const size_t nIr = 3; // reality

    int         Lv,Ls,Lc,Lr;
    llvm::Value * iv,is,ic,ir;

    std::tie(Lv,iv) = a.at(nIv);
    std::tie(Ls,is) = a.at(nIs);
    std::tie(Lc,ic) = a.at(nIc);
    std::tie(Lr,ir) = a.at(nIr);

    llvm::Value * Iv = llvm_create_value(Lv);
    llvm::Value * Is = llvm_create_value(Ls);
    llvm::Value * Ic = llvm_create_value(Lc);
    llvm::Value * Ir = llvm_create_value(Lr);

    // offset = ((ir * Ic + ic) * Is + is) * Iv + iv

    if (lay == JitDeviceLayout::Coalesced) {
      return llvm_add(llvm_mul(llvm_add(llvm_mul( llvm_add(llvm_mul(ir,Ic),ic),Is),is),Iv),iv);
    } else
      return llvm_add(llvm_mul(llvm_add(llvm_mul( llvm_add(llvm_mul(iv,Ir),ir),Ic),ic),Is),is);
  }
#endif


#if 0
  llvm::Value * datalayout_stack( JitDeviceLayout lay , IndexDomainVector a ) {
    assert(a.size() > 0);
    llvm::Value * offset = llvm_create_value(0);
    for( auto x = a.rbegin() ; x != a.rend() ; x++ ) {
      int         Index;
      llvm::Value * index;
      std::tie(Index,index) = *x;
      llvm::Value * Index_jit = llvm_create_value(Index);
      offset = llvm_add( llvm_mul( offset , Index_jit ) , index );
    }
    return offset;
  }
#endif


  llvm::Value * datalayout( JitDeviceLayout lay , IndexDomainVector a ) {
    assert(a.size() > 0);

    // QDPIO::cout << "datalayout, domains = ";
    // for ( auto& i : a )
    //   QDPIO::cout << i.first << ", ";
    // QDPIO::cout << std::endl;
    
    // In case of a coalesced layout (OLattice)
    // We reverse the data layout given by the natural nesting order
    // of aggregates, i.e. reality slowest, lattice fastest
    // In case of a scalar layout (sums,comms buffers,OScalar)
    // We actually use the index order/data layout given by the
    // nesting order of aggregates
    if ( lay == JitDeviceLayout::Coalesced )
      {
	if (a.size() == 6)
	  {
	    std::reverse( a.begin() , a.end() );
	  }
	else
	  {
	    std::reverse( a.begin() , a.end() );
	  }
      }

    llvm::Value * offset = llvm_create_value(0);
    for( auto x = a.begin() ; x != a.end() ; x++ ) {
      int         Index;
      llvm::Value * index;
      std::tie(Index,index) = *x;
      llvm::Value * Index_jit = llvm_create_value(Index);
      offset = llvm_add( llvm_mul( offset , Index_jit ) , index );
    }

    if (a.hasAdd())
      {
	offset = llvm_add( offset , a.getAdd() );
      }
    
    return offset;
  }



} // namespace
