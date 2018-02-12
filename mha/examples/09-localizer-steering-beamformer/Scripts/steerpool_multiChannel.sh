set -x

cd /home/mha/15-07-08_doa-mha-demonstrator/visualisation_web/

python mha_server.py -b firefox pseudo3d --classification-id svm --pooling-id pool --pool-path mha.chain.doasvm_mon.pool
