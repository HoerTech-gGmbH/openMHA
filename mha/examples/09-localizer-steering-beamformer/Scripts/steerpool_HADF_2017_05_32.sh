set -x

cd /home/mha/15-07-08_doa-mha-demonstrator/visualisation_web/

python mha_server.py -b firefox pseudo3d --classification-id svm --pooling-id pool --pool-path mha.mhachain.alt_Steer_BF.chain_Steer_BF.analysispath.plug.doachain.doasvm_mon.pool
