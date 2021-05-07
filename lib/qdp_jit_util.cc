#include "qdp.h"

namespace QDP {


  llvm::Function    *func_seed2float;
  llvm::Function    *func_seedMultiply;

  namespace JITSTATS {
    long lattice2dev  = 0;   // changing lattice data layout to device format
    long lattice2host = 0;   // changing lattice data layout to host format
    long jitted       = 0;   // functions not in DB, thus jit-built
#ifdef QDP_CUDA_SPECIAL
    std::map<int,long> special;
    std::map<int,std::string> special_names = {
      {0,"quarkContract13"},
      {1,"quarkContract14"},
      {2,"quarkContract23"},
      {3,"quarkContract24"}
    };
#endif
  }

  namespace {
    void *sync_hst_ptr;
    void *sync_dev_ptr;
  }

  namespace
  {
    typedef std::map< std::string , int > DBTuneType;
    DBTuneType db_tune;
    bool db_tune_modified = false;
    int db_tune_count = 0;
  }

  namespace
  {
    std::vector<int> ringBufferOScalar;
    int ringBufferNext;
  }


  void jit_util_ringBuffer_init()
  {
    ringBufferOScalar.resize( jit_config_get_oscalar_ringbuffer_size() , -1 );
    ringBufferNext = 0;
  }
  

  int jit_util_ringBuffer_allocate( size_t size , const void *hstPtr )
  {
    int& buf_elem = ringBufferOScalar.at( ringBufferNext );

    if ( buf_elem >= 0)
      {
	//QDPIO::cout << "ringBuffer: sign off " << buf_elem << std::endl;
      	QDP_get_global_cache().signoff( buf_elem );
      }
    
    buf_elem = QDP_get_global_cache().registrateOwnHostMemNoPage( size, hstPtr );
    //QDPIO::cout << "ringBuffer: allocated " << buf_elem << std::endl;

    ringBufferNext = ( ringBufferNext + 1 ) % ringBufferOScalar.size();
    //QDPIO::cout << "ringBufferNext = " << ringBufferNext << std::endl;

    return buf_elem;
  }
  

  int jit_util_get_tune_count()
  {
    return db_tune_count;
  }
  
  void db_tune_write( std::string filename )
  {
    if (!db_tune_modified)
      return;
    
    BinaryFileWriter db(filename);

    write( db , (int)db_tune.size() );
    for ( DBTuneType::iterator it = db_tune.begin() ; it != db_tune.end() ; ++it ) 
      {
	write(db , it->first);
	write(db , it->second);
      }
    db.close();
    QDPIO::cout << "  tuning file updated" << std::endl;
  }

  
  void db_tune_read( std::string filename )
  {
    QDPIO::cout << "Tuning DB" << std::endl;
    QDPIO::cout << "  checking file                       : " << filename << std::endl;
    {
      ifstream f(filename.c_str());
      if (! f.good() )
	{
	  QDPIO::cout << "  creating new tuning file" << std::endl;
	  return;
	}
    }
    
    QDPIO::cout << "  opening file" << std::endl;

    BinaryFileReader db(filename);

    int n;
    read( db , n );
    
    QDPIO::cout << "  number of tuning records            : " << n << std::endl;

    for( int i = 0 ; i < n ; ++i )
      {
	std::string key;
	int config;

	read( db , key , 10*1024 );
	read( db , config );

	db_tune[ key ] = config;

	//QDPIO::cout << "read: " << config << "\t" << key << std::endl;
      }
    db.close();

    QDPIO::cout << "  done reading tuning file" << std::endl;
  }

  
  bool db_tune_find_info( JitFunction& function )
  {
    std::string key = jit_util_get_static_dynamic_string( function.get_pretty() );
    
    DBTuneType::iterator it = db_tune.find( key );

    if ( it != db_tune.end() )
      {
	function.set_threads_per_block( it->second );
	return true;
      }

    return false;
  }

  
  void db_tune_insert_info( JitFunction& function , int config )
  {
    std::string key = jit_util_get_static_dynamic_string( function.get_pretty() );
    db_tune[key] = config;
    db_tune_modified = true;
    db_tune_count++;
  }

  


  
  void jit_util_sync_init()
  {
    gpu_host_alloc( &sync_hst_ptr , sizeof(int) );
    gpu_malloc( &sync_dev_ptr , sizeof(int) );
    *(int*)sync_hst_ptr = 0;
  }

