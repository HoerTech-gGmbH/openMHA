// Encapsulation of the build steps to perform when compiling openMHA
def openmha_build_steps() {
  // The stage names we use for compiling openMHA have the structure
  // "system && arch", where system is bionic,
  // xenial, trusty, windows, or mac, and arch is x86_64, i686,
  // or armv7.  Both are separated by an && operator and spaces.
  // This string is also used as a valid label expression for
  // jenkins.  The appropriate nodes have the respective labels.
  // We might need to extend this in future to include the
  // "mhadev" label, to differentiate build environments
  // for the same system and architecture but with different
  // library / tool dependencies.

  // Extract components from stage name:
  def sys, arch
  (sys,arch) = env.STAGE_NAME.split(/ *&& */) // regexp for missing/extra spaces

  // checkout openMHA from version control system, the exact same revision that
  // triggered this job on each build slave
  checkout scm

  // Avoid that artifacts from previous builds influence this build
  sh "git reset --hard && git clean -ffdx"

  // Autodetect libs/compiler
  sh "./configure"

  // On linux, we also create debian packages
  def linux = (sys != "windows" && sys != "mac")
  def debs = linux ? " deb" : ""
  sh ("make install unit-tests" + debs)

  // The system tests perform timing measurements which may fail when
  // system load is high. Retry in that case, up to 2 times.
  retry(3){sh "make -C mha/mhatest"}

  if (linux) {
    // Store debian packets for later retrieval by the repository manager
    stash name: (arch+"_"+system), includes: 'mha/tools/packaging/deb/hoertech/'
  }
}

pipeline {
    agent {label "jenkinsmaster"}
    stages {
        stage("build") {
            parallel {
                stage(           "bionic && x86_64") {
                    agent {label "bionic && x86_64"}
                    steps {openmha_build_steps()}
                }
                stage(           "bionic && i686") {
                    agent {label "bionic && i686"}
                    steps {openmha_build_steps()}
                }
                stage(           "xenial && x86_64") {
                    agent {label "xenial && x86_64"}
                    steps {openmha_build_steps()}
                }
                stage(           "xenial && i686") {
                    agent {label "xenial && i686"}
                    steps {openmha_build_steps()}
                }
                stage(           "trusty && x86_64") {
                    agent {label "trusty && x86_64"}
                    steps {openmha_build_steps()}
                }
                stage(           "trusty && i686") {
                    agent {label "trusty && i686"}
                    steps {openmha_build_steps()}
                }
                stage(           "bionic && armv7") {
                    agent {label "bionic && armv7"}
                    steps {openmha_build_steps()}
                }
                stage(           "xenial && armv7") {
                    agent {label "xenial && armv7"}
                    steps {openmha_build_steps()}
                }
                stage(           "windows && x86_64") {
                    agent {label "windows && x86_64"}
                    steps {openmha_build_steps()}
                }
                stage(           "mac && x86_64") {
                    agent {label "mac && x86_64"}
                    steps {openmha_build_steps()}
                }
            }
        }
        stage("publish") {
            agent {label "aptly"}
            steps {
                checkout([$class: 'GitSCM', branches: [[name: "$BRANCH_NAME"]], doGenerateSubmoduleConfigurations: false, extensions: [[$class: 'CleanCheckout']], submoduleCfg: [], userRemoteConfigs: [[url: "$GIT_URL-aptly"]]])

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
