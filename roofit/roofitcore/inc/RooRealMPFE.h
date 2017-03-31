/*****************************************************************************
 * Project: RooFit                                                           *
 * Package: RooFitCore                                                       *
 *    File: $Id: RooRealMPFE.h,v 1.7 2007/05/11 09:11:30 verkerke Exp $
 * Authors:                                                                  *
 *   WV, Wouter Verkerke, UC Santa Barbara, verkerke@slac.stanford.edu       *
 *   DK, David Kirkby,    UC Irvine,         dkirkby@uci.edu                 *
 *                                                                           *
 * Copyright (c) 2000-2005, Regents of the University of California          *
 *                          and Stanford University. All rights reserved.    *
 *                                                                           *
 * Redistribution and use in source and binary forms,                        *
 * with or without modification, are permitted according to the terms        *
 * listed in LICENSE (http://roofit.sourceforge.net/license.txt)             *
 *****************************************************************************/
#ifndef ROO_REAL_MPFE
#define ROO_REAL_MPFE

#include "RooAbsReal.h"
#include "RooRealProxy.h"
#include "RooListProxy.h"
#include "RooArgList.h"
#include "RooMPSentinel.h"
#include "TStopwatch.h"
#include <vector>
#include <iostream>
#include <string>

class RooArgSet ;
namespace RooFit { class BidirMMapPipe; }

class RooRealMPFE : public RooAbsReal {
public:
  // Constructors, assignment etc
  RooRealMPFE(const char *name, const char *title, RooAbsReal& arg, Bool_t calcInline=kFALSE) ;
  RooRealMPFE(const RooRealMPFE& other, const char* name=0);
  virtual TObject* clone(const char* newname) const { return new RooRealMPFE(*this,newname); }
  virtual ~RooRealMPFE();

  void calculate() const ;
  virtual Double_t getValV(const RooArgSet* nset=0) const ;
  void standby() ;

  void setVerbose(Bool_t clientFlag=kTRUE, Bool_t serverFlag=kTRUE) ;

  void applyNLLWeightSquared(Bool_t flag) ;

  void enableOffsetting(Bool_t flag) ;

  void followAsSlave(RooRealMPFE& master) { _updateMaster = &master ; }
  
  protected:

  // Function evaluation
  virtual Double_t evaluate() const ;
  friend class RooAbsTestStatistic ;
  friend class RooTestStatMPDriver ;
  virtual void constOptimizeTestStatistic(ConstOpCode opcode, Bool_t doAlsoTracking=kTRUE) ;
  virtual Double_t getCarry() const;

  enum State { Initialize,Client,Server,Inline } ;
  State _state ;

  enum Message { SendReal=0, SendCat, Calculate, Retrieve, ReturnValue, Terminate, 
    ConstOpt, Verbose, LogEvalError, ApplyNLLW2, EnableOffset, CalculateNoOffset,
    SetCpuAffinity,
    EnableTimingRATS, DisableTimingRATS,
    EnableTimingNamedAbsArg, DisableTimingNamedAbsArg,
    MeasureCommunicationTime,
    RetrieveTimings
  };

  std::ostream& operator<<(std::ostream& out, const Message value){
    const char* s = 0;
#define PROCESS_VAL(p) case(p): s = #p; break;
    switch(value){
      PROCESS_VAL(SendReal);
      PROCESS_VAL(SendCat);
      PROCESS_VAL(Calculate);
      PROCESS_VAL(Retrieve);
      PROCESS_VAL(ReturnValue);
      PROCESS_VAL(Terminate);
      PROCESS_VAL(ConstOpt);
      PROCESS_VAL(Verbose);
      PROCESS_VAL(LogEvalError);
      PROCESS_VAL(ApplyNLLW);
      PROCESS_VAL(EnableOffset);
      PROCESS_VAL(CalculateNoOffset);
      PROCESS_VAL(SetCpuAffinity);
      PROCESS_VAL(EnableTimingRATS);
      PROCESS_VAL(DisableTimingRATS);
      PROCESS_VAL(EnableTimingNamedAbsArg);
      PROCESS_VAL(DisableTimingNamedAbsArg);
      PROCESS_VAL(MeasureCommunicationTime);
      PROCESS_VAL(RetrieveTimings);
      default: {
        s = "unknown Message!";
        break;
      }
    }
#undef PROCESS_VAL

    return out << s;
  }

  void initialize() ; 
  void initVars() ;
  void serverLoop() ;

  void doApplyNLLW2(Bool_t flag) ;

  RooRealProxy _arg ; // Function to calculate in parallel process
  RooListProxy _vars ;   // Variables
  RooArgList _saveVars ;  // Copy of variables
  mutable Bool_t _calcInProgress ;
  Bool_t _verboseClient ;
  Bool_t _verboseServer ;
  Bool_t _inlineMode ;
  mutable Bool_t _forceCalc ;
  mutable RooAbsReal::ErrorLoggingMode _remoteEvalErrorLoggingState ;

  RooFit::BidirMMapPipe *_pipe; //! connection to child

  mutable std::vector<Bool_t> _valueChanged ; //! Flags if variable needs update on server-side
  mutable std::vector<Bool_t> _constChanged ; //! Flags if variable needs update on server-side
  RooRealMPFE* _updateMaster ; //! Update master
  mutable Bool_t _retrieveDispatched ; //!
  mutable Double_t _evalCarry; //!

  static RooMPSentinel _sentinel ;

  void setCpuAffinity(int cpu);

private:
  RooArgSet* _components = 0;
  RooAbsArg* _findComponent(std::string name);

  RooArgSet _numIntSet;

  void _initNumIntSet(const RooArgSet& obs);
  void _setTimingNumIntSet(Bool_t flag = kTRUE);

  std::map<std::string, double> collectTimingsFromServer() const;

  void _time_communication_overhead() const;

  ClassDef(RooRealMPFE,2) // Multi-process front-end for parallel calculation of a real valued function
};

#endif