  void jit_util_sync_done()
  {
    gpu_host_free( sync_hst_ptr );
    gpu_free( sync_dev_ptr );
  }

  void jit_util_sync_copy()
  {
    gpu_memcpy_d2h( sync_hst_ptr , sync_dev_ptr , sizeof(int) );
  }

  // For AMD the workgroup sizes are passed as kernel parameters
  // For now as placeholder values
  //
  void JIT_AMD_add_workgroup_sizes( std::vector<QDPCache::ArgKey>& ids )
  {
    JitParam jit_ntidx( QDP_get_global_cache().addJitParamInt( -1 ) );
    JitParam jit_nctaidx( QDP_get_global_cache().addJitParamInt( -1 ) );

    ids.insert(ids.begin(), jit_nctaidx.get_id() );
    ids.insert(ids.begin(), jit_ntidx.get_id() );
  }



  void jit_stats_lattice2dev()  { ++JITSTATS::lattice2dev; }
  void jit_stats_lattice2host() { ++JITSTATS::lattice2host; }
  void jit_stats_jitted()       { ++JITSTATS::jitted; }
#ifdef QDP_CUDA_SPECIAL
  void jit_stats_special(int i) { ++JITSTATS::special[i]; }
#endif
  
  long get_jit_stats_lattice2dev()  { return JITSTATS::lattice2dev; }
  long get_jit_stats_lattice2host() { return JITSTATS::lattice2host; }
  long get_jit_stats_jitted()       { return JITSTATS::jitted; }
#ifdef QDP_CUDA_SPECIAL
  long get_jit_stats_special(int i) { return JITSTATS::special[i]; }
  std::map<int,std::string>& get_jit_stats_special_names() { return JITSTATS::special_names; }
#endif

  // seedMultiply
  //
  // We build a function that takes 2 seeds (4 ints each)
  // and returns 1 seed (as a literal aggregate)
  //
  void jit_build_seedMultiply() {
    std::vector< llvm::Type* > vecArgTypes;

    vecArgTypes.push_back( llvm_get_builder()->getInt32Ty() );
    vecArgTypes.push_back( llvm_get_builder()->getInt32Ty() );
    vecArgTypes.push_back( llvm_get_builder()->getInt32Ty() );
    vecArgTypes.push_back( llvm_get_builder()->getInt32Ty() );

    vecArgTypes.push_back( llvm_get_builder()->getInt32Ty() );
    vecArgTypes.push_back( llvm_get_builder()->getInt32Ty() );
    vecArgTypes.push_back( llvm_get_builder()->getInt32Ty() );
    vecArgTypes.push_back( llvm_get_builder()->getInt32Ty() );

    llvm::Type* ret_types[] = { llvm_get_builder()->getInt32Ty(),
				llvm_get_builder()->getInt32Ty(),
				llvm_get_builder()->getInt32Ty(),
				llvm_get_builder()->getInt32Ty() };
    
    llvm::StructType* ret_type = llvm::StructType::get(llvm_get_context(), 
						       llvm::ArrayRef< llvm::Type * >( ret_types , 4 ) );

    llvm::FunctionType *funcType = llvm::FunctionType::get( ret_type , 
							    llvm::ArrayRef<llvm::Type*>( vecArgTypes.data() , 
											 vecArgTypes.size() ) ,
							    false );
    llvm::Function* F = llvm::Function::Create(funcType, llvm::Function::InternalLinkage, "seedMultiply", llvm_get_module());

    std::vector< llvm::Value* > args;
    unsigned Idx = 0;
    for (llvm::Function::arg_iterator AI = F->arg_begin(), AE = F->arg_end() ; AI != AE ; ++AI, ++Idx) {
      std::ostringstream oss;
      oss << "arg" << Idx;
      AI->setName( oss.str() );
      args.push_back(&*AI);
    }

    llvm::BasicBlock* entry = llvm::BasicBlock::Create(llvm_get_context(), "entrypoint", F);
    llvm_get_builder()->SetInsertPoint(entry);

    typedef RScalar<WordREG<int> >  T;
    PSeedREG<T> s1,s2;

    s1.elem(0).elem().setup( args[0] );
    s1.elem(1).elem().setup( args[1] );
    s1.elem(2).elem().setup( args[2] );
    s1.elem(3).elem().setup( args[3] );

    s2.elem(0).elem().setup( args[4] );
    s2.elem(1).elem().setup( args[5] );
    s2.elem(2).elem().setup( args[6] );
    s2.elem(3).elem().setup( args[7] );

    s1 = s1 * s2;

    llvm::Value* ret_val[] = { s1.elem(0).elem().get_val() ,
			       s1.elem(1).elem().get_val() ,
			       s1.elem(2).elem().get_val() ,
			       s1.elem(3).elem().get_val() };

    llvm_get_builder()->CreateAggregateRet( ret_val , 4 );

    func_seedMultiply = F;
  }


