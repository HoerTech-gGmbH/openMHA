
qjackctl -s &
sleep 2
(
    cd ~/mha/mha/configurations/forschergruppe/demo/Binaural_Hearingaid/MHA-Configurations/
    xterm -e 'mha ?read:Jack_live_resampling_doasvm_4Chan_pool_bte.cfg;sleep 20' &
)
(
    sleep 2
    cd ~/mha/mha/configurations/forschergruppe/demo/BinauralHearingaid/Scripts
    source activate doasvm_demo
    ./steerpool_multiChannel.sh
)
