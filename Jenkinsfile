// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2018 2019 2020 2021 HörTech gGmbH
// Copyright © 2021 2022 Hörzentrum Oldenburg gGmbH
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
// @param stage_name the stage name is "system && arch && devenv" where system
//                   is bionic, focal, windows, or mac, arch is x86_64, aarch64,
//                   or armv7, and devenv is either mhadev or mhadoc.
//                   All parts are separated by an && operator and spaces.
//                   This string is also used as a valid label expression for
//                   jenkins.  The appropriate nodes have the respective labels.
def openmha_build_steps(stage_name) {
  // Extract components from stage_name:
  def system, arch, devenv
  (system,arch,devenv) = stage_name.split(/ *&& */) // regexp for missing/extra spaces

  // platform booleans
  def linux = (system != "windows" && system != "mac")
  def windows = (system == "windows")
  def mac = (system == "mac")
  def docs = (devenv == "mhadoc") //Create PDFs only on 1 of the parallel builds

  // Compilation on ARM is the slowest, assign 5 CPU cores to each ARM build job
  def cpus = 2 // default on other systems is 2 cores
  if (arch == "armv7" || arch == "aarch64") {
    cpus = 5
  }
  if (system == "windows") {
    cpus = 5
  }

  def additional_cpus_for_docs = 7

  // checkout openMHA from version control system, the exact same revision that
  // triggered this job on each build slave
  checkout scm

  // Avoid that artifacts from previous builds influence this build
  sh "git reset --hard && git clean -ffdx"

  // Save time by using precompiled external libs if possible.
  // Install pre-compiled external libraries for the common branches
  copyArtifacts(projectName: "MHA/external_libs/external_libs_development",
                  selector:    lastSuccessful())
  sh "tar xvzf external_libs.tgz"

  // if we notice any differences between the sources of the precompiled
  // dependencies and the current sources, we cannot help but need to recompile
  sh "git diff --exit-code || (git reset --hard && git clean -ffdx)"

  // Autodetect libs/compiler
  sh "./configure"

  if (docs) {
    // Create the cross-platform artifacts (PDFs and debs).

    // All other platforms have to wait for the documents to be created and
    // published here, therefore we give this task additional cpus to create
    // the docs.
    sh ("make -j ${cpus + additional_cpus_for_docs} doc")

    // Store generated PDF documents as Jenkins artifacts
    stash name: "docs", includes: '*.pdf'
    sh ("echo stashed docs on $system $arch at \$(date -R)")
    archiveArtifacts 'pdf-*.zip'

    // The docker slave that builds the docs also builds the architecture
    // independent debian packages.  Again, all linux build jobs have to
    // wait for these debs created here, hence the additional cpus again.
    sh ("make -j ${cpus + additional_cpus_for_docs} deb")

    // make deb leaves copies of the platform-independent debs in base directory
    stash name: "debs", includes: '*.deb'
    sh ("echo stashed debs on $system $arch at \$(date -R)")
    archiveArtifacts '*.deb'

    // Doxygen generates html version of developer documentation.  Publish.
    archiveArtifacts 'mha/doc/mhadoc/html/**'
  }

  // Build executables, plugins, execute tests
  sh ("make -j $cpus test")

  // Retrieve the documents and matlab coder, wait if they are not ready yet
  def wait_time = 1 // short sleep time for first iteration
  def attempt = 0
  retry(45){
    sleep(wait_time)
    wait_time = 15 // longer sleep times for subsequent iterations
    attempt = attempt + 1
    sh ("echo unstash docs attempt $attempt on $system $arch at \$(date -R)")
    unstash "docs"
    unstash "MatlabCoderGeneration"
  }

  if (linux) {
    if (!docs) { // The docs builder already has the debs.  Built them above.
      // Retrieve the architecture-independent debs, wait until they are ready.
      wait_time = 1
      attempt = 0
      retry(45){
        sleep(wait_time)
        wait_time = 15
        attempt = attempt + 1
        sh ("echo unstash debs attempt $attempt on $system $arch at \$(date -R)")
        unstash "debs"
        sh ("touch *.deb")
      }

      sh ("make -j $cpus deb")
    }
    // Store debian packages
    stash name: (arch+"_"+system), includes: 'mha/tools/packaging/deb/hoertech/'
    archiveArtifacts 'mha/tools/packaging/deb/hoertech/*/*.deb'
  }

  if (windows) {
    sh ("make -j $cpus exe")
    // Store windows installer
    archiveArtifacts 'mha/tools/packaging/exe/*.exe'
  }

  if (mac) {
    sh ("make -j $cpus pkg")
    // Store mac installer
    archiveArtifacts 'mha/tools/packaging/pkg/*.pkg'
  }

  // Check reproducibility: No package should contain "modified" in its name
  sh ('if find mha/tools/packaging | grep modified;' +
        'then echo error: Some installation packages have \"modified\" as part'+
        '          of their file name, which means that some git-controlled' +
        '          files contained modifications when these installation' +
        '          packages were created.  This should not happen because the' +
        '          resulting installer packages are not reproducible.  Find' +
        '          the cause and fix the error.;' +
        '     exit 1;' +
        'fi')
}

