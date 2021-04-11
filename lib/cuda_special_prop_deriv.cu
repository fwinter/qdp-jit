#include<vector>
#include<iostream>
#include<utility>
#include<algorithm>
#include<map>

#include <cuda_special.h>

namespace {

  __device__ int getGlobalIdx_2D_1D()
  {
    int blockId = blockIdx.y* gridDim.x+ blockIdx.x;
    int threadId = blockId * blockDim.x + threadIdx.x;
    return threadId;
  }



  template<class T>
  class cuComplex
  {
  public:
  
    __device__ cuComplex() {}
  
    __device__ cuComplex( T a, T b ) : r(a), i(b)  {}

    __device__ T real() const { return r; }

    __device__ T imag() const { return i; }

    __device__ cuComplex& operator+=(const cuComplex& a) {
      r = r + a.real();
      i = i + a.imag();
      return *this;
    }

    __device__ cuComplex& operator=(const cuComplex& a) {
      r = a.real();
      i = a.imag();
      return *this;
    }

  
  private:
    T   r;
    T   i;
  };

  template<class T>
  __device__ cuComplex<T> adj( const cuComplex<T>& a )
  {
    return cuComplex<T>( a.real() , -a.imag() );
  }


  template<class T>
  __device__ cuComplex<T> operator*( const cuComplex<T>& a , const cuComplex<T>& b )
  {
    return cuComplex<T>(a.real()*b.real() - a.imag()*b.imag(), a.imag()*b.real() + a.real()*b.imag());
  }

#if 0
  __device__ cuComplex operator+( const cuComplex& a , const cuComplex& b )
  {
    return cuComplex( a.real() + b.real() , a.imag() + b.imag() );
  }

  __device__ cuComplex operator-( const cuComplex& a , const cuComplex& b )
  {
    return cuComplex( a.real() - b.real() , a.imag() - b.imag() );
  }
#endif

  //
  // QDP-JIT datalayout
  //
  __device__ int iu( int N , int c1 , int c2 , int z, int idx)
  {
    return ((z * 3 + c1) * 3 + c2) * N + idx;
  }

  __device__ int iQ( int N , int s1 , int s2 , int c1 , int c2 , int z, int idx)
  {
    return ((((z * 3 + c1) * 3 + c2) * 4 + s1 ) * 4 + s2 ) * N + idx;
  }

  __device__ int iQscalar( int s1 , int s2 , int c1 , int c2 , int z, int idx)
  {
    return ((((idx * 4 + s1) * 4 + s2) * 3 + c1) * 3 + c2 ) * 2 + z;
  }



} // namespace



