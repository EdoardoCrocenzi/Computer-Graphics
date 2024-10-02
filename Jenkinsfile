pipeline {
  agent {
    docker {
      image 'docker-agent-alpine'
    }

  }
  stages {
    stage('Qualcosa') {
      steps {
        echo "Facendo Qualcosa"
        sh ''' echo "Sto facendo qualcosa" '''
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
