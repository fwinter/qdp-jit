// $Id: generic_blas_vcaxmy3.h,v 1.1 2004-04-01 14:49:50 bjoo Exp $

/*! @file
 *  @brief Generic Scalar VAXPY routine
 *
 */

#ifndef QDP_GENERIC_BLAS_VCAXMY3
#define QDP_GENERIC_BLAS_VCAXMY3

QDP_BEGIN_NAMESPACE(QDP);
// (Vector) out = (Complex) (*scalep) * (Vector) InScale - (Vector) Add
inline
void vcaxmy3(REAL *Out,REAL *scalep,REAL *InScale, REAL *Add,int n_3vec)
{
  register double a_r;
  register double a_i;

  register double x0r;
  register double x0i;
  
  register double x1r;
  register double x1i;
  
  register double x2r;
  register double x2i;
  
  register double y0r;
  register double y0i;
  
  register double y1r;
  register double y1i;
  
  register double y2r;
  register double y2i;
  
  register double z0r;
  register double z0i;
  
  register double z1r;
  register double z1i;
  
  register double z2r;
  register double z2i;
  
  a_r =(double)(*scalep);
  a_i =(double)*(scalep+1);
  
  register int index_x = 0;
  register int index_y = 0;
  register int index_z = 0;
  
  register int counter;

  if( n_3vec > 0 ) { 
    // Prefetch whole vectors
    x0r = (double)InScale[index_x++];    
    x0i = (double)InScale[index_x++];
    y0r = (double)Add[index_y++];
    y0i = (double)Add[index_y++];

    x1r = (double)InScale[index_x++];    
    x1i = (double)InScale[index_x++];
    y1r = (double)Add[index_y++];
    y1i = (double)Add[index_y++];
 
    x2r = (double)InScale[index_x++];    
    x2i = (double)InScale[index_x++];
    y2r = (double)Add[index_y++];

    for( counter = 0; counter < n_3vec-1; counter++) {
      y2i = (double)Add[index_y++];
      z0r = a_r * x0r - y0r;    
      z0i = a_i * x0r - y0i;
      x0r = (double)InScale[index_x++];    
      y0r = (double)Add[index_y++];     
      z0r -= a_i * x0i;
      Out[index_z++] = (REAL)z0r;
      z0i += a_r * x0i;
      Out[index_z++] = (REAL)z0i;
      x0i = (double)InScale[index_x++];



      z1r = a_r * x1r - y1r;
      y0i = (double)Add[index_y++];      
      z1i = a_i * x1r - y1i;
      x1r = (double)InScale[index_x++];    
      y1r = (double)Add[index_y++];
      z1r -= a_i * x1i;
      Out[index_z++] = (REAL)z1r;
      z1i += a_r * x1i;
      Out[index_z++] = (REAL)z1i;
      x1i = (double)InScale[index_x++];

      z2r = a_r * x2r - y2r;
      y1i = (double)Add[index_y++];
      z2i = a_i * x2r - y2i;
      x2r = (double)InScale[index_x++];    
      y2r = (double)Add[index_y++];
      z2r -= a_i * x2i;
      Out[index_z++] = (REAL)z2r;
      z2i += a_r * x2i;
      Out[index_z++] = (REAL)z2i;
      x2i = (double)InScale[index_x++];
     
    }

    y2i = (double)Add[index_y++];
    z0r = a_r * x0r - y0r;
    z0i = a_i * x0r - y0i;       
    z0r -= a_i * x0i;
    Out[index_z++] = (REAL)z0r;
    z0i += a_r * x0i;
    Out[index_z++] = (REAL)z0i;

    z1r = a_r * x1r - y1r;
    z1i = a_i * x1r - y1i;        
    z1r -= a_i * x1i;
    Out[index_z++]= (REAL)z1r;
    z1i += a_r * x1i;
    Out[index_z++]= (REAL)z1i;

    z2r = a_r * x2r - y2r;
    z2i = a_i * x2r - y2i;
    z2r -= a_i * x2i;
    Out[index_z++]= (REAL)z2r;
    z2i += a_r * x2i;
    Out[index_z++]= (REAL)z2i;
    
  }
}


QDP_END_NAMESPACE(QDP);

#endif // guard