template<class T>
__global__ void cuda_prop_deriv( int threads, int N , int start ,
				 T* dest,
				 T* Q1,
				 T* Q2,
				 T* u1,
				 T* u2,
				 int* goff0m, T* recv0m,  
				 int* goff0p, T* recv0p,
				 bool do_site_perm, int* site_perm
				 )
{
  int th_idx = getGlobalIdx_2D_1D();
  if (th_idx >= threads)
    return;

  int idx = th_idx + start;

  if (do_site_perm)
    idx = site_perm[ idx ];


  cuComplex<T> Qs[3][3];
  cuComplex<T> us[3][3];
  cuComplex<T> res[3][3];


  for ( int spin_i = 0 ; spin_i < 4 ; ++spin_i )
    {
      for ( int spin_j = 0 ; spin_j < 4 ; ++spin_j )
  	{
	  // First term
	  // u[mu] * shift( Q1 , FORWARD , mu )
	  
	  for( int c1 = 0 ; c1 < 3 ; ++c1 )
	    {
	      for( int c2 = 0 ; c2 < 3 ; ++c2)
		{
		  if (goff0m[idx] < 0)
		    Qs[c1][c2] = cuComplex<T>(  recv0m[ iQscalar(spin_i,spin_j,c1,c2,0,-goff0m[idx]-1) ] , recv0m[ iQscalar(spin_i,spin_j,c1,c2,1,-goff0m[idx]-1) ] );
		  else
		    Qs[c1][c2] = cuComplex<T>(  Q1[ iQ(N,spin_i,spin_j,c1,c2,0,goff0m[idx]) ] , Q1[ iQ(N,spin_i,spin_j,c1,c2,1,goff0m[idx]) ] );

		  us[c1][c2] = cuComplex<T>(  u1[ iu(N,c1,c2,0,idx) ] , u1[ iu(N,c1,c2,1,idx) ] );
		}
	    }

	  for( int c1 = 0 ; c1 < 3 ; ++c1 )
	    {
	      for( int c2 = 0 ; c2 < 3 ; ++c2)
		{
		  res[c1][c2] = us[c1][0] * Qs[0][c2];
	
		  for( int k = 1 ; k < 3 ; ++k )
		    res[c1][c2] += us[c1][k] * Qs[k][c2];
		}
	    }
	      
  
	  for( int c1 = 0 ; c1 < 3 ; ++c1 )
	    {
	      for( int c2 = 0 ; c2 < 3 ; ++c2)
		{
		  dest[ iQ(N,spin_i,spin_j,c1,c2,0,idx) ] -= res[c1][c2].real();
		  dest[ iQ(N,spin_i,spin_j,c1,c2,1,idx) ] -= res[c1][c2].imag();
		}
	    }

	  
	  // Second term
	  // shift( adj( u[mu] ) * Q2 , BACKWARD , mu )
	  
	  if (goff0p[idx] < 0)
	    {
	      for( int c1 = 0 ; c1 < 3 ; ++c1 )
		{
		  for( int c2 = 0 ; c2 < 3 ; ++c2)
		    {
		      res[c1][c2] = cuComplex<T>(  recv0p[ iQscalar(spin_i,spin_j,c1,c2,0,-goff0p[idx]-1) ] , recv0p[ iQscalar(spin_i,spin_j,c1,c2,1,-goff0p[idx]-1) ] );
		    }
		}
	    }
	  else
	    {
	      for( int c1 = 0 ; c1 < 3 ; ++c1 )
		{
		  for( int c2 = 0 ; c2 < 3 ; ++c2)
		    {
		      Qs[c1][c2] = cuComplex<T>(  Q2[ iQ(N,spin_i,spin_j,c1,c2,0,goff0p[idx]) ] , Q2[ iQ(N,spin_i,spin_j,c1,c2,1,goff0p[idx]) ] );

		      us[c1][c2] = cuComplex<T>(  u2[ iu(N,c1,c2,0,goff0p[idx]) ] , u2[ iu(N,c1,c2,1,goff0p[idx]) ] );
		    }
		}

	      for( int c1 = 0 ; c1 < 3 ; ++c1 )
		{
		  for( int c2 = 0 ; c2 < 3 ; ++c2)
		    {
		      res[c1][c2] = adj( us[0][c1] ) * Qs[0][c2];
	
		      for( int k = 1 ; k < 3 ; ++k )
			res[c1][c2] += adj( us[k][c1] ) * Qs[k][c2];
		    }
		}
	    }

	  for( int c1 = 0 ; c1 < 3 ; ++c1 )
	    {
	      for( int c2 = 0 ; c2 < 3 ; ++c2)
		{
		  dest[ iQ(N,spin_i,spin_j,c1,c2,0,idx) ] -= res[c1][c2].real();
		  dest[ iQ(N,spin_i,spin_j,c1,c2,1,idx) ] -= res[c1][c2].imag();
		}
	    }

	  
	} // spin_j
    } // spin_i
}






