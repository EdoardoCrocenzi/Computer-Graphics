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

    stage('QUALYS_PROVA') {
      steps {
        getImageVulnsFromQualys apiServer: 'https://qualysapi.qg2.apps.qualys.eu', credentialsId: 'fb69c8f2-4083-4399-8626-04202421d02b', dockerUrl: '/var/run/docker.sock:/var/run/docker.sock:ro', imageIds: '2094c64747cc, eb03a60b3dba', pollingInterval: '30', useLocalConfig: true, vulnsTimeout: '600'
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