  std::vector<llvm::Value *> llvm_seedMultiply( llvm::Value* a0 , llvm::Value* a1 , llvm::Value* a2 , llvm::Value* a3 , 
						llvm::Value* a4 , llvm::Value* a5 , llvm::Value* a6 , llvm::Value* a7 ) {
    assert(a0 && "llvm_seedToFloat a0");
    assert(a1 && "llvm_seedToFloat a1");
    assert(a2 && "llvm_seedToFloat a2");
    assert(a3 && "llvm_seedToFloat a3");
    assert(a4 && "llvm_seedToFloat a4");
    assert(a5 && "llvm_seedToFloat a5");
    assert(a6 && "llvm_seedToFloat a6");
    assert(a7 && "llvm_seedToFloat a7");

    assert(func_seedMultiply && "llvm_seedMultiply func_seedMultiply");

    llvm::Value* pack[] = { a0,a1,a2,a3,a4,a5,a6,a7 };

    llvm::Value* ret_val = llvm_get_builder()->CreateCall( func_seedMultiply , llvm::ArrayRef< llvm::Value *>( pack ,  8 ) );

    std::vector<llvm::Value *> ret;
    ret.push_back( llvm_get_builder()->CreateExtractValue( ret_val , 0 ) );
    ret.push_back( llvm_get_builder()->CreateExtractValue( ret_val , 1 ) );
    ret.push_back( llvm_get_builder()->CreateExtractValue( ret_val , 2 ) );
    ret.push_back( llvm_get_builder()->CreateExtractValue( ret_val , 3 ) );

    return ret;
  }



  void jit_build_seedToFloat() {

    std::vector< llvm::Type* > vecArgTypes;
    vecArgTypes.push_back( llvm_get_builder()->getInt32Ty() );
    vecArgTypes.push_back( llvm_get_builder()->getInt32Ty() );
    vecArgTypes.push_back( llvm_get_builder()->getInt32Ty() );
    vecArgTypes.push_back( llvm_get_builder()->getInt32Ty() );

    llvm::FunctionType *funcType = llvm::FunctionType::get( llvm_get_builder()->getFloatTy(), 
							    llvm::ArrayRef<llvm::Type*>( vecArgTypes.data() , 
											 vecArgTypes.size() ) ,
							    false );
    llvm::Function* F = llvm::Function::Create(funcType, llvm::Function::InternalLinkage, "seedToFloat", llvm_get_module());

    std::vector< llvm::Value* > args;
    unsigned Idx = 0;
    for (llvm::Function::arg_iterator AI = F->arg_begin(), AE = F->arg_end() ; AI != AE ; ++AI, ++Idx) {
      std::ostringstream oss;
      oss << "arg" << Idx;
      AI->setName( oss.str() );
      args.push_back(&*AI);
    }

    llvm::BasicBlock* entry = llvm::BasicBlock::Create(llvm_get_context(), "entrypoint", F);
    llvm_get_builder()->SetInsertPoint(entry);

    typedef RScalar<WordREG<int> >  T;
    PSeedREG<T> s1;
    s1.elem(0).elem().setup( args[0] );
    s1.elem(1).elem().setup( args[1] );
    s1.elem(2).elem().setup( args[2] );
    s1.elem(3).elem().setup( args[3] );

     UnaryReturn<PSeedREG<T>, FnSeedToFloat>::Type_t  d; // QDP::PScalarREG<QDP::RScalarREG<QDP::WordREG<float> > >
    typedef  RealScalar<T>::Type_t  S;                                   // QDP::RScalarREG<QDP::WordREG<float> >

    S  twom11(1.0 / 2048.0);
    S  twom12(1.0 / 4096.0);
    S  fs1, fs2;

    //  recast_rep(fs1, s1.elem(0));
    fs1 = S(s1.elem(0));
    d.elem() = twom12 * S(s1.elem(0));

    //  recast_rep(fs1, s1.elem(1));
    fs1 = S(s1.elem(1));
    fs2 = fs1 + d.elem();
    d.elem() = twom12 * fs2;

    //  recast_rep(fs1, s1.elem(2));
    fs1 = S(s1.elem(2));
    fs2 = fs1 + d.elem();
    d.elem() = twom12 * fs2;

    //  recast_rep(fs1, s1.elem(3));
    fs1 = S(s1.elem(3));
    fs2 = fs1 + d.elem();
    d.elem() = twom11 * fs2;

    llvm_get_builder()->CreateRet( d.elem().elem().get_val() );

    func_seed2float = F;
  }


