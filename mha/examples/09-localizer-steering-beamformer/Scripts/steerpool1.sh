set -x

cd /home/mha/DOA_SVM_Visualisation/15-07-08_doa-mha-demonstrator/visualisation_web/

python mha_server.py -b firefox pseudo3d --classification-id svm1 --pooling-id Pooling1 --pool-path mha.chain.doasvm_mon.pool1
