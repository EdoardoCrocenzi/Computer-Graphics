pipeline {
  agent {
    node {
      label 'docker-agent-alpine'
    }

  }
  stages {
    stage('Qualcosa') {
      steps {
        echo "Facendo Qualcosa"
        sh ''' echo "Sto facendo qualcosa" '''
      }
    }

  stages {
    stage('QUALYS_PROVA') {
      steps {
        qualysVulnerabilityAnalyzer apiServer: 'https://qualysapi.qg2.apps.qualys.eu', credsId: 'fb69c8f2-4083-4399-8626-04202421d02b', optionProfile: '', platform: 'EU_PLATFORM_2', pollingInterval: '2', scanName: '[job_name]_jenkins_build_[build_number]', scannerName: '', vulnsTimeout: '60*2'
      }
    }
    

    stage('Qualcos\'altro') {
      steps {
        echo "Facendo Qualcos\'altro"
        sh '''echo "Sto facendo qualcos\'altro"'''
      }
    }

  }
}
