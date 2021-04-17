#include "qdp.h"


namespace QDP {

  void
  function_sum_convert_ind_exec( JitFunction& function, 
				 int size, int threads, int blocks, int shared_mem_usage,
				 int in_id, int out_id, int siteTableId )
  {
    // Make sure 'threads' is a power of two (the jit kernel make this assumption)
    if ( (threads & (threads - 1)) != 0 )
      {
	QDPIO::cerr << "internal error: function_sum_convert_ind_exec not power of 2\n";
	QDP_abort(1);
      }

#ifdef QDP_DEEP_LOG
    function.start = 0;
    function.count = blocks;
    function.set_dest_id( out_id );
#endif
    
    int lo = 0;
    int hi = size;
    

    JitParam jit_lo( QDP_get_global_cache().addJitParamInt( lo ) );
    JitParam jit_hi( QDP_get_global_cache().addJitParamInt( hi ) );
  
    std::vector<QDPCache::ArgKey> ids;
    ids.push_back( jit_lo.get_id() );
    ids.push_back( jit_hi.get_id() );
    ids.push_back( siteTableId );
    ids.push_back( in_id );
    ids.push_back( out_id );

    jit_launch_explicit_geom( function , ids , getGeom( hi-lo , threads ) , shared_mem_usage );
  }


  void
  function_summulti_convert_ind_exec( JitFunction& function, 
				      int size, int threads, int blocks, int shared_mem_usage,
				      int in_id, int out_id,
				      int numsubsets,
				      const multi1d<int>& sizes,
				      const multi1d<QDPCache::ArgKey>& table_ids )
  {
    // Make sure 'threads' is a power of two (the jit kernel make this assumption)
    if ( (threads & (threads - 1)) != 0 )
      {
	QDPIO::cerr << "internal error: function_summulti_convert_ind_exec not power of 2\n";
	QDP_abort(1);
      }

#ifdef QDP_DEEP_LOG
    function.start = 0;
    function.count = blocks;
    function.set_dest_id( out_id );
#endif

    int sizes_id = QDP_get_global_cache().add( sizes.size()*sizeof(int) , QDPCache::Flags::OwnHostMemory , QDPCache::Status::host , sizes.slice() , NULL , NULL );

    JitParam jit_numsubsets( QDP_get_global_cache().addJitParamInt( numsubsets ) );
    JitParam jit_tables(     QDP_get_global_cache().addMulti(       table_ids  ) );
						      
    std::vector<QDPCache::ArgKey> ids;
    ids.push_back( jit_numsubsets.get_id() );
    ids.push_back( sizes_id );
    ids.push_back( jit_tables.get_id() );
    ids.push_back( in_id );
    ids.push_back( out_id );

    jit_launch_explicit_geom( function , ids , getGeom( size , threads ) , shared_mem_usage );

    QDP_get_global_cache().signoff(sizes_id);
  }



  void
  function_summulti_exec( JitFunction& function, 
			  int size, int threads, int blocks, int shared_mem_usage,
			  int in_id, int out_id,
			  int numsubsets,
			  const multi1d<int>& sizes )
  {
    // Make sure 'threads' is a power of two (the jit kernel make this assumption)
    if ( (threads & (threads - 1)) != 0 )
      {
	QDPIO::cerr << "internal error: function_summulti_exec not power of 2\n";
	QDP_abort(1);
      }

    int sizes_id = QDP_get_global_cache().add( sizes.size()*sizeof(int) , QDPCache::Flags::OwnHostMemory , QDPCache::Status::host , sizes.slice() , NULL , NULL );

    JitParam jit_numsubsets( QDP_get_global_cache().addJitParamInt( numsubsets ) );
						      
    std::vector<QDPCache::ArgKey> ids;
    ids.push_back( jit_numsubsets.get_id() );
    ids.push_back( sizes_id );
    ids.push_back( in_id );
    ids.push_back( out_id );

    jit_launch_explicit_geom( function , ids , getGeom( size , threads ) , shared_mem_usage );

    QDP_get_global_cache().signoff(sizes_id);
  }



  void
  function_sum_convert_exec( JitFunction& function, 
			     int size, int threads, int blocks, int shared_mem_usage,
			     int in_id, int out_id)
  {
    // Make sure 'threads' is a power of two (the jit kernel make this assumption)
    if ( (threads & (threads - 1)) != 0 )
      {
	QDPIO::cerr << "internal error: function_sum_convert_exec not power of 2\n";
	QDP_abort(1);
      }

#ifdef QDP_DEEP_LOG
    function.start = 0;
    function.count = blocks;
    function.set_dest_id( out_id );
#endif

    int lo = 0;
    int hi = size;

    JitParam jit_lo( QDP_get_global_cache().addJitParamInt( lo ) );
    JitParam jit_hi( QDP_get_global_cache().addJitParamInt( hi ) );
  
    std::vector<QDPCache::ArgKey> ids;
    ids.push_back( jit_lo.get_id() );
    ids.push_back( jit_hi.get_id() );
    ids.push_back( in_id );
    ids.push_back( out_id );

    jit_launch_explicit_geom( function , ids , getGeom( size , threads ) , shared_mem_usage );
  }