  llvm::Value * llvm_seedToFloat( llvm::Value* a0 , llvm::Value* a1 , llvm::Value* a2 , llvm::Value* a3 ) {
    assert(a0 && "llvm_seedToFloat a0");
    assert(a1 && "llvm_seedToFloat a1");
    assert(a2 && "llvm_seedToFloat a2");
    assert(a3 && "llvm_seedToFloat a3");
    assert(func_seed2float && "llvm_seedToFloat func_seed2float");
    return llvm_get_builder()->CreateCall( func_seed2float , {a0,a1,a2,a3} );
  }


  void jit_get_function(JitFunction& f)
  {
    llvm_exit();

    llvm_build_function(f);
  }



  

  std::vector<ParamRef> jit_function_preamble_param( const char* ftype , const char* pretty )
  {
    llvm_start_new_function( ftype , pretty );

    ParamRef p_ordered      = llvm_add_param<bool>();
    ParamRef p_th_count     = llvm_add_param<int>();
    ParamRef p_start        = llvm_add_param<int>();
    ParamRef p_end          = llvm_add_param<int>();
    ParamRef p_member_array = llvm_add_param<bool*>();

    return { p_ordered , p_th_count , p_start , p_end , p_member_array };
  }


  llvm::Value *jit_function_preamble_get_idx( const std::vector<ParamRef>& vec )
  {
    llvm::Value * r_ordered      = llvm_derefParam( vec[0] );
    llvm::Value * r_th_count     = llvm_derefParam( vec[1] );
    llvm::Value * r_start        = llvm_derefParam( vec[2] );
                                   llvm_derefParam( vec[3]);     // r_end not used
    ParamRef      p_member_array = vec[4];

    llvm::Value * r_idx_phi0 = llvm_thread_idx();

    llvm::Value * r_idx_phi1;

    llvm_cond_exit( llvm_ge( r_idx_phi0 , r_th_count ) );

    llvm::BasicBlock * block_ordered = llvm_new_basic_block();
    llvm::BasicBlock * block_not_ordered = llvm_new_basic_block();
    llvm::BasicBlock * block_ordered_exit = llvm_new_basic_block();
    llvm::BasicBlock * cond_exit;
    llvm_cond_branch( r_ordered , block_ordered , block_not_ordered );
    {
      llvm_set_insert_point(block_not_ordered);
      llvm::Value* r_ismember     = llvm_array_type_indirection( p_member_array , r_idx_phi0 );
      llvm::Value* r_ismember_not = llvm_not( r_ismember );
      cond_exit = llvm_cond_exit( r_ismember_not ); 
      llvm_branch( block_ordered_exit );
    }
    {
      llvm_set_insert_point(block_ordered);
      r_idx_phi1 = llvm_add( r_idx_phi0 , r_start );
      llvm_branch( block_ordered_exit );
    }
    llvm_set_insert_point(block_ordered_exit);

    llvm::PHINode* r_idx = llvm_phi( r_idx_phi0->getType() , 2 );

    r_idx->addIncoming( r_idx_phi0 , cond_exit );
    r_idx->addIncoming( r_idx_phi1 , block_ordered );

    return r_idx;
  }

#if 0
  JitForLoop::JitForLoop( int start , int end )
  {
    JitForLoop( llvm_create_value(start) , llvm_create_value(end) );
  }

