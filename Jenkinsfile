pipeline {
  agent {
    docker {
      image 'jenkins/agent:alpine-jdk11'
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