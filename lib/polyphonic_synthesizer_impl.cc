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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cmath>
#include <gnuradio/io_signature.h>
#include <volk/volk.h>
#include "polyphonic_synthesizer_impl.h"

namespace gr {
  namespace obrot {

    polyphonic_synthesizer::sptr
    polyphonic_synthesizer::make(double samp_rate, int start_octave, int end_octave)
    {
      return gnuradio::get_initial_sptr
        (new polyphonic_synthesizer_impl(samp_rate, start_octave, end_octave));
    }

    /*
     * The private constructor
     */
    polyphonic_synthesizer_impl::polyphonic_synthesizer_impl(double samp_rate, int start_octave, int end_octave)
      : gr::sync_block("polyphonic_synthesizer",
              gr::io_signature::make(0, 0, 0),
              gr::io_signature::make(1, 1, sizeof(float))),
      d_samp_rate(samp_rate),
      d_start_note(start_octave * 12), d_end_note((1 + end_octave) * 12),
      d_squelch(0.0)
    {
      const int alignment_multiple = volk_get_alignment() / sizeof(float);
      set_alignment(std::max(1, alignment_multiple));

      configure_ncos();
      d_note_mags.assign(MAX_NOTES, 0.0);

      message_port_register_in(pmt::mp("in"));
      set_msg_handler(pmt::mp("in"), boost::bind(&polyphonic_synthesizer_impl::on_pdu, this, _1));
    }

    /*
     * Our virtual destructor.
     */
    polyphonic_synthesizer_impl::~polyphonic_synthesizer_impl()
    {
    }

    double
    polyphonic_synthesizer_impl::note_to_freq(int note_index)
    {
      double f0 = 16.352;
      double a = std::pow(2.0, 1.0 / 12.0);
      return f0 * std::pow(a, note_index);
    }

    void
    polyphonic_synthesizer_impl::configure_ncos()
    {
      d_ncos.clear();

      for(int note_index; note_index < MAX_NOTES; note_index++) {
        double freq = note_to_freq(note_index);
        gr::nco<float,float> nco;
        nco.set_freq(2 * M_PI * freq / d_samp_rate);
        d_ncos.push_back(nco);
      }
    }

    void
    polyphonic_synthesizer_impl::on_pdu(pmt::pmt_t msg)
    {
      if(!pmt::is_pair(msg)) {
        throw std::runtime_error("Received a malformed PDU, must be a pair");
      }

      pmt::pmt_t vec = pmt::cdr(msg);

      if(!pmt::is_f32vector(vec)) {
        throw std::runtime_error("PDU vector must be uniform float32");
      }

      int len = pmt::length(vec);
      int note_count = d_end_note - d_start_note;
      if(len < note_count) {
        throw std::runtime_error("Magnitude vector is less than number of available notes");
      }

      int binning = std::ceil((double) len / note_count);

      {
        boost::unique_lock<boost::mutex> lock(d_note_mx);
        d_note_mags.assign(MAX_NOTES, 0.0);

        for(int ii = 0; ii < len; ii++) {
          int note_index = d_start_note + (int) std::floor(ii / binning);
          d_note_mags[note_index] += pmt::f32vector_ref(vec, ii);
        }
        //std::cout << "setting note mags to" << std::endl;
        float max = 0.0;
        for(int note_index = 0; note_index < MAX_NOTES; note_index++) {
          float val = d_note_mags[note_index];
          if(val > max) {
            max = val;
          }
          //std::cout << "  " << note_index << " -> " << val << std::endl;
        }
        //std::cout << "done setting note mags" << std::endl;
        //std::cout << "max " << max << std::endl;

        float desired_peak = 0.05;
        // Steeper equalizer slope means treble will be attenuated more
        float equalizer_slope = 0.5;
        if(max > desired_peak) {
          float scale = desired_peak / max;

          //std::cout << "normalizing note mags" << std::endl;
          for(int note_index = 0; note_index < MAX_NOTES; note_index++) {
            float equalize_factor = 1.0 / (1.0 + (note_index * equalizer_slope));
            d_note_mags[note_index] *= (scale * equalize_factor);
          }
          //std::cout << "done normalizing note mags" << std::endl;
        }
      }
    }

    int
    polyphonic_synthesizer_impl::work(int noutput_items,
                                      gr_vector_const_void_star &input_items,
                                      gr_vector_void_star &output_items)
    {
      float *out = (float *) output_items[0];

      std::vector<float> tonebuf;
      tonebuf.reserve(noutput_items);

      // Zero the output buffer
      std::fill(out, out + noutput_items, 0.0);

      // Iterate through each note
      for(int note_index = d_start_note; note_index < d_end_note; note_index++) {
        float mag = d_note_mags[note_index];

        // Only emit notes that are above the squelch
        if(mag > 0.0) {
          // Generate the pure tone at appropriate magnitude
          d_ncos[note_index].sin(tonebuf.data(), noutput_items, mag);
          // Mix it to the output signal
          volk_32f_x2_add_32f(out, out, (const float *) tonebuf.data(), noutput_items);
        }
      }

      return noutput_items;
    }

  } /* namespace obrot */
} /* namespace gr */

