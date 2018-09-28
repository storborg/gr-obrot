/* -*- c++ -*- */
/* 
 * Copyright 2018 gr-obrot author.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef INCLUDED_OBROT_POLYPHONIC_SYNTHESIZER_IMPL_H
#define INCLUDED_OBROT_POLYPHONIC_SYNTHESIZER_IMPL_H

#include <mutex>
#include <gnuradio/nco.h>
#include <obrot/polyphonic_synthesizer.h>

namespace gr {
  namespace obrot {

    class polyphonic_synthesizer_impl : public polyphonic_synthesizer
    {
      private:
      double d_samp_rate;
      int d_start_note;
      int d_end_note;
      float d_squelch;
      std::vector<gr::nco<float, float>> d_ncos;
      std::vector<float> d_note_mags;
      boost::mutex d_note_mx;

      const int MAX_NOTES = 8 * 12;

      double note_to_freq(int note_index);
      void configure_ncos();

      public:
      polyphonic_synthesizer_impl(double samp_rate, int start_octave, int end_octave);
      ~polyphonic_synthesizer_impl();

      void on_pdu(pmt::pmt_t msg);

      // Where all the action really happens
      int work(int noutput_items,
               gr_vector_const_void_star &input_items,
               gr_vector_void_star &output_items);
    };

  } // namespace obrot
} // namespace gr

#endif /* INCLUDED_OBROT_POLYPHONIC_SYNTHESIZER_IMPL_H */

