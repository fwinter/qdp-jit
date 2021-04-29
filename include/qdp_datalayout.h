// -*- C++ -*-

#ifndef QDP_DATALAYOUT_H
#define QDP_DATALAYOUT_H


namespace QDP {

  typedef std::pair< int , llvm::Value * > IndexDomain;

  class IndexDomainVector: public std::vector< IndexDomain >
  {
    bool has_add = false;
    llvm::Value *start_m;
  public:
#if 1
    IndexDomainVector(): std::vector<IndexDomain>() {}
    
    IndexDomainVector(const IndexDomainVector& rhs): std::vector<IndexDomain>(rhs), has_add(rhs.has_add), start_m(rhs.start_m) {}
    IndexDomainVector& operator=(const IndexDomainVector& rhs)
    {
      std::vector<IndexDomain>::operator=(rhs);
      
      has_add = rhs.has_add;
      start_m = rhs.start_m;
      
      return *this;
    }
#endif
    
    bool hasAdd() const
    {
      return has_add;
    }

    llvm::Value *getAdd() const
    {
      return start_m;
    }
    
    void setAdd( llvm::Value * start )
    {
      start_m = start;
      has_add = true;
    }
  };
  

  llvm::Value * datalayout( JitDeviceLayout lay , IndexDomainVector a );
  //llvm::Value * datalayout_stack(IndexDomainVector a);

} // namespace QDP

#endif
