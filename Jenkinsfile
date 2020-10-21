pipeline {
    agent any

    stages {
        stage('Checkout') {
            steps {
                checkout([
                    $class: 'GitSCM',
                    branches: [
                        [name: '*/master']
                    ],
                    browser: [
                        $class: 'GithubWeb',
                        repoUrl: 'https://github.com/PhischDotOrg/stm32f1-bluepill'
                    ],
                    doGenerateSubmoduleConfigurations: false,
                    extensions: [
                        [
                            $class: 'SubmoduleOption',
                            depth: 1,
                            disableSubmodules: false,
                            parentCredentials: true,
                            recursiveSubmodules: true,
                            reference: '',
                            shallow: true,
                            threads: 8,
                            trackingSubmodules: false
                        ]
                    ],
                    submoduleCfg: [
                    ],
                    userRemoteConfigs: [
                        [
                            credentialsId: 'a88f9971-dae1-4a4d-9a8f-c7af88cad71b',
                            url: 'git@github.com:PhischDotOrg/stm32f1-bluepill.git'
                        ]
                    ]
                ])                
            }
        }
        stage('Build') {
            steps {
                cmakeBuild buildDir: 'build',
                  buildType: "Debug",
                  cleanBuild: true,
                  cmakeArgs: "-DCMAKE_TOOLCHAIN_FILE=${WORKSPACE}/common/Generic_Cortex_M3.ctools",
                  installation: 'InSearchPath',
                  sourceDir: "${WORKSPACE}",
                  steps: [
                    [ args: 'all' ]
                  ]
            }
        }
    }
}