  JitForLoop::JitForLoop( int start , llvm::Value* end )
  {
    JitForLoop( llvm_create_value(start) , end );
  }

  JitForLoop::JitForLoop( llvm::Value* start , int end )
  {
    JitForLoop( start , llvm_create_value(end) );
  }
#endif

  JitForLoop::JitForLoop( llvm::Value* start , llvm::Value* end )
  {
    block_outer = llvm_get_insert_point();
    block_loop_cond = llvm_new_basic_block();
    block_loop_body = llvm_new_basic_block();
    block_loop_exit = llvm_new_basic_block();

    llvm_branch( block_loop_cond );
    llvm_set_insert_point(block_loop_cond);
  
    r_i = llvm_phi( llvm_get_type<int>() , 2 );

    r_i->addIncoming( start , block_outer );

    llvm_cond_branch( llvm_lt( r_i , end ) , block_loop_body , block_loop_exit );

    llvm_set_insert_point( block_loop_body );
  }
  llvm::Value * JitForLoop::index()
  {
    return r_i;
  }
  void JitForLoop::end()
  {
    llvm::Value * r_i_plus = llvm_add( r_i , llvm_create_value(1) );
    r_i->addIncoming( r_i_plus , llvm_get_insert_point() );
  
    llvm_branch( block_loop_cond );

    llvm_set_insert_point(block_loop_exit);
  }





 
  JitForLoopPower::JitForLoopPower( llvm::Value* i_start  )
  {
    block_outer = llvm_get_insert_point();
    block_loop_cond = llvm_new_basic_block();
    block_loop_body = llvm_new_basic_block();
    block_loop_exit = llvm_new_basic_block();

    llvm_branch( block_loop_cond );
    llvm_set_insert_point(block_loop_cond);
  
    r_i = llvm_phi( llvm_get_type<int>() , 2 );

    r_i->addIncoming( i_start , block_outer );

    llvm_cond_branch( llvm_gt( r_i , llvm_create_value( 0 ) ) , block_loop_body , block_loop_exit );

    llvm_set_insert_point( block_loop_body );
  }
  llvm::Value * JitForLoopPower::index()
  {
    return r_i;
  }
  void JitForLoopPower::end()
  {
    llvm::Value * r_i_plus = llvm_shr( r_i , llvm_create_value(1) );
    r_i->addIncoming( r_i_plus , llvm_get_insert_point() );
  
    llvm_branch( block_loop_cond );

    llvm_set_insert_point(block_loop_exit);
  }
 



  

  llvm::Value* llvm_epsilon_1st( int p1 , llvm::Value* j )
  {
    return llvm_rem( llvm_add( j , llvm_create_value( p1 ) ) , llvm_create_value( 3 ) );

  }
  
  llvm::Value* llvm_epsilon_2nd( int p2 , llvm::Value* i )
  {
    return llvm_rem( llvm_add( i , llvm_create_value( p2 ) ) , llvm_create_value( 3 ) );
  }


