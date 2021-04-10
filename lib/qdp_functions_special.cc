#include "qdp.h"
#include "cuda_special.h"

namespace QDP
{

  //void QDP::evaluate(QDP::OLattice<T>&, const Op&, const QDP::QDPExpr<RHS, QDP::OLattice<T1> >&, const QDP::Subset&) [with T = QDP::PSpinMatrix<QDP::PColorMatrix<QDP::RComplex<QDP::Word<float> >, 3>, 4>; T1 = QDP::PSpinMatrix<QDP::PColorMatrix<QDP::RComplex<QDP::Word<float> >, 3>, 4>; Op = QDP::OpAssign; RHS = QDP::BinaryNode<QDP::FnQuarkContract13, QDP::Reference<QDP::QDPType<QDP::PSpinMatrix<QDP::PColorMatrix<QDP::RComplex<QDP::Word<float> >, 3>, 4>, QDP::OLattice<QDP::PSpinMatrix<QDP::PColorMatrix<QDP::RComplex<QDP::Word<float> >, 3>, 4> > > >, QDP::Reference<QDP::QDPType<QDP::PSpinMatrix<QDP::PColorMatrix<QDP::RComplex<QDP::Word<float> >, 3>, 4>, QDP::OLattice<QDP::PSpinMatrix<QDP::PColorMatrix<QDP::RComplex<QDP::Word<float> >, 3>, 4> > > > >]

  void evaluate( OLattice< PSpinMatrix<PColorMatrix<RComplex<Word<float> >, 3>, 4> >& dest,
		 const OpAssign& op,
		 const QDPExpr<
		 BinaryNode<FnQuarkContract13,
		   Reference<QDPType<PSpinMatrix<PColorMatrix<RComplex<Word<float> >, 3>, 4>, OLattice<PSpinMatrix<PColorMatrix<RComplex<Word<float> >, 3>, 4> > > >,
  		   Reference<QDPType<PSpinMatrix<PColorMatrix<RComplex<Word<float> >, 3>, 4>, OLattice<PSpinMatrix<PColorMatrix<RComplex<Word<float> >, 3>, 4> > > > >  ,
		 OLattice< PSpinMatrix<PColorMatrix<RComplex<Word<float> >, 3>, 4> > >& rhs,
		 const Subset& s)
  {
    //QDPIO::cout << "in template specialization quarkContract13 \n";

    std::vector<QDPCache::ArgKey> ids = {
      dest.getId(),
      rhs.expression().left().getId(),
      rhs.expression().right().getId()
    };

    std::vector<void*> args = QDP_get_global_cache().get_kernel_args( ids , false );

    // std::cout << args[0] << "\n";
    // std::cout << args[1] << "\n";
    // std::cout << args[2] << "\n";

    evaluate_special_quarkContract13( Layout::sitesOnNode() , args );    
  }


  void evaluate( OLattice< PSpinMatrix<PColorMatrix<RComplex<Word<float> >, 3>, 4> >& dest,
		 const OpAssign& op,
		 const QDPExpr<
		 BinaryNode<FnQuarkContract14,
		   Reference<QDPType<PSpinMatrix<PColorMatrix<RComplex<Word<float> >, 3>, 4>, OLattice<PSpinMatrix<PColorMatrix<RComplex<Word<float> >, 3>, 4> > > >,
  		   Reference<QDPType<PSpinMatrix<PColorMatrix<RComplex<Word<float> >, 3>, 4>, OLattice<PSpinMatrix<PColorMatrix<RComplex<Word<float> >, 3>, 4> > > > >  ,
		 OLattice< PSpinMatrix<PColorMatrix<RComplex<Word<float> >, 3>, 4> > >& rhs,
		 const Subset& s)
  {
    //QDPIO::cout << "in template specialization quarkContract14 \n";

    std::vector<QDPCache::ArgKey> ids = {
      dest.getId(),
      rhs.expression().left().getId(),
      rhs.expression().right().getId()
    };

    std::vector<void*> args = QDP_get_global_cache().get_kernel_args( ids , false );

    // std::cout << args[0] << "\n";
    // std::cout << args[1] << "\n";
    // std::cout << args[2] << "\n";

    evaluate_special_quarkContract14( Layout::sitesOnNode() , args );    
  }


