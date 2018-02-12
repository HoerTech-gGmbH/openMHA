
qjackctl -s &
sleep 2
(
    cd ~/mha/mha/configurations/forschergruppe/demo/Binaural_Hearingaid/MHA-Configurations/
    xterm -e 'mha ?read:Jack_live_resampling_doasvm_4Chan_pool_acSteerMVDR_monaural.cfg;sleep 20' &
)
(
    sleep 2
    cd ~/mha/mha/configurations/forschergruppe/demo/Binaural_Hearingaid/Scripts
    source activate doasvm_demo
    ./steerBFpool_multiChannel.sh
)
