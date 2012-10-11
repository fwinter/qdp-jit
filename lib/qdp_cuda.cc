// -*- c++ -*-


#include <iostream>

#include "qdp_config_internal.h" 

#include "qdp_init.h"
#include "qdp_deviceparams.h"
#include "qdp_cuda.h"
#include "cuda.h"

using namespace std;



namespace QDP {

  void CudaRes(const std::string& s,CUresult ret) {
    if (ret != CUDA_SUCCESS) {
      std::cout << "cuda error: " << s << "\n";
      exit(1);
    }
  }


  void CudaInit() {
    std::cout << "cuda init\n";
    cuInit(0);

    int deviceCount = 0;
    cuDeviceGetCount(&deviceCount);
    if (deviceCount == 0) { 
      std::cout << "There is no device supporting CUDA.\n"; 
      exit(1); 
    }
  }





  CUstream * QDPcudastreams;
  CUevent * QDPevCopied;

  CUdevice cuDevice;
  CUcontext cuContext;


  void * CudaGetKernelStream() {
    return (void*)&QDPcudastreams[KERNEL];
  }

  void CudaCreateStreams() {
    QDPcudastreams = new CUstream[2];
    for (int i=0; i<2; i++) {
      QDP_info_primary("JIT: Creating CUDA stream %d",i);
      cuStreamCreate(&QDPcudastreams[i],0);
    }
    QDP_info_primary("JIT: Creating CUDA event for transfers");
    QDPevCopied = new CUevent;
    cuEventCreate(QDPevCopied,CU_EVENT_BLOCKING_SYNC);
  }

  void CudaSyncKernelStream() {
    cuStreamSynchronize(QDPcudastreams[KERNEL]);
  }

  void CudaSyncTransferStream() {
    cuStreamSynchronize(QDPcudastreams[TRANSFER]);
  }

  void CudaRecordAndWaitEvent() {
    cuEventRecord( *QDPevCopied , QDPcudastreams[TRANSFER] );
    cuStreamWaitEvent( QDPcudastreams[KERNEL] , *QDPevCopied , 0);
  }

  void CudaSetDevice(int dev)
  {
    CUresult ret;
    std::cout << "trying to get device " << dev << "\n";
    ret = cuDeviceGet(&cuDevice, dev);
    CudaRes("",ret);
    std::cout << "trying to create a context \n";
    ret = cuCtxCreate(&cuContext, 0, cuDevice);
    CudaRes("",ret);

    int unified;
    ret = cuDeviceGetAttribute( &unified, CU_DEVICE_ATTRIBUTE_UNIFIED_ADDRESSING, dev );
    CudaRes("cuDeviceGetAttribute",ret);
    std::cout << "device supports UA = " << unified << "\n";
  }

  void CudaGetDeviceCount(int * count)
  {
    cuDeviceGetCount( count );
  }


  bool CudaHostRegister(void * ptr , size_t size)
  {
    CUresult ret;
    int flags = 0;
    QDP_info_primary("CUDA host register ptr=%p (%u) size=%lu (%u)",ptr,(unsigned)((size_t)ptr%4096) ,(unsigned long)size,(unsigned)((size_t)size%4096));
    ret = cuMemHostRegister(ptr, size, flags);
    CudaRes("cuMemHostRegister",ret);
    return true;
  }

  
  void CudaHostUnregister(void * ptr )
  {
    CUresult ret;
    ret = cuMemHostUnregister(ptr);
    CudaRes("cuMemHostUnregister",ret);
  }
  

  bool CudaHostAlloc(void **mem , const size_t size, const int flags)
  {
    CUresult ret;
    ret = cuMemHostAlloc(mem,size,flags);
    CudaRes("cudaHostAlloc",ret);
    return ret == CUDA_SUCCESS;
  }


  void CudaHostFree(const void *mem)
  {
    CUresult ret;
    ret = cuMemFreeHost((void *)mem);
    CudaRes("cuMemFreeHost",ret);
  }




  void CudaMemcpy( const void * dest , const void * src , size_t size)
  {
    CUresult ret;
#ifdef GPU_DEBUG_DEEP
    QDP_debug_deep("cudaMemcpy dest=%p src=%p size=%d" ,  dest , src , size );
#endif

    ret = cuMemcpy((CUdeviceptr)const_cast<void*>(dest),
		   (CUdeviceptr)const_cast<void*>(src),
		   size);

    CudaRes("cuMemcpy",ret);
  }


