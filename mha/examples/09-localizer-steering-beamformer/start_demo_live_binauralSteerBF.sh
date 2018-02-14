# This script starts a live demo of a 4-channel binaural beamformer that is
# steered by a probabilistic sound source localization algorithm.
# Note that in this example configuration file a localization model and
# beamformer filter coefficients are given a according to a specific hearing aid
# setup.

# Start JACK server
# Make sure that JACK has been set up according to your hardware interface and
# the parameters 'srate', 'nchannels_in', 'fragsize' given in the configuration
# file loaded by openMHA below.
# Also have a look at the io.con_in and io.con_out settings
# The example file assumes a binaural hearing aid setup with two microphones on
# each side:
# io.con_in = [<front-left> <front-right> <rear-left> <rear-right>]
# io.con_out = [<left> <right>]
qjackctl -s &
sleep 2

# Start openMHA in xterm
# Read configuration file containing the whole setup
(
        xterm -e 'mha ?read:Jack_live_resampling_doasvm_4Chan_16K_pool_acSteerMVDR_binaural.cfg;sleep 20' &

)

# Start visualisation assuming that the visualisation framework has been setup
# as described in README_visualization.md
(
    sleep 2
    source activate doasvm_demo
    set -x
    cd ./visualisation_web/

    python mha_server.py -b firefox pseudo3d --classification-id svm --pooling-id pool --pool-path mha.doachain.doasvm_mon.pool
)
