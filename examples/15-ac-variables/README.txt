This directory contains examples on AC variable usage.

The openMHA configuration file acvars_simple_example.cfg loads a signal
processing chain containing two rmslevel plugins and a gain plugin. The two
rmslevel plugins measure the signal level before and after the gain plugin.

The gain plugin is configured to amplify the right channel and leave the left
channel unmodified.

Both rmslevel plugins, which are loaded with configured names L1 and L2 produce
AC vars (see openMHA_application_manual.pdf, section 2.2) containing the
measured sound levels.  To inspect what AC variables are created by a plugin,
invoke analysemhaplugin with the plugin name as parameter, in this case

   analysemhaplugin rmslevel

and read the first approx. 20 lines of the generated output, example:

-- waveform processing ---------------------------
empty AC space before prepare.
Successfully prepared for waveform processing.
AC variables after prepare:
  rmslevel_level
  rmslevel_level_db
  rmslevel_peak
  rmslevel_peak_db

The level in db is stored in the variable which, by default, is named
rmslevel_level_db.  The configuration in acvars_simple_example.cfg is loading
the rmslevel plugins with configured names L1 and L2, so the names of the
AC variables in our use case should be L1_level_db and L2_level_db.  This
can be verified by invoking analysemhaplugin with the configured name:

analysemhaplugin rmslevel:L1

-- waveform processing ---------------------------
empty AC space before prepare.
Successfully prepared for waveform processing.
AC variables after prepare:
  L1_level
  L1_level_db
  L1_peak
  L1_peak_db

The example configuration file then prints, after processing, the levels in
both audio channels before and after the gain application:

mha "?read:acvars_simple_example.cfg"

# 
# vector<float> (monitor)
[6.5034132 5.51731634]
# 
# vector<float> (monitor)
[6.5034132 11.5173197]

The level in the left channel was not modified by the gain plugin, but the
level in the right channel was increased by 6 dB as expected.  Please refer
to the configuration file for details.
