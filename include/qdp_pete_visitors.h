#ifndef QDP_PETE_VIS_H
#define QDP_PETE_VIS_H


namespace QDP {







struct ShiftPhase1
{
  ShiftPhase1(const Subset& _s):subset(_s) {}
  const Subset& subset;
};

struct ShiftPhase2
{
};



struct ViewLeaf
{
  JitDeviceLayout layout_m;
  IndexDomainVector idv_m;
  
  ViewLeaf( JitDeviceLayout layout , llvm::Value * index ) : layout_m(layout)
  {
    idv_m.push_back( make_pair( Layout::sitesOnNode() , index ) );
  }
  
  ViewLeaf( JitDeviceLayout layout , IndexDomainVector idv ): layout_m(layout), idv_m(idv)
  {
  }
  
  JitDeviceLayout getLayout() const { return layout_m; }

  llvm::Value *getIndex() const
  {
    llvm::Value *ret;
    
    if (idv_m.empty())
      {
	QDPIO::cerr << "ViewLeaf: asking for index but IDV empty." << std::endl;
	QDP_abort(1);
      }
    else
      {
	llvm::Value * thread_idx = llvm_create_value(0);
	for( auto x = idv_m.begin() ; x != idv_m.end() ; x++ )
	  {
	    int         Index;
	    llvm::Value * index;
	    std::tie(Index,index) = *x;
	    llvm::Value * Index_jit = llvm_create_value(Index);
	    thread_idx = llvm_add( llvm_mul( thread_idx , Index_jit ) , index );
	  }
	ret = thread_idx;
      }
    return ret;
  }


};



struct ParamLeaf {};





struct AddressLeaf
{
  ~AddressLeaf() {
    for (auto i : ids_signoff) {
      QDP_get_global_cache().signoff(i);
    }
  }
  
  AddressLeaf(const Subset& s): subset(s) {}
  AddressLeaf(const AddressLeaf& cp) = delete;

  AddressLeaf& operator=(const AddressLeaf& cp) = delete;

  
  mutable std::vector<int> ids;
  mutable std::vector<int> ids_signoff;
  const Subset& subset;

  void setId( int id ) const {
    ids.push_back( id );
  }
  
  template<class T> void setLit( T f ) const;

  
};

  template<>
  void AddressLeaf::setLit<float>( float f ) const;
  
  template<>
  void AddressLeaf::setLit<double>( double d ) const;
  
  template<>
  void AddressLeaf::setLit<int>( int i ) const;
  
  template<>
  void AddressLeaf::setLit<int64_t>( int64_t i ) const;
  
  template<>
  void AddressLeaf::setLit<bool>( bool b ) const;


  
  

  template<class LeafType, class LeafTag>
  struct AddOpParam
  { };

  template<class LeafType>
  struct AddOpParam<LeafType,ParamLeaf>
  { 
    typedef LeafType Type_t;
    static LeafType apply(const LeafType&, const ParamLeaf& p) { return LeafType(); }
  };

  template<class LeafType, class LeafTag>
  struct AddOpAddress
  { };

  template<class LeafType>
  struct AddOpAddress<LeafType,AddressLeaf>
  { 
    static void apply(const LeafType&, const AddressLeaf& p) {}
  };






  struct DynKeyTag
  {
    const DynKey& key;
  
    DynKeyTag(const DynKey& k): key(k) {}

    DynKeyTag(const DynKeyTag& cp) = delete;
    DynKeyTag& operator=(const DynKeyTag& cp) = delete;
  };


  template<class T>
  struct LeafFunctor< T , DynKeyTag >
  {
    typedef bool Type_t;
    inline static
    Type_t apply(const T &s, const DynKeyTag &) 
    {
      return false;
    }
  };



  
}

#endif
