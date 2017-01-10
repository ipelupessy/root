/***************************************************************************** 
 * Project: RooFit                                                           * 
 *                                                                           * 
 * This code was autogenerated by RooClassFactory                            * 
 *****************************************************************************/ 

// Your description goes here... 

#include "Riostream.h" 

#include "RooStarMomentMorph.h" 
#include "RooAbsCategory.h" 
#include "RooRealIntegral.h"
#include "RooRealConstant.h"
#include "RooRealVar.h"
#include "RooFormulaVar.h"
#include "RooCustomizer.h"
#include "RooAddPdf.h"
#include "RooAddition.h"
#include "RooAbsMoment.h"
#include "RooLinearVar.h"
#include "RooChangeTracker.h"
#include "RooNumIntConfig.h"
#include "RooHistPdf.h"

#include "TMath.h"
#include "TVector.h"

using namespace std ;

ClassImp(RooStarMomentMorph) 


//_____________________________________________________________________________
RooStarMomentMorph::RooStarMomentMorph() : 
  _curNormSet(0), 
  _M(0), 
  _setting(RooStarMomentMorph::Linear), 
  _nnuisvar(0),
  _useHorizMorph(true)
{

  // coverity[UNINIT_CTOR]
  _parItr    = _parList.createIterator() ;
  _obsItr    = _obsList.createIterator() ;
  _pdfItr    = _pdfList.createIterator() ; 
}



//_____________________________________________________________________________
RooStarMomentMorph::RooStarMomentMorph(const char *name, const char *title, 
				       const RooArgList& parList,
				       const RooArgList& obsList,
				       const RooArgList& pdfList, // {dn0,up0,dn1,up1,...,nominal}
				       const std::vector<int>& nnuispoints, // number of variation (not counting nominal) for each NP (2,2)
				       const std::vector<double>& nrefpoints, // (-1,1,-1,1)
				       const Setting& setting) :
  RooAbsPdf(name,title), 
  _cacheMgr(this,10,kTRUE,kTRUE),
  _parList("parList","List of fit parameters",this),
  _obsList("obsList","List of variables",this),
  _pdfList("pdfList","List of pdfs",this),
  _nnuis(nnuispoints),
  _nref(nrefpoints),
  _M(0),
  _setting(setting),
  _nnuisvar(0),
  _useHorizMorph(true)
{ 
  // CTOR

  // fit parameters
  TIterator* parItr = parList.createIterator() ;
  RooAbsArg* par ;
  for (Int_t i=0; (par = (RooAbsArg*)parItr->Next()); ++i) {
    if (!dynamic_cast<RooAbsReal*>(par)) {
      coutE(InputArguments) << "RooStarMomentMorph::ctor(" << GetName() << ") ERROR: parameter " << par->GetName() << " is not of type RooAbsReal" << endl ;
      throw string("RooStarMomentMorh::ctor() ERROR parameter is not of type RooAbsReal") ;
    }
    _parList.add(*par) ;
  }
  delete parItr ;

  // observables
  TIterator* obsItr = obsList.createIterator() ;
  RooAbsArg* var ;
  for (Int_t i=0; (var = (RooAbsArg*)obsItr->Next()); ++i) {
    if (!dynamic_cast<RooAbsReal*>(var)) {
      coutE(InputArguments) << "RooStarMomentMorph::ctor(" << GetName() << ") ERROR: variable " << var->GetName() << " is not of type RooAbsReal" << endl ;
      throw string("RooStarMomentMorh::ctor() ERROR variable is not of type RooAbsReal") ;
    }
    _obsList.add(*var) ;
  }
  delete obsItr ;

  // reference p.d.f.s
  TIterator* pdfItr = pdfList.createIterator() ;
  RooAbsPdf* pdf ;
  for (Int_t i=0; (pdf = dynamic_cast<RooAbsPdf*>(pdfItr->Next())); ++i) {
    if (!pdf) {
      coutE(InputArguments) << "RooStarMomentMorph::ctor(" << GetName() << ") ERROR: pdf " << pdf->GetName() << " is not of type RooAbsPdf" << endl ;
      throw string("RooStarMomentMorph::ctor() ERROR pdf is not of type RooAbsPdf") ;
    }
    _pdfList.add(*pdf) ;
  }
  delete pdfItr ;

  _parItr    = _parList.createIterator() ;
  _obsItr    = _obsList.createIterator() ;
  _pdfItr    = _pdfList.createIterator() ; 


  // initialization
  initialize();
} 


