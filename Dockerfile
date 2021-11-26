# docker image build -t webserv:1 . && docker container run -it -p 8080:8080 -p 8081:8081 -p 9090:9090 -p 443:443 webserv:1
FROM ubuntu:latest

RUN apt-get -y update && apt-get install -y
RUN apt-get -y install clang
RUN apt-get -y install build-essential

# COPY ./ATwebserv /webserv

# WORKDIR /webserv

# CMD ["./compil.sh"]
# CMD ["bash"]