
qjackctl -s &
sleep 2
(
    cd ~/mha/mha/configurations/forschergruppe/demo/Binaural_Hearingaid/MHA-Configurations/
    xterm -e 'mha ?read:Jack_live_resampling_doasvm_4Chan_16K_pool_acSteerMVDR_binaural.cfg;sleep 20' &
    # xterm -e 'mha ?read:Jack_live_analysispath_doasvm_4Chan_32K_acSteerMVDR_binaural.cfg;sleep 20' &
)
(
    sleep 2
    cd ~/mha/mha/configurations/forschergruppe/demo/Binaural_Hearingaid/Scripts
    source activate doasvm_demo
    ./steerBFpool_16K_multiChannel.sh
    # ./steerBFpool_multiChannel.sh
)