//_____________________________________________________________________________
RooStarMomentMorph::RooStarMomentMorph(const RooStarMomentMorph& other, const char* name) :  
  RooAbsPdf(other,name), 
  _cacheMgr(other._cacheMgr,this),
  _curNormSet(0),
  _parList("parList",this,other._parList),
  _obsList("obsList",this,other._obsList),
  _pdfList("pdfList",this,other._pdfList),
  _nnuis(other._nnuis),
  _nref(other._nref),
  _M(0),
  _setting(other._setting),
  _nnuisvar(other._nnuisvar),
  _useHorizMorph(other._useHorizMorph)
{ 
 

  _parItr    = _parList.createIterator() ;
  _obsItr    = _obsList.createIterator() ;
  _pdfItr    = _pdfList.createIterator() ; 

  // nref is resized in initialize, so reduce the size here
  if (_nref.size()>0) {
    _nref.resize( _nref.size()-1 );
  }

  // initialization
  initialize();
} 

//_____________________________________________________________________________
RooStarMomentMorph::~RooStarMomentMorph() 
{
  if (_parItr) delete _parItr;
  if (_obsItr) delete _obsItr;
  if (_pdfItr) delete _pdfItr;
  if (_M)      delete _M;
}



//_____________________________________________________________________________
void RooStarMomentMorph::initialize() 
{

  unsigned int nPar = _parList.getSize();
  unsigned int nPdf = _pdfList.getSize();

  // other quantities needed
  if (nPar!=_nnuis.size()) {
    coutE(InputArguments) << "RooStarMomentMorph::initialize(" << GetName() << ") ERROR: nPar != nNuis" << endl ;
    assert(0) ;
  }

  // total number of nuisance parameter variations
  _nnuisvar = 0; 
  for (unsigned int k=0; k<_nnuis.size(); ++k) { _nnuisvar += _nnuis[k]; }

  // check of minimal number inputs
  for (unsigned int k=0; k<_nnuis.size(); ++k) { 
    if (_nnuis[k]<1) {
      coutE(InputArguments) << "RooStarMomentMorph::initialize(" << GetName() << ") ERROR: nNuis < 1 for variation: " << k << endl ;
      assert(0);
    }
  }



  // check number of input pdfs
  if (nPdf!=_nnuisvar+1) {
    coutE(InputArguments) << "RooStarMomentMorph::initialize(" << GetName() << ") ERROR: nPdf != nNuis variations" << endl ;
    assert(0) ;
  }

  // bit of a hack: last element corresponds to nnuis value of nominal pdf, ala pdflist
  _nref.resize( _nref.size()+1 );
  _nref[_nnuisvar] = 0.;


  //// MB : old left-over code ...
  //   TVector* dm = new TVector(nPdf);
  //   _M = new TMatrixD(nPdf,nPdf);
  
  //   // transformation matrix for non-linear extrapolation, needed in evaluate()
  //   TMatrixD M(nPdf,nPdf);
  //   for (unsigned int i=0; i<_nref->size(); ++i) {
  //     (*dm)[i] = (*_nref)[i]-(*_nref)[0];
  //     M(i,0) = 1.;
  //     if (i>0) M(0,i) = 0.;
  //   }
  //   for (unsigned int i=1; i<_nref->size(); ++i) {
  //     for (unsigned int j=1; j<_nref->size(); ++j) {
  //       M(i,j) = TMath::Power((*dm)[i],(double)j);
  //     }
  //   }
  //   (*_M) = M.Invert();
  
  //   delete dm ;
}