  void f1(int l,int r)
  {
    int i = (r + 1) % 3;
    int j = (l + 1) % 3;
    cout << "s1.elem(" << i << "," << j << ") * ";
  }



#if 0
  llvm::Value* jit_ternary( llvm::Value* cond , llvm::Value* val_true , llvm::Value* val_false )
  {
    llvm::BasicBlock * block_exit  = llvm_new_basic_block();
    llvm::BasicBlock * block_true  = llvm_new_basic_block();
    llvm::BasicBlock * block_false = llvm_new_basic_block();

    llvm_cond_branch( cond , block_true , block_false );
    {
      llvm_set_insert_point(block_true);
      llvm_branch( block_exit );
    }
    {
      llvm_set_insert_point(block_false);
      llvm_branch( block_exit );
    }
    llvm_set_insert_point(block_exit);

    llvm::PHINode* r = llvm_phi( val_true->getType() , 2 );
    
    r->addIncoming( val_true , block_true );
    r->addIncoming( val_false , block_false );

    return r;
  }
#endif


  
  llvm::Value* jit_ternary( llvm::Value* cond , const JitDefer& def_true , const JitDefer& def_false )
  {
    llvm::BasicBlock * block_exit  = llvm_new_basic_block();
    llvm::BasicBlock * block_true  = llvm_new_basic_block();
    llvm::BasicBlock * block_false = llvm_new_basic_block();

    llvm::Value* r_true;
    llvm::Value* r_false;
    
    llvm_cond_branch( cond , block_true , block_false );
    {
      llvm_set_insert_point(block_true);
      r_true = def_true.val();
      llvm_branch( block_exit );
    }
    {
      llvm_set_insert_point(block_false);
      r_false = def_false.val();
      llvm_branch( block_exit );
    }
    llvm_set_insert_point(block_exit);

    llvm::PHINode* r = llvm_phi( r_true->getType() , 2 );
    
    r->addIncoming( r_true , block_true );
    r->addIncoming( r_false , block_false );

    return r;
  }

  
  llvm::Value* jit_ternary( llvm::Value* cond , llvm::Value*    val_true , llvm::Value*    val_false )
  {
    return jit_ternary( cond , JitDeferValue(val_true) , JitDeferValue(val_false) );
  }

  
  llvm::Value* jit_ternary( llvm::Value* cond , const JitDefer& val_true , llvm::Value*    val_false )
  {
    return jit_ternary( cond , val_true , JitDeferValue(val_false) );
  }

  
  llvm::Value* jit_ternary( llvm::Value* cond , llvm::Value*    val_true , const JitDefer& val_false )
  {
    return jit_ternary( cond , JitDeferValue(val_true) , val_false );
  }

  
  
  


  void jit_tune( JitFunction& function , int th_count , QDPCache::KernelArgs_t& args)
  {
    if ( ! function.get_enable_tuning() )
      {
	QDPIO::cout << "Tuning disabled for: " << function.get_kernel_name() << std::endl;
	function.set_threads_per_block( jit_config_get_threads_per_block() );
	return;
      }


    if (db_tune_find_info( function ))
      {
	return;
      }
    

    size_t field_size      = QDP_get_global_cache().getSize       ( function.get_dest_id() );

    std::vector<QDPCache::ArgKey> vec_id;
    vec_id.push_back( function.get_dest_id() );
    std::vector<void*> vec_ptrs = QDP_get_global_cache().get_dev_ptrs( vec_id );
    void* dev_ptr = vec_ptrs.at(0);
  
    void* host_ptr;

    if ( ! (host_ptr = malloc( field_size )) )
      {
	QDPIO::cout << "Tuning: Cannot allocate host memory!" << endl;
	QDP_abort(1);
      }
    
    if (jit_config_get_tuning_verbose())
      {
	QDPIO::cout << "Starting tuning of: " << function.get_kernel_name() << std::endl;
	QDPIO::cout << function.get_pretty() << std::endl;
      }


    //QDPIO::cout << "d2h: start = " << f.start << "  count = " << f.count << "  size_T = " << f.size_T << "   \t";
    
    gpu_memcpy_d2h( host_ptr , dev_ptr , field_size );

    // --------------------
    // Main tuning loop

    int config = -1;
    double best_time;


    for( int threads_per_block = jit_config_get_threads_per_block_min() ;
	 threads_per_block <= jit_config_get_threads_per_block_max() ;
	 threads_per_block += jit_config_get_threads_per_block_step() )
      {
	kernel_geom_t geom = getGeom( th_count , threads_per_block );

	StopWatch w;
	int loops = jit_config_get_threads_per_block_loops();

	
	for( int i = 0 ; i < loops+5 ; ++i )
	  {
	    if (i==5)
	      gpu_record_start();

	    JitResult result = gpu_launch_kernel( function,
						  geom.Nblock_x,geom.Nblock_y,1,
						  geom.threads_per_block,1,1,
						  0, // shared mem
						  args );
	  
	    if (result != JitResult::JitSuccess) {
	      QDPIO::cerr << "Tuning: jit launch error, grid=(" << geom.Nblock_x << "," << geom.Nblock_y << "1), block=(" << threads_per_block << ",1,1)\n";
	      QDP_abort(1);
	    }

	  }
	
	//w.stop();
	gpu_record_stop();
	gpu_event_sync();
	float ms = gpu_get_time();

	//double ms = w.getTimeInMicroseconds();

	if (jit_config_get_tuning_verbose())
	  {
	    QDPIO::cout << "blocksize\t" << threads_per_block << "\ttime = \t" << ms << std::endl;
	  }
	
	if (config == -1 || best_time > ms)
	  {
	    best_time = ms;

	    config = threads_per_block;
	  }
      }

    if (jit_config_get_tuning_verbose())
      {
	QDPIO::cout << "best   \t" << config << "\ttime = \t" << best_time << std::endl;
      }
    
    function.set_threads_per_block( config );

    // DB tune
    db_tune_insert_info( function , config );

    // -------------------

    // Restore memory for 'dest'
    gpu_memcpy_h2d( dev_ptr , host_ptr , field_size );
    
    free( host_ptr );
  }
  


