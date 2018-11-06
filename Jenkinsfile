pipeline {
    agent {label "jenkinsmaster"}
    stages {
        stage("build") {
            parallel {
                stage("bionic && x86_64") {
                    agent {label "bionic && x86_64"}
                    steps {
                        checkout([$class: 'GitSCM', branches: [[name: 'development']], browser: [$class: 'Phabricator', repo: 'OMD', repoUrl: 'https://dev.openmha.org/'], doGenerateSubmoduleConfigurations: false, extensions: [[$class: 'CleanCheckout']], submoduleCfg: [], userRemoteConfigs: [[url: 'ssh://git@mha.physik.uni-oldenburg.de/openMHA']]])
                        sh "./configure"
                        sh "make install unit-tests deb"
                        retry(3){sh "make -C mha/mhatest"}
                        stash name: 'x86_64_bionic', includes: 'mha/tools/packaging/deb/hoertech/'
                        sh 'ls -lR mha/tools/packaging/deb/hoertech/'
                    }
                }
                stage("bionic && i686") {
                    agent {label "bionic && i686"}
                    steps {
                        checkout([$class: 'GitSCM', branches: [[name: 'development']], browser: [$class: 'Phabricator', repo: 'OMD', repoUrl: 'https://dev.openmha.org/'], doGenerateSubmoduleConfigurations: false, extensions: [[$class: 'CleanCheckout']], submoduleCfg: [], userRemoteConfigs: [[url: GIT_URL]]])
                        sh "./configure"
                        sh "make install unit-tests deb"
                        retry(3){sh "make -C mha/mhatest"}
                        stash name: 'i686_bionic', includes: 'mha/tools/packaging/deb/hoertech/'
                        sh 'ls -lR mha/tools/packaging/deb/hoertech/'
                    }
                }
                stage("xenial && x86_64") {
                    agent {label "xenial && x86_64"}
                    steps {
                        checkout([$class: 'GitSCM', branches: [[name: 'development']], browser: [$class: 'Phabricator', repo: 'OMD', repoUrl: 'https://dev.openmha.org/'], doGenerateSubmoduleConfigurations: false, extensions: [[$class: 'CleanCheckout']], submoduleCfg: [], userRemoteConfigs: [[url: "$GIT_URL"]]])
                        sh "./configure"
                        sh "make install unit-tests deb"
                        retry(3){sh "make -C mha/mhatest"}
                        stash name: 'x86_64_xenial', includes: 'mha/tools/packaging/deb/hoertech/'
                        sh 'ls -lR mha/tools/packaging/deb/hoertech/'
                    }
                }
                stage("xenial && i686") {
                    agent {label "xenial && i686"}
                    steps {
                        checkout([$class: 'GitSCM', branches: [[name: 'development']], browser: [$class: 'Phabricator', repo: 'OMD', repoUrl: 'https://dev.openmha.org/'], doGenerateSubmoduleConfigurations: false, extensions: [[$class: 'CleanCheckout']], submoduleCfg: [], userRemoteConfigs: [[url: 'ssh://git@mha.physik.uni-oldenburg.de/openMHA']]])
                        sh "./configure"
                        sh "make install unit-tests deb"
                        retry(3){sh "make -C mha/mhatest"}
                        stash name: 'i686_xenial', includes: 'mha/tools/packaging/deb/hoertech/'
                        sh 'ls -lR mha/tools/packaging/deb/hoertech/'
                    }
                }
                stage("trusty && x86_64") {
                    agent {label "trusty && x86_64"}
                    steps {
                        checkout([$class: 'GitSCM', branches: [[name: 'development']], browser: [$class: 'Phabricator', repo: 'OMD', repoUrl: 'https://dev.openmha.org/'], doGenerateSubmoduleConfigurations: false, extensions: [[$class: 'CleanCheckout']], submoduleCfg: [], userRemoteConfigs: [[url: 'ssh://git@mha.physik.uni-oldenburg.de/openMHA']]])
                        sh "./configure --cxxstandard=c++11"
                        sh "make install unit-tests deb"
                        retry(3){sh "make -C mha/mhatest"}
                        stash name: 'x86_64_trusty', includes: 'mha/tools/packaging/deb/hoertech/'
                        sh 'ls -lR mha/tools/packaging/deb/hoertech/'
                    }
                }
                stage("trusty && i686") {
                    agent {label "trusty && i686"}
                    steps {
                        checkout([$class: 'GitSCM', branches: [[name: 'development']], browser: [$class: 'Phabricator', repo: 'OMD', repoUrl: 'https://dev.openmha.org/'], doGenerateSubmoduleConfigurations: false, extensions: [[$class: 'CleanCheckout']], submoduleCfg: [], userRemoteConfigs: [[url: 'ssh://git@mha.physik.uni-oldenburg.de/openMHA']]])
                        sh "./configure --cxxstandard=c++11"
                        sh "make install unit-tests deb"
                        retry(3){sh "make -C mha/mhatest"}
                        stash name: 'i686_trusty', includes: 'mha/tools/packaging/deb/hoertech/'
                        sh 'ls -lR mha/tools/packaging/deb/hoertech/'
                    }
                }
                stage("bionic && armv7") {
                    agent {label "bionic && armv7"}
                    steps {
                        checkout([$class: 'GitSCM', branches: [[name: 'development']], browser: [$class: 'Phabricator', repo: 'OMD', repoUrl: 'https://dev.openmha.org/'], doGenerateSubmoduleConfigurations: false, extensions: [[$class: 'CleanCheckout']], submoduleCfg: [], userRemoteConfigs: [[url: 'ssh://git@mha.physik.uni-oldenburg.de/openMHA']]])
                        sh "./configure"
                        sh "make install unit-tests deb"
                        retry(3){sh "make -C mha/mhatest"}
                        stash name: 'armv7_bionic', includes: 'mha/tools/packaging/deb/hoertech/'
                        sh 'ls -lR mha/tools/packaging/deb/hoertech/'
                    }
                }
                stage("xenial && armv7") {
                    agent {label "xenial && armv7"}
                    steps {
                        checkout([$class: 'GitSCM', branches: [[name: 'development']], browser: [$class: 'Phabricator', repo: 'OMD', repoUrl: 'https://dev.openmha.org/'], doGenerateSubmoduleConfigurations: false, extensions: [[$class: 'CleanCheckout']], submoduleCfg: [], userRemoteConfigs: [[url: 'ssh://git@mha.physik.uni-oldenburg.de/openMHA']]])
                        sh "./configure"
                        sh "make install unit-tests deb"
                        retry(3){sh "make -C mha/mhatest"}
                        stash name: 'armv7_xenial', includes: 'mha/tools/packaging/deb/hoertech/'
                        sh 'ls -lR mha/tools/packaging/deb/hoertech/'
                    }
                }
                stage("windows && x86_64") {
                    agent {label "windows && x86_64"}
                    steps {
                        checkout([$class: 'GitSCM', branches: [[name: 'development']], browser: [$class: 'Phabricator', repo: 'OMD', repoUrl: 'https://dev.openmha.org/'], doGenerateSubmoduleConfigurations: false, extensions: [[$class: 'CleanCheckout']], submoduleCfg: [], userRemoteConfigs: [[url: 'ssh://git@mha.physik.uni-oldenburg.de/openMHA']]])
                        sh "./configure"
                        sh "make install unit-tests"
                        retry(3){sh "make -C mha/mhatest"}
                    }
                }
                stage("mac && x86_64") {
                    agent {label "Darwin"}
                    steps {
                        checkout([$class: 'GitSCM', branches: [[name: 'development']], browser: [$class: 'Phabricator', repo: 'OMD', repoUrl: 'https://dev.openmha.org/'], doGenerateSubmoduleConfigurations: false, extensions: [[$class: 'CleanCheckout']], submoduleCfg: [], userRemoteConfigs: [[url: 'ssh://git@mha.physik.uni-oldenburg.de/openMHA']]])
                        sh "./configure"
                        sh "make install unit-tests"
                        retry(3){sh "make -C mha/mhatest"}
                    }
                }
            }
        }
        stage("publish") {
            agent {label "aptly"}
            steps {
                checkout([$class: 'GitSCM', branches: [[name: 'master']], doGenerateSubmoduleConfigurations: false, extensions: [[$class: 'CleanCheckout']], submoduleCfg: [], userRemoteConfigs: [[url: 'ssh://git@mha.physik.uni-oldenburg.de/openmha-aptly']]])

                sh "git remote -v"
                
                // receive all deb packages from openmha build
                unstash "x86_64_bionic"
                unstash "i686_bionic"
                unstash "x86_64_xenial"
                unstash "i686_xenial"
                unstash "x86_64_trusty"
                unstash "i686_trusty"
                unstash "armv7_bionic"
                unstash "armv7_xenial"
                
                // copy fresh packages to our stash of packages 
                sh "cp -anv mha/tools/packaging/deb/hoertech/* /packages/"
                
                // prepare the repository databases
                sh "./aptly-initialize-databases.sh"
                
                // Delete old packages.
                // Not yet implemented.
                
                // Fill aptly databases with packages
                sh "aptly repo add openMHA-bionic-development /packages/bionic/*"
                sh "aptly repo add openMHA-xenial-development /packages/xenial/*"
                sh "aptly repo add openMHA-trusty-development /packages/trusty/*"

                // Create snapshots
                sh "aptly snapshot create snap-bionic from repo openMHA-bionic-development"
                sh "aptly snapshot create snap-xenial from repo openMHA-xenial-development"
                sh "aptly snapshot create snap-trusty from repo openMHA-trusty-development"

                // Publish the snapshots to local directory
                sh "./aptly-publish-locally.sh"

                // Mirror local directory to hoertech server
                sh "./aptly-mirror-repository-to-server.sh"
            }
        }
    }
}
