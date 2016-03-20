FROM ruby:2.2.4
ENV LC_ALL C
ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update -qq && apt-get install -y build-essential libsasl2-dev emacs

ENV APP_HOME /usr/src

WORKDIR $APP_HOME
ADD . $APP_HOME
RUN bundle install --without benchmark
RUN bundle exec ruby -Ilib:test test/setup.rb

CMD bundle exec rake
