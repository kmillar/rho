/*CXXR $Id$
 *CXXR
 *CXXR This file is part of CXXR, a project to refactor the R interpreter
 *CXXR into C++.  It may consist in whole or in part of program code and
 *CXXR documentation taken from the R project itself, incorporated into
 *CXXR CXXR (and possibly MODIFIED) under the terms of the GNU General Public
 *CXXR Licence.
 *CXXR 
 *CXXR CXXR is Copyright (C) 2008 Andrew R. Runnalls, subject to such other
 *CXXR copyrights and copyright restrictions as may be stated below.
 *CXXR 
 *CXXR CXXR is not part of the R project, and bugs and other issues should
 *CXXR not be reported via r-bugs or other R project channels; instead refer
 *CXXR to the CXXR website.
 *CXXR */

/*
 *  R : A Computer Langage for Statistical Data Analysis
 *  Copyright (C) 1995, 1996  Robert Gentleman and Ross Ihaka
 *  Copyright (C) 2001-4  The R Development Core Team
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

/* <UTF8> char here is either ASCII or handled as a whole */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "Defn.h"

using namespace CXXR;

/*  append - append second to the tail of first    */
/*           This operation is non-destructive     */
/*           i.e. first and second are duplicated  */

#ifdef UNUSED
SEXP Rf_append(SEXP first, SEXP second)
{
    SEXP e;

    PROTECT(second);
    first = duplicate(first);
    UNPROTECT(1);
    PROTECT(first);
    second = duplicate(second);
    UNPROTECT(1);
    for (e = first; CDR(e) != R_NilValue; e = CDR(e));
    SETCDR(e, second);
    return first;
}
#endif


/* This is called by function() {}, where an invalid
   body should be impossible. When called from 
   other places (eg do_asfunction) they 
   should do this checking in advance */

/*  mkCLOSXP - return a closure with formals f,  */
/*             body b, and environment rho       */

SEXP attribute_hidden mkCLOSXP(SEXP formals, SEXP body, SEXP rho)
{
    SEXP c;
    PROTECT(formals);
    PROTECT(body);
    PROTECT(rho);
    c = new RObject(CLOSXP);

#ifdef not_used_CheckFormals
    if(isList(formals))
	SET_FORMALS(c, formals);
    else
        error(_("invalid formal arguments for \"function\""));
#else
    SET_FORMALS(c, formals);
#endif
    if(isList(body) || isLanguage(body) || isSymbol(body)
       || isExpression(body) || isVector(body)
#ifdef BYTECODE
       || isByteCode(body)
#endif
       )
	SET_BODY(c, body);
    else
        error(_("invalid body argument for \"function\"\n\
Should NEVER happen; please bug.report() [mkCLOSXP]"));

    if(rho == R_NilValue)
	SET_CLOENV(c, R_GlobalEnv);
    else
	SET_CLOENV(c, rho);
    UNPROTECT(3);
    return c;
}

/* mkChar - make a character (CHARSXP) variable -- see Rinlinedfuns.h */

/*  mkSYMSXP - return a symsxp with the string  */
/*             name inserted in the name field  */

static int isDDName(const String* name)
{
    const char *buf;
    char *endp;
    long val;

    buf = name->c_str();
    if( !strncmp(buf, "..", 2) && strlen(buf) > 2 ) {
        buf += 2;
	val = strtol(buf, &endp, 10);
        if( *endp != '\0')
	    return 0;
	else
	    return 1;
    }
    return 0;
}

Symbol::Symbol(const String* name, RObject* val,
	       const BuiltInFunction* internal_func)
    : RObject(SYMSXP), m_name(name), m_value(val),
      m_internalfunc(internal_func)
{
    if (name && isDDName(name))
	m_gpbits |= DDVAL_MASK;
}