//_____________________________________________________________________________
RooStarMomentMorph::CacheElem* RooStarMomentMorph::getCache(const RooArgSet* /*nset*/) const
{
  CacheElem* cache = (CacheElem*) _cacheMgr.getObj(0,(RooArgSet*)0) ;
  if (cache) {
    return cache ;
  }


  Int_t nObs = _obsList.getSize();
  Int_t nPdf = _pdfList.getSize();

  RooAbsReal* null = 0 ;
  vector<RooAbsReal*> meanrv(nPdf*nObs,null);
  vector<RooAbsReal*> sigmarv(nPdf*nObs,null); 
  vector<RooAbsReal*> myrms(nObs,null);      
  vector<RooAbsReal*> mypos(nObs,null);      
  vector<RooAbsReal*> slope(nPdf*nObs,null); 
  vector<RooAbsReal*> offsetVar(nPdf*nObs,null); 
  vector<RooAbsReal*> transVar(nPdf*nObs,null); 
  vector<RooAbsReal*> transPdf(nPdf,null);      

  RooArgSet ownedComps ;

  RooArgList fracl ;

  // fraction parameters
  RooArgList coefList("coefList");    // fractions multiplied with input pdfs
  RooArgList coefList2("coefList2");  // fractions multiplied with mean position of observable contribution
  RooArgList coefList3("coefList3");  // fractions multiplied with rms position of observable contribution

  for (Int_t i=0; i<3*nPdf; ++i) {

    std::string fracName = Form("frac_%d",i);
    double initval=(i==nPdf-1||i==2*nPdf-1||i==3*nPdf-1) ? 1. : 0.;
    RooRealVar* frac=new RooRealVar(fracName.c_str(),fracName.c_str(),initval); // to be set later 

    fracl.add(*frac); 
    if      (i<nPdf)   coefList .add(*frac) ;
    else if (i<2*nPdf) coefList2.add(*frac) ;
    else               coefList3.add(*frac) ;
    ownedComps.add(*frac) ;
  }

  if (_useHorizMorph) {
    
    // mean and sigma
    RooArgList obsList(_obsList);
    for (Int_t i=0; i<nPdf; ++i) {      
      for (Int_t j=0; j<nObs; ++j) {
	
	RooAbsMoment* mom = nObs==1 ?
          ((RooAbsPdf*)_pdfList.at(i))->sigma((RooRealVar&)*obsList.at(j)) :
          ((RooAbsPdf*)_pdfList.at(i))->sigma((RooRealVar&)*obsList.at(j), obsList);

        mom->setLocalNoDirtyInhibit(kTRUE);
        mom->mean()->setLocalNoDirtyInhibit(kTRUE);
	
        sigmarv[sij(i,j)] = mom;
        meanrv [sij(i,j)] = mom->mean();
        ownedComps.add(*sigmarv[sij(i,j)]);
	
      }
    }    
  
    // slope and offset (to be set later, depend on nuisance parameters)
    for (Int_t j=0; j<nObs; ++j) {
      
      RooArgList meanList("meanList");
      RooArgList rmsList("rmsList");
      
      for (Int_t i=0; i<nPdf; ++i) {
	meanList.add(*meanrv [sij(i,j)]);
	rmsList. add(*sigmarv[sij(i,j)]);
      }
      
      std::string myrmsName = Form("%s_rms_%d",GetName(),j);
      std::string myposName = Form("%s_pos_%d",GetName(),j);
      
      mypos[j] = new RooAddition(myposName.c_str(),myposName.c_str(),meanList,coefList2);
      myrms[j] = new RooAddition(myrmsName.c_str(),myrmsName.c_str(),rmsList,coefList3);
      
      ownedComps.add(RooArgSet(*myrms[j],*mypos[j])) ;
    }
  }

  //for (Int_t i=0;i<_parList.getSize();i++) {    
  //RooRealVar* par=(RooRealVar*)_parList.at(i);
  //par->Print();
  //}

  // construction of unit pdfs
  _pdfItr->Reset();
  RooAbsPdf* pdf;
  RooArgList transPdfList;

  for (Int_t i=0; i<nPdf; ++i) {
    _obsItr->Reset() ;
    RooRealVar* var ;

    pdf = (RooAbsPdf*)_pdfItr->Next();

    std::string pdfName = Form("pdf_%d",i);
    RooCustomizer cust(*pdf,pdfName.c_str());

    for (Int_t j=0; j<nObs; ++j) {

      // slope and offset formulas
      std::string slopeName  = Form("%s_slope_%d_%d", GetName(),i,j);
      std::string offsetName = Form("%s_offset_%d_%d",GetName(),i,j);

      slope[sij(i,j)]  = _useHorizMorph ? 
	(RooAbsReal*)new RooFormulaVar(slopeName.c_str(),"@0/@1",
				       RooArgList(*sigmarv[sij(i,j)],*myrms[j])) : 
	(RooAbsReal*)new RooConstVar(slopeName.c_str(),slopeName.c_str(),1.0); // 
      
      
      offsetVar[sij(i,j)] = _useHorizMorph ? 
	(RooAbsReal*)new RooFormulaVar(offsetName.c_str(),"@0-(@1*@2)",
				       RooArgList(*meanrv[sij(i,j)],*mypos[j],*slope[sij(i,j)])) : 	
	(RooAbsReal*)new RooConstVar(offsetName.c_str(),offsetName.c_str(),0.0); // 
      
      ownedComps.add(RooArgSet(*slope[sij(i,j)],*offsetVar[sij(i,j)])) ;
      
      // linear transformations, so pdf can be renormalized easily
      var = (RooRealVar*)(_obsItr->Next());
      std::string transVarName = Form("%s_transVar_%d_%d",GetName(),i,j);
      transVar[sij(i,j)] = new RooLinearVar(transVarName.c_str(),transVarName.c_str(),*var,*slope[sij(i,j)],*offsetVar[sij(i,j)]);

      // *** WVE this is important *** this declares that frac effectively depends on the morphing parameters
      // This will prevent the likelihood optimizers from erroneously declaring terms constant
      transVar[sij(i,j)]->addServerList((RooAbsCollection&)_parList);

      ownedComps.add(*transVar[sij(i,j)]) ;
      cust.replaceArg(*var,*transVar[sij(i,j)]);
    }
    transPdf[i] = (RooAbsPdf*) cust.build() ;
    transPdfList.add(*transPdf[i]);

    ownedComps.add(*transPdf[i]) ;
  }
  // sum pdf
  
  std::string sumpdfName = Form("%s_sumpdf",GetName());

  RooAbsPdf* theSumPdf = new RooAddPdf(sumpdfName.c_str(),sumpdfName.c_str(),transPdfList,coefList);
  theSumPdf->setAttribute("NeverConstant") ;
  theSumPdf->addOwnedComponents(ownedComps) ;
  
  // change tracker for fraction parameters
  std::string trackerName = Form("%s_frac_tracker",GetName()) ;
  RooChangeTracker* tracker = new RooChangeTracker(trackerName.c_str(),trackerName.c_str(),_parList,kTRUE) ;

  // Store it in the cache
  cache = new CacheElem(*theSumPdf,*tracker,fracl) ;
  _cacheMgr.setObj(0,0,cache,0) ;

  //const_cast<RooStarMomentMorph*>(this)->addServer(*theSumPdf,kFALSE,kFALSE) ;
  coutI(InputArguments) << "RooStarMorphPdf::getCache("<< GetName() << ") created cache element"<< endl ;

  return cache ;
}



