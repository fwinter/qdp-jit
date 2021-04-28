#include "qdp.h"


namespace QDP
{
  namespace
  {
    // Datalayout
    JitLayout jit_layout = JitLayout::cb2;
    int       gpu_warpsize = 32;
    
    // Memory Pool
    size_t thread_stack = 512 * sizeof(REAL);
    bool use_total_pool_size = false;
    size_t pool_size = 0;
    bool use_defrag = false;
    int max_allocation_size = -1;
    size_t pool_alignment = 128;
    size_t min_total_reserved_GPU_memory = 50*1024*1024; // 50 MB


    // In case memory allocation fails, decrease Pool size by this amount for next try.
    size_t pool_size_decrement = 10 * 1024*1024;   // 10MB

    bool pool_count_allocations = false;

    bool verbose_output = false;

    
    // Kernel Launch & Tuning
    //
    bool tuning = false;
    bool tuning_verbose = false;
    int threads_per_block_min = 8;
    int threads_per_block_max = 256;
    int threads_per_block_step = 8;
    int threads_per_block_loops = 1000; // Number of loops to measure (after dry run of 5)
    std::string tuning_file = "qdp-jit.tuning.dat";

    // Datalayout
    //
    int blocksize_x = 128;
    int blocksize_y = 1;

    
    // Delay output when QDPIO is not ready yet
    std::vector<std::string> delayed_output;

    // Ring buffer size for OScalars
    int oscalar_ringbuffer_size = 100;
    
#ifdef QDP_BACKEND_ROCM
    int  codegen_opt = 1;
#endif

#ifdef QDP_BACKEND_CUDA
    int CUDA_FTZ = 0;
#endif
    
#ifdef QDP_DEEP_LOG
    bool deep_log = false;
    bool deep_log_create = false;
    std::string deep_log_name = "qdp-jit-log.dat";
#endif
  }


  int jit_config_gpu_warpsize()
  {
    return gpu_warpsize;
  }

  
  JitLayout jit_config_get_layout()
  {
    return jit_layout;
  }

  void jit_config_set_layout( JitLayout l )
  {
    jit_layout = l;
  }
  
  

#ifdef QDP_BACKEND_CUDA
  int  jit_config_get_CUDA_FTZ()   { return CUDA_FTZ; }
  void jit_config_set_CUDA_FTZ(int i)   { CUDA_FTZ = i; }
#endif
  

  void jit_config_delayed_message(std::string txt)
  {
    delayed_output.push_back(txt);
  }

  void jit_config_print_delayed_message()
  {
    for ( auto i : delayed_output )
      {
	QDPIO::cout << i << std::endl;
      }
  }
  
  void jit_config_print()
  {
    QDPIO::cout << "Kernel launch config:\n";
    QDPIO::cout << "  blocksize_x                         : " << blocksize_x       << "\n";
    QDPIO::cout << "  blocksize_y                         : " << blocksize_y       << "\n";
    QDPIO::cout << "Memory pool config:\n";
    if (use_total_pool_size)
    QDPIO::cout << "  memory pool size (user request)     : " << pool_size/1024/1024 << " MB\n";
    else
      {
    QDPIO::cout << "  reserved memory per thread          : " << thread_stack << " bytes\n";
    QDPIO::cout << "  resulting memory pool size          : " << jit_config_get_pool_size()/1024/1024 << " MB\n";
      }
#ifdef QDP_BACKEND_CUDA
    QDPIO::cout << "Code generation:\n";
    QDPIO::cout << "  CUDA flush denormals to zero        : " << jit_config_get_CUDA_FTZ() << std::endl;
#endif      
  }

  
  std::string jit_config_get_tuning_file() { return tuning_file; }
  void jit_config_set_tuning_file( std::string v) { tuning_file = v; }
  
  bool jit_config_get_tuning() { return tuning; }
  void jit_config_set_tuning(bool v) { tuning = v; }

