/* -*- c++ -*- */

#define OBROT_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "obrot_swig_doc.i"

%{
#include "obrot/polyphonic_synthesizer.h"
%}


%include "obrot/polyphonic_synthesizer.h"
GR_SWIG_BLOCK_MAGIC2(obrot, polyphonic_synthesizer);