  void jit_launch(JitFunction& function,int th_count,std::vector<QDPCache::ArgKey>& ids)
  {
#ifndef QDP_BACKEND_SPIRV
    // For ROCm we add the __threads_per_group and 
    // __grid_size_x as parameters to the kernel.
#ifdef QDP_BACKEND_ROCM
    JIT_AMD_add_workgroup_sizes( ids );
#endif
    
    QDPCache::KernelArgs_t args( QDP_get_global_cache().get_kernel_args(ids) );

    // Check for no-op
    if ( th_count == 0 )
      return;

    // Increment the call counter
    function.inc_call_counter();


    if (  jit_config_get_tuning()  &&  function.get_threads_per_block() == -1  )
      {
	jit_tune( function , th_count , args );
      }

    
    int threads_per_block;
    
    if ( function.get_threads_per_block() == -1 )
      {
	threads_per_block = jit_config_get_threads_per_block();
      }
    else
      {
	threads_per_block = function.get_threads_per_block();
      }

    //QDPIO::cout << "jit_launch using block size = " << threads_per_block << std::endl;
    

    kernel_geom_t geom = getGeom( th_count , threads_per_block );

    JitResult result = gpu_launch_kernel( function,
					  geom.Nblock_x,geom.Nblock_y,1,
					  geom.threads_per_block,1,1,
					  0, // shared mem
					  args );

    if (result != JitResult::JitSuccess) {
      QDPIO::cerr << "jit launch error, grid=(" << geom.Nblock_x << "," << geom.Nblock_y << "1), block=(" << threads_per_block << ",1,1)\n";
      QDP_abort(1);
    }
#endif
  }


  void jit_launch_explicit_geom( JitFunction& function , std::vector<QDPCache::ArgKey>& ids , const kernel_geom_t& geom , unsigned int shared )
  {
    // For ROCm we add the __threads_per_group and 
    // __grid_size_x as parameters to the kernel.
#ifdef QDP_BACKEND_ROCM
    JIT_AMD_add_workgroup_sizes( ids );
#endif

    QDPCache::KernelArgs_t args( QDP_get_global_cache().get_kernel_args(ids) );

    // Increment the call counter
    function.inc_call_counter();

    JitResult result = gpu_launch_kernel( function,
					  geom.Nblock_x,geom.Nblock_y,1,
					  geom.threads_per_block,1,1,
					  shared,
					  args );

    if (result != JitResult::JitSuccess) {
      QDPIO::cerr << "jit launch explicit geom error, grid=(" << geom.Nblock_x << "," << geom.Nblock_y << "1), block=(" << geom.threads_per_block << ",1,1)\n";
      QDP_abort(1);
    }
  }



  std::string jit_util_get_static_dynamic_string( const std::string& pretty )
  {
    std::ostringstream oss;
    
    oss << gpu_get_arch() << "_";

    for ( int i = 0 ; i < Nd ; ++i )
      oss << Layout::subgridLattSize()[i] << "_";

    oss << pretty;

    return oss.str();
  }

  
  


  

} //namespace
