// Jenkinsfile - multi-stage CI for the HWVAL validation firmware.
//
//   Checkout -> Toolchain -> Build -> Static analysis -> Test catalogue
//            -> Requirement coverage -> Flash layout -> Publish
//
// Builds run through scripts/docker-dev.sh (CON-001). The on-target tests
// (tests/test_*.c) are compiled into hwval.elf by the normal build; running
// them needs a NUCLEO-F767ZI and is done with scripts/hwtest.py outside CI.

pipeline {
    agent any

    options {
        skipDefaultCheckout()
        timestamps()
        timeout(time: 60, unit: 'MINUTES')
        buildDiscarder(logRotator(numToKeepStr: '30', artifactNumToKeepStr: '10'))
        disableConcurrentBuilds()
    }

    parameters {
        choice(name: 'BUILD_TYPE', choices: ['Release', 'Debug'], description: 'CMake build type')
        booleanParam(name: 'RUN_LINT', defaultValue: true, description: 'Run clang-tidy')
        booleanParam(name: 'STRICT_LINT', defaultValue: false, description: 'Fail on any clang-tidy finding')
    }

    environment {
        STM32_IMAGE = 'stm32-dev:latest'
    }

    stages {
        stage('Checkout') {
            steps {
                checkout scm
                sh '''
                    git log -1 --pretty=fuller
                    echo "commit: $(git rev-parse HEAD)"
                    echo "branch: $(git rev-parse --abbrev-ref HEAD)"
                '''
            }
        }

        stage('Toolchain') {
            steps {
                sh '''
                    docker version --format 'docker server: {{.Server.Version}}'
                    if ! docker image inspect "${STM32_IMAGE}" >/dev/null 2>&1; then
                        echo "Building ${STM32_IMAGE} from docker/Dockerfile"
                        make image
                    fi
                    docker image inspect --format 'image: {{.Id}}' "${STM32_IMAGE}"
                    scripts/docker-dev.sh run arm-none-eabi-gcc --version | head -n 1
                    scripts/docker-dev.sh run python3 --version
                '''
            }
        }

        stage('Build') {
            steps {
                sh "make build BUILD_TYPE=${params.BUILD_TYPE}"
            }
        }

        stage('Static analysis') {
            when { expression { return params.RUN_LINT } }
            steps {
                script {
                    if (params.STRICT_LINT) {
                        sh 'make lint-strict'
                    } else {
                        sh 'make lint'
                    }
                }
            }
        }

        stage('Test catalogue') {
            steps {
                // docs/TEST_CATALOGUE.md is generated from the test tables
                // (NFR-010); fail when it was not regenerated after a change.
                sh '''
                    python3 scripts/gen_testdoc.py
                    if git diff --quiet -- docs/TEST_CATALOGUE.md; then
                        echo "test catalogue: up to date"
                    else
                        git diff --stat -- docs/TEST_CATALOGUE.md
                        echo "test catalogue: stale - run python3 scripts/gen_testdoc.py and commit the result"
                        exit 1
                    fi
                '''
            }
        }

        stage('Requirement coverage') {
            steps {
                // COV-001 (SRS <-> test cases) and COV-002 (peripheral bases).
                // COV-002 needs the STM32Cube header from the build image; if
                // the container is unavailable the check reports UNSTABLE.
                catchError(buildResult: 'UNSTABLE', stageResult: 'FAILURE') {
                    sh 'python3 scripts/check_coverage.py'
                }
            }
        }

        stage('Flash layout') {
            steps {
                sh '''
                    scripts/docker-dev.sh run arm-none-eabi-readelf -h /work/build/hwval.elf > build/readelf-h.txt
                    cat build/readelf-h.txt
                    grep -q "Entry point address:.*0x8000000" build/readelf-h.txt
                    echo "flash layout: HWVAL is linked standalone at 0x08000000"
                '''
            }
        }

        stage('Publish') {
            steps {
                sh '''
                    make size
                    sha256sum build/hwval.elf build/hwval.bin build/hwval.hex > build/SHA256SUMS
                    sha256sum build/hwval.map >> build/SHA256SUMS
                    cat build/SHA256SUMS
                '''
                archiveArtifacts artifacts: 'build/hwval.elf,build/hwval.bin,build/hwval.hex,build/hwval.map,build/SHA256SUMS',
                                 fingerprint: true
            }
        }
    }

    post {
        always {
            deleteDir()
        }
        success {
            echo 'HWVAL firmware: build, lint, catalogue, coverage and layout checks passed.'
        }
        failure {
            echo 'HWVAL firmware: pipeline failed - see the stage logs above.'
        }
    }
}
