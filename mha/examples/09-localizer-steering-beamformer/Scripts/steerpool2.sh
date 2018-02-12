set -x

cd /home/mha/DOA_SVM_Visualisation/15-07-08_doa-mha-demonstrator/visualisation_web/

python mha_server.py -b firefox 2d --classification-id svm2 --pooling-id Pooling2 --pool-path mha.chain.doasvm_mon.pool2 --ws-port 8887
