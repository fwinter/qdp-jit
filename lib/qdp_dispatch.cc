#include "qdp.h"

#if defined(QDP_USE_OMP_THREADS)

#include <omp.h>
namespace QDP {    
  
  int qdpNumThreads() {
    return omp_get_max_threads();
  }

  void qdpSetNumThreads(int n) {
    omp_set_num_threads(n);
  }

  void jit_dispatch( void* function , const AddressLeaf& args)
  {
    // void (*FP)(int64_t,int64_t,int64_t,bool,int64_t,void*) = 
    //   (void (*)(int64_t,int64_t,int64_t,bool,int64_t,void*))(intptr_t)function;

    //void (*FP)(void*) = (void (*)(void*))(intptr_t)function;

    void (*FP)(int64_t,int64_t,void*) = (void (*)(int64_t,int64_t,void*))(intptr_t)function;

    // int threads_num;
    // int64_t myId;
    int64_t lo = 0;
    int64_t hi = Layout::sitesOnNode();
    void * addr = args.addr.data();

#if 0
    if (args.addr.size() == 2) {
      float* f0 = (float*)args.addr[0].ptr;
      float* f1 = (float*)args.addr[1].ptr;

      QDPIO::cerr << "dest after jit dispatch:\n";
      for (int i=0;i<Layout::vol();i++) {
	QDPIO::cerr << f0[i] << " ";
      }
      QDPIO::cerr << "\n";
      QDPIO::cerr << "src before jit dispatch:\n";
      for (int i=0;i<Layout::vol();i++) {
	QDPIO::cerr << f1[i] << " ";
      }
      QDPIO::cerr << "\n";
    }
#endif



    //QDPIO::cerr << "dispatch... " << args.addr.size() << "\n";

    // #pragma omp parallel shared(site_count, threads_num, ordered, start, addr) private(myId, lo, hi) default(shared)
    //     {
    //       threads_num = omp_get_num_threads();
    //       myId = omp_get_thread_num();
    //       lo = ((site_count/inner)*myId/threads_num)*inner;
    //       hi = ((site_count/inner)*(myId+1)/threads_num)*inner;

    //std::cout << "dispatch: " << lo << " " << hi << " " << myId << " " << ordered << " " << start << "\n";

    //FP( lo , hi , myId , ordered, start, addr );
    //FP( addr );
    FP( lo, hi, addr );

#if 0
    if (args.addr.size() == 2) {
      float* f0 = (float*)args.addr[0].ptr;
      float* f1 = (float*)args.addr[1].ptr;

      QDPIO::cerr << "dest after jit dispatch:\n";
      for (int i=0;i<Layout::vol();i++) {
	QDPIO::cerr << f0[i] << " ";
      }
      QDPIO::cerr << "\n";
    }
#endif

    //    }

    //QDPIO::cerr << "         (returned)\n";
  }


}

#else 

#if defined(QDP_USE_QMT_THREADS)
#error "QMT threading not implemented"
#endif

#error "Must use threading"

#endif
