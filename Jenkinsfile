pipeline {
  agent {
    docker {
      image 'docker-agent-alpine'
    }

  }
  stages {
    stage('Qualcosa') {
      steps {
        sh 'echo "Sto facendo qualcosa"'
      }
    }

    stage('Qualcos\'altro') {
      steps {
        sh 'echo "Sto facendo qualcos\'altro"'
      }
    }

  }
}