  bool jit_config_get_tuning_verbose() { return tuning_verbose; }
  void jit_config_set_tuning_verbose(bool v) { tuning_verbose = v; }

  bool jit_config_get_verbose_output() { return verbose_output; }
  void jit_config_set_verbose_output(bool v) { verbose_output = v; }

  int jit_config_get_oscalar_ringbuffer_size() { return oscalar_ringbuffer_size; }
  void jit_config_set_oscalar_ringbuffer_size(int n) { oscalar_ringbuffer_size = n; }


  int jit_config_get_blocksize_x() { return blocksize_x; }
  void jit_config_set_blocksize_x(int n) { blocksize_x = n; }

  int jit_config_get_blocksize_y() { return blocksize_y; }
  void jit_config_set_blocksize_y(int n) { blocksize_y = n; }

  
#ifdef QDP_BACKEND_ROCM
  int jit_config_get_codegen_opt() { return codegen_opt; }
  void jit_config_set_codegen_opt(int opt) { codegen_opt = opt; }
#endif
  
  size_t jit_config_get_pool_alignment() { return pool_alignment; }
  void jit_config_set_pool_alignment(size_t size ) { pool_alignment = size; }

  
  int jit_config_get_max_allocation() { return max_allocation_size; }
  void jit_config_set_max_allocation(int size ) { max_allocation_size = size; }
  
  bool jit_config_pool_stats() { return pool_count_allocations; }
  void jit_set_config_pool_stats() { pool_count_allocations = true; }


#ifdef QDP_DEEP_LOG
  bool        jit_config_deep_log() { return deep_log; }
  bool        jit_config_deep_log_create() { return deep_log_create; }
  std::string jit_config_deep_log_name() { return deep_log_name; }
  
  void        jit_config_deep_set( std::string name , bool create )
  {
    deep_log = true;
    deep_log_create = create;
    deep_log_name = name;
  }
#endif

  
  bool qdp_jit_config_defrag()
  {
    return use_defrag;
  }

  void qdp_jit_set_defrag()
  {
    use_defrag = true;
  }

  

  size_t qdp_jit_config_pool_size_decrement()
  {
    return pool_size_decrement;
  }
  
  


  void jit_config_set_threads_per_block_min( int t )  {    threads_per_block_min = t;  }
  void jit_config_set_threads_per_block_max( int t )  {    threads_per_block_max = t;  }
  void jit_config_set_threads_per_block_step( int t )  {   threads_per_block_step = t;  }
  void jit_config_set_threads_per_block_loops( int t )  {   threads_per_block_loops = t;  }
  int jit_config_get_threads_per_block_min()  {    return threads_per_block_min;  }
  int jit_config_get_threads_per_block_max()  {    return threads_per_block_max;  }
  int jit_config_get_threads_per_block_step()  {   return threads_per_block_step; }
  int jit_config_get_threads_per_block_loops()  {  return threads_per_block_loops; }

  


  void jit_config_set_pool_size( size_t val )
  {
    use_total_pool_size = true;
    pool_size = val;
  }
  
  void jit_config_set_thread_stack( int stack )
  {
    thread_stack = stack;
  }

  size_t jit_config_get_pool_size()
  {
    if (use_total_pool_size)
      {
	return pool_size;
      }
    else
      {
	size_t tmp = gpu_mem_free() - (size_t)Layout::sitesOnNode() * thread_stack;

	if ( (size_t)Layout::sitesOnNode() * thread_stack < min_total_reserved_GPU_memory )
	  {
	    tmp = gpu_mem_free() - min_total_reserved_GPU_memory;
	  }
	  
	return tmp;
      }
  }


#if defined(QDP_USE_PROFILING)
  namespace
  {
    std::map<std::string,int> _map_cpu;
  }

  void qdp_jit_CPU_add( const std::string& pretty )
  {
    _map_cpu[ pretty ]++;
  }
  
  std::map<std::string,int>& qdp_jit_CPU_getall()
  {
    return _map_cpu;
  }
#endif

}