  void
  function_sum_exec( JitFunction& function, 
		     int size, int threads, int blocks, int shared_mem_usage,
		     int in_id, int out_id)
  {
    // Make sure 'threads' is a power of two (the jit kernel make this assumption)
    if ( (threads & (threads - 1)) != 0 )
      {
	QDPIO::cerr << "internal error: function_sum_exec not power of 2\n";
	QDP_abort(1);
      }

#ifdef QDP_DEEP_LOG
    function.start = 0;
    function.count = blocks;
    function.set_dest_id( out_id );
#endif

    int lo = 0;
    int hi = size;

    JitParam jit_lo( QDP_get_global_cache().addJitParamInt( lo ) );
    JitParam jit_hi( QDP_get_global_cache().addJitParamInt( hi ) );
  
    std::vector<QDPCache::ArgKey> ids;
    ids.push_back( jit_lo.get_id() );
    ids.push_back( jit_hi.get_id() );
    ids.push_back( in_id );
    ids.push_back( out_id );

    jit_launch_explicit_geom( function , ids , getGeom( size , threads ) , shared_mem_usage );
  }



  void function_global_max_exec( JitFunction& function, 
				 int size, int threads, int blocks, int shared_mem_usage,
				 int in_id, int out_id)
  {
    // Make sure 'threads' is a power of two (the jit kernel make this assumption)
    if ( (threads & (threads - 1)) != 0 )
      {
	QDPIO::cerr << "internal error: function_global_max_exec not power of 2\n";
	QDP_abort(1);
      }

    
#ifdef QDP_DEEP_LOG
    function.start = 0;
    function.count = blocks;
    function.set_dest_id( out_id );
#endif
    
    int lo = 0;
    int hi = size;

    JitParam jit_lo( QDP_get_global_cache().addJitParamInt( lo ) );
    JitParam jit_hi( QDP_get_global_cache().addJitParamInt( hi ) );
  
    std::vector<QDPCache::ArgKey> ids;
    ids.push_back( jit_lo.get_id() );
    ids.push_back( jit_hi.get_id() );
    ids.push_back( in_id );
    ids.push_back( out_id );

    jit_launch_explicit_geom( function , ids , getGeom( size , threads ) , shared_mem_usage );
  }



  
  void
  function_isfinite_convert_exec( JitFunction& function, 
				  int size, int threads, int blocks, int shared_mem_usage,
				  int in_id, int out_id)
  {
    // Make sure 'threads' is a power of two (the jit kernel make this assumption)
    if ( (threads & (threads - 1)) != 0 )
      {
	QDPIO::cerr << "internal error: function_isfinite_convert_exec not power of 2\n";
	QDP_abort(1);
      }

#ifdef QDP_DEEP_LOG
    function.start = 0;
    function.count = blocks;
    function.set_dest_id( out_id );
#endif
    
    int lo = 0;
    int hi = size;

    JitParam jit_lo( QDP_get_global_cache().addJitParamInt( lo ) );
    JitParam jit_hi( QDP_get_global_cache().addJitParamInt( hi ) );
  
    std::vector<QDPCache::ArgKey> ids;
    ids.push_back( jit_lo.get_id() );
    ids.push_back( jit_hi.get_id() );
    ids.push_back( in_id );
    ids.push_back( out_id );
 
    jit_launch_explicit_geom( function , ids , getGeom( size , threads ) , shared_mem_usage );
  }



  void
  function_bool_reduction_exec( JitFunction& function, 
				int size, int threads, int blocks, int shared_mem_usage,
				int in_id, int out_id)
  {
    // Make sure 'threads' is a power of two (the jit kernel make this assumption)
    if ( (threads & (threads - 1)) != 0 )
      {
	QDPIO::cerr << "internal error: function_bool_reduction_exec not power of 2\n";
	QDP_abort(1);
      }

#ifdef QDP_DEEP_LOG
    function.start = 0;
    function.count = blocks;
    function.set_dest_id( out_id );
#endif

    int lo = 0;
    int hi = size;

    JitParam jit_lo( QDP_get_global_cache().addJitParamInt( lo ) );
    JitParam jit_hi( QDP_get_global_cache().addJitParamInt( hi ) );
  
    std::vector<QDPCache::ArgKey> ids;
    ids.push_back( jit_lo.get_id() );
    ids.push_back( jit_hi.get_id() );
    ids.push_back( in_id );
    ids.push_back( out_id );

    jit_launch_explicit_geom( function , ids , getGeom( size , threads ) , shared_mem_usage );
  }

  
}

