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


  llvm::Value * datalayout( JitDeviceLayout lay , IndexDomainVector in ) {
    assert(in.size() > 0);

    IndexDomainVector out;
    
    // QDPIO::cout << "datalayout, domains = ";
    // for ( auto& i : a )
    //   QDPIO::cout << i.first << ", ";
    // QDPIO::cout << std::endl;
    
    if ( lay == JitDeviceLayout::Coalesced )
      {
	if (in.size() == 6)
	  {
	    std::string nesting = jit_config_get_nesting_order();
	    std::string order   = jit_config_get_loop_order();

	    if (nesting.size() != order.size())
	      {
		QDPIO::cerr << "Internal error: Nesting and loop order string size mismatch." << std::endl;
		QDP_abort(1);
	      }

	    std::map<char,int> nestmap;
	    for( int i = 0 ; i < nesting.size() ; ++i )
	      {
		nestmap[ nesting[i] ] = i;
	      }

	    out = in;
	    for( int i = 0 ; i < order.size() ; ++i )
	      {
		int pe = nestmap[ order[i] ];
		out[i] = in[ pe ];
		//QDPIO::cout << "pos " << i << "  --> " << pe << std::endl;
	      }
	  }
	else
	  {
	    out = in;
	    std::reverse( out.begin() , out.end() );
	  }
      }
    else
      {
	out = in;
      }

    if (in.hasAdd() != out.hasAdd())
      {
	QDPIO::cerr << "Internal error: ..." << std::endl;
	QDP_abort(1);
	
      }

    llvm::Value * offset = llvm_create_value(0);
    for( auto x = out.begin() ; x != out.end() ; x++ ) {
      int         Index;
      llvm::Value * index;
      std::tie(Index,index) = *x;
      llvm::Value * Index_jit = llvm_create_value(Index);
      offset = llvm_add( llvm_mul( offset , Index_jit ) , index );
    }

    if (out.hasAdd())
      {
	offset = llvm_add( offset , out.getAdd() );
      }
    
    return offset;
  }



} // namespace