//_____________________________________________________________________________
RooArgList RooStarMomentMorph::CacheElem::containedArgs(Action) 
{
  return RooArgList(*_sumPdf,*_tracker) ; 
}



//_____________________________________________________________________________
RooStarMomentMorph::CacheElem::~CacheElem() 
{ 
  delete _sumPdf ; 
  delete _tracker ; 
  //if (_owner) _owner->removeServer(*_sumPdf) ;
} 



//_____________________________________________________________________________
Double_t RooStarMomentMorph::getVal(const RooArgSet* set) const 
{
  // Special version of getVal() overrides RooAbsReal::getVal() to save value of current normalization set
  _curNormSet = set ? (RooArgSet*)set : (RooArgSet*)&_obsList ;
  return RooAbsPdf::getVal(set) ;
}



//_____________________________________________________________________________
RooAbsPdf* RooStarMomentMorph::sumPdf(const RooArgSet* nset) 
{
  CacheElem* cache = getCache(nset ? nset : _curNormSet) ;
  
  if (!cache->_fractionsCalculated || cache->_tracker->hasChanged(kTRUE)) {
    cache->calculateFractions(*this,kFALSE); // verbose turned off
  } 
  
  return cache->_sumPdf ;
}


//_____________________________________________________________________________
Double_t RooStarMomentMorph::evaluate() const 
{ 
  CacheElem* cache = getCache(_curNormSet) ;

  if (!cache->_fractionsCalculated || cache->_tracker->hasChanged(kTRUE)) {
    cache->calculateFractions(*this,kFALSE); // verbose turned off
  } 

  Double_t ret = cache->_sumPdf->getVal(_pdfList.nset());
  if (ret<0.) ret=0.;
  /*cout<<"(RSMM) "<<this->GetName();
  _parItr->Reset();  
  for (Int_t j=0; j<_parList.getSize(); j++) {    
    RooRealVar* m = (RooRealVar*)(_parItr->Next());
    cout<<", par="<<m->getVal();
  }

  cout<<", m4l="<<_obsList.getRealValue("m4l");
  cout<<":  "<<ret<<endl;
  //exit(3);
  */

  return ret ;
}


