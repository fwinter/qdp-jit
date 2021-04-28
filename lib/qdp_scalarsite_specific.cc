
/*! @file
 * @brief Scalar-like architecture specific routines
 * 
 * Routines common to all scalar-like architectures including scalar and parscalar
 */


#include "qdp.h"
#include "qdp_util.h"

namespace QDP {

//-----------------------------------------------------------------------------
namespace Layout
{

  namespace {
    std::vector< std::shared_ptr<LatticeInteger> > latCoord(Nd);
  }

  void destroyLatticeCoordinate()
  {
    latCoord.clear();
  }
  
  //! coord[mu]  <- mu  : fill with lattice coord in mu direction
  /* Assumes no inner grid */
  LatticeInteger latticeCoordinate(int mu)
  {
    if (mu < 0 || mu >= Nd)
      QDP_error_exit("dimension out of bounds");

    if (!latCoord[mu]) {
      //QDPIO::cout << "creating latticeCoordinate " << mu << "\n";
      const int nodeSites = Layout::sitesOnNode();
      const int nodeNumber = Layout::nodeNumber();
      LatticeInteger d;
      for(int i=0; i < nodeSites; ++i) 
	{
	  Integer cc = Layout::siteCoords(nodeNumber,i)[mu];
	  d.elem(i) = cc.elem();
	}
      latCoord[mu] = std::make_shared<LatticeInteger>(d);
      //*latCoord[mu] = d;
    }
    
    return *latCoord[mu];
  }
}


//-----------------------------------------------------------------------------
// IO routine solely for debugging. Only defined here
template<class T>
ostream& operator<<(ostream& s, const multi1d<T>& s1)
{
  for(int i=0; i < s1.size(); ++i)
    s << " " << s1[i];

  return s;
}


//-----------------------------------------------------------------------------
//! Constructor from a function object
void Set::make(const SetFunc& fun)
{
  int nsubset_indices = fun.numSubsets();
  const int nodeSites = Layout::sitesOnNode();
  const int nodeNumber = Layout::nodeNumber();

#if QDP_DEBUG >= 2
  QDP_info("Set a subset: nsubset = %d",nsubset_indices);
#endif

  // This actually allocates the subsets
  sub.resize(nsubset_indices);

  // Create the space of the colorings of the lattice
  lat_color.resize(nodeSites);

  // Create the array holding the array of sitetable info
  sitetables.resize(nsubset_indices);

  // Create the array holding the array of membertable info
  membertables.resize(nsubset_indices);

  signOffTables();
  
  idSiteTable.resize(nsubset_indices);
  idMemberTable.resize(nsubset_indices);

  
  // Loop over linear sites determining their color
  for(int linear=0; linear < nodeSites; ++linear)
  {
    multi1d<int> coord = Layout::siteCoords(nodeNumber, linear);

    int node   = Layout::nodeNumber(coord);
    int lin    = Layout::linearSiteIndex(coord);
    int icolor = fun(coord);

#if QDP_DEBUG >= 3
    cerr<<"linear="<<linear<<" coord="<<coord<<" node="<<node<<" col="<<icolor << endl;
#endif

    // Sanity checks
    if (node != nodeNumber)
      QDP_error_exit("Set: found site with node outside current node!");

    if (lin != linear)
      {
	QDPIO::cout << coord[0] << "," << coord[1] << "," << coord[2] << "," << coord[3] << ": " << lin << " != " << linear << std::endl;
	QDP_error_exit("Set: inconsistent linear sites");
      }

    if (icolor < 0 || icolor >= nsubset_indices)
      QDP_error_exit("Set: coloring is outside legal range: color[%d]=%d",linear,icolor);

    // The coloring of this linear site
    lat_color[linear] = icolor;
  }




  /*
   * Loop over the lexicographic sites.
   * This implementation of the Set will always use a
   * sitetable.
   */
  for(int cb=0; cb < nsubset_indices; ++cb)
  {
    // Always construct the sitetables. 

    multi1d<bool>& membertable = membertables[cb];
    membertable.resize(nodeSites);

    // First loop and see how many sites are needed
    int num_sitetable = 0;
    for(int linear=0; linear < nodeSites; ++linear)
      if (lat_color[linear] == cb) {
	++num_sitetable;
	membertable[linear] = true;
      } else {
	membertable[linear] = false;
      }

    // Now take the inverse of the lattice coloring to produce
    // the site list
    multi1d<int>& sitetable = sitetables[cb];
    sitetable.resize(num_sitetable);


    // Site ordering stuff for later
    bool ordRep;
    int start, end;

    // Handle the case that there are no sites
    if (num_sitetable > 0)
    {
      // For later sanity, initialize this to something 
      for(int i=0; i < num_sitetable; ++i)
	sitetable[i] = -1;

      for(int linear=0, j=0; linear < nodeSites; ++linear)
	if (lat_color[linear] == cb)
	  sitetable[j++] = linear;


      // Check *if* this coloring is contiguous and find the start
      // and ending sites
      ordRep = true;
      start = sitetable[0];   // this is the beginning
      end = sitetable[sitetable.size()-1];  // the absolute last site
      
      // Now look for a hole
      for(int prev=sitetable[0], i=0; i < sitetable.size(); ++i)
	if (sitetable[i] != prev++)
	{
#if QDP_DEBUG >= 2
	  QDP_info("Set(%d): sitetable[%d]=%d",cb,i,sitetable[i]);
#endif
	
	  // Found a hold. The rep is not ordered.
	  ordRep = false;
	  start = end = -1;
	  break;
	}
    }
    else  // num_sitetable == 0
    {
      ordRep = false;
      start = end = -1;
    }


    idSiteTable[cb]   = sitetable.size()   > 0 ? QDP_get_global_cache().registrateOwnHostMem( sitetable.size()   * sizeof(int)  , sitetable.slice() , NULL ) : -1 ;
    idMemberTable[cb] = membertable.size() > 0 ? QDP_get_global_cache().registrateOwnHostMem( membertable.size() * sizeof(bool) , membertable.slice() , NULL ) : -1 ;


    sub[cb].make(ordRep, start, end, &sitetables[cb], &idSiteTable[cb], cb, this, &membertables[cb], &idMemberTable[cb] , -1 ); // -1 for the masterset id which is still unknown

#if QDP_DEBUG >= 2
    QDP_info("Subset(%d)",cb);
#endif
  }

  MasterSet::Instance().registrate( *this );

  registered=true;
}
	  

} // namespace QDP;
