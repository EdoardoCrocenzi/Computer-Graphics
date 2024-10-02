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
        getImageVulnsFromQualys dockerUrl: '/var/run/docker.sock:/var/run/docker.sock:ro', imageIds: '2094c64747cc, eb03a60b3dba', useGlobalConfig: true
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
