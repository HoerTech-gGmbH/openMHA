
qjackctl -s &
sleep 2
(
        xterm -e 'mha ?read:Jack_live_resampling_doasvm_4Chan_16K_pool_acSteerMVDR_binaural.cfg;sleep 20' &

)
(
    sleep 2
    source activate doasvm_demo
    set -x
    cd ./visualisation_web/

    python mha_server.py -b firefox pseudo3d --classification-id svm --pooling-id pool --pool-path mha.doachain.doasvm_mon.pool
)