  void evaluate( OLattice< PSpinMatrix<PColorMatrix<RComplex<Word<float> >, 3>, 4> >& dest,
		 const OpAssign& op,
		 const QDPExpr<
		 BinaryNode<FnQuarkContract23,
		   Reference<QDPType<PSpinMatrix<PColorMatrix<RComplex<Word<float> >, 3>, 4>, OLattice<PSpinMatrix<PColorMatrix<RComplex<Word<float> >, 3>, 4> > > >,
  		   Reference<QDPType<PSpinMatrix<PColorMatrix<RComplex<Word<float> >, 3>, 4>, OLattice<PSpinMatrix<PColorMatrix<RComplex<Word<float> >, 3>, 4> > > > >  ,
		 OLattice< PSpinMatrix<PColorMatrix<RComplex<Word<float> >, 3>, 4> > >& rhs,
		 const Subset& s)
  {
    //QDPIO::cout << "in template specialization quarkContract23 \n";

    std::vector<QDPCache::ArgKey> ids = {
      dest.getId(),
      rhs.expression().left().getId(),
      rhs.expression().right().getId()
    };

    std::vector<void*> args = QDP_get_global_cache().get_kernel_args( ids , false );

    // std::cout << args[0] << "\n";
    // std::cout << args[1] << "\n";
    // std::cout << args[2] << "\n";

    evaluate_special_quarkContract23( Layout::sitesOnNode() , args );
  }

  
  void evaluate( OLattice< PSpinMatrix<PColorMatrix<RComplex<Word<float> >, 3>, 4> >& dest,
		 const OpAssign& op,
		 const QDPExpr<
		 BinaryNode<FnQuarkContract24,
		   Reference<QDPType<PSpinMatrix<PColorMatrix<RComplex<Word<float> >, 3>, 4>, OLattice<PSpinMatrix<PColorMatrix<RComplex<Word<float> >, 3>, 4> > > >,
  		   Reference<QDPType<PSpinMatrix<PColorMatrix<RComplex<Word<float> >, 3>, 4>, OLattice<PSpinMatrix<PColorMatrix<RComplex<Word<float> >, 3>, 4> > > > >  ,
		 OLattice< PSpinMatrix<PColorMatrix<RComplex<Word<float> >, 3>, 4> > >& rhs,
		 const Subset& s)
  {
    //QDPIO::cout << "in template specialization quarkContract24 \n";

    std::vector<QDPCache::ArgKey> ids = {
      dest.getId(),
      rhs.expression().left().getId(),
      rhs.expression().right().getId()
    };

    std::vector<void*> args = QDP_get_global_cache().get_kernel_args( ids , false );

    // std::cout << args[0] << "\n";
    // std::cout << args[1] << "\n";
    // std::cout << args[2] << "\n";

    evaluate_special_quarkContract24( Layout::sitesOnNode() , args );
  }


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

    QDPIO::cout << "goff, recv \n";

    a.setId( op.map.getGoffsetsId(a.subset) );
    a.setId( map.hasOffnode() ? fnmap.getCached().getRecvBufId() : -1 );
  }
};


  typedef BinaryNode<OpAdd, BinaryNode<OpMultiply, Reference<QDPType<PScalar<PColorMatrix<RComplex<Word<float> >, 3> >, OLattice<PScalar<PColorMatrix<RComplex<Word<float> >, 3> > > > >, UnaryNode<FnMap, Reference<QDPType<PSpinMatrix<PColorMatrix<RComplex<Word<float> >, 3>, 4>, OLattice<PSpinMatrix<PColorMatrix<RComplex<Word<float> >, 3>, 4> > > > > >, UnaryNode<FnMap, BinaryNode<OpAdjMultiply, UnaryNode<OpIdentity, Reference<QDPType<PScalar<PColorMatrix<RComplex<Word<float> >, 3> >, OLattice<PScalar<PColorMatrix<RComplex<Word<float> >, 3> > > > > >, Reference<QDPType<PSpinMatrix<PColorMatrix<RComplex<Word<float> >, 3>, 4>, OLattice<PSpinMatrix<PColorMatrix<RComplex<Word<float> >, 3>, 4> > > > > > >
  RHS_prop_deriv;

  void evaluate( OLattice< PSpinMatrix<PColorMatrix<RComplex<Word<float> >, 3>, 4> >& dest,
  		 const OpSubtractAssign& op,
  		 const QDPExpr<
  		 RHS_prop_deriv,
		 OLattice< PSpinMatrix<PColorMatrix<RComplex<Word<float> >, 3>, 4> > >& rhs,
  		 const Subset& s )
  {
    QDPIO::cout << "in template specialization prop_deriv \n";
    
    typedef QDPExpr< RHS_prop_deriv , OLattice< PSpinMatrix<PColorMatrix<RComplex<Word<float> >, 3>, 4> > >  Expr;
    typedef typename CreateLeaf<Expr>::Leaf_t Expr_t;
    const Expr_t &e = CreateLeaf<Expr>::make(rhs);

    const int func_num = 4;
    const int default_blocksize = 128;

    jumper_jit_stats_special(func_num);

  
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


    QDPIO::cout << "number of ids = " << addr_leaf.ids.size() << std::endl;
    

    if (offnode_maps == 0)
      {
	QDPIO::cout << "no offnode comms\n";
	
	int th_count = s.hasOrderedRep() ? s.numSiteTable() : Layout::sitesOnNode();

	std::vector<QDPCache::ArgKey> ids;
	for(unsigned i=0; i < addr_leaf.ids.size(); ++i)
	  ids.push_back( addr_leaf.ids[i] );

	std::vector<void*> args = QDP_get_global_cache().get_kernel_args( ids , true );

	evaluate_special_prop_deriv( th_count , Layout::sitesOnNode() , s.start() , args , false , func_num );
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

	  evaluate_special_prop_deriv( th_count , Layout::sitesOnNode(), 0 , args , true , func_num );
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

	  evaluate_special_prop_deriv( th_count , Layout::sitesOnNode(), 0 , args , true , func_num );
	}
      }

  }
  


  
} //namespace
