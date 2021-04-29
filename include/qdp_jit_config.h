#ifndef QDP_JIT_CONFIG_H
#define QDP_JIT_CONFIG_H

namespace QDP
{
  enum JitLayout {lexico,cb2,cb3d,cb32,vnode};
  JitLayout jit_config_get_layout();
  void jit_config_set_layout( JitLayout l );

  std::string jit_config_get_nesting_order();
  std::string jit_config_get_loop_order();
  void jit_config_set_loop_order(std::string s);
  
  int jit_config_gpu_warpsize();

  int jit_config_get_blocksize_x();
  void jit_config_set_blocksize_x(int n);

  int jit_config_get_blocksize_y();
  void jit_config_set_blocksize_y(int n);

  int jit_config_get_oscalar_ringbuffer_size();
  void jit_config_set_oscalar_ringbuffer_size(int n);

  bool jit_config_get_tuning();
  void jit_config_set_tuning(bool v);

  bool jit_config_get_tuning_verbose();
  void jit_config_set_tuning_verbose(bool v);

  void jit_config_delayed_message(std::string txt);
  void jit_config_print_delayed_message();

  std::string jit_config_get_tuning_file();
  void jit_config_set_tuning_file( std::string v);

  void jit_config_set_threads_per_block_min( int t );
  void jit_config_set_threads_per_block_max( int t );
  void jit_config_set_threads_per_block_step( int t );
  void jit_config_set_threads_per_block_loops( int t );
  int jit_config_get_threads_per_block_min();
  int jit_config_get_threads_per_block_max();
  int jit_config_get_threads_per_block_step();
  int jit_config_get_threads_per_block_loops();

  bool jit_config_get_verbose_output();
  void jit_config_set_verbose_output(bool v);

  size_t jit_config_get_pool_alignment();
  void jit_config_set_pool_alignment(size_t size );

#ifdef QDP_BACKEND_ROCM
  int jit_config_get_codegen_opt();
  void jit_config_set_codegen_opt(int opt);
#endif

#ifdef QDP_BACKEND_CUDA
  int  jit_config_get_CUDA_FTZ();
  void jit_config_set_CUDA_FTZ(int i);
#endif

  
  void jit_config_set_pool_size( size_t val );
  void jit_config_set_thread_stack( int stack );
  size_t jit_config_get_pool_size();

  int  jit_config_get_max_allocation();
  void jit_config_set_max_allocation(int size );

  bool qdp_jit_config_defrag();
  void qdp_jit_set_defrag();

  void jit_config_set_threads_per_block( int t );
  int  jit_config_get_threads_per_block();

  size_t qdp_jit_config_pool_size_decrement();
  
  void jit_config_print();

  bool jit_config_pool_stats();
  void jit_set_config_pool_stats();
  
#ifdef QDP_DEEP_LOG
  bool        jit_config_deep_log();
  bool        jit_config_deep_log_create();
  std::string jit_config_deep_log_name();

  void        jit_config_deep_set( std::string name , bool create );
#endif

#if defined(QDP_USE_PROFILING)
  void qdp_jit_CPU_add( const std::string& pretty );
  std::map<std::string,int>& qdp_jit_CPU_getall();
#endif

}


#endif
