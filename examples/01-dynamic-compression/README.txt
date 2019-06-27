This is a minimal example on how to use the openMHA to apply dynamic range compression on audio
input.

The file dynamiccompression.cfg shows how to do file to file processing with the MHAIOFile plugin,
dynamiccompression_live.cfg, example_dc_live.cfg and example_dc_live_double.cfg demonstrate usage of the MHAIOJack plugin,
the latter with the added complexity of double buffering between the Jack audio server and the audio processing.

The configuration files are extensively documented. For more information on how to start the openMHA and
set up audio processing please refer to the openMHA starting guide (openMHA_starting_guide.pdf)
and the plugin manual (openMHA_plugins.pdf).