  void CudaMemcpyAsync( const void * dest , const void * src , size_t size )
  {
    CUresult ret;
#ifdef GPU_DEBUG_DEEP
    QDP_debug_deep("cudaMemcpy dest=%p src=%p size=%d" ,  dest , src , size );
#endif

    if (DeviceParams::Instance().getAsyncTransfers()) {
      ret = cuMemcpyAsync((CUdeviceptr)const_cast<void*>(dest),
			  (CUdeviceptr)const_cast<void*>(src),
			  size,QDPcudastreams[TRANSFER]);
    } else {
      std::cout << "using sync copy\n";
      ret = cuMemcpy((CUdeviceptr)const_cast<void*>(dest),
		     (CUdeviceptr)const_cast<void*>(src),
		     size);
    }

    CudaRes("cuMemcpyAsync",ret);
  }

  
  void CudaMemcpyH2DAsync( void * dest , const void * src , size_t size )
  {
    CUresult ret;
#ifdef GPU_DEBUG_DEEP
    QDP_debug_deep("CudaMemcpyH2DAsync dest=%p src=%p size=%d" ,  dest , src , size );
#endif

    if (DeviceParams::Instance().getAsyncTransfers()) {
      ret = cuMemcpyHtoDAsync((CUdeviceptr)const_cast<void*>(dest),
			      src,
			      size,QDPcudastreams[TRANSFER]);
    } else {
      std::cout << "using sync H2D copy\n";
      ret = cuMemcpyHtoD((CUdeviceptr)const_cast<void*>(dest),
			 src,
			 size);
    }

    CudaRes("cuMemcpyH2DAsync",ret);
  }

  void CudaMemcpyD2HAsync( void * dest , const void * src , size_t size )
  {
    CUresult ret;
#ifdef GPU_DEBUG_DEEP
    QDP_debug_deep("CudaMemcpyD2HAsync dest=%p src=%p size=%d" ,  dest , src , size );
#endif

    if (DeviceParams::Instance().getAsyncTransfers()) {
      ret = cuMemcpyDtoHAsync( dest,
			      (CUdeviceptr)const_cast<void*>(src),
			      size,QDPcudastreams[TRANSFER]);
    } else {
      std::cout << "using sync D2H copy\n";
      ret = cuMemcpyDtoH( const_cast<void*>(dest),
			 (CUdeviceptr)const_cast<void*>(src),
			 size);
    }

    CudaRes("cuMemcpyH2DAsync",ret);
  }


  void CudaMemcpyH2D( void * dest , const void * src , size_t size )
  {
    CUresult ret;
#ifdef GPU_DEBUG_DEEP
    QDP_debug_deep("CudaMemcpyH2D dest=%p src=%p size=%d" ,  dest , src , size );
#endif
    ret = cuMemcpyHtoD((CUdeviceptr)const_cast<void*>(dest), src, size);
    CudaRes("cuMemcpyH2D",ret);
  }

  void CudaMemcpyD2H( void * dest , const void * src , size_t size )
  {
    CUresult ret;
#ifdef GPU_DEBUG_DEEP
    QDP_debug_deep("CudaMemcpyD2H dest=%p src=%p size=%d" ,  dest , src , size );
#endif
    ret = cuMemcpyDtoH( dest, (CUdeviceptr)const_cast<void*>(src), size);
    CudaRes("cuMemcpyH2D",ret);
  }


  bool CudaMalloc(void **mem , size_t size )
  {
    CUresult ret;
    ret = cuMemAlloc( (CUdeviceptr*)mem,size);
#ifdef GPU_DEBUG_DEEP
    QDP_debug_deep( "CudaMalloc %p", *mem );
#endif
    CudaRes("cuMemAlloc",ret);
    return ret == CUDA_SUCCESS;
  }

  void CudaFree(const void *mem )
  {
#ifdef GPU_DEBUG_DEEP
    QDP_debug_deep( "CudaFree %p", mem );
#endif
    CUresult ret;
    ret = cuMemFree((CUdeviceptr)const_cast<void*>(mem));
    CudaRes("cuMemFree",ret);
  }

  void CudaThreadSynchronize()
  {
#ifdef GPU_DEBUG_DEEP
    QDP_debug_deep( "cudaThreadSynchronize" );
#endif
    cuCtxSynchronize();
  }

  void CudaDeviceSynchronize()
  {
#ifdef GPU_DEBUG_DEEP
    QDP_debug_deep( "cudaDeviceSynchronize" );
#endif
    cuCtxSynchronize();
  }

}

