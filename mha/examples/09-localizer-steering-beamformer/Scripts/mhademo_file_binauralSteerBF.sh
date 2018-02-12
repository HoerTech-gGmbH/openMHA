
qjackctl -s &
sleep 2
(
    cd ~/mha/mha/configurations/forschergruppe/demo/Binaural_Hearingaid/MHA-Configurations/
    xterm -e 'mha ?read:Jack_resampling_doasvm_4Chan_16K_pool_acSteerMVDR_binaural.cfg;sleep 20' &
)
(
    sleep 2
    cd ~/mha/mha/configurations/forschergruppe/demo/Binaural_Hearingaid/Scripts
    source activate doasvm_demo
    ./steerBFpool_file_multiChannel.sh
)