void evaluate_special_prop_deriv_float( int threads, int N , int start, std::vector<void*> args , bool do_siteperm , int func_num )
{
  const int default_blocksize = 128;

  jumper_jit_stats_special(func_num);
  
  if (cuda_special_get_maxgridx() == -1)
    {
      std::cerr << "evaluate_dslash, cuda_special_maxgridx not set\n";
      exit(1);
    }
  
  int threads_per_block = default_blocksize;
  if ( cuda_special_get_blocksize().count( func_num ) > 0 )
    threads_per_block = cuda_special_get_blocksize()[ func_num ];
  
  std::pair<int,int> size = getBlockDim( N , threads_per_block );
  dim3 grid(  size.first , size.second , 1 );
  dim3 block( threads_per_block , 1 , 1 );

  // std::cout << "launching : grid( " << size.first << " , " << size.second << " , 1)   ";
  // std::cout << "launching : block( " << threads_per_block << " , 1 , 1 )\n";

  // 0 dest 
  // 1 u
  // 2,3 shift (off,recv)
  // 4 Q

  // 5,6 shift
  // 7 u
  // 8 Q

  float* dest = (float*)*(void**)args[0];

  int* goff[2];
  goff[0] = (int*)*(void**)args[2];
  goff[1] = (int*)*(void**)args[5];

  float* recv[2];
  recv[0] = (float*)*(void**)args[3];
  recv[1] = (float*)*(void**)args[6];

  float* Q1 = (float*)*(void**)args[4];
  float* Q2 = (float*)*(void**)args[8];

  float* u1 = (float*)*(void**)args[1];
  float* u2 = (float*)*(void**)args[7];

  int* site_perm = do_siteperm ? (int*)*(void**)args[9] : NULL;


  cuda_prop_deriv<float><<< grid , block >>>( threads, N , start ,
					      dest ,
					      Q1 , Q2,
					      u1 , u1,
					      goff[0] , recv[0] , 
					      goff[1] , recv[1] , 
					      do_siteperm, site_perm
					      );

}


void evaluate_special_prop_deriv_double( int threads, int N , int start, std::vector<void*> args , bool do_siteperm , int func_num )
{
  const int default_blocksize = 128;

  jumper_jit_stats_special(func_num);
  
  if (cuda_special_get_maxgridx() == -1)
    {
      std::cerr << "evaluate_dslash, cuda_special_maxgridx not set\n";
      exit(1);
    }
  
  int threads_per_block = default_blocksize;
  if ( cuda_special_get_blocksize().count( func_num ) > 0 )
    threads_per_block = cuda_special_get_blocksize()[ func_num ];
  
  std::pair<int,int> size = getBlockDim( N , threads_per_block );
  dim3 grid(  size.first , size.second , 1 );
  dim3 block( threads_per_block , 1 , 1 );

  // std::cout << "launching : grid( " << size.first << " , " << size.second << " , 1)   ";
  // std::cout << "launching : block( " << threads_per_block << " , 1 , 1 )\n";

  // 0 dest 
  // 1 u
  // 2,3 shift (off,recv)
  // 4 Q

  // 5,6 shift
  // 7 u
  // 8 Q

  double* dest = (double*)*(void**)args[0];

  int* goff[2];
  goff[0] = (int*)*(void**)args[2];
  goff[1] = (int*)*(void**)args[5];

  double* recv[2];
  recv[0] = (double*)*(void**)args[3];
  recv[1] = (double*)*(void**)args[6];

  double* Q1 = (double*)*(void**)args[4];
  double* Q2 = (double*)*(void**)args[8];

  double* u1 = (double*)*(void**)args[1];
  double* u2 = (double*)*(void**)args[7];

  int* site_perm = do_siteperm ? (int*)*(void**)args[9] : NULL;


  cuda_prop_deriv<double><<< grid , block >>>( threads, N , start ,
					      dest ,
					      Q1 , Q2,
					      u1 , u1,
					      goff[0] , recv[0] , 
					      goff[1] , recv[1] , 
					      do_siteperm, site_perm
					      );

}


#if 0
void linkage_hack()
{
  std::vector<void*> dummy;
  evaluate_special_prop_deriv<float> ( 0, 0 , 0, dummy , false , 0 );
  evaluate_special_prop_deriv<double>( 0, 0 , 0, dummy , false , 0 );
}
#endif