//_____________________________________________________________________________
RooRealVar* RooStarMomentMorph::CacheElem::frac(Int_t i ) 
{ 
  return (RooRealVar*)(_frac.at(i))  ; 
}



//_____________________________________________________________________________
const RooRealVar* RooStarMomentMorph::CacheElem::frac(Int_t i ) const 
{ 
  return (RooRealVar*)(_frac.at(i))  ; 
}


//_____________________________________________________________________________
void RooStarMomentMorph::CacheElem::calculateFractions(const RooStarMomentMorph& self, Bool_t /* verbose */) const
{
  _fractionsCalculated=true;

  switch (self._setting) {
  case Linear:
  case SineLinear:
    {
      //int nObs=self._obsList.getSize();
      
      // loop over parList
      self._parItr->Reset();
      int nnuis=0;
      
      // zero all fractions
      int nPdf=self._pdfList.getSize();	
      for (Int_t i=0; i<3*nPdf; ++i) {
	double initval=(i==nPdf-1||i==2*nPdf-1||i==3*nPdf-1) ? 1. : 0.;
	((RooRealVar*)frac(i))->setVal(initval);      
      }
      for (Int_t j=0; j<self._parList.getSize(); j++) {
	
	RooRealVar* m = (RooRealVar*)(self._parItr->Next());
	double m0=m->getVal();
	
	if (m0==0.) continue;

	int imin = self.ijlo(j,m0);
	int imax = self.ijhi(j,m0);
	
	double mlo=self._nref[imin];
	double mhi=self._nref[imax];
	
	// get reference for this obs
	nnuis+=self._nnuis[j];
	       
	double mfrac = (imax>imin) ? (mhi-m0)/(mhi-mlo) : (mlo-m0)/(mhi-mlo);
	if (mfrac> 1.) mfrac= 1.;
	if (mfrac<-1.) mfrac=-1.;
	
	if (self._setting==SineLinear) {
	  mfrac = TMath::Sin( TMath::PiOver2()*mfrac ); // this gives a continuous differentiable transition between grid points. 
	}

	((RooRealVar*)frac(imin))->setVal(((RooRealVar*)frac(imin))->getVal()+mfrac); 
	((RooRealVar*)frac(nPdf+imin))->setVal(((RooRealVar*)frac(nPdf+imin))->getVal()+mfrac); 
	((RooRealVar*)frac(2*nPdf+imin))->setVal(((RooRealVar*)frac(2*nPdf+imin))->getVal()+mfrac); 

	((RooRealVar*)frac(imax))->setVal(((RooRealVar*)frac(imax))->getVal()-mfrac); 
	((RooRealVar*)frac(nPdf+imax))->setVal(((RooRealVar*)frac(nPdf+imax))->getVal()-mfrac); 
	((RooRealVar*)frac(2*nPdf+imax))->setVal(((RooRealVar*)frac(2*nPdf+imax))->getVal()-mfrac); 	
      }

      break;
    }
  default:
    {
      cerr << "RooStarMomentMorph::calculateFractions() ERROR: only Linear Setting implemented!" << endl;
      throw string("RooStarMomentMorph::calculateFractions() ERROR only Linear Setting implemented") ;   
      break;
    }
  }
  
  //// MB : As an example: old left-over code. 
  

  //   Int_t nPdf = self._pdfList.getSize();
  
  //   // HACK: for now, for code to compile
  //   Double_t selfm = 0.0;
  
  //   Double_t dm = selfm - (*self._nref)[0];
  
  //   // fully non-linear
  //   double sumposfrac=0.;
  //   for (Int_t i=0; i<nPdf; ++i) {
  //     double ffrac=0.;
  //     for (Int_t j=0; j<nPdf; ++j) { ffrac += (*self._M)(j,i) * (j==0?1.:TMath::Power(dm,(double)j)); }
  //     if (ffrac>=0) sumposfrac+=ffrac;
  //     // fractions for pdf
  //     ((RooRealVar*)frac(i))->setVal(ffrac);
  //     // fractions for rms and mean
  //     ((RooRealVar*)frac(nPdf+i))->setVal(ffrac);
  //     if (verbose) { cout << ffrac << endl; }
  //   }
  
  //   // various mode settings
  //   int imin = self.idxmin(selfm);
  //   int imax = self.idxmax(selfm);
  //   double mfrac = (selfm-(*self._nref)[imin])/((*self._nref)[imax]-(*self._nref)[imin]);
  //   switch (self._setting) {
  //     case NonLinear:
  //       // default already set above
  //     break;
  //     case Linear: 
  //       for (Int_t i=0; i<2*nPdf; ++i)
  //         ((RooRealVar*)frac(i))->setVal(0.);      
  //       if (imax>imin) { // m in between mmin and mmax
  //         ((RooRealVar*)frac(imin))->setVal(1.-mfrac); 
  //         ((RooRealVar*)frac(nPdf+imin))->setVal(1.-mfrac);
  //         ((RooRealVar*)frac(imax))->setVal(mfrac);
  //         ((RooRealVar*)frac(nPdf+imax))->setVal(mfrac);
  //       } else if (imax==imin) { // m outside mmin and mmax
  //         ((RooRealVar*)frac(imin))->setVal(1.);
  //         ((RooRealVar*)frac(nPdf+imin))->setVal(1.);
  //       }
  //     break;
  //     case NonLinearLinFractions:
  //       for (Int_t i=0; i<nPdf; ++i)
  //         ((RooRealVar*)frac(i))->setVal(0.);
  //       if (imax>imin) { // m in between mmin and mmax
  //         ((RooRealVar*)frac(imin))->setVal(1.-mfrac);
  //         ((RooRealVar*)frac(imax))->setVal(mfrac);
  //       } else if (imax==imin) { // m outside mmin and mmax
  //         ((RooRealVar*)frac(imin))->setVal(1.);
  //       }
  //     break;
  //     case NonLinearPosFractions:
  //       for (Int_t i=0; i<nPdf; ++i) {
  //         if (((RooRealVar*)frac(i))->getVal()<0) ((RooRealVar*)frac(i))->setVal(0.);
  //         ((RooRealVar*)frac(i))->setVal(((RooRealVar*)frac(i))->getVal()/sumposfrac);
  //       }
  //     break;
  //   }
  

}



