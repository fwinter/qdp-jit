#ifndef QDP_FUNCTIONS_SPECIAL_PROP_DERIV_H
#define QDP_FUNCTIONS_SPECIAL_PROP_DERIV_H

#include "cuda_special.h"

namespace QDP
{

  template <class Op>
  struct ParenVisitor
  { 
    static void start(Op op, const AddressLeaf& t) {}
    static void center(Op op, const AddressLeaf& t) {}
    static void finish(Op op, const AddressLeaf& t) {}
  };

  

  template <class T>
  struct TagVisitor<T, AddressLeaf>: public ParenVisitor<T>
  {
    static void visit(T op, const AddressLeaf& t) {}
  };

  
  template <>
  struct TagVisitor<FnMap, AddressLeaf> : public ParenVisitor<FnMap>
  {
    static void visit(const FnMap& op, const AddressLeaf& a)
    {
      const Map& map(op.map);
      FnMap& fnmap = const_cast<FnMap&>(op);

      //QDPIO::cout << "goff, recv \n";

      a.setId( op.map.getGoffsetsId(a.subset) );
      a.setId( map.hasOffnode() ? fnmap.getCached().getRecvBufId() : -1 );
    }
  };


  template<class T>
  void evaluate( OLattice< PSpinMatrix<PColorMatrix<RComplex<Word<T> >, 3>, 4> >& dest,
		 const OpSubtractAssign& op,
		 const QDPExpr< BinaryNode<OpAdd, BinaryNode<OpMultiply, Reference<QDPType<PScalar<PColorMatrix<RComplex<Word<T> >, 3> >, OLattice<PScalar<PColorMatrix<RComplex<Word<T> >, 3> > > > >, UnaryNode<FnMap, Reference<QDPType<PSpinMatrix<PColorMatrix<RComplex<Word<T> >, 3>, 4>, OLattice<PSpinMatrix<PColorMatrix<RComplex<Word<T> >, 3>, 4> > > > > >, UnaryNode<FnMap, BinaryNode<OpAdjMultiply, UnaryNode<OpIdentity, Reference<QDPType<PScalar<PColorMatrix<RComplex<Word<T> >, 3> >, OLattice<PScalar<PColorMatrix<RComplex<Word<T> >, 3> > > > > >, Reference<QDPType<PSpinMatrix<PColorMatrix<RComplex<Word<T> >, 3>, 4>, OLattice<PSpinMatrix<PColorMatrix<RComplex<Word<T> >, 3>, 4> > > > > > > , OLattice< PSpinMatrix<PColorMatrix<RComplex<Word<T> >, 3>, 4> > >& rhs,
		 const Subset& s )
  {
    typedef BinaryNode<OpAdd, BinaryNode<OpMultiply, Reference<QDPType<PScalar<PColorMatrix<RComplex<Word<T> >, 3> >, OLattice<PScalar<PColorMatrix<RComplex<Word<T> >, 3> > > > >, UnaryNode<FnMap, Reference<QDPType<PSpinMatrix<PColorMatrix<RComplex<Word<T> >, 3>, 4>, OLattice<PSpinMatrix<PColorMatrix<RComplex<Word<T> >, 3>, 4> > > > > >, UnaryNode<FnMap, BinaryNode<OpAdjMultiply, UnaryNode<OpIdentity, Reference<QDPType<PScalar<PColorMatrix<RComplex<Word<T> >, 3> >, OLattice<PScalar<PColorMatrix<RComplex<Word<T> >, 3> > > > > >, Reference<QDPType<PSpinMatrix<PColorMatrix<RComplex<Word<T> >, 3>, 4>, OLattice<PSpinMatrix<PColorMatrix<RComplex<Word<T> >, 3>, 4> > > > > > > RHS_prop_deriv;

    static bool printed = false;
    if (!printed)
      {
	QDPIO::cout << "in template specialization prop_deriv \n";
	printed = true;
      }
    
    typedef QDPExpr< RHS_prop_deriv , OLattice< PSpinMatrix<PColorMatrix<RComplex<Word<T> >, 3>, 4> > >  Expr;
    typedef typename CreateLeaf<Expr>::Leaf_t Expr_t;
    const Expr_t &e = CreateLeaf<Expr>::make(rhs);

    const int func_num = 4;
    const int default_blocksize = 128;

    //jumper_jit_stats_special(func_num);

  
    if (!s.hasOrderedRep())
      {
	QDPIO::cout << "Special prop deriv not supported on subsets with unordered representation." << std::endl;
	QDP_abort(1);
      }

    ShiftPhase1 phase1(s);
    int offnode_maps = forEach(rhs, phase1 , BitOrCombine());

    AddressLeaf addr_leaf(s);
    forEach(dest, addr_leaf, NullCombine());
    //AddOpAddress<Op,AddressLeaf>::apply(op,addr_leaf);

    typedef ForEachInOrder< RHS_prop_deriv , AddressLeaf , AddressLeaf , NullTag> Address_t;
    Address_t::apply( e , addr_leaf , addr_leaf , NullTag());


    //QDPIO::cout << "number of ids = " << addr_leaf.ids.size() << std::endl;
    

    if (offnode_maps == 0)
      {
	//QDPIO::cout << "no offnode comms\n";
	
	int th_count = s.hasOrderedRep() ? s.numSiteTable() : Layout::sitesOnNode();

	std::vector<QDPCache::ArgKey> ids;
	for(unsigned i=0; i < addr_leaf.ids.size(); ++i)
	  ids.push_back( addr_leaf.ids[i] );

	std::vector<void*> args = QDP_get_global_cache().get_kernel_args( ids , true );

	if (sizeof(T) == 4)
	  evaluate_special_prop_deriv_float( th_count , Layout::sitesOnNode() , s.start() , args , false , func_num );
	else
	  evaluate_special_prop_deriv_double( th_count , Layout::sitesOnNode() , s.start() , args , false , func_num );
      }
    else
      {
	// 1st. call: inner
	{
	  int th_count = MasterMap::Instance().getCountInner(s,offnode_maps);
      
	  std::vector<QDPCache::ArgKey> ids;
	  for(unsigned i=0; i < addr_leaf.ids.size(); ++i) 
	    ids.push_back( addr_leaf.ids[i] );

	  //QDPIO::cout << "inner goffset num = " << ids.size() << std::endl;

	  ids.push_back( MasterMap::Instance().getIdInner(s,offnode_maps) );

	  std::vector<void*> args = QDP_get_global_cache().get_kernel_args( ids , true );

	  if (sizeof(T) == 4)
	    evaluate_special_prop_deriv_float( th_count , Layout::sitesOnNode(), 0 , args , true , func_num );
	  else
	    evaluate_special_prop_deriv_double( th_count , Layout::sitesOnNode(), 0 , args , true , func_num );
	}

	// 2nd call: face
	{
	  ShiftPhase2 phase2;
	  forEach(rhs, phase2 , NullCombine());

	  int th_count = MasterMap::Instance().getCountFace(s,offnode_maps);
      
	  std::vector<QDPCache::ArgKey> ids;
	  for(unsigned i=0; i < addr_leaf.ids.size(); ++i) 
	    ids.push_back( addr_leaf.ids[i] );

	  //QDPIO::cout << "face goffset num = " << ids.size() << std::endl;
	
	  ids.push_back( MasterMap::Instance().getIdFace(s,offnode_maps) );

	  std::vector<void*> args = QDP_get_global_cache().get_kernel_args( ids , true );

	  if (sizeof(T) == 4)
	    evaluate_special_prop_deriv_float( th_count , Layout::sitesOnNode(), 0 , args , true , func_num );
	  else
	    evaluate_special_prop_deriv_double( th_count , Layout::sitesOnNode(), 0 , args , true , func_num );
	}
      }

  }

}
#endif
