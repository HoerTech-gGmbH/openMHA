// keep track of platforms for which debian packages are built
debian_stashes = []
debian_systems = []

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
    def stash_name = arch + "_" + system
    stash name: stash_name, includes: 'mha/tools/packaging/deb/hoertech/'
    debian_stashes.add(stash_name)
    debian_systems.add(sys)
    debian_systems.unique()
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
		debian_stashes.each{stash -> unstash stash}

		sh "ls -lR"
		
                // copy fresh packages to our stash of packages 
                sh "cp -anv mha/tools/packaging/deb/hoertech/* /packages/"
                
                // prepare the repository databases
                sh("./aptly-initialize-these-databases.sh " + debian_systems.join(" "))
                
                // Delete old packages.
                // Not yet implemented.
                
		debian_systems.each { sys ->
                  // Fill aptly databases with packages
                  sh "aptly repo add openMHA-$sys-$BRANCH_NAME /packages/$sys/*"
                  // Create snapshots
              	  sh "aptly snapshot create snap-$sys from repo openMHA-$sys-$BRANCH_NAME"
	        }

                // Publish the snapshots to local directory
                sh "./aptly-publish-locally-these.sh " + debian_systems.join(" "))

                // Mirror local directory to hoertech server
                sh "./aptly-mirror-repository-to-server.sh"
            }
        }
    }
}