//_____________________________________________________________________________
Int_t RooStarMomentMorph::ij(const Int_t& i, const Int_t& j) const
{
  // i = nuisance parameter
  // j = variation number

  if (i<0 || i>=static_cast<int>(_nnuis.size())) return -1;
  if (j<0 || j>=_nnuis[i]) return -1;

  Int_t idx = 0;  
  for (int k=0; k<i; ++k) { idx += _nnuis[k]; }
  idx += j;

  return idx;
}


//_____________________________________________________________________________
Int_t RooStarMomentMorph::ijhi(const Int_t& i, const double& nval) const
{
  // find higher index on right side of nval
  // if lo==hi, pass the next-to highest index in line

  // i = nuisance parameter
  if (i<0 || i>=static_cast<int>(_nnuis.size())) return -1;

  // initialize
  int ijxlo = ij(i,0);
  double nlo=_nref[ijxlo];
  if ( _nref[_nnuisvar]<nlo ) { ijxlo=_nnuisvar; }

  int ijxhi = ij(i,_nnuis[i]-1);
  double nhi=_nref[ijxhi];
  if (_nref[_nnuisvar]>nhi ) { 
    ijxhi=_nnuisvar; 
    nhi = _nref[_nnuisvar];
  }

  int ijxhiprev = ij(i,0);
  for (Int_t j=0; j<_nnuis[i]; ++j) {
    int k = ij(i,j);
    if ( _nref[k]>=nval && _nref[k]<nhi ) { 
      nhi=_nref[k]; ijxhi=k;
    } else {
      ijxhiprev=k;
      break;
    }
  }
  if ( _nref[_nnuisvar]<nhi && _nref[_nnuisvar]>=nval) { 
    ijxhiprev = ijxhi;    
    ijxhi=_nnuisvar; 
  }
  if ( ijxhi!=static_cast<int>(_nnuisvar) && _nref[_nnuisvar]<_nref[ijxhiprev] && _nref[_nnuisvar]>=_nref[ijxhi] ) {
    ijxhiprev = _nnuisvar;
  }

  // if lo==hi, pass the next hi in line
  return ( ijxhi!=ijxlo ? ijxhi : ijxhiprev );
}


