// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2018 HörTech gGmbH
//
// openMHA is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, version 3 of the License.
//
// openMHA is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License, version 3 for more details.
//
// You should have received a copy of the GNU Affero General Public License, 
// version 3 along with openMHA.  If not, see <http://www.gnu.org/licenses/>.

// Encapsulation of the build steps to perform when compiling openMHA
// @param stage_name the stage name is "system && arch" where system is bionic,
//                   xenial, trusty, windows, or mac, and arch is x86_64, i686,
//                   or armv7. Both are separated by an && operator and spaces.
//                   This string is also used as a valid label expression for
//                   jenkins. The appropriate nodes have the respective labels.
//                   We might need to extend this in future to include the
//                   "mhadev" label, to differentiate build environments
//                   for the same system and architecture but with different
//                   library / tool dependencies.
def openmha_build_steps(stage_name) {
  // Extract components from stage_name:
  def system, arch
  (system,arch) = stage_name.split(/ *&& */) // regexp for missing/extra spaces

  // Compilation on ARM is the slowest, assign 2 CPU cores to each ARM build job
  def cpus = (arch == "armv7") ? 2 : 1

  // checkout openMHA from version control system, the exact same revision that
  // triggered this job on each build slave
  checkout scm

  // Avoid that artifacts from previous builds influence this build
  sh "git reset --hard && git clean -ffdx"

  // Install pre-compiled external libraries
  copyArtifacts(projectName: "test_folder/artifact_producer/$BRANCH_NAME",
                selector: lastSuccessful())
  sh "cat artifact"

  // Autodetect libs/compiler
  sh "./configure"

  // On linux, we also create debian packages
  def linux = (system != "windows" && system != "mac")
  def debs = linux ? " deb" : ""
  sh ("make -j $cpus install unit-tests" + debs)

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
                stage(                         "bionic && x86_64") {
                    agent {label               "bionic && x86_64"}
                    steps {openmha_build_steps("bionic && x86_64")}
                }
                stage(                         "xenial && x86_64") {
                    agent {label               "xenial && x86_64"}
                    steps {openmha_build_steps("xenial && x86_64")}
                }
                stage(                         "trusty && x86_64") {
                    agent {label               "trusty && x86_64"}
                    steps {openmha_build_steps("trusty && x86_64")}
                }
                // We can also build for 32 bits. Deactivated to save
                // CPU cycles on Jenkins server.
                // stage(                         "bionic && i686") {
                //     agent {label               "bionic && i686"}
                //     steps {openmha_build_steps("bionic && i686")}
                // }
                // stage(                         "xenial && i686") {
                //     agent {label               "xenial && i686"}
                //     steps {openmha_build_steps("xenial && i686")}
                // }
                // stage(                         "trusty && i686") {
                //     agent {label               "trusty && i686"}
                //     steps {openmha_build_steps("trusty && i686")}
                // }
                stage(                         "bionic && armv7") {
                    agent {label               "bionic && armv7"}
                    steps {openmha_build_steps("bionic && armv7")}
                }
                stage(                         "xenial && armv7") {
                    agent {label               "xenial && armv7"}
                    steps {openmha_build_steps("xenial && armv7")}
                }
                stage(                         "windows && x86_64") {
                    agent {label               "windows && x86_64"}
                    steps {openmha_build_steps("windows && x86_64")}
                }
                stage(                         "mac && x86_64") {
                    agent {label               "mac && x86_64"}
                    steps {openmha_build_steps("mac && x86_64")}
                }
            }
        }
        stage("publish") {
            agent {label "aptly"}
            // do not publish packages for any branches except these
            when { anyOf { branch 'master'; branch 'development' } }
            steps {
                checkout([$class: 'GitSCM', branches: [[name: "$BRANCH_NAME"]], doGenerateSubmoduleConfigurations: false, extensions: [[$class: 'CleanCheckout']], submoduleCfg: [], userRemoteConfigs: [[url: "$GIT_URL-aptly"]]])

                // receive all deb packages from openmha build
                unstash "x86_64_bionic"
                unstash "x86_64_xenial"
                unstash "x86_64_trusty"
                unstash "armv7_bionic"
                unstash "armv7_xenial"
                // We can also build for 32 bits. Deactivated to save
                // CPU cycles on Jenkins server.
                // unstash "i686_bionic"
                // unstash "i686_xenial"
                // unstash "i686_trusty"

                // Copies the new debs to the stash of existing debs,
                // creates an apt repository, uploads.
                sh "make"
            }
        }
    }

    // Email notification on failed build taken from
    // https://jenkins.io/doc/pipeline/tour/post/
    // multiple recipients are comma-separated:
    // https://jenkins.io/doc/pipeline/steps/workflow-basic-steps/#-mail-%20mail
    post {
        failure {
//            mail to: 't.herzke@hoertech.de,p.maanen@hoertech.de,g.grimm@hoertech.de',
            mail to: 't.herzke@hoertech.de',
                 subject: "Failed Pipeline: ${currentBuild.fullDisplayName}",
                 body: "Something is wrong with ${env.BUILD_URL}"
        }
    }
}
