// @(#)root/base:$Name:  $:$Id: RtypesImp.h,v 1.12 2002/11/11 11:27:47 brun Exp $
// Author: Philippe Canal   23/2/02

/*************************************************************************
 * Copyright (C) 1995-2002, Rene Brun, Fons Rademakers and al.           *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_RtypesImp
#define ROOT_RtypesImp

#ifndef G__DICTIONARY
#error RtypesImp.h should only be included by ROOT dictionaries.
#endif

#include "Api.h"

namespace ROOT {
   inline void GenericShowMembers(const char *topClassName,
                                  void *obj, TMemberInspector &R__insp,
                                  char *R__parent,
                                  bool transientMember)
   {
      // This could be faster if we implemented this either as a templated
      // function or by rootcint-generated code using the typeid (i.e. the
      // difference is a lookup in a TList instead of in a map).

      // To avoid a spurrious error message in case the data member is
      // transient and does not have a dictionary we check first.
      if (transientMember) {
         G__ClassInfo b(topClassName);
         if (!b.IsLoaded()) return;
      }

      TClass *top = gROOT->GetClass(topClassName);
      if (top) {
         ShowMembersFunc_t show = top->GetShowMembersWrapper();
         if (show) show(obj, R__insp, R__parent);
      }
   }

  class operatorNewHelper { };
}

// This is to provide a placement operator new on all platforms
inline void *operator new(size_t size, ROOT::operatorNewHelper *p) {
   return((void*)p);
}

#endif
