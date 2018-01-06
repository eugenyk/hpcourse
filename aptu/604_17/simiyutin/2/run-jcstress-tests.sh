cd lock-free-list
mvn clean install
cd ..
cd lock-free-list-jcstress
mvn clean install
java -jar target/jcstress.jar