//_____________________________________________________________________________
Int_t RooStarMomentMorph::ijlo(const Int_t& i, const double& nval) const
{

  // find lower index on left side of nval
  // if lo==hi, pass the next-to lower index in line

  // i = nuisance parameter
  if (i<0 || i>=static_cast<int>(_nnuis.size())) return -1;

  // initialize
  int ijxhi = ij(i,_nnuis[i]-1);
  
  double nhi=_nref[ijxhi];

  if ( _nref[_nnuisvar]>nhi ) { ijxhi=_nnuisvar; }

  int ijxlo = ij(i,0);

  double nlo= _nref[ijxlo];

  if ( _nref[_nnuisvar]<nlo ) { 
    ijxlo=_nnuisvar; 
    nlo = _nref[_nnuisvar];
  }

  int ijxloprev = _nnuis[i]-1;

  for (Int_t j=_nnuis[i]-1; j>=0; --j) {
    int k = ij(i,j);
    if ( _nref[k]>nlo && _nref[k]<=nval ) { 
      nlo=_nref[k]; ijxlo=k; 
    } else {
      ijxloprev=k;
      break;
    }
  }
  if ( _nref[_nnuisvar]>nlo && _nref[_nnuisvar]<=nval ) { 
    ijxloprev = ijxlo;
    ijxlo=_nnuisvar; 
  }
  if ( ijxlo!=static_cast<int>(_nnuisvar) && _nref[_nnuisvar]>_nref[ijxloprev] && _nref[_nnuisvar]<=_nref[ijxlo] ) {
    ijxloprev = _nnuisvar;
  }

  return ( ijxlo!=ijxhi ? ijxlo : ijxloprev  );
}

//_____________________________________________________________________________
Bool_t RooStarMomentMorph::setBinIntegrator(RooArgSet& allVars) 
{

  if(allVars.getSize()==1){
    RooAbsReal* temp = const_cast<RooStarMomentMorph*>(this);
    temp->specialIntegratorConfig(kTRUE)->method1D().setLabel("RooBinIntegrator")  ;
    int nbins = ((RooRealVar*) allVars.first())->numBins();
    temp->specialIntegratorConfig(kTRUE)->getConfigSection("RooBinIntegrator").setRealValue("numBins",nbins);
    return true;
  }else{
    cout << "Currently BinIntegrator only knows how to deal with 1-d "<<endl;
    return false;
  }
  return false;
}