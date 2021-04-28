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
