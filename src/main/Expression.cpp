/*
 *  R : A Computer Language for Statistical Data Analysis
 *  Copyright (C) 2008-2014  Andrew R. Runnalls.
 *  Copyright (C) 2014 and onwards the CXXR Project Authors.
 *
 *  CXXR is not part of the R project, and bugs and other issues should
 *  not be reported via r-bugs or other R project channels; instead refer
 *  to the CXXR website.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, a copy is available at
 *  http://www.r-project.org/Licenses/
 */

/** @file Expression.cpp
 *
 * @brief Class CXXR::Expression and associated C interface.
 */

#include "CXXR/Expression.h"

#include <iostream>
#include "R_ext/Error.h"
#include "localization.h"
#include "CXXR/ArgList.hpp"
#include "CXXR/Environment.h"
#include "CXXR/Evaluator.h"
#include "CXXR/FunctionBase.h"
#include "CXXR/GCStackRoot.hpp"
#include "CXXR/StackChecker.hpp"
#include "CXXR/Symbol.h"

using namespace std;
using namespace CXXR;

// Force the creation of non-inline embodiments of functions callable
// from C:
namespace CXXR {
    namespace ForceNonInline {
	SEXP (*lconsp)(SEXP cr, SEXP tl) = Rf_lcons;
   }
}

GCRoot<> R_CurrentExpr;

Expression* Expression::clone() const
{
    return new Expression(*this);
}

RObject* Expression::evaluate(Environment* env)
{
    IncrementStackDepthScope scope;

    GCStackRoot<FunctionBase> func;
    RObject* head = car();
    if (head->sexptype() == SYMSXP) {
	Symbol* symbol = static_cast<Symbol*>(head);
	func = findFunction(symbol, env);
	if (!func)
	    error(_("could not find function \"%s\""),
		  symbol->name()->c_str());
    } else {
	RObject* val = Evaluator::evaluate(head, env);
	if (!FunctionBase::isA(val))
	    error(_("attempt to apply non-function"));
	func = static_cast<FunctionBase*>(val);
    }
    func->maybeTrace(this);
    ArgList arglist(tail(), ArgList::RAW);
    return func->apply(&arglist, env, this);
}

const char* Expression::typeName() const
{
    return staticTypeName();
}

BOOST_CLASS_EXPORT_IMPLEMENT(CXXR::Expression)

// ***** C interface *****

SEXP Rf_currentExpression()
{
    return R_CurrentExpr;
}

SEXP Rf_lcons(SEXP cr, SEXP tl)
{
    GCStackRoot<> crr(cr);
    GCStackRoot<PairList> tlr(SEXP_downcast<PairList*>(tl));
    return new Expression(crr, tlr);
}

void Rf_setCurrentExpression(SEXP e)
{
    R_CurrentExpr = e;
}