pipeline {
    agent {label "pipeline"}
    options {
        buildDiscarder(logRotator(daysToKeepStr: '7', artifactDaysToKeepStr: '7'))
    }
    stages {
        stage("build") {
            parallel {
                stage("Matlab Coder Generation") {
                    agent {label "matlabcoder"}
                    steps {
                        dir('mha/mhatest') {
                            sh "matlab -nodisplay -r 'generate_matlab_coder_native;quit'"
                        }
                        stash name: "MatlabCoderGeneration",
                              includes: 'mha/plugins/matlabcoder_skeleton/'
                        archiveArtifacts 'mha/plugins/matlabcoder_skeleton/**'
                    }
                }
                stage(                         "jammy && x86_64 && mhadoc") {
                    agent {label               "jammy && x86_64 && mhadoc"}
                    steps {openmha_build_steps("jammy && x86_64 && mhadoc")}
                }
                stage(                         "focal && x86_64 && mhadev") {
                    agent {label               "focal && x86_64 && mhadev"}
                    steps {openmha_build_steps("focal && x86_64 && mhadev")}
                }
                stage(                         "bionic && x86_64 && mhadev") {
                    agent {label               "bionic && x86_64 && mhadev"}
                    steps {openmha_build_steps("bionic && x86_64 && mhadev")}
                }
                stage(                         "bullseye && armv7 && mhadev") {
                    agent {label               "bullseye && armv7 && mhadev"}
                    steps {openmha_build_steps("bullseye && armv7 && mhadev")}
                }
                stage(                         "bullseye && aarch64 && mhadev") {
                    agent {label               "bullseye && aarch64 && mhadev"}
                    steps {openmha_build_steps("bullseye && aarch64 && mhadev")}
                }
                stage(                         "bionic && armv7 && mhadev") {
                    agent {label               "bionic && armv7 && mhadev"}
                    steps {openmha_build_steps("bionic && armv7 && mhadev")}
                }
                stage(                         "windows && x86_64 && mhadev") {
                    agent {label               "windows && x86_64 && mhadev"}
                    steps {openmha_build_steps("windows && x86_64 && mhadev")}
                }
                stage(                         "mac && x86_64 && mhadev") {
                    agent {label               "mac && x86_64 && mhadev"}
                    steps {openmha_build_steps("mac && x86_64 && mhadev")}
                }
            }
        }
        stage("debian packages for apt") {
            agent {label "aptly"}
            // publish packages for branch development automatically
            when { branch 'development' }

            // Releases are not automatically uploaded to apt because
            // we still need to check that all installers function
            // after they have been created. When this verification has
            // been done, then start a Replay build of master on Jenkins,
            // and change this Jenkinsfile as follows: comment-out the "when"
            // line above, remove the comment before the following "when"
            // line, and then Run the replay build.
            // when { anyOf { branch 'master'; branch 'development' } }
            steps {
                // receive all deb packages from openmha build
                unstash "x86_64_jammy"
                unstash "x86_64_focal"
                unstash "x86_64_bionic"
                unstash "armv7_bullseye"
                unstash "aarch64_bullseye"
                unstash "armv7_bionic"

                // Copies the new debs to the stash of existing debs,
                sh "./configure || true; make storage"
                build job:         "/Packaging/hoertech-aptly/$BRANCH_NAME",
                      quietPeriod: 300,
                      wait:        false
            }
        }
        stage("push updates in development branch to github when build successful") {
            // This stage needs an agent that has the git-switch command.
            agent {label "focal && mhadev"}

            // This stage is only executed if all prevous stages were successful
            when { branch 'development' } // and only for branch development!

            steps {
                // Make sure we have a git checkout
                checkout scm

                // Make sure branch development is not shallow in this clone
                sh "git fetch --unshallow || true"
                
                // Jenkins builds work in detached head mode, because when you
                // have git commits to the same branch in quick succession,
                // this Jenkins build may not build the latest state of branch
                // development, but an earlier state.  Furthermore, there can
                // be situations where this build of the earlier state is
                // successful, but the build on the latest state of branch
                // development will later fail.  Because of this possibility,
                // we may only push the current git commit to github and not
                // the tip of branch develpment.  We must not checkout branch
                // development here, but create a new branch here at the
                // detached head and push the new branch to github as branch
                // development there.
                sh "git switch --force-create temporary-branch-name-for-jenkins"

                // Push this state here to branch development on github. For
                // details about the ssh key, see https://dev.openmha.org/K36
                withCredentials(bindings: [sshUserPrivateKey(                  \
                                             credentialsId: 'K36',             \
                                             keyFileVariable: 'GIT_SSH_KEY')]) {
                    sh '''
                    GIT_SSH_COMMAND="ssh -i $GIT_SSH_KEY -o IdentitiesOnly=yes -o StrictHostKeyChecking=no" \
                    git push git@github.com:HoerTech-gGmbH/openMHA.git          \
                        temporary-branch-name-for-jenkins:development
                    '''
                }
            }
        }
    }

    // Email notification on failed build taken from
    // https://jenkins.io/doc/pipeline/tour/post/
    // multiple recipients are comma-separated:
    // https://jenkins.io/doc/pipeline/steps/workflow-basic-steps/#-mail-%20mail
    post {
        failure {
            mail to: 'tobiasherzke@openmha.com,paulmaanen@openmha.com',
                 subject: "Failed Pipeline: ${currentBuild.fullDisplayName}",
                 body: "Something is wrong with ${env.BUILD_URL}"
        }
    }
}
